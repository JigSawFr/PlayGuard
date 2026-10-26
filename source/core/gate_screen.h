// gate_screen — what the PIN gate of a protected forwarder shows around the
// system's PIN screen (core/gate_nx.h), as PlayGuard's loading screen in
// every state: centred, the homebrew's icon and name, "Protected by
// PlayGuard", then a bar filling while it opens (the mark's arc turning),
// else how it went in the state's colour; Cadran's dial behind, in
// PlayGuard's theme (dark or light). Drawn into a 32-bit RGBA buffer:
// the console's framebuffer, or an image on a computer (tools/gate_preview.c,
// tests/gate_screen).
//
// Plain C, no libnx; text and the icon through extern/stb/pg_stb.h.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GATE_SCREEN_W 1280
#define GATE_SCREEN_H 720

typedef enum {
    GateScreen_Asking = 0,   // before the system's PIN screen
    GateScreen_Opening,      // the PIN entered: the homebrew starts
    GateScreen_Free,         // in its free hours (or no PIN on the console): it starts
    GateScreen_Refused,      // left with B: back to the HOME menu
    GateScreen_Error,        // the PIN screen did not open: back to the HOME menu
    GateScreen_Paused,       // paused after refusals: back to the HOME menu, the PIN not asked
    GateScreen_Closed,       // in its hours without any access: back to the HOME menu
    GateScreen_Limit,        // the day's play time used up: back to the HOME menu
    GateScreen_Loading,      // the opening, before the system's PIN screen
} GateScreenState;

// The rules a forwarder carries (GateScreen.rules), each an icon by the
// badge, as PlayGuard's list of forwarders shows them.
#define GATE_RULE_FREE   1u   // hours without the PIN: an open padlock
#define GATE_RULE_CLOSED 2u   // hours it never opens: a square (stop)
#define GATE_RULE_LIMIT  4u   // closed once today's play time is used up: a timer

typedef struct {
    const unsigned char *brand_font;   // Sora (Latin): "PlayGuard", the badge, the keys
    const unsigned char *text_font;    // any language (the console's own font): the name, the lines
    const unsigned char *icon_jpeg;    // the homebrew's icon, or NULL
    size_t               icon_size;
    const char          *name;         // UTF-8
    const char          *status;       // the line under the badge, as the state says it
    const char          *hint;         // a smaller one under it, or NULL
    GateScreenState      state;
    // A line under the hint, or NULL: the tries refused today (`note_warn`,
    // amber), or when it starts without the PIN (teal).
    const char          *note;
    int                  note_warn;
    unsigned             rules;        // GATE_RULE_*, 0: none shown
    const char          *badge;        // "Protected by PlayGuard" in its language, or NULL (English)
    // Ⓐ's and Ⓑ's labels, bottom right, as the HOME menu puts its buttons ("Try again", "HOME menu"); NULL: none.
    const char          *button_a;
    const char          *button_b;
    // The opening (GateScreen_Loading, _Opening, _Free), as PlayGuard's
    // loading screen: its bar filled to `progress` (0..1), the glint on
    // PlayGuard's mark `spin` of the way round it (in turns).
    float                progress;
    float                spin;
    int                  light;        // PlayGuard's light theme (0: its dark one)
} GateScreen;

// Draws `s` into `px` (`w` x `h` pixels, `stride` pixels a row, RGBA: R in
// the lowest byte). The layout is made for 1280 x 720: the text centred,
// clear of the dial, the status and the hint on up to two lines each; a missing
// font draws no text, a missing icon PlayGuard's mark in its place.
void gate_screen_draw(uint32_t *px, int w, int h, int stride, const GateScreen *s);

// The same in two parts, for the opening's animation: what stays, once,
// then each frame, over a copy of it, what moves (the bar, the mark's arc;
// nothing in the other states). gate_screen_draw draws both.
void gate_screen_draw_still(uint32_t *px, int w, int h, int stride, const GateScreen *s);
void gate_screen_draw_motion(uint32_t *px, int w, int h, int stride, const GateScreen *s);

#ifdef __cplusplus
}
#endif
