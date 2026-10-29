// palette — PlayGuard's own theme keys (playguard/…): the colours its own
// views draw with that none of Aurore's or Cadran's keys gives, as plain
// values (no view, no nanovg). ui::install_palette() adds them to both
// variants before the theme file is read, so a theme file can restyle them
// like any other key (docs/themes.md); a host suite checks the pairs below
// in both variants (tests/contrast), Theme::checkContrast those of a theme
// file. Everything else PlayGuard draws takes Cadran's keys. None of these
// keys is a surface (a card, a panel): a theme's surface_opacity has nothing
// of PlayGuard's own to make translucent (Theme::addSurfaceKey), its cards
// are Cadran's (cadran/surf, surf2).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_palette.hpp>

namespace playguard::palette
{
using cadran::palette::Contrast;
using cadran::palette::Rgba;

struct Key
{
    const char* name;
    Rgba dark, light;
};

constexpr Key KEYS[] = {
    // The loading screen's dial (view/loading_dial), drawn as the PIN gate
    // draws it (core/gate_screen.c): its teal glow, the day's arc, the arc's
    // track, and the rings and minutes' lines, given at full strength (a
    // dark line, on a light background, drawn twice as strong as a light
    // one).
    { "playguard/loading/glow", { 0x2E, 0xC4, 0xA6, 23 }, { 0x2E, 0xC4, 0xA6, 23 } },
    { "playguard/loading/arc", { 0x2E, 0xC4, 0xA6, 41 }, { 0x2E, 0xC4, 0xA6, 41 } },
    { "playguard/loading/track", { 0x1F, 0x33, 0x55, 71 }, { 0x1F, 0x33, 0x55, 30 } },
    { "playguard/loading/line", { 0xFF, 0xFF, 0xFF, 255 }, { 0x0B, 0x14, 0x24, 255 } },
    // The play timer's ring: the point of light at its arc's end.
    { "playguard/ring/tip", { 0xFF, 0xFF, 0xFF, 255 }, { 0xFF, 0xFF, 0xFF, 255 } },
    // The "!" on a cover's coral disc (a game that stays, view/cover_strip).
    { "playguard/badge/ink", cadran::palette::NAVY, { 0xFF, 0xFF, 0xFF, 255 } },
};

// PlayGuard's pictures a theme may replace (`image.<key> = file.png`,
// Aurore's image slots, docs/themes.md): the key, PlayGuard's own picture
// (a resource), the largest side it is made at. install_palette() declares
// them before the theme file is read.
struct ImageSlot
{
    const char* key;
    const char* fallback;
    int max_side;
};

constexpr ImageSlot IMAGE_SLOTS[] = {
    // The loading screen's picture in the middle (view/loading_mark): the
    // app's icon, drawn 168 units square.
    { "playguard/loading/art", "@res/img/app_icon.jpg", 512 },
    // The name under it (activity/loading_activity), 360 x 88 units: the
    // wordmark, in light ink (wordmark-light.png, the light theme's, is
    // PlayGuard's own choice when the theme gives none).
    { "playguard/loading/wordmark", "@res/img/wordmark.png", 720 },
};

// The pairs PlayGuard's keys keep readable (an ink on what it is drawn on):
// the badge's mark is a shape, 3:1 (WCAG 1.4.11). The others are light
// around what says something, not what says it.
constexpr Contrast CONTRAST[] = {
    { "playguard/badge/ink", "cadran/coral", 3.0f },
};
}   // namespace playguard::palette
