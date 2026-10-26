// gate_screen — the PIN gate's screen (see gate_screen.h). Antialiased
// shapes from their distance fields, text through stb_truetype.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate_screen.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "pg_stb.h"

typedef struct {
    float r, g, b;
} Rgb;

// PlayGuard's mark keeps the brand's colours in both themes (as the app's
// header): its night, its shield in Aurore's aurora (teal to violet).
static const Rgb NAVY = { 0x0B, 0x14, 0x24 }, MARK_NIGHT = { 0x0E, 0x1A, 0x33 }, MARK_PLAY = { 0xEE, 0xF2, 0xF8 },
                 AURORA_TEAL = { 0x00, 0xFF, 0xCC }, AURORA_VIOLET = { 0x8A, 0x5C, 0xFF }, WHITE = { 0xFF, 0xFF, 0xFF };

// Cadran's two palettes (ui/cadran_palette.hpp): each ink for text and
// lines (4.5:1 on its background), each fill for what glows or fills.
typedef struct {
    Rgb   bg_top, bg_bottom;        // the background, a little lighter (dark) or whiter (light) at the top
    Rgb   text, text2, text3;
    Rgb   plate;                    // under the icon
    float plate_a;
    Rgb   bar_track;                // the opening's bar, empty
    Rgb   dial_track;               // the dial's ring, at dial_a
    float dial_a;
    Rgb   line;                     // the dial's rings and minutes, at line_k times their strength
    float line_k, glow_k;           // the glows' strength
    Rgb   key, key_ink;             // Ⓐ's and Ⓑ's discs and letters
    Rgb   teal, teal_fill, amber, amber_fill, coral, coral_fill;
} Palette;

static const Palette DARK = {
    { 0x0E, 0x1A, 0x2E }, { 0x07, 0x0E, 0x1A }, { 0xEE, 0xF2, 0xF8 }, { 0xA9, 0xB5, 0xC9 }, { 0x83, 0x92, 0xAA },
    { 0x1A, 0x2A, 0x45 }, 0.9f, { 0x1F, 0x33, 0x55 }, { 0x1F, 0x33, 0x55 }, 0.28f, { 0xFF, 0xFF, 0xFF }, 1.0f, 1.0f,
    { 0xEE, 0xF2, 0xF8 }, { 0x0B, 0x14, 0x24 }, { 0x2E, 0xC4, 0xA6 }, { 0x2E, 0xC4, 0xA6 }, { 0xFF, 0xB5, 0x47 },
    { 0xFF, 0xB5, 0x47 }, { 0xFF, 0x7A, 0x7A }, { 0xFF, 0x7A, 0x7A },
};
static const Palette LIGHT = {
    { 0xF4, 0xF6, 0xF9 }, { 0xE6, 0xEA, 0xF0 }, { 0x0B, 0x14, 0x24 }, { 0x3F, 0x4C, 0x63 }, { 0x5F, 0x6E, 0x88 },
    { 0xFF, 0xFF, 0xFF }, 1.0f, { 0xD3, 0xD9, 0xE2 }, { 0x0B, 0x14, 0x24 }, 0.08f, { 0x0B, 0x14, 0x24 }, 2.0f, 1.4f,
    { 0x0B, 0x14, 0x24 }, { 0xFF, 0xFF, 0xFF }, { 0x0A, 0x70, 0x5F }, { 0x14, 0xA3, 0x8A }, { 0x8A, 0x52, 0x00 },
    { 0xE0, 0x93, 0x1B }, { 0xB7, 0x1C, 0x1C }, { 0xE5, 0x48, 0x4D },
};

typedef struct {
    uint32_t      *px;
    int            w, h, stride;
    const Palette *pal;
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

// PlayGuard's mark (branding/mark.svg): the icon's shield, its outline in
// the aurora's colours around the night, the play sign inside; `r` half its
// height. While it opens a glint goes round the outline (the motion, `spin`).
// The shield as a polygon in the 256 box of branding/mark.svg (its curves
// sampled), about its centre (128, 130).
#define SHIELD_N 52
static float shield_x[SHIELD_N], shield_y[SHIELD_N];

static void shield_points(void)
{
    if (shield_y[0] != 0.0f) return;   // made once (its first point is the top, -88)
    // M128 42 C154 54 180 60 204 60 L204 126 C204 172 174 200 128 218 C82 200 52 172 52 126 L52 60 C76 60 102 54 128 42
    static const float C[4][8] = { { 128, 42, 154, 54, 180, 60, 204, 60 },
                                   { 204, 126, 204, 172, 174, 200, 128, 218 },
                                   { 128, 218, 82, 200, 52, 172, 52, 126 },
                                   { 52, 60, 76, 60, 102, 54, 128, 42 } };
    int n = 0;
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 13; i++) {
            const float t = (float)i / 13.0f, u = 1 - t;
            const float a = u * u * u, b = 3 * u * u * t, d = 3 * u * t * t, e = t * t * t;
            shield_x[n] = a * C[c][0] + b * C[c][2] + d * C[c][4] + e * C[c][6] - 128;
            shield_y[n] = a * C[c][1] + b * C[c][3] + d * C[c][5] + e * C[c][7] - 130;
            n++;
        }
}

// Signed distance from (x, y) to the shield scaled by `k` about (cx, cy), negative inside.
static float shield_sd(float x, float y, float cx, float cy, float k)
{
    float d = 1e9f, s = 1;
    for (int i = 0, j = SHIELD_N - 1; i < SHIELD_N; j = i, i++) {
        const float vix = cx + shield_x[i] * k, viy = cy + shield_y[i] * k;
        const float vjx = cx + shield_x[j] * k, vjy = cy + shield_y[j] * k;
        const float ex = vjx - vix, ey = vjy - viy, wx = x - vix, wy = y - viy;
        const float t = clampf((wx * ex + wy * ey) / (ex * ex + ey * ey), 0.0f, 1.0f);
        const float bx = wx - ex * t, by = wy - ey * t;
        d = fminf(d, bx * bx + by * by);
        const bool c1 = y >= viy, c2 = y < vjy, c3 = ex * wy > ey * wx;
        if ((c1 && c2 && c3) || (!c1 && !c2 && !c3)) s = -s;
    }
    return s * sqrtf(d);
}

static Rgb mix(Rgb a, Rgb b, float t)
{
    return (Rgb){ a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
}

static void mark(Canvas *cv, float cx, float cy, float r)
{
    shield_points();
    const float k = r / 88.0f, t = 16 * k;
    for (int py = (int)(cy - 92 * k) - 2; py <= (int)(cy + 92 * k) + 2; py++)
        for (int px = (int)(cx - 80 * k) - 2; px <= (int)(cx + 80 * k) + 2; px++) {
            const float x = px + 0.5f, y = py + 0.5f;
            const float d = shield_sd(x, y, cx, cy, k);
            blend(cv, px, py, MARK_NIGHT, cover(d + t / 2));
            // The outline, centred on the edge, teal at the top left to violet at the bottom right.
            const float along = clampf(((x - cx) / (152 * k) + (y - cy) / (176 * k)) * 0.5f + 0.5f, 0.0f, 1.0f);
            blend(cv, px, py, mix(AURORA_TEAL, AURORA_VIOLET, along), cover(fabsf(d) - t / 2));
        }
    play(cv, cx + 2 * k, cy, 64 * k, 5 * k, MARK_PLAY);
}

// The glint, `turn` (0..1) of the way round the outline from the top.
static void mark_glint(Canvas *cv, float cx, float cy, float r, float turn)
{
    shield_points();
    const float k = r / 88.0f;
    const float f = (turn - floorf(turn)) * SHIELD_N;
    const int i = (int)f % SHIELD_N, j = (i + 1) % SHIELD_N;
    const float u = f - floorf(f);
    const float x = cx + (shield_x[i] + (shield_x[j] - shield_x[i]) * u) * k;
    const float y = cy + (shield_y[i] + (shield_y[j] - shield_y[i]) * u) * k;
    glow(cv, x, y, 22 * k, WHITE, 0.55f);
    fill_circle(cv, x, y, 6 * k, WHITE, 0.9f);
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
    fill_circle(cv, x + w / 2, y + s - body * 0.55f, s * 0.08f, cv->pal->bg_top, 1);
}

// The rules' icons (GATE_RULE_*), each `s` square, its top left at (x, y),
// drawn as the app's (view/cadran_icon.cpp): an open padlock, a square, a timer.
static void open_padlock(Canvas *cv, float x, float y, float s, Rgb c)
{
    const float w = s * 0.78f, body = s * 0.5f, bx = x + (s - w) / 2, top = y + s - body;
    // The shackle up and to the left, its right leg out of the body: open.
    arc(cv, bx + w * 0.35f, top - s * 0.12f, w * 0.26f, s * 0.11f, -3.1415927f, 3.1415927f, c, 1);
    fill_rrect(cv, bx + w * 0.35f - w * 0.26f - s * 0.055f, top - s * 0.12f, s * 0.11f, s * 0.14f, s * 0.03f, c, 1);
    fill_rrect(cv, bx, top, w, body, s * 0.1f, c, 1);
    fill_circle(cv, bx + w / 2, top + body * 0.48f, s * 0.07f, cv->pal->bg_top, 1);
}

static void stop_square(Canvas *cv, float x, float y, float s, Rgb c)
{
    fill_rrect(cv, x + s * 0.2f, y + s * 0.2f, s * 0.6f, s * 0.6f, s * 0.1f, c, 1);
}

static void timer(Canvas *cv, float x, float y, float s, Rgb c)
{
    const float cx = x + s / 2, cy = y + s * 0.56f, r = s * 0.36f;
    arc(cv, cx, cy, r, s * 0.1f, 0, 6.2831853f, c, 1);
    fill_rrect(cv, cx - s * 0.04f, cy - r * 0.72f, s * 0.08f, r * 0.8f, s * 0.04f, c, 1);   // the hand, at 12
    fill_rrect(cv, cx - s * 0.12f, y + s * 0.02f, s * 0.24f, s * 0.08f, s * 0.04f, c, 1);   // the button
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

// `s` centred on `cx`, at most `max` wide.
static void centred(Canvas *cv, const unsigned char *font, float px_size, float cx, float baseline, const char *s, Rgb c,
                    float max)
{
    const float tw = measure(font, px_size, s);
    text(cv, font, px_size, cx - (tw < max ? tw : max) / 2, baseline, s, c, 1, max);
}

// Whether `font` draws every character of `s`.
static bool covers(const unsigned char *font, const char *s)
{
    PgFont f;
    if (!font || !s || !pg_font_init(&f, font, 16)) return false;
    while (*s)
        if (!pg_font_has(&f, utf8_next(&s))) return false;
    return true;
}

// Punctuation a line does not start with (French puts a space before it).
static bool clings(const char *s)
{
    return *s == ':' || *s == ';' || *s == '!' || *s == '?' || !strncmp(s, "\xC2\xBB", 2);
}

// The width of `s`'s first `len` bytes.
static float width_of(const PgFont *f, const char *s, size_t len)
{
    char buf[512];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, s, len);
    buf[len] = '\0';
    return text_width(f, buf);
}

// `s` centred on `cx` from `baseline`, on up to `max_lines` lines `step`
// apart, each at most `max` wide; the last one cut with "…". Broken at a space (after a
// colon when the rest then fits, else where the two last lines come out
// even), else (a language without them) between two characters. How many
// lines it drew; with `cv` NULL, how many it would draw, drawing nothing.
static int paragraph(Canvas *cv, const unsigned char *font, float px_size, float cx, float baseline, float step,
                     const char *s, Rgb c, float max, int max_lines)
{
    PgFont f;
    if (!font || !s || !*s || !pg_font_init(&f, font, px_size)) return 0;
    char buf[512];
    int n = 0;
    while (*s && n < max_lines) {
        if (n == max_lines - 1 || text_width(&f, s) <= max) {
            if (cv) centred(cv, font, px_size, cx, baseline + n * step, s, c, max);
            return n + 1;
        }
        // Where it may break: the spaces within `max`, and the last character that fits.
        const char *spaces[64];
        int k = 0;
        const char *p = s, *fit = NULL;
        while (*p) {
            const char *q = p;
            utf8_next(&q);
            if ((size_t)(q - s) >= sizeof(buf) || width_of(&f, s, (size_t)(q - s)) > max) break;
            fit = q;
            if (*q == ' ' && !clings(q + 1) && k < 64) spaces[k++] = q;
            p = q;
        }
        const char *end = k ? spaces[k - 1] : fit;
        const float rest_of_last = k ? text_width(&f, spaces[k - 1] + 1) : max + 1;
        if (k && rest_of_last <= max) {
            // Two lines will do: after a colon, else the most even.
            float best = 1e9f;
            for (int i = 0; i < k; i++) {
                const float w1 = width_of(&f, s, (size_t)(spaces[i] - s)), w2 = text_width(&f, spaces[i] + 1);
                if (w2 > max) continue;
                if (spaces[i][-1] == ':') {
                    end = spaces[i];
                    break;
                }
                const float worst = w1 > w2 ? w1 : w2;
                if (worst < best) best = worst, end = spaces[i];
            }
        }
        if (!end) {   // not even one character: it alone
            end = s;
            utf8_next(&end);
        }
        const size_t len = (size_t)(end - s) < sizeof(buf) ? (size_t)(end - s) : sizeof(buf) - 1;
        memcpy(buf, s, len);
        buf[len] = '\0';
        if (cv) centred(cv, font, px_size, cx, baseline + n * step, buf, c, max);
        n++;
        s = end;
        while (*s == ' ') s++;
    }
    return n;
}

// `size`, or `smaller` when `s` needs more than `lines` lines `max` wide at `size`.
static float fitting(const unsigned char *font, float size, float smaller, const char *s, float max, int lines)
{
    return paragraph(NULL, font, size, 0, 0, 0, s, WHITE, max, 64) > lines ? smaller : size;
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

// The background, a gradient from its top to its bottom.
static void background(Canvas *cv)
{
    const Rgb a = cv->pal->bg_top, b = cv->pal->bg_bottom;
    for (int y = 0; y < cv->h; y++) {
        const float t = (float)y / (float)cv->h;
        const uint32_t r = (uint32_t)(a.r + (b.r - a.r) * t), g = (uint32_t)(a.g + (b.g - a.g) * t),
                       bl = (uint32_t)(a.b + (b.b - a.b) * t);
        for (int x = 0; x < cv->w; x++) cv->px[(size_t)y * (size_t)cv->stride + (size_t)x] = 0xFF000000u | bl << 16 | g << 8 | r;
    }
}

// Cadran's dial on the right: its glow, two rings, the minutes, the day's
// arc in `accent` (a fill).
static void dial(Canvas *cv, Rgb accent)
{
    const Palette *p = cv->pal;
    const float dx = 1196, dy = 340, k = p->line_k;
    glow(cv, dx, dy, 290, accent, 0.09f * p->glow_k);
    arc(cv, dx, dy, 226, 1.5f, 0, 6.2831853f, p->line, 0.05f * k);
    arc(cv, dx, dy, 138, 1.5f, 0, 6.2831853f, p->line, 0.04f * k);
    for (int i = 0; i < 60; i++) {
        const float ang = i * 6.2831853f / 60.0f, len = i % 5 == 0 ? 16.0f : 7.0f;
        const float sx = sinf(ang), sy = -cosf(ang);
        for (float t = 0; t < len; t += 0.5f) {
            const float rr = 235 + t;
            blend(cv, (int)(dx + sx * rr), (int)(dy + sy * rr), p->line, (i % 5 == 0 ? 0.12f : 0.06f) * k);
        }
    }
    arc(cv, dx, dy, 182, 18, 0, 6.2831853f, p->dial_track, p->dial_a);
    arc(cv, dx, dy, 182, 18, 0, 6.2831853f * 0.75f, accent, 0.16f * p->glow_k);
}

// Every state the same screen, as PlayGuard's loading screen (activity/
// loading.xml): centred, the homebrew's icon over a soft glow, its name,
// "Protected by PlayGuard" (and the rules it carries); under them the bar
// while it opens, else how it went (the state's colour), what to do, the
// note; Cadran's dial on the right, behind; at the bottom, PlayGuard's mark
// (its arc turning while it opens) and its name; once refused, Ⓐ and Ⓑ
// bottom right, as the HOME menu puts its buttons.
#define OPEN_ICON  184.0f
#define OPEN_ICON_Y 130.0f
#define OPEN_BAR_W 420.0f
#define OPEN_BAR_H 8.0f
#define OPEN_BAR_Y 508.0f
#define OPEN_MARK_R 17.0f
#define OPEN_FOOT_Y 664.0f
#define TEXT_MAX   620.0f   // the lines' width, centred: left of the dial's minutes

static bool opening(const GateScreen *s)
{
    return s->state == GateScreen_Loading || s->state == GateScreen_Opening || s->state == GateScreen_Free;
}

// The state's colour: its ink (its line), its fill (the glow, the frame,
// the dial's arc). Teal while it asks or opens, coral once it will not
// open, amber for an error or a pause.
static void accent(const Canvas *cv, GateScreenState state, Rgb *ink, Rgb *fill)
{
    const Palette *p = cv->pal;
    if (state == GateScreen_Refused || state == GateScreen_Closed || state == GateScreen_Limit) *ink = p->coral, *fill = p->coral_fill;
    else if (state == GateScreen_Error || state == GateScreen_Paused) *ink = p->amber, *fill = p->amber_fill;
    else *ink = p->teal, *fill = p->teal_fill;
}

// Where the bottom's mark goes: left of "PlayGuard", the two centred.
static float opening_mark_x(const GateScreen *s, float w)
{
    const float group = OPEN_MARK_R * 2 + 12 + measure(s->brand_font, 24, "PlayGuard");
    return w / 2 - group / 2 + OPEN_MARK_R;
}

// The badge (its padlock, its words) and the rules' icons after it,
// centred on `cx` at `y` (the icons' top).
static void badge(Canvas *cv, const GateScreen *s, float cx, float y)
{
    const Rgb teal = cv->pal->teal;
    const char *words = s->badge && *s->badge ? s->badge : "Protected by PlayGuard";
    const unsigned char *font = covers(s->brand_font, words) ? s->brand_font : s->text_font;
    const float lock = 22, rule = 22, gap = 12;
    unsigned rules = 0;
    for (unsigned i = 0; i < 3; i++) rules += (s->rules >> i) & 1u;
    const float bw = measure(font, 22, words), extra = rules ? 18 + rules * rule + (rules - 1) * 8 : 0;
    float x = cx - (lock + gap + bw + extra) / 2;
    padlock(cv, x, y, lock, teal);
    text(cv, font, 22, x + lock + gap, y + 20, words, teal, 1, 0);
    x += lock + gap + bw + 18;
    // The rules: an open padlock, a square, a timer (view/cadran_icon.cpp's).
    void (*const DRAW[])(Canvas *, float, float, float, Rgb) = { open_padlock, stop_square, timer };
    for (unsigned i = 0; i < 3; i++) {
        if (!(s->rules & (1u << i))) continue;
        DRAW[i](cv, x, y, rule, teal);
        x += rule + 8;
    }
}

// How it went under the badge: its line in the state's ink, what to do,
// the note; each a size smaller when two lines are not enough (German's
// longer words). Kept above the mark: when they need more room, they
// start higher, up to under the badge.
static void outcome(Canvas *cv, const GateScreen *s, float cx, Rgb ink)
{
    const Palette *p = cv->pal;
    const Rgb line = s->state == GateScreen_Asking ? p->text2 : ink;
    const float ss = fitting(s->text_font, 28, 24, s->status, TEXT_MAX, 2), hs = fitting(s->text_font, 22, 20, s->hint, TEXT_MAX, 2);
    const int ls = paragraph(NULL, s->text_font, ss, 0, 0, 0, s->status, line, TEXT_MAX, ss < 28 ? 3 : 2);
    const int lh = paragraph(NULL, s->text_font, hs, 0, 0, 0, s->hint, line, TEXT_MAX, 2);
    const bool note = s->note && *s->note;
    const float step_s = ss * 1.25f, step_h = hs * 1.3f;
    const float height = (ls > 0 ? (ls - 1) * step_s : 0) + (lh > 0 ? 38 + (lh - 1) * step_h : 0) + (note ? 36 : 0);
    const float y0 = fmaxf(490, fminf(OPEN_BAR_Y + 14, 628 - height));
    paragraph(cv, s->text_font, ss, cx, y0, step_s, s->status, line, TEXT_MAX, ss < 28 ? 3 : 2);
    float y = y0 + (ls > 0 ? (ls - 1) * step_s : 0);
    if (lh > 0) {
        y += 38;
        paragraph(cv, s->text_font, hs, cx, y, step_h, s->hint, p->text2, TEXT_MAX, 2);
        y += (lh - 1) * step_h;
    }
    if (note) centred(cv, s->text_font, 20, cx, y + 36, s->note, s->note_warn ? p->amber : p->teal, TEXT_MAX);
}

// Ⓐ and Ⓑ bottom right, level with the mark: each a disc with its letter,
// then its label, from the right.
static void buttons(Canvas *cv, const GateScreen *s)
{
    const Palette *p = cv->pal;
    const char *const keys[2] = { "B", "A" }, *const labels[2] = { s->button_b, s->button_a };
    float left = (float)cv->w - 64;
    for (int i = 0; i < 2; i++) {
        left -= measure(s->text_font, 22, labels[i]);
        text(cv, s->text_font, 22, left, OPEN_FOOT_Y + 8, labels[i], p->text2, 1, 0);
        left -= 10 + 30;
        fill_circle(cv, left + 15, OPEN_FOOT_Y, 15, p->key, 1);
        text(cv, s->brand_font, 20, left + 15 - measure(s->brand_font, 20, keys[i]) / 2, OPEN_FOOT_Y + 7, keys[i], p->key_ink, 1, 0);
        left -= 28;
    }
}

static void screen_still(Canvas *cv, const GateScreen *s)
{
    const Palette *p = cv->pal;
    Rgb ink, fill;
    accent(cv, s->state, &ink, &fill);
    background(cv);
    dial(cv, fill);
    const float cx = (float)cv->w / 2;
    // The icon over its glow, in a frame: a hairline in the state's colour
    // while it asks or opens, stronger once it went otherwise (how it went at
    // a glance); else PlayGuard's mark.
    const bool settled = !opening(s) && s->state != GateScreen_Asking;
    const float ix = cx - OPEN_ICON / 2, iy = OPEN_ICON_Y;
    glow(cv, cx, iy + OPEN_ICON / 2, 330, fill, (settled ? 0.16f : 0.13f) * p->glow_k);
    fill_rrect(cv, ix - 5, iy - 5, OPEN_ICON + 10, OPEN_ICON + 10, 44, p->plate, p->plate_a);
    if (!s->icon_jpeg || !icon(cv, s->icon_jpeg, s->icon_size, ix, iy, OPEN_ICON, 40)) {
        fill_rrect(cv, ix, iy, OPEN_ICON, OPEN_ICON, 40, NAVY, 1);
        mark(cv, cx, iy + OPEN_ICON / 2, 68);
    }
    if (settled) stroke_rrect(cv, ix - 6, iy - 6, OPEN_ICON + 12, OPEN_ICON + 12, 45, 4, fill, 0.85f);
    else stroke_rrect(cv, ix - 5, iy - 5, OPEN_ICON + 10, OPEN_ICON + 10, 44, 2, fill, 0.35f);

    // Its name; the badge under it.
    centred(cv, s->text_font, 50, cx, 400, s->name, p->text, TEXT_MAX);
    badge(cv, s, cx, 428);

    if (opening(s)) {
        // The bar's track, then how it goes and the note under it.
        fill_rrect(cv, cx - OPEN_BAR_W / 2, OPEN_BAR_Y, OPEN_BAR_W, OPEN_BAR_H, OPEN_BAR_H / 2, p->bar_track, 1);
        centred(cv, s->text_font, 22, cx, OPEN_BAR_Y + 48, s->status, p->text2, TEXT_MAX);
        if (s->note && *s->note) centred(cv, s->text_font, 20, cx, OPEN_BAR_Y + 82, s->note, s->note_warn ? p->amber : p->teal, TEXT_MAX);
    } else {
        outcome(cv, s, cx, ink);
    }

    // PlayGuard, quietly at the bottom: its mark (a glint goes round it while
    // it opens, drawn by the motion).
    const float mx = opening_mark_x(s, (float)cv->w);
    mark(cv, mx, OPEN_FOOT_Y, OPEN_MARK_R);
    text(cv, s->brand_font, 24, mx + OPEN_MARK_R + 12, OPEN_FOOT_Y + 9, "PlayGuard", p->text2, 1, 0);
    if (s->button_a && *s->button_a && s->button_b && *s->button_b) buttons(cv, s);
}

static void opening_motion(Canvas *cv, const GateScreen *s)
{
    const float cx = (float)cv->w / 2;
    const float p = clampf(s->progress, 0.0f, 1.0f);
    // At least its round ends, so it shows from the start.
    if (p > 0) {
        const float fw = fmaxf(OPEN_BAR_H, OPEN_BAR_W * p);
        fill_rrect(cv, cx - OPEN_BAR_W / 2, OPEN_BAR_Y, fw, OPEN_BAR_H, OPEN_BAR_H / 2, cv->pal->teal_fill, 1);
    }
    const float turn = s->spin - floorf(s->spin);
    mark_glint(cv, opening_mark_x(s, (float)cv->w), OPEN_FOOT_Y, OPEN_MARK_R, turn);
}

static bool canvas(Canvas *cv, uint32_t *px, int w, int h, int stride, const GateScreen *s)
{
    if (!px || w <= 0 || h <= 0 || stride < w || !s) return false;
    *cv = (Canvas){ px, w, h, stride, s->light ? &LIGHT : &DARK };
    return true;
}

void gate_screen_draw_still(uint32_t *px, int w, int h, int stride, const GateScreen *s)
{
    Canvas cv;
    if (canvas(&cv, px, w, h, stride, s)) screen_still(&cv, s);
}

void gate_screen_draw_motion(uint32_t *px, int w, int h, int stride, const GateScreen *s)
{
    Canvas cv;
    if (canvas(&cv, px, w, h, stride, s) && opening(s)) opening_motion(&cv, s);
}

void gate_screen_draw(uint32_t *px, int w, int h, int stride, const GateScreen *s)
{
    gate_screen_draw_still(px, w, h, stride, s);
    gate_screen_draw_motion(px, w, h, stride, s);
}
