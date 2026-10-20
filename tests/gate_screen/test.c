// Host tests for source/core/gate_screen.c and gate_text.c: the PIN gate's
// screen drawn into a buffer as the console's framebuffer (a stride wider
// than the screen left alone), the night blue at its corners, the badge's
// teal and the name's light text where they go, the state's colour on its
// line; drawn with no font, no icon or a broken one without a fault; the
// lines there in every language libnx names, the console's font for each.
// The look itself: make gate-preview.
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gate_screen.h"
#include "gate_text.h"
#include "../../source/sim/sim_homebrew_icons.h"

#define W GATE_SCREEN_W
#define H GATE_SCREEN_H
#define STRIDE (W + 16)

static unsigned char *font;
static uint32_t *px;

static void rgb(int x, int y, int *r, int *g, int *b)
{
    const uint32_t p = px[(size_t)y * STRIDE + (size_t)x];
    *r = (int)(p & 0xFF), *g = (int)(p >> 8 & 0xFF), *b = (int)(p >> 16 & 0xFF);
}

// Pixels in the box that are close to (r, g, b).
static int count_near(int x0, int y0, int x1, int y1, int r, int g, int b, int tol)
{
    int n = 0;
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            int pr, pg, pb;
            rgb(x, y, &pr, &pg, &pb);
            if (abs(pr - r) <= tol && abs(pg - g) <= tol && abs(pb - b) <= tol) n++;
        }
    return n;
}

static void draw_note(GateScreenState state, const unsigned char *icon, size_t icon_n, const unsigned char *f,
                      const char *note, int warn)
{
    for (size_t i = 0; i < (size_t)STRIDE * H; i++) px[i] = 0x12345678u;
    const GateScreen s = { f, f, icon, icon_n, "Star Atlas", "Status line", "A hint", "PlayGuard · tagline", "21:03", state,
                           note, warn };
    gate_screen_draw(px, W, H, STRIDE, &s);
}

static void draw(GateScreenState state, const unsigned char *icon, size_t icon_n, const unsigned char *f)
{
    draw_note(state, icon, icon_n, f, NULL, 0);
}

static void test_screen(void)
{
    draw(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font);
    // The night blue in the corners; the stride's margin left as it was.
    int r, g, b;
    rgb(2, 2, &r, &g, &b);
    CHECK(r < 0x20 && g < 0x28 && b < 0x40 && b > r);
    rgb(2, H - 3, &r, &g, &b);
    CHECK(r < 0x14 && g < 0x1C && b < 0x2C);
    CHECK(px[W] == 0x12345678u && px[(size_t)(H - 1) * STRIDE + W + 15] == 0x12345678u);
    // The badge's teal, the name's light text, the icon's own colours.
    CHECK(count_near(470, 335, 720, 366, 0x2E, 0xC4, 0xA6, 40) > 300);
    CHECK(count_near(400, 245, 640, 295, 0xEE, 0xF2, 0xF8, 30) > 400);
    CHECK(count_near(112, 214, 344, 446, 0x0B, 0x14, 0x24, 6) < 2000);   // not the empty mark's navy

    // Refused: its line in coral; an error in amber.
    draw(GateScreen_Refused, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font);
    CHECK(count_near(400, 420, 700, 460, 0xFF, 0x7A, 0x7A, 40) > 150);
    draw(GateScreen_Error, NULL, 0, font);
    CHECK(count_near(400, 420, 700, 460, 0xFF, 0xB5, 0x47, 40) > 150);
    // A note under the hint: amber for the day's refusals, teal for the free hours; none, nothing there.
    CHECK(count_near(400, 525, 800, 556, 0xFF, 0xB5, 0x47, 40) == 0);
    draw_note(GateScreen_Refused, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, "Refused today: 3", 1);
    CHECK(count_near(400, 525, 800, 556, 0xFF, 0xB5, 0x47, 40) > 100);
    draw_note(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, "Without the PIN: Sat-Sun", 0);
    CHECK(count_near(400, 525, 800, 556, 0x2E, 0xC4, 0xA6, 40) > 100);
    draw(GateScreen_Error, NULL, 0, font);
    // No icon: PlayGuard's mark in its place (its teal ring).
    CHECK(count_near(112, 214, 344, 446, 0x2E, 0xC4, 0xA6, 30) > 2000);

    // A broken icon, no font: drawn all the same, without text.
    static const unsigned char broken[] = { 0xFF, 0xD8, 0xFF, 0x00, 1, 2, 3 };
    draw(GateScreen_Opening, broken, sizeof(broken), NULL);
    CHECK(count_near(400, 245, 640, 295, 0xEE, 0xF2, 0xF8, 30) == 0);
    gate_screen_draw(NULL, W, H, STRIDE, NULL);   // nothing to draw into: nothing happens
    const GateScreen empty = { 0 };
    gate_screen_draw(px, 64, 32, 64, &empty);     // a tiny buffer: clipped
}

static void test_text(void)
{
    for (int lang = -1; lang <= 18; lang++)
        for (int t = 0; t < GateText_Count; t++) CHECK(gate_text(lang, (GateText)t)[0] != '\0');
    CHECK(strcmp(gate_text(2, GateText_Refused), "Code PIN non saisi : retour au menu HOME.") == 0);
    CHECK(strstr(gate_text(13, GateText_Asking), "NIP") != NULL);           // Canadian French
    CHECK(strcmp(gate_text(99, GateText_Asking), gate_text(1, GateText_Asking)) == 0);
    CHECK(gate_text(1, GateText_Count)[0] == '\0');
    CHECK(gate_text_font(0) == GateFont_Standard && gate_text_font(7) == GateFont_Korean);
    CHECK(gate_text_font(6) == GateFont_ChineseSimplified && gate_text_font(15) == GateFont_ChineseSimplified);
    CHECK(gate_text_font(11) == GateFont_ChineseTraditional && gate_text_font(16) == GateFont_ChineseTraditional);
}

int main(void)
{
    FILE *f = fopen("resources/font/sora_bold_latin.ttf", "rb");
    if (!CHECK(f)) return CHECK_DONE("");
    fseek(f, 0, SEEK_END);
    const long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    font = (unsigned char *)malloc((size_t)n);
    CHECK(fread(font, 1, (size_t)n, f) == (size_t)n);
    fclose(f);
    px = (uint32_t *)malloc(sizeof(uint32_t) * STRIDE * H);
    test_screen();
    test_text();
    free(px);
    free(font);
    return CHECK_DONE("gate_screen layout, colours, missing pieces and lines in every language assertions passed");
}
