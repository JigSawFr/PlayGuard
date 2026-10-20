// gate_screen — what the PIN gate of a protected forwarder shows around the
// system's PIN screen (core/gate_nx.h): the homebrew's icon and name,
// "Protected by PlayGuard", and how it went, on Cadran's night blue with its
// dial. Drawn into a 32-bit RGBA buffer: the console's framebuffer, or an
// image on a computer (tools/gate_preview.c, tests/gate_screen).
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
} GateScreenState;

typedef struct {
    const unsigned char *brand_font;   // Sora (Latin): "PlayGuard", the badge, the clock
    const unsigned char *text_font;    // any language (the console's own font): the name, the lines
    const unsigned char *icon_jpeg;    // the homebrew's icon, or NULL
    size_t               icon_size;
    const char          *name;         // UTF-8
    const char          *status;       // the line under the badge, as the state says it
    const char          *hint;         // a smaller one under it, or NULL
    const char          *footer;       // PlayGuard's tagline, bottom left
    const char          *clock;        // "21:03", top right, or NULL
    GateScreenState      state;
    // A line under the hint, or NULL: the tries refused today (`note_warn`,
    // amber), or when it starts without the PIN (teal).
    const char          *note;
    int                  note_warn;
} GateScreen;

// Draws `s` into `px` (`w` x `h` pixels, `stride` pixels a row, RGBA: R in
// the lowest byte). The layout is made for 1280 x 720; a missing font draws
// no text, a missing icon PlayGuard's mark in its place.
void gate_screen_draw(uint32_t *px, int w, int h, int stride, const GateScreen *s);

#ifdef __cplusplus
}
#endif
