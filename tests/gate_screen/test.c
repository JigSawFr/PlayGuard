// Host tests for source/core/gate_screen.c and gate_text.c: the PIN gate's
// screen drawn into a buffer as the console's framebuffer (a stride wider
// than the screen left alone), the night blue at its corners, the badge's
// teal and the name's light text where they go, the state's colour on its
// line, a long one wrapped clear of the dial, the light theme; drawn with no font, no icon or a broken one without a fault; the
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

static void draw_look(GateScreenState state, const unsigned char *icon, size_t icon_n, const unsigned char *f,
                      const char *note, int warn, unsigned rules, int light)
{
    for (size_t i = 0; i < (size_t)STRIDE * H; i++) px[i] = 0x12345678u;
    const GateScreen s = { f, f, icon, icon_n, "Star Atlas", "Status line", "A hint", state,
                           note, warn, rules, NULL, NULL, NULL, 0, 0, light };
    gate_screen_draw(px, W, H, STRIDE, &s);
}

static void draw_note(GateScreenState state, const unsigned char *icon, size_t icon_n, const unsigned char *f,
                      const char *note, int warn)
{
    draw_look(state, icon, icon_n, f, note, warn, 0, 0);
}

static void draw(GateScreenState state, const unsigned char *icon, size_t icon_n, const unsigned char *f)
{
    draw_note(state, icon, icon_n, f, NULL, 0);
}

// The rules' icons after the badge: teal there with them, more for three.
static void test_rules(void)
{
    draw_look(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, NULL, 0, 0, 0);
    const int none = count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40);
    draw_look(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, NULL, 0,
              GATE_RULE_FREE | GATE_RULE_CLOSED | GATE_RULE_LIMIT, 0);
    const int three = count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40);
    draw_look(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, NULL, 0, GATE_RULE_LIMIT, 0);
    const int one = count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40);
    CHECK(none > 300);
    CHECK(one > none + 40 && three > one + 80);
}

// Every state centred, as the opening: the icon, the name, the badge in
// the same places; how it went under them in the state's colour.
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
    CHECK(count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40) > 300);
    CHECK(count_near(560, 350, 720, 405, 0xEE, 0xF2, 0xF8, 30) > 400);
    CHECK(count_near(548, 130, 732, 314, 0x0B, 0x14, 0x24, 6) < 2000);   // not the empty mark's navy
    CHECK(count_near(0, 0, 400, H, 0xEE, 0xF2, 0xF8, 30) == 0);          // nothing on the left any more

    // Refused: its line in coral; an error in amber.
    draw(GateScreen_Refused, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font);
    CHECK(count_near(380, 495, 900, 530, 0xFF, 0x7A, 0x7A, 40) > 150);
    CHECK(count_near(560, 350, 720, 405, 0xEE, 0xF2, 0xF8, 30) > 400);   // the name where the opening has it
    draw(GateScreen_Error, NULL, 0, font);
    CHECK(count_near(380, 495, 900, 530, 0xFF, 0xB5, 0x47, 40) > 150);
    // A note under the hint: amber for the day's refusals, teal for the free hours; none, nothing there.
    CHECK(count_near(380, 574, 900, 604, 0xFF, 0xB5, 0x47, 40) == 0);
    draw_note(GateScreen_Refused, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, "Refused today: 3", 1);
    CHECK(count_near(380, 574, 900, 604, 0xFF, 0xB5, 0x47, 40) > 100);
    draw_note(GateScreen_Asking, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, "Without the PIN: Sat-Sun", 0);
    CHECK(count_near(380, 574, 900, 604, 0x2E, 0xC4, 0xA6, 40) > 100);
    draw(GateScreen_Error, NULL, 0, font);
    // No icon: PlayGuard's mark in its place (its teal ring).
    CHECK(count_near(548, 130, 732, 314, 0x2E, 0xC4, 0xA6, 30) > 2000);
    // Above PlayGuard's mark at the bottom, with its arc whole.
    CHECK(count_near(540, 640, 740, 690, 0x2E, 0xC4, 0xA6, 30) > 100);

    // A broken icon, no font: drawn all the same, without text.
    static const unsigned char broken[] = { 0xFF, 0xD8, 0xFF, 0x00, 1, 2, 3 };
    draw(GateScreen_Refused, broken, sizeof(broken), NULL);
    CHECK(count_near(380, 350, 900, 630, 0xEE, 0xF2, 0xF8, 30) == 0);
    gate_screen_draw(NULL, W, H, STRIDE, NULL);   // nothing to draw into: nothing happens
    const GateScreen empty = { 0 };
    gate_screen_draw(px, 64, 32, 64, &empty);     // a tiny buffer: clipped
}

// PlayGuard's light theme: its cool white, the text in navy, each state's
// ink darkened for it.
static void test_light(void)
{
    draw_look(GateScreen_Refused, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), font, "Refused today: 3", 1, 0, 1);
    int r, g, b;
    rgb(2, 2, &r, &g, &b);
    CHECK(r > 0xE8 && g > 0xE8 && b > 0xE8);
    rgb(2, H - 3, &r, &g, &b);
    CHECK(r > 0xD8 && b > r);
    CHECK(count_near(560, 350, 720, 405, 0x0B, 0x14, 0x24, 30) > 400);   // the name in navy
    CHECK(count_near(380, 495, 900, 530, 0xB7, 0x1C, 0x1C, 40) > 150);   // the coral ink
    CHECK(count_near(380, 574, 900, 604, 0x8A, 0x52, 0x00, 40) > 100);   // the amber ink
    CHECK(count_near(380, 424, 900, 454, 0x0A, 0x70, 0x5F, 40) > 300);   // the badge's teal ink
    // The opening the same: its bar in the teal fill, its track light.
    GateScreen s = { font, font, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), "Star Atlas", "Starting", NULL,
                     GateScreen_Opening, NULL, 0, 0, NULL, NULL, NULL, 0.5f, 0.0f, 1 };
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(432, 508, 628, 516, 0x14, 0xA3, 0x8A, 30) > 1200);
    CHECK(count_near(652, 508, 848, 516, 0xD3, 0xD9, 0xE2, 10) > 1200);
}

// A long status wraps under itself, centred, clear of the dial; the badge
// in the console's language, in the console's font when PlayGuard's lacks its letters.
static void test_wrap(void)
{
    for (size_t i = 0; i < (size_t)STRIDE * H; i++) px[i] = 0;
    const char *longer = "Code PIN non saisi : retour au menu HOME, puis encore un peu de texte pour la seconde ligne.";
    GateScreen s = { font, font, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), "Star Atlas", longer, "A hint",
                     GateScreen_Refused, NULL, 0, 0, "Protégé par PlayGuard", NULL, NULL, 0, 0, 0 };
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(330, 498, 950, 528, 0xFF, 0x7A, 0x7A, 40) > 150);   // its first line
    CHECK(count_near(330, 530, 950, 558, 0xFF, 0x7A, 0x7A, 40) > 150);   // its second one
    CHECK(count_near(945, 400, W, 640, 0xFF, 0x7A, 0x7A, 40) == 0);      // none over the dial
    CHECK(count_near(0, 400, 320, 640, 0xFF, 0x7A, 0x7A, 40) == 0);      // nor far left
    CHECK(count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40) > 300);   // the badge
    // Letters Sora has not: drawn all the same (the console's font, here Sora too).
    s.badge = "\xE5\x8F\x97 PlayGuard \xE4\xBF\x9D\xE6\x8A\xA4";
    s.status = "\xE8\xAF\xB7\xE8\xBE\x93\xE5\x85\xA5\xE5\xAE\xB6\xE9\x95\xBF\xE6\x8E\xA7\xE5\x88\xB6PIN\xE4\xBB\xA5\xE5\x90\xAF\xE5\x8A\xA8\xE3\x80\x82";
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40) > 100);   // the badge, its padlock
}

// Once refused: Ⓐ and Ⓑ bottom right, level with the mark; the icon's
// frame in the state's colour, a faint one while it asks.
static void test_buttons(void)
{
    for (size_t i = 0; i < (size_t)STRIDE * H; i++) px[i] = 0;
    GateScreen s = { font, font, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), "Star Atlas", "Refused", "A hint",
                     GateScreen_Refused, NULL, 0, 0, NULL, NULL, NULL, 0, 0, 0 };
    gate_screen_draw(px, W, H, STRIDE, &s);
    const int none = count_near(860, 645, W - 60, 685, 0xEE, 0xF2, 0xF8, 20);
    const int frame = count_near(536, 140, 548, 300, 0xFF, 0x7A, 0x7A, 60);
    s.button_a = "Try again";
    s.button_b = "HOME menu";
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(none < 20);
    CHECK(count_near(860, 645, W - 60, 685, 0xEE, 0xF2, 0xF8, 20) > 800);   // two light discs
    CHECK(frame > 300);
    s.state = GateScreen_Asking;
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(536, 140, 548, 300, 0xFF, 0x7A, 0x7A, 60) == 0);
}

// Once it starts: the bar filled as far as `progress` says, drawn by the
// motion alone over the still.
static void test_opening(void)
{
    for (size_t i = 0; i < (size_t)STRIDE * H; i++) px[i] = 0x12345678u;
    GateScreen s = { font, font, SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), "Star Atlas", "Starting", NULL,
                     GateScreen_Opening, "45 min left", 0, GATE_RULE_FREE, NULL, NULL, NULL, 0.5f, 0.25f, 0 };
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(px[W] == 0x12345678u);                                          // the stride's margin left alone
    CHECK(count_near(560, 350, 720, 405, 0xEE, 0xF2, 0xF8, 30) > 400);   // the name, centred
    CHECK(count_near(0, 0, 400, H, 0xEE, 0xF2, 0xF8, 30) == 0);          // no text on the left
    CHECK(count_near(380, 424, 900, 454, 0x2E, 0xC4, 0xA6, 40) > 200);   // the badge
    CHECK(count_near(960, 100, W, 600, 0x2E, 0xC4, 0xA6, 40) == 0);      // no text over the dial
    CHECK(count_near(1000, 100, W, 600, 0x1F, 0x33, 0x55, 30) > 2000);   // the dial's track, behind
    const int half = count_near(432, 508, 628, 516, 0x2E, 0xC4, 0xA6, 30);
    CHECK(half > 1200);                                                   // the bar's first half filled
    CHECK(count_near(652, 508, 848, 516, 0x2E, 0xC4, 0xA6, 30) == 0);    // its second not yet
    const size_t bytes = sizeof(uint32_t) * STRIDE * H;
    uint32_t *whole = (uint32_t *)malloc(bytes);
    memcpy(whole, px, bytes);

    // The still: the bar's track alone; with the motion over it, the same as the whole.
    gate_screen_draw_still(px, W, H, STRIDE, &s);
    CHECK(count_near(432, 508, 628, 516, 0x2E, 0xC4, 0xA6, 30) == 0);
    gate_screen_draw_motion(px, W, H, STRIDE, &s);
    CHECK(memcmp(px, whole, bytes) == 0);

    // Before the PIN screen the same screen; full at the end; free hours the
    // same screen; no motion in the other states, no bar either.
    s.state = GateScreen_Loading;
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(432, 508, 628, 516, 0x2E, 0xC4, 0xA6, 30) == half);
    CHECK(count_near(560, 350, 720, 405, 0xEE, 0xF2, 0xF8, 30) > 400);
    s.progress = 1.0f;
    s.state = GateScreen_Free;
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(652, 508, 848, 516, 0x2E, 0xC4, 0xA6, 30) > 1200);
    s.state = GateScreen_Refused;
    gate_screen_draw_still(px, W, H, STRIDE, &s);
    memcpy(whole, px, bytes);
    gate_screen_draw_motion(px, W, H, STRIDE, &s);
    CHECK(memcmp(px, whole, bytes) == 0);
    CHECK(count_near(430, 506, 850, 518, 0x1F, 0x33, 0x55, 10) < 200);   // no bar's track
    CHECK(count_near(560, 350, 720, 405, 0xEE, 0xF2, 0xF8, 30) > 400);   // the same name, centred
    free(whole);

    // No icon: PlayGuard's mark in its place; no font: no text, no fault.
    s.state = GateScreen_Opening;
    s.icon_jpeg = NULL;
    s.brand_font = s.text_font = NULL;
    gate_screen_draw(px, W, H, STRIDE, &s);
    CHECK(count_near(548, 130, 732, 314, 0x2E, 0xC4, 0xA6, 30) > 2000);
    CHECK(count_near(400, 350, 880, 620, 0xEE, 0xF2, 0xF8, 30) == 0);
}

static void test_text(void)
{
    for (int lang = -1; lang <= 18; lang++)
        for (int t = 0; t < GateText_Count; t++) CHECK(gate_text(lang, (GateText)t)[0] != '\0');
    CHECK(strcmp(gate_text(2, GateText_Refused), "Code PIN non saisi : retour au menu HOME.") == 0);
    CHECK(strcmp(gate_text(2, GateText_Badge), "Protégé par PlayGuard") == 0);
    CHECK(strcmp(gate_text(1, GateText_Badge), "Protected by PlayGuard") == 0);
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
    test_rules();
    test_wrap();
    test_buttons();
    test_light();
    test_opening();
    test_text();
    free(px);
    free(font);
    return CHECK_DONE("gate_screen layout, colours, missing pieces and lines in every language assertions passed");
}
