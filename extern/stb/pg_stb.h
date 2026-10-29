// pg_stb — stb_truetype and stb_image (JPEG only), static in pg_stb.c,
// behind a few functions of PlayGuard's: the PIN gate's screen
// (source/core/gate_screen.c) draws text and a homebrew's icon with them.
// nanovg (Aurore) builds its own copies with its own allocator, which
// these do not touch.
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char opaque[256];   // stbtt_fontinfo
    float         scale;
    int           ascent, descent, line_gap;
} PgFont;

// The font at `data` (a .ttf, kept by the caller), drawn `px` pixels high.
int  pg_font_init(PgFont *f, const unsigned char *data, float px);
// A codepoint's advance and left bearing, in pixels; the kerning to the next.
float pg_font_advance(const PgFont *f, int cp);
float pg_font_kern(const PgFont *f, int cp, int next);
// Whether the font draws `cp` (not its missing glyph).
int  pg_font_has(const PgFont *f, int cp);
// Its coverage bitmap (free it with pg_free), its size and offset from the pen.
unsigned char *pg_font_glyph(const PgFont *f, int cp, float shift_x, int *w, int *h, int *x0, int *y0);

// A JPEG as RGBA (free it with pg_free); NULL when it cannot be read.
unsigned char *pg_jpeg_rgba(const unsigned char *data, size_t size, int *w, int *h);

void pg_free(void *p);

#ifdef __cplusplus
}
#endif
