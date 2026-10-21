// gate_nx — the gate, with libnx (see gate_nx.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate_nx.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <switch.h>

#include "gate.h"
#include "gate_screen.h"
#include "gate_text.h"
#include "nx_types.h"
#include "pctl_ops_internal.h"

#define GATE_DIR "sdmc:/config/playguard/"

// libnx's own exit: 1 ends the application (back to the HOME menu) instead
// of going back to the forwarder's loader.
extern u32 __nx_applet_exit_mode;

// The console's local time: its day of the week (0 Sunday), its minute of
// the day, and "YYYY-MM-DD HH:MM" for the log. False when it cannot be read.
static bool local_time(int *wday, int *minute, char *when, size_t size)
{
    u64 t = 0;
    TimeCalendarTime cal;
    TimeCalendarAdditionalInfo info;
    if (R_FAILED(timeGetCurrentTime(TimeType_LocalSystemClock, &t)) ||
        R_FAILED(timeToCalendarTimeWithMyRule(t, &cal, &info)))
        return false;
    *wday = (int)info.wday;
    *minute = cal.hour * 60 + cal.minute;
    snprintf(when, size, "%04u-%02u-%02u %02u:%02u", cal.year, cal.month, cal.day, cal.hour, cal.minute);
    return true;
}

// `nro`'s free hours (GATE_RULES_NAME); false when it has none.
static bool hours_of(const char *nro, GateHours *h)
{
    static char text[4096];
    FILE *f = fopen(GATE_DIR GATE_RULES_NAME, "rb");
    if (!f) return false;
    const size_t n = fread(text, 1, sizeof(text) - 1, f);
    fclose(f);
    text[n] = '\0';
    return gate_rule_find(text, n, nro, h);
}

// The tries `nro` was refused on `day` ("2026-10-20"), as the log has them
// since PlayGuard last read it.
static int refused_on(const char *nro, const char *day)
{
    static char text[GATE_LOG_MAX];
    FILE *f = fopen(GATE_DIR GATE_LOG_NAME, "rb");
    if (!f) return 0;
    const size_t n = fread(text, 1, sizeof(text), f);
    fclose(f);
    return gate_log_count(text, n, nro, day, GateResult_Refused);
}

// The system's PIN screen, unless the console has no PIN.
static GateResult ask_pin(void)
{
    if (R_FAILED(pctlInitialize())) return GateResult_Error;
    u32 len = 0;
    const Result rc = rd_u32(pctlGetServiceSession_Service(), 1206, &len);
    // The PIN screen opens its own privileged session: ours must be closed.
    pctlExit();
    if (R_FAILED(rc)) return GateResult_Error;
    if (len == 0) return GateResult_Free;
    const Result shown = pctlauthShowForConfiguration();
    if (R_SUCCEEDED(shown)) return GateResult_Pin;
    return shown == PCTL_RC_AUTH_CANCELLED ? GateResult_Refused : GateResult_Error;
}

static void log_try(const char *when, GateResult r, const char *nro)
{
    char line[FS_MAX_PATH + 64];
    const size_t n = gate_log_format(when, r, nro, line, sizeof(line));
    if (n == 0) return;
    // Owner-writable only (fopen would ask for 0666), as the rescue module's files.
    const int fd = open(GATE_DIR GATE_LOG_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    FILE *f = fd >= 0 ? fdopen(fd, "ab") : NULL;
    if (!f) {
        if (fd >= 0) close(fd);
        return;
    }
    // Not past its size: PlayGuard empties it at its next start.
    if (ftell(f) < GATE_LOG_MAX) fwrite(line, 1, n, f);
    fclose(f);
}

// ---------------------------------------------------------------- the screen

// What the gate shows around the system's PIN screen (core/gate_screen.h).
// Nothing when the framebuffer cannot be had: the PIN screen still opens.
typedef struct {
    bool           on;
    Framebuffer    fb;
    unsigned char *brand;    // romfs:/font/sora_bold_latin.ttf (the gate's romfs, or PlayGuard's)
    const unsigned char *text;   // the console's shared font for its language
    unsigned char *icon;     // the homebrew's, from its .nro
    size_t         icon_size;
    char           name[0x201];
    char           clock[8];
    int            language;
} Screen;

static unsigned char *read_all(const char *path, size_t max, size_t *n)
{
    *n = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = size > 0 && (size_t)size <= max ? (unsigned char *)malloc((size_t)size) : NULL;
    if (data && fread(data, 1, (size_t)size, f) == (size_t)size) *n = (size_t)size;
    else {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

static uint32_t u32_at(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint64_t u64_at(const unsigned char *p)
{
    return (uint64_t)u32_at(p) | (uint64_t)u32_at(p + 4) << 32;
}

// The .nro's icon and the name its NACP gives (its first one); its file
// name else.
static void read_nro(const char *nro, Screen *sc)
{
    const char *base = strrchr(nro, '/');
    snprintf(sc->name, sizeof(sc->name), "%.*s", (int)sizeof(sc->name) - 1, base ? base + 1 : nro);
    char *dot = strrchr(sc->name, '.');
    if (dot && strcasecmp(dot, ".nro") == 0) *dot = '\0';
    FILE *f = fopen(nro, "rb");
    if (!f) return;
    unsigned char head[0x80], aset[0x38];
    if (fread(head, 1, sizeof(head), f) == sizeof(head) && memcmp(head + 0x10, "NRO0", 4) == 0 &&
        fseek(f, (long)u32_at(head + 0x18), SEEK_SET) == 0 && fread(aset, 1, sizeof(aset), f) == sizeof(aset) &&
        memcmp(aset, "ASET", 4) == 0) {
        const long base_at = (long)u32_at(head + 0x18);
        const uint64_t icon_at = u64_at(aset + 8), icon_n = u64_at(aset + 0x10);
        const uint64_t nacp_at = u64_at(aset + 0x18), nacp_n = u64_at(aset + 0x20);
        if (icon_n > 0 && icon_n <= 0x40000 && fseek(f, base_at + (long)icon_at, SEEK_SET) == 0) {
            sc->icon = (unsigned char *)malloc((size_t)icon_n);
            if (sc->icon && fread(sc->icon, 1, (size_t)icon_n, f) == icon_n) sc->icon_size = (size_t)icon_n;
            else {
                free(sc->icon);
                sc->icon = NULL;
            }
        }
        // The NACP's 16 names, 0x300 bytes apart: the first one written.
        static unsigned char nacp[0x3000];
        if (nacp_n >= sizeof(nacp) && fseek(f, base_at + (long)nacp_at, SEEK_SET) == 0 &&
            fread(nacp, 1, sizeof(nacp), f) == sizeof(nacp))
            for (int i = 0; i < 16; i++) {
                const char *n = (const char *)nacp + i * 0x300;
                if (!n[0]) continue;
                snprintf(sc->name, sizeof(sc->name), "%.*s", 0x1FF, n);
                break;
            }
    }
    fclose(f);
}

static void screen_open(Screen *sc, const char *nro, const char *when)
{
    memset(sc, 0, sizeof(*sc));
    sc->language = 1;   // American English
    u64 code = 0;
    SetLanguage lang;
    if (R_SUCCEEDED(setInitialize())) {
        if (R_SUCCEEDED(setGetSystemLanguage(&code)) && R_SUCCEEDED(setMakeLanguage(code, &lang))) sc->language = (int)lang;
        setExit();
    }
    static const PlSharedFontType TYPE[] = { PlSharedFontType_Standard, PlSharedFontType_ChineseSimplified,
                                             PlSharedFontType_ChineseTraditional, PlSharedFontType_KO };
    PlFontData font;
    if (R_SUCCEEDED(plInitialize(PlServiceType_User)) &&
        R_SUCCEEDED(plGetSharedFontByType(&font, TYPE[gate_text_font(sc->language)])))
        sc->text = (const unsigned char *)font.address;
    size_t n = 0;
    sc->brand = read_all("romfs:/font/sora_bold_latin.ttf", 0x100000, &n);
    if (!sc->text) sc->text = sc->brand;
    read_nro(nro, sc);
    // "2026-10-20 21:03": its time.
    const char *space = strchr(when, ' ');
    if (space) snprintf(sc->clock, sizeof(sc->clock), "%.5s", space + 1);
    if (R_FAILED(framebufferCreate(&sc->fb, nwindowGetDefault(), GATE_SCREEN_W, GATE_SCREEN_H, PIXEL_FORMAT_RGBA_8888, 1)))
        return;
    if (R_FAILED(framebufferMakeLinear(&sc->fb))) {
        framebufferClose(&sc->fb);
        return;
    }
    sc->on = true;
}

static void screen_show(Screen *sc, GateScreenState state, const char *note, int note_warn)
{
    if (!sc->on) return;
    static const GateText STATUS[] = { GateText_Asking, GateText_Opening, GateText_Free, GateText_Refused, GateText_Error };
    const GateText hint = state == GateScreen_Asking ? GateText_AskingHint
                          : state == GateScreen_Refused ? GateText_RefusedHint
                                                        : GateText_Count;
    char footer[160];
    snprintf(footer, sizeof(footer), "PlayGuard \xC2\xB7 %s", gate_text(sc->language, GateText_Tagline));
    const GateScreen s = {
        sc->brand, sc->text, sc->icon, sc->icon_size, sc->name, gate_text(sc->language, STATUS[state]),
        hint == GateText_Count ? NULL : gate_text(sc->language, hint), footer, sc->clock, state, note, note_warn,
    };
    u32 stride = 0;
    uint32_t *px = (uint32_t *)framebufferBegin(&sc->fb, &stride);
    if (!px) return;
    gate_screen_draw(px, GATE_SCREEN_W, GATE_SCREEN_H, (int)(stride / 4), &s);
    framebufferEnd(&sc->fb);
}

static void screen_close(Screen *sc)
{
    if (sc->on) framebufferClose(&sc->fb);
    sc->on = false;
    free(sc->brand);
    free(sc->icon);
    plExit();
}

bool gate_nx_run(void)
{
    if (!envHasArgv()) return false;
    char nro[FS_MAX_PATH];
    const char *argv = NULL;
    if (!gate_argv_parse((const char *)envGetArgv(), nro, sizeof(nro), &argv)) return false;

    int wday = -1, minute = -1;
    char when[32] = "?";
    const bool timed = local_time(&wday, &minute, when, sizeof(when));
    Screen sc;
    screen_open(&sc, nro, when);
    GateHours hours;
    const bool has_hours = hours_of(nro, &hours);
    // The day's refusals so far ("2026-10-20 21:03": its day), and when it
    // starts without the PIN: said while it asks.
    char day[11] = "";
    if (timed) snprintf(day, sizeof(day), "%.10s", when);
    const int refused = timed ? refused_on(nro, day) : 0;
    char note[256] = "", span[200];
    if (refused > 0) snprintf(note, sizeof(note), gate_text(sc.language, GateText_Tried), refused);
    else if (has_hours && gate_hours_text(&hours, gate_text(sc.language, GateText_Days), span, sizeof(span)))
        snprintf(note, sizeof(note), gate_text(sc.language, GateText_FreeHours), span);
    GateResult r;
    if (timed && has_hours && gate_hours_free(&hours, wday, minute)) {
        r = GateResult_Free;
    } else {
        screen_show(&sc, GateScreen_Asking, note, refused > 0);
        r = ask_pin();
    }
    log_try(when, r, nro);
    // How it went, a moment: longer when it does not start. Refused, the
    // day's count with this one.
    if (r == GateResult_Refused && timed) snprintf(note, sizeof(note), gate_text(sc.language, GateText_Tried), refused + 1);
    static const GateScreenState SHOWN[] = { GateScreen_Opening, GateScreen_Free, GateScreen_Refused, GateScreen_Error };
    screen_show(&sc, SHOWN[r], r == GateResult_Refused ? note : NULL, 1);
    if (sc.on) svcSleepThread(r == GateResult_Pin || r == GateResult_Free ? 700000000ULL : 1800000000ULL);
    screen_close(&sc);
    if ((r == GateResult_Pin || r == GateResult_Free) && envHasNextLoad() && R_SUCCEEDED(envSetNextLoad(nro, argv)))
        return true;
    __nx_applet_exit_mode = 1;
    return true;
}
