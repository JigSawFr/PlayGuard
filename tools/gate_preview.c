// The PIN gate's screen (source/core/gate_screen.c) as PNG files, one per
// state, to look at it on a computer:
//   make gate-preview [GATE_LANG=2] [GATE_THEME=light]  ->  build/gate-preview/<state>.png
// (the language as libnx's SetLanguage numbers it: 1 American English, 2 French …;
// PlayGuard's dark theme unless GATE_THEME is light)
// With PlayGuard's own font for all text (the console draws the name and
// the lines in its own font, which has every language) and a made-up
// homebrew's icon (source/sim/sim_homebrew_icons.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gate.h"
#include "gate_screen.h"
#include "gate_text.h"
#include "stb_image_write.h"
#include "../source/sim/sim_homebrew_icons.h"

static unsigned char *slurp(const char *path, size_t *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = (unsigned char *)malloc((size_t)size);
    *n = fread(data, 1, (size_t)size, f);
    fclose(f);
    return data;
}

static void sink(void *ctx, void *data, int size)
{
    fwrite(data, 1, (size_t)size, (FILE *)ctx);
}

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "build/gate-preview";
    size_t n = 0;
    unsigned char *font = slurp("resources/font/sora_bold_latin.ttf", &n);
    if (!font) {
        fprintf(stderr, "resources/font/sora_bold_latin.ttf not found (run from the repository)\n");
        return 1;
    }
    const int lang = argc > 2 ? atoi(argv[2]) : 1;
    const int light = argc > 3 && strcmp(argv[3], "light") == 0;
    static const struct {
        const char *file;
        GateText status, hint;
        GateScreenState state;
    } SHOTS[] = {
        { "loading", GateText_Asking, GateText_Count, GateScreen_Loading },
        { "asking", GateText_Asking, GateText_AskingHint, GateScreen_Asking },
        { "opening", GateText_Opening, GateText_Count, GateScreen_Opening },
        { "free", GateText_Free, GateText_Count, GateScreen_Free },
        { "refused", GateText_Refused, GateText_RefusedHint, GateScreen_Refused },
        { "error", GateText_Error, GateText_Count, GateScreen_Error },
        { "paused", GateText_Paused, GateText_PausedHint, GateScreen_Paused },
        { "closed", GateText_Closed, GateText_ClosedHint, GateScreen_Closed },
        { "limit", GateText_Limit, GateText_LimitHint, GateScreen_Limit },
    };
    // Asking: its free hours (the weekend, 09:00 to 20:00); refused: the
    // day's second refusal.
    const GateHours weekend = { 0x41, 540, 1200 };
    char span[200], hours[256] = "", tried[128];
    if (gate_hours_text(&weekend, gate_text(lang, GateText_Days), span, sizeof(span)))
        snprintf(hours, sizeof(hours), gate_text(lang, GateText_FreeHours), span);
    snprintf(tried, sizeof(tried), gate_text(lang, GateText_Tried), 2);
    // Paused until 21:18; closed in the evening.
    const GateHours evening = { 0x7F, 1260, 420 };
    char paused[128], closed[256] = "";
    char in[64], at[96];
    snprintf(in, sizeof(in), gate_text(lang, GateText_InMinutes), 12);
    snprintf(at, sizeof(at), "21:18 (%s)", in);
    snprintf(paused, sizeof(paused), gate_text(lang, GateText_PausedUntil), at);
    // Opening: the day's play time left; free: until 20:00; an error: its code.
    char minutes[64], left[128], until[128], code[16], error[128];
    gate_minutes_text(45, gate_text(lang, GateText_Minutes), gate_text(lang, GateText_HoursMinutes), minutes, sizeof(minutes));
    snprintf(left, sizeof(left), gate_text(lang, GateText_TimeLeft), minutes);
    snprintf(until, sizeof(until), gate_text(lang, GateText_FreeUntil), "20:00");
    gate_error_code(168u | (12u << 9), code, sizeof(code));
    snprintf(error, sizeof(error), gate_text(lang, GateText_ErrorCode), code);
    if (gate_hours_text(&evening, gate_text(lang, GateText_Days), span, sizeof(span)))
        snprintf(closed, sizeof(closed), gate_text(lang, GateText_ClosedHours), span);
    uint32_t *px = (uint32_t *)malloc(sizeof(uint32_t) * GATE_SCREEN_W * GATE_SCREEN_H);
    for (size_t i = 0; i < sizeof(SHOTS) / sizeof(SHOTS[0]); i++) {
        const GateScreen s = { font, font, i == 5 ? NULL : SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS),
                               "Star Atlas", gate_text(lang, SHOTS[i].status),
                               SHOTS[i].hint == GateText_Count ? NULL : gate_text(lang, SHOTS[i].hint), SHOTS[i].state,
                               SHOTS[i].state == GateScreen_Asking || SHOTS[i].state == GateScreen_Loading ? hours
                               : SHOTS[i].state == GateScreen_Refused ? tried
                               : SHOTS[i].state == GateScreen_Paused  ? paused
                               : SHOTS[i].state == GateScreen_Closed  ? closed
                               : SHOTS[i].state == GateScreen_Opening ? left
                               : SHOTS[i].state == GateScreen_Free    ? until
                               : SHOTS[i].state == GateScreen_Error   ? error
                                                                      : NULL,
                               SHOTS[i].state != GateScreen_Asking && SHOTS[i].state != GateScreen_Opening &&
                                   SHOTS[i].state != GateScreen_Free && SHOTS[i].state != GateScreen_Loading,
                               // Its rules: hours without the PIN, its closed hours, the day's limit.
                               GATE_RULE_FREE | GATE_RULE_CLOSED | GATE_RULE_LIMIT, gate_text(lang, GateText_Badge),
                               // Refused: Ⓐ to try again, Ⓑ for the HOME menu.
                               SHOTS[i].state == GateScreen_Refused ? gate_text(lang, GateText_RetryA) : NULL,
                               SHOTS[i].state == GateScreen_Refused ? gate_text(lang, GateText_RetryB) : NULL,
                               // Opening: its bar and the mark's arc part of the way.
                               0.6f, 0.1f, light };
        gate_screen_draw(px, GATE_SCREEN_W, GATE_SCREEN_H, GATE_SCREEN_W, &s);
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.png", out, SHOTS[i].file);
        // Owner-writable only (fopen would ask for 0666).
        const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        FILE *f = fd >= 0 ? fdopen(fd, "wb") : NULL;
        if (!f) {
            if (fd >= 0) close(fd);
            fprintf(stderr, "%s cannot be written\n", path);
            return 1;
        }
        stbi_write_png_to_func(sink, f, GATE_SCREEN_W, GATE_SCREEN_H, 4, px, GATE_SCREEN_W * 4);
        fclose(f);
        printf("%s\n", path);
    }
    free(px);
    free(font);
    return 0;
}
