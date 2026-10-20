// gate_screen — the PIN gate's screen (see gate_screen.h). Antialiased
// shapes from their distance fields, text through stb_truetype.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate_screen.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "pg_stb.h"

// Cadran's dark palette (ui/cadran_frame.cpp).
typedef struct {
    float r, g, b;
} Rgb;
static const Rgb NAVY = { 0x0B, 0x14, 0x24 }, TEXT = { 0xEE, 0xF2, 0xF8 }, TEXT2 = { 0xA9, 0xB5, 0xC9 },
                 TEXT3 = { 0x83, 0x92, 0xAA }, SURF2 = { 0x1A, 0x2A, 0x45 }, TRACK = { 0x1F, 0x33, 0x55 },
                 TEAL = { 0x2E, 0xC4, 0xA6 }, AMBER = { 0xFF, 0xB5, 0x47 }, CORAL = { 0xFF, 0x7A, 0x7A },
                 WHITE = { 0xFF, 0xFF, 0xFF };

typedef struct {
    uint32_t *px;
    int       w, h, stride;
} Canvas;

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

// `c` over the pixel at (x, y), with `a` of 0..1.
static void blend(Canvas *cv, int x, int y, Rgb c, float a)
{
    if (x < 0 || y < 0 || x >= cv->w || y >= cv->h || a <= 0.0f) return;
    if (a > 1.0f) a = 1.0f;
    uint32_t *p = &cv->px[(size_t)y * (size_t)cv->stride + (size_t)x];
    const float r = (float)(*p & 0xFF), g = (float)(*p >> 8 & 0xFF), b = (float)(*p >> 16 & 0xFF);
    const uint32_t nr = (uint32_t)(r + (c.r - r) * a + 0.5f), ng = (uint32_t)(g + (c.g - g) * a + 0.5f),
                   nb = (uint32_t)(b + (c.b - b) * a + 0.5f);
    *p = 0xFF000000u | nb << 16 | ng << 8 | nr;
}

// Coverage of a pixel from its signed distance to an edge (negative inside).
static float cover(float d)
{
    return clampf(0.5f - d, 0.0f, 1.0f);
}

// ---------------------------------------------------------------- shapes

static float rrect_sd(float px, float py, float x, float y, float w, float h, float r)
{
    const float cx = x + w / 2, cy = y + h / 2;
    const float qx = fabsf(px - cx) - (w / 2 - r), qy = fabsf(py - cy) - (h / 2 - r);
    const float ox = qx > 0 ? qx : 0, oy = qy > 0 ? qy : 0;
    return sqrtf(ox * ox + oy * oy) + fminf(fmaxf(qx, qy), 0.0f) - r;
}

static void fill_rrect(Canvas *cv, float x, float y, float w, float h, float r, Rgb c, float a)
{
    for (int py = (int)floorf(y) - 1; py <= (int)ceilf(y + h) + 1; py++)
        for (int px = (int)floorf(x) - 1; px <= (int)ceilf(x + w) + 1; px++)
            blend(cv, px, py, c, a * cover(rrect_sd(px + 0.5f, py + 0.5f, x, y, w, h, r)));
}

// The outline of a rounded rectangle, `t` wide, inside its edge.
static void stroke_rrect(Canvas *cv, float x, float y, float w, float h, float r, float t, Rgb c, float a)
{
    for (int py = (int)floorf(y) - 1; py <= (int)ceilf(y + h) + 1; py++)
        for (int px = (int)floorf(x) - 1; px <= (int)ceilf(x + w) + 1; px++) {
            const float d = rrect_sd(px + 0.5f, py + 0.5f, x, y, w, h, r);
            blend(cv, px, py, c, a * cover(fabsf(d + t / 2) - t / 2));
        }
}

static void fill_circle(Canvas *cv, float cx, float cy, float r, Rgb c, float a)
{
    for (int py = (int)(cy - r) - 1; py <= (int)(cy + r) + 1; py++)
        for (int px = (int)(cx - r) - 1; px <= (int)(cx + r) + 1; px++) {
            const float dx = px + 0.5f - cx, dy = py + 0.5f - cy;
            blend(cv, px, py, c, a * cover(sqrtf(dx * dx + dy * dy) - r));
        }
}

// An arc of the circle of radius `r`, `t` wide, with round ends, from
// `from` for `span` radians clockwise (0 at the top); a whole ring for 2π.
static void arc(Canvas *cv, float cx, float cy, float r, float t, float from, float span, Rgb c, float a)
{
    const float pi2 = 6.2831853f;
    const bool whole = span >= pi2 - 1e-4f;
    const float ex[2] = { cx + r * sinf(from), cx + r * sinf(from + span) };
    const float ey[2] = { cy - r * cosf(from), cy - r * cosf(from + span) };
    const float out = r + t / 2 + 1;
    for (int py = (int)(cy - out); py <= (int)(cy + out); py++)
        for (int px = (int)(cx - out); px <= (int)(cx + out); px++) {
            const float dx = px + 0.5f - cx, dy = py + 0.5f - cy;
            float d = fabsf(sqrtf(dx * dx + dy * dy) - r) - t / 2;
            if (!whole) {
                float ang = atan2f(dx, -dy) - from;   // clockwise from the top, relative to `from`
                while (ang < 0) ang += pi2;
                while (ang >= pi2) ang -= pi2;
                if (ang > span) {
                    float best = 1e9f;
                    for (int i = 0; i < 2; i++) {
                        const float ux = px + 0.5f - ex[i], uy = py + 0.5f - ey[i];
                        best = fminf(best, sqrtf(ux * ux + uy * uy) - t / 2);
                    }
                    d = best;
                }
            }
            blend(cv, px, py, c, a * cover(d));
        }
}

// A soft disc: `a` at its centre, nothing from `r` out.
static void glow(Canvas *cv, float cx, float cy, float r, Rgb c, float a)
{
    for (int py = (int)(cy - r); py <= (int)(cy + r); py++)
        for (int px = (int)(cx - r); px <= (int)(cx + r); px++) {
            const float dx = px + 0.5f - cx, dy = py + 0.5f - cy;
            const float k = 1.0f - sqrtf(dx * dx + dy * dy) / r;
            if (k > 0) blend(cv, px, py, c, a * k * k);
        }
}

// The triangle of PlayGuard's play button, its corners rounded by `round`.
static void play(Canvas *cv, float cx, float cy, float size, float round, Rgb c)
{
    // M112 100 L160 128 L112 156 in the 256 box of branding/mark.svg, about its centre.
    const float k = size / 56.0f;
    const float ax = cx - 16 * k, ay = cy - 28 * k, bx = cx + 32 * k, by = cy, qx = cx - 16 * k, qy = cy + 28 * k;
    const float vx[3] = { ax, bx, qx }, vy[3] = { ay, by, qy };
    for (int py = (int)(ay - round) - 1; py <= (int)(qy + round) + 1; py++)
        for (int px = (int)(ax - round) - 1; px <= (int)(bx + round) + 1; px++) {
            const float x = px + 0.5f, y = py + 0.5f;
            // Signed distance to the triangle (inside negative).
            float d = 1e9f;
            float s = 1;
            for (int i = 0, j = 2; i < 3; j = i, i++) {
                const float ex = vx[j] - vx[i], ey = vy[j] - vy[i], wx = x - vx[i], wy = y - vy[i];
                const float t = clampf((wx * ex + wy * ey) / (ex * ex + ey * ey), 0.0f, 1.0f);
                const float bxx = wx - ex * t, byy = wy - ey * t;
                d = fminf(d, bxx * bxx + byy * byy);
                const bool c1 = y >= vy[i], c2 = y < vy[j], c3 = ex * wy > ey * wx;
                if ((c1 && c2 && c3) || (!c1 && !c2 && !c3)) s = -s;
            }
            blend(cv, px, py, c, cover(s * sqrtf(d) - round));
        }
}

// PlayGuard's mark (branding/mark.svg): the play button inside the daily
// play-time ring, `r` its radius.
static void mark(Canvas *cv, float cx, float cy, float r)
{
    const float k = r / 78.0f;
    fill_circle(cv, cx, cy, r, NAVY, 1);
    arc(cv, cx, cy, 62 * k, 16 * k, 0, 6.2831853f, TRACK, 1);
    arc(cv, cx, cy, 62 * k, 16 * k, 0, 6.2831853f * 292.0f / 390.0f, TEAL, 1);
    play(cv, cx + 0 * k, cy, 56 * k, 4 * k, TEXT);
}

// A padlock, `s` high, its body's top left at (x, y).
static void padlock(Canvas *cv, float x, float y, float s, Rgb c)
{
    const float w = s * 0.86f, body = s * 0.6f;
    arc(cv, x + w / 2, y + s - body, w * 0.3f, s * 0.12f, -1.5707963f, 3.1415927f, c, 1);
    for (int py = (int)(y + s - body - w * 0.3f); py <= (int)(y + s - body); py++) {
        // The shackle's legs, down to the body.
        blend(cv, (int)(x + w / 2 - w * 0.3f), py, c, 1);
        blend(cv, (int)(x + w / 2 + w * 0.3f) - 1, py, c, 1);
    }
    fill_rrect(cv, x, y + s - body, w, body, s * 0.12f, c, 1);
    fill_circle(cv, x + w / 2, y + s - body * 0.55f, s * 0.08f, NAVY, 1);
}

// ---------------------------------------------------------------- text

static int utf8_next(const char **s)
{
    const unsigned char *p = (const unsigned char *)*s;
    int cp, n;
    if (p[0] < 0x80) cp = p[0], n = 1;
    else if ((p[0] & 0xE0) == 0xC0) cp = p[0] & 0x1F, n = 2;
    else if ((p[0] & 0xF0) == 0xE0) cp = p[0] & 0x0F, n = 3;
    else if ((p[0] & 0xF8) == 0xF0) cp = p[0] & 0x07, n = 4;
    else {
        (*s)++;
        return 0xFFFD;
    }
    for (int i = 1; i < n; i++) {
        if ((p[i] & 0xC0) != 0x80) {
            *s += i;
            return 0xFFFD;
        }
        cp = cp << 6 | (p[i] & 0x3F);
    }
    *s += n;
    return cp;
}

static float text_width(const PgFont *f, const char *s)
{
    float w = 0;
    int prev = 0;
    while (s && *s) {
        const int cp = utf8_next(&s);
        if (prev) w += pg_font_kern(f, prev, cp);
        w += pg_font_advance(f, cp);
        prev = cp;
    }
    return w;
}

// `s` from (x, baseline), at most `max` wide (cut with "…"); its width.
static float text(Canvas *cv, const unsigned char *font, float px_size, float x, float baseline, const char *s, Rgb c,
                  float alpha, float max)
{
    PgFont f;
    if (!font || !s || !*s || !pg_font_init(&f, font, px_size)) return 0;
    const float ell = pg_font_advance(&f, 0x2026);
    const bool cut = max > 0 && text_width(&f, s) > max;
    float pen = x;
    int prev = 0;
    while (*s) {
        const char *at = s;
        const int cp = utf8_next(&s);
        if (prev) pen += pg_font_kern(&f, prev, cp);
        const float adv = pg_font_advance(&f, cp);
        if (cut && pen + adv + ell > x + max) {
            s = "\xE2\x80\xA6";   // the rest as "…"
            (void)at;
            const int e = utf8_next(&s);
            int gw, gh, gx, gy;
            unsigned char *g = pg_font_glyph(&f, e, pen - floorf(pen), &gw, &gh, &gx, &gy);
            for (int yy = 0; g && yy < gh; yy++)
                for (int xx = 0; xx < gw; xx++)
                    blend(cv, (int)floorf(pen) + gx + xx, (int)baseline + gy + yy, c, alpha * g[yy * gw + xx] / 255.0f);
            pg_free(g);
            pen += ell;
            break;
        }
        int gw = 0, gh = 0, gx = 0, gy = 0;
        unsigned char *g = pg_font_glyph(&f, cp, pen - floorf(pen), &gw, &gh, &gx, &gy);
        for (int yy = 0; g && yy < gh; yy++)
            for (int xx = 0; xx < gw; xx++)
                blend(cv, (int)floorf(pen) + gx + xx, (int)baseline + gy + yy, c, alpha * g[yy * gw + xx] / 255.0f);
        pg_free(g);
        pen += adv;
        prev = cp;
    }
    return pen - x;
}

static float measure(const unsigned char *font, float px_size, const char *s)
{
    PgFont f;
    if (!font || !s || !pg_font_init(&f, font, px_size)) return 0;
    return text_width(&f, s);
}

// ---------------------------------------------------------------- icon

// The homebrew's icon, `size` square at (x, y), its corners rounded by `r`
// (bilinear, from whatever size it is).
static bool icon(Canvas *cv, const unsigned char *jpeg, size_t n, float x, float y, float size, float r)
{
    int iw = 0, ih = 0;
    unsigned char *rgba = pg_jpeg_rgba(jpeg, n, &iw, &ih);
    if (!rgba || iw <= 0 || ih <= 0) {
        pg_free(rgba);
        return false;
    }
    for (int py = (int)y; py < (int)(y + size); py++)
        for (int px = (int)x; px < (int)(x + size); px++) {
            const float a = cover(rrect_sd(px + 0.5f, py + 0.5f, x, y, size, size, r));
            if (a <= 0) continue;
            const float u = clampf((px + 0.5f - x) / size * iw - 0.5f, 0, (float)iw - 1),
                        v = clampf((py + 0.5f - y) / size * ih - 0.5f, 0, (float)ih - 1);
            const int u0 = (int)u, v0 = (int)v, u1 = u0 + 1 < iw ? u0 + 1 : u0, v1 = v0 + 1 < ih ? v0 + 1 : v0;
            const float fu = u - u0, fv = v - v0;
            Rgb c;
            float *ch[3] = { &c.r, &c.g, &c.b };
            for (int k = 0; k < 3; k++) {
                const float p00 = rgba[(v0 * iw + u0) * 4 + k], p10 = rgba[(v0 * iw + u1) * 4 + k],
                            p01 = rgba[(v1 * iw + u0) * 4 + k], p11 = rgba[(v1 * iw + u1) * 4 + k];
                *ch[k] = (p00 * (1 - fu) + p10 * fu) * (1 - fv) + (p01 * (1 - fu) + p11 * fu) * fv;
            }
            blend(cv, px, py, c, a);
        }
    pg_free(rgba);
    return true;
}

// ---------------------------------------------------------------- the screen

void gate_screen_draw(uint32_t *px, int w, int h, int stride, const GateScreen *s)
{
    if (!px || w <= 0 || h <= 0 || stride < w || !s) return;
    Canvas cv = { px, w, h, stride };

    // Night blue, a little lighter at the top.
    for (int y = 0; y < h; y++) {
        const float t = (float)y / (float)h;
        const uint32_t r = (uint32_t)(0x0E + (0x07 - 0x0E) * t), g = (uint32_t)(0x1A + (0x0E - 0x1A) * t),
                       b = (uint32_t)(0x2E + (0x1A - 0x2E) * t);
        for (int x = 0; x < w; x++) px[(size_t)y * (size_t)stride + (size_t)x] = 0xFF000000u | b << 16 | g << 8 | r;
    }

    // Cadran's dial on the right: its glow, two rings, the minutes, the day's arc.
    const Rgb accent = s->state == GateScreen_Refused ? CORAL : s->state == GateScreen_Error ? AMBER : TEAL;
    const float dx = 1130, dy = 340;
    glow(&cv, dx, dy, 330, accent, 0.09f);
    arc(&cv, dx, dy, 262, 1.5f, 0, 6.2831853f, WHITE, 0.05f);
    arc(&cv, dx, dy, 160, 1.5f, 0, 6.2831853f, WHITE, 0.04f);
    for (int i = 0; i < 60; i++) {
        const float ang = i * 6.2831853f / 60.0f, len = i % 5 == 0 ? 18.0f : 8.0f;
        const float sx = sinf(ang), sy = -cosf(ang);
        for (float t = 0; t < len; t += 0.5f) {
            const float rr = 272 + t;
            blend(&cv, (int)(dx + sx * rr), (int)(dy + sy * rr), WHITE, i % 5 == 0 ? 0.12f : 0.06f);
        }
    }
    arc(&cv, dx, dy, 211, 20, 0, 6.2831853f, TRACK, 0.28f);
    arc(&cv, dx, dy, 211, 20, 0, 6.2831853f * 0.75f, accent, 0.16f);

    // The header: PlayGuard's mark and name; the clock.
    mark(&cv, 92, 76, 30);
    text(&cv, s->brand_font, 30, 136, 87, "PlayGuard", TEXT, 1, 0);
    if (s->clock && *s->clock) {
        const float cw = measure(s->brand_font, 30, s->clock);
        text(&cv, s->brand_font, 30, (float)w - 64 - cw, 87, s->clock, TEXT2, 1, 0);
    }

    // The homebrew: its icon over a glow (else PlayGuard's mark), its name.
    const float ix = 112, iy = 214, isz = 232;
    glow(&cv, ix + isz / 2, iy + isz / 2, 220, accent, 0.18f);
    fill_rrect(&cv, ix - 6, iy - 6, isz + 12, isz + 12, 38, SURF2, 0.9f);
    if (!s->icon_jpeg || !icon(&cv, s->icon_jpeg, s->icon_size, ix, iy, isz, 32)) {
        fill_rrect(&cv, ix, iy, isz, isz, 32, NAVY, 1);
        mark(&cv, ix + isz / 2, iy + isz / 2, 86);
    }
    stroke_rrect(&cv, ix - 6, iy - 6, isz + 12, isz + 12, 38, 2, WHITE, 0.08f);

    const float tx = 400;
    text(&cv, s->text_font, 58, tx, 290, s->name, TEXT, 1, (float)w - tx - 80);

    // "Protected by PlayGuard", a teal badge with its padlock.
    static const char BADGE[] = "Protected by PlayGuard";
    const float bw = measure(s->brand_font, 26, BADGE) + 92;
    fill_rrect(&cv, tx, 322, bw, 54, 27, TEAL, 0.14f);
    stroke_rrect(&cv, tx, 322, bw, 54, 27, 2, TEAL, 0.55f);
    padlock(&cv, tx + 26, 334, 30, TEAL);
    text(&cv, s->brand_font, 26, tx + 70, 358, BADGE, TEAL, 1, 0);

    // How it goes.
    const Rgb line = s->state == GateScreen_Asking ? TEXT2 : accent;
    if (s->state != GateScreen_Asking) fill_circle(&cv, tx + 9, 442, 7, accent, 1);
    text(&cv, s->text_font, 32, s->state == GateScreen_Asking ? tx : tx + 28, 454, s->status, line, 1,
         (float)w - tx - 80);
    text(&cv, s->text_font, 26, tx, 500, s->hint, TEXT3, 1, (float)w - tx - 80);

    // The footer: a hairline, PlayGuard's tagline and site.
    for (int x = 64; x < w - 64; x++) blend(&cv, x, h - 82, WHITE, 0.08f);
    text(&cv, s->text_font, 24, 64, (float)h - 40, s->footer, TEXT3, 1, (float)w - 480);
    static const char SITE[] = "playguard.link";
    text(&cv, s->brand_font, 24, (float)w - 64 - measure(s->brand_font, 24, SITE), (float)h - 40, SITE, TEXT3, 1, 0);
}
