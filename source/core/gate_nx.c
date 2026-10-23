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

// The network clock (System Settings cannot set it): its POSIX time, its
// day of the week and its minute of the day. False when it cannot be read.
static bool network_time(u64 *t, int *wday, int *minute)
{
    TimeCalendarTime cal;
    TimeCalendarAdditionalInfo info;
    if (R_FAILED(timeGetCurrentTime(TimeType_NetworkSystemClock, t)) ||
        R_FAILED(timeToCalendarTimeWithMyRule(*t, &cal, &info)))
        return false;
    *wday = (int)info.wday;
    *minute = cal.hour * 60 + cal.minute;
    return true;
}

// The steady clock (core/gate.h, GateSteady): false when it cannot be read.
static bool steady_time(GateSteady *out)
{
    TimeSteadyClockTimePoint tp;
    if (R_FAILED(timeGetStandardSteadyClockTimePoint(&tp)) || tp.time_point < 0) return false;
    memcpy(out->source, tp.source_id.uuid, sizeof(out->source));
    out->seconds = (uint64_t)tp.time_point;
    return true;
}

// `nro`'s hours in `file` (GATE_RULES_NAME: free, GATE_CLOSED_NAME: none);
// false when it has none.
static bool hours_of(const char *file, const char *nro, GateHours *h)
{
    static char text[4096];
    FILE *f = fopen(file, "rb");
    if (!f) return false;
    const size_t n = fread(text, 1, sizeof(text) - 1, f);
    fclose(f);
    text[n] = '\0';
    return gate_rule_find(text, n, nro, h);
}

// The pause file (GATE_PAUSE_NAME), its length; 0 when there is none.
static char pause_text[GATE_PAUSE_MAX];
static size_t pause_read(void)
{
    FILE *f = fopen(GATE_DIR GATE_PAUSE_NAME, "rb");
    if (!f) return 0;
    const size_t n = fread(pause_text, 1, sizeof(pause_text) - 1, f);
    fclose(f);
    pause_text[n] = '\0';
    return n;
}

// The pause file after this try (owner-writable only, as the log), by
// `rule` (the forwarder's), else the file's; a pause that starts timed by
// the steady clock too (`steady`, NULL when it was not read).
static void pause_write(size_t len, const char *nro, GateResult r, uint64_t now, const GatePauseRule *rule,
                        const GateSteady *steady)
{
    static char next[GATE_PAUSE_MAX];
    const size_t n = gate_pause_update_steady(pause_text, len, nro, r, now, rule, steady, next, sizeof(next));
    if (n == 0 && len == 0) return;
    const int fd = open(GATE_DIR GATE_PAUSE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE *f = fd >= 0 ? fdopen(fd, "wb") : NULL;
    if (!f) {
        if (fd >= 0) close(fd);
        return;
    }
    fwrite(next, 1, n, f);
    fclose(f);
}

// "21:30", the local time of `posix`.
static void clock_of(uint64_t posix, char *out, size_t size)
{
    TimeCalendarTime cal;
    TimeCalendarAdditionalInfo info;
    if (R_SUCCEEDED(timeToCalendarTimeWithMyRule(posix, &cal, &info))) snprintf(out, size, "%02u:%02u", cal.hour, cal.minute);
    else snprintf(out, size, "?");
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

// The system's PIN screen, unless the console has no PIN. `*code`: why it
// did not open (GateResult_Error), for its error code on the screen.
static GateResult ask_pin(Result *code)
{
    *code = 0;
    Result rc = pctlInitialize();
    if (R_FAILED(rc)) {
        *code = rc;
        return GateResult_Error;
    }
    u32 len = 0;
    rc = rd_u32(pctlGetServiceSession_Service(), 1206, &len);
    // The PIN screen opens its own privileged session: ours must be closed.
    pctlExit();
    if (R_FAILED(rc)) {
        *code = rc;
        return GateResult_Error;
    }
    if (len == 0) return GateResult_Free;
    const Result shown = pctlauthShowForConfiguration();
    if (R_SUCCEEDED(shown)) return GateResult_Pin;
    if (shown == PCTL_RC_AUTH_CANCELLED) return GateResult_Refused;
    *code = shown;
    return GateResult_Error;
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
    unsigned       rules;    // GATE_RULE_*: its icons by the badge
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
    // Two buffers: with one, the compositor keeps it while it is shown, and
    // the next frame (how it went, after the PIN) waited for it for ever.
    if (R_FAILED(framebufferCreate(&sc->fb, nwindowGetDefault(), GATE_SCREEN_W, GATE_SCREEN_H, PIXEL_FORMAT_RGBA_8888, 2)))
        return;
    if (R_FAILED(framebufferMakeLinear(&sc->fb))) {
        framebufferClose(&sc->fb);
        return;
    }
    sc->on = true;
}

// `buttons`: Ⓐ to try again, Ⓑ for the HOME menu (once refused).
static void screen_show(Screen *sc, GateScreenState state, const char *note, int note_warn, bool buttons)
{
    if (!sc->on) return;
    static const GateText STATUS[] = { GateText_Asking, GateText_Opening, GateText_Free,  GateText_Refused,
                                       GateText_Error,  GateText_Paused,  GateText_Closed, GateText_Limit };
    const GateText hint = state == GateScreen_Asking    ? GateText_AskingHint
                          : state == GateScreen_Refused ? GateText_RefusedHint
                          : state == GateScreen_Paused  ? GateText_PausedHint
                          : state == GateScreen_Closed  ? GateText_ClosedHint
                          : state == GateScreen_Limit   ? GateText_LimitHint
                                                        : GateText_Count;
    char footer[160];
    snprintf(footer, sizeof(footer), "PlayGuard \xC2\xB7 %s", gate_text(sc->language, GateText_Tagline));
    const GateScreen s = {
        sc->brand, sc->text, sc->icon, sc->icon_size, sc->name, gate_text(sc->language, STATUS[state]),
        hint == GateText_Count ? NULL : gate_text(sc->language, hint), footer, sc->clock, state, note, note_warn,
        sc->rules, gate_text(sc->language, GateText_Badge),
        buttons ? gate_text(sc->language, GateText_RetryA) : NULL, buttons ? gate_text(sc->language, GateText_RetryB) : NULL,
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

// The console's play timer: whether it is on, today's time left
// (nanoseconds, below 0 once past) and whether it restricts play now.
// False when it cannot be read.
static bool play_timer(bool *on, s64 *left, bool *restricted)
{
    *on = false;
    *left = 1;
    *restricted = false;
    if (R_FAILED(pctlInitialize())) return false;
    Service *srv = pctlGetServiceSession_Service();
    u64 remaining = 1;   // a TimeSpan in nanoseconds, signed
    const bool read = R_SUCCEEDED(rd_bool(srv, 1453, on)) && R_SUCCEEDED(rd_u64(srv, 1454, &remaining));
    if (read && *on) rd_bool(srv, 1455, restricted);
    pctlExit();
    *left = (s64)remaining;
    return read;
}

// The play timer on, and today's time used up (or the console restricting
// play now). False when it cannot be read: the PIN then.
static bool play_time_used_up(void)
{
    bool on, restricted;
    s64 left;
    return play_timer(&on, &left, &restricted) && on && (left <= 0 || restricted);
}

// "Play time left today: 45 min", or "" when the timer is off or past.
static void time_left_note(int language, char *out, size_t size)
{
    out[0] = '\0';
    bool on, restricted;
    s64 left;
    if (!play_timer(&on, &left, &restricted) || !on || restricted || left <= 0) return;
    char d[64];
    const uint64_t minutes = ((uint64_t)left + 59999999999ULL) / 60000000000ULL;
    if (gate_minutes_text(minutes, gate_text(language, GateText_Minutes), gate_text(language, GateText_HoursMinutes), d,
                          sizeof(d)))
        snprintf(out, size, gate_text(language, GateText_TimeLeft), d);
}

// "It asks again at 21:18 (in 12 min).": when the pause ends, from `now`.
static void paused_note(int language, uint64_t until, uint64_t now, char *out, size_t size)
{
    char at[16], in[64], both[96];
    clock_of(until, at, sizeof(at));
    snprintf(in, sizeof(in), gate_text(language, GateText_InMinutes), (int)((until - now + 59) / 60));
    snprintf(both, sizeof(both), "%s (%s)", at, in);
    snprintf(out, size, gate_text(language, GateText_PausedUntil), both);
}

// ---------------------------------------------------------------- the watchdog

// Should the gate stop on its way to the homebrew (a screen that never
// comes back, as the single buffer did), the console would stay on it with
// no way out but HOME: past WATCHDOG_NS without news, the gate ends,
// back to the HOME menu. Not while the system's PIN screen is open.
#define WATCHDOG_NS 10000000000ULL
static UEvent g_watch_done;
static volatile u64 g_watch_deadline;   // system ticks; 0: not watching

static void watch_kick(void)
{
    g_watch_deadline = armGetSystemTick() + armNsToTicks(WATCHDOG_NS);
}

static void watch_pause(void)
{
    g_watch_deadline = 0;
}

static void watchdog(void *arg)
{
    (void)arg;
    while (R_FAILED(waitSingle(waiterForUEvent(&g_watch_done), 500000000ULL))) {
        const u64 deadline = g_watch_deadline;
        if (deadline && armGetSystemTick() > deadline) svcExitProcess();
    }
}

// ---------------------------------------------------------------- once refused

// How long the refused screen waits for Ⓐ or Ⓑ before the HOME menu.
#define RETRY_WAIT_NS 15000000000ULL

// Ⓐ: try again (true); Ⓑ, +, or nothing for a while: the HOME menu.
static bool wants_retry(void)
{
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    padUpdate(&pad);   // what is held from the PIN screen is not a press
    const u64 start = armGetSystemTick();
    while (appletMainLoop()) {
        watch_kick();
        padUpdate(&pad);
        const u64 down = padGetButtonsDown(&pad);
        if (down & HidNpadButton_A) return true;
        if (down & (HidNpadButton_B | HidNpadButton_Plus)) return false;
        if (armTicksToNs(armGetSystemTick() - start) > RETRY_WAIT_NS) return false;
        svcSleepThread(16000000ULL);
    }
    return false;
}

bool gate_nx_run(void)
{
    if (!envHasArgv()) return false;
    char nro[FS_MAX_PATH];
    const char *argv = NULL;
    GateRules rules;
    if (!gate_argv_rules((const char *)envGetArgv(), nro, sizeof(nro), &argv, &rules)) return false;

    int wday = -1, minute = -1;
    char when[32] = "?";
    bool timed = local_time(&wday, &minute, when, sizeof(when));
    Screen sc;
    screen_open(&sc, nro, when);
    ueventCreate(&g_watch_done, false);
    Thread watch;
    const bool watching = R_SUCCEEDED(threadCreate(&watch, watchdog, NULL, NULL, 0x2000, 0x2C, -2)) &&
                          R_SUCCEEDED(threadStart(&watch));
    // The rules: those the forwarder carries (its RomFS, on the console's
    // storage). One made before carries none: no free hours then (the SD
    // card's file is not trusted), its closed hours and its pause from the
    // files, until PlayGuard makes it again.
    GateHours free_spans[GATE_SPANS], closed_spans[GATE_SPANS];
    unsigned free_n = 0, closed_n = 0;
    GatePauseRule carried_pause = { 0, 0 };
    const GatePauseRule *pause_rule = NULL;
    if (rules.carried) {
        memcpy(free_spans, rules.free, sizeof(free_spans));
        memcpy(closed_spans, rules.closed, sizeof(closed_spans));
        free_n = rules.free_n;
        closed_n = rules.closed_n;
        if (rules.has_pause) {
            carried_pause.tries = rules.pause_tries;
            carried_pause.minutes = rules.pause_minutes;
            pause_rule = &carried_pause;
        }
    } else if (hours_of(GATE_DIR GATE_CLOSED_NAME, nro, &closed_spans[0])) {
        closed_n = 1;
    }
    sc.rules = (free_n ? GATE_RULE_FREE : 0u) | (closed_n ? GATE_RULE_CLOSED : 0u) | (rules.limit ? GATE_RULE_LIMIT : 0u);
    const char *days = gate_text(sc.language, GateText_Days);
    GateResult r = GateResult_Error;
    char note[256] = "", span[256];
    bool shown = false;   // how it went already on the screen (refused, Ⓐ not pressed)
    // Each try; once refused, Ⓐ tries again (until a pause starts).
    for (bool first = true;; first = false) {
        if (!first) timed = local_time(&wday, &minute, when, sizeof(when));
        u64 now = 0, net = 0;
        const bool clocked = R_SUCCEEDED(timeGetCurrentTime(TimeType_LocalSystemClock, &now));
        // The console clock moved away from the network clock (System
        // Settings sets the one, not the other): each rule at its strictest.
        // No free hours, closed by either clock, and the pause timed by the
        // network clock, so setting the time forward does not end it.
        // Without the network clock, nothing vouches for the console clock:
        // no free hours either, its closed hours by it, and the pause by the
        // steady clock too, which no setting moves.
        int net_wday = -1, net_minute = -1;
        const bool netted = network_time(&net, &net_wday, &net_minute);
        const bool moved = clocked && gate_clocks_differ(now, netted, net);
        if (moved) now = net;
        GateSteady steady = { { 0 }, 0 }, steady_until = { { 0 }, 0 };
        const bool steadied = steady_time(&steady);
        const size_t pause_len = pause_read();
        uint64_t until = 0, left = 0;
        gate_pause_find(pause_text, pause_len, nro, NULL, &until);
        const bool steady_found = gate_pause_steady_find(pause_text, pause_len, nro, &steady_until);
        // The day's refusals so far ("2026-10-20 21:03": its day), else when
        // it starts without the PIN, else the day's play time left: said
        // while it asks.
        char day[11] = "";
        if (timed) snprintf(day, sizeof(day), "%.10s", when);
        const int refused = timed ? refused_on(nro, day) : 0;
        note[0] = '\0';
        if (refused > 0) snprintf(note, sizeof(note), gate_text(sc.language, GateText_Tried), refused);
        else if (free_n && gate_spans_text(free_spans, free_n, days, span, sizeof(span)))
            snprintf(note, sizeof(note), gate_text(sc.language, GateText_FreeHours), span);
        else if (rules.limit)
            time_left_note(sc.language, note, sizeof(note));
        Result code = 0;
        // Its hours without any access first (the PIN changes nothing then),
        // then the day's time used up, then a pause after refusals, then its
        // free hours; else the PIN.
        if ((timed && gate_spans_hold(closed_spans, closed_n, wday, minute)) ||
            (moved && gate_spans_hold(closed_spans, closed_n, net_wday, net_minute))) {
            r = GateResult_Closed;
            note[0] = '\0';
            if (gate_spans_text(closed_spans, closed_n, days, span, sizeof(span)))
                snprintf(note, sizeof(note), gate_text(sc.language, GateText_ClosedHours), span);
        } else if (rules.limit && play_time_used_up()) {
            r = GateResult_Limit;
        } else if (clocked && gate_pause_holds(until, now, steady_found ? &steady_until : NULL,
                                               steadied ? &steady : NULL, &left)) {
            r = GateResult_Paused;
            paused_note(sc.language, now + left, now, note, sizeof(note));
        } else if (timed && gate_free_trusted(netted, moved) && gate_spans_hold(free_spans, free_n, wday, minute)) {
            r = GateResult_Free;
        } else {
            // Its free hours would hold by one clock: why the PIN all the same.
            if (moved && ((timed && gate_spans_hold(free_spans, free_n, wday, minute)) ||
                          gate_spans_hold(free_spans, free_n, net_wday, net_minute)))
                snprintf(note, sizeof(note), "%s", gate_text(sc.language, GateText_ClockDiffers));
            else if (!netted && timed && gate_spans_hold(free_spans, free_n, wday, minute))
                snprintf(note, sizeof(note), "%s", gate_text(sc.language, GateText_ClockUnknown));
            watch_kick();
            screen_show(&sc, GateScreen_Asking, note, refused > 0, false);
            watch_pause();
            r = ask_pin(&code);
        }
        watch_kick();
        log_try(when, r, nro);
        if (clocked) pause_write(pause_len, nro, r, now, pause_rule, steadied ? &steady : NULL);
        // How it went, said under it: paused or closed, as above.
        if (r != GateResult_Paused && r != GateResult_Closed) note[0] = '\0';
        if (r == GateResult_Refused) {
            // The day's count with this one, or the pause it starts now.
            uint64_t next_until = 0;
            static char after[GATE_PAUSE_MAX];
            const size_t n = clocked ? gate_pause_update_steady(pause_text, pause_len, nro, r, now, pause_rule,
                                                                steadied ? &steady : NULL, after, sizeof(after))
                                     : 0;
            if (n && gate_pause_find(after, n, nro, NULL, &next_until) && next_until > now) {
                paused_note(sc.language, next_until, now, note, sizeof(note));
                break;   // paused: no trying again
            }
            if (timed) snprintf(note, sizeof(note), gate_text(sc.language, GateText_Tried), refused + 1);
            if (sc.on) {
                screen_show(&sc, GateScreen_Refused, note, 1, true);
                if (wants_retry()) continue;
                shown = true;
            }
            break;
        }
        if (r == GateResult_Free && timed) {
            const int end = gate_spans_until(free_spans, free_n, wday, minute);
            if (end >= 0) {
                char at[16];
                snprintf(at, sizeof(at), "%02u:%02u", (unsigned)end / 60u, (unsigned)end % 60u);
                snprintf(note, sizeof(note), gate_text(sc.language, GateText_FreeUntil), at);
            }
        } else if (r == GateResult_Pin && rules.limit) {
            time_left_note(sc.language, note, sizeof(note));
        } else if (r == GateResult_Error && code) {
            char c[16];
            if (gate_error_code((uint32_t)code, c, sizeof(c)))
                snprintf(note, sizeof(note), gate_text(sc.language, GateText_ErrorCode), c);
        }
        break;
    }
    // How it went, a moment: shorter when it starts (the homebrew waits),
    // longer when it does not. Once refused, the screen above was it.
    static const GateScreenState SHOWN[] = { GateScreen_Opening, GateScreen_Free,   GateScreen_Refused, GateScreen_Error,
                                             GateScreen_Paused,  GateScreen_Closed, GateScreen_Limit };
    const bool opens = r == GateResult_Pin || r == GateResult_Free;
    if (sc.on && !shown) {
        screen_show(&sc, SHOWN[r], note[0] ? note : NULL, !opens, false);
        svcSleepThread(opens ? 250000000ULL : 2500000000ULL);
    }
    screen_close(&sc);
    if (watching) {
        ueventSignal(&g_watch_done);
        threadWaitForExit(&watch);
        threadClose(&watch);
    }
    if (opens && envHasNextLoad() && R_SUCCEEDED(envSetNextLoad(nro, argv))) return true;
    __nx_applet_exit_mode = 1;
    return true;
}
