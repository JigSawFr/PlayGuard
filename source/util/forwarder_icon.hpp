// forwarder_icon — the icon of an NRO forwarder: the HOME menu draws a
// 256 x 256 JPEG, and an .nro may carry another size, a PNG, or none. As
// sphaira does, an icon that is not one is decoded, scaled to 256 x 256 and
// written as a JPEG; none at all, a stand-in is drawn. Images read by
// borealis' stb_image (nanovg), written by extern/stb/stb_image_write. Plain
// C++ (no UI) for the host tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <vector>

namespace forwarder_icon
{

// `image` as the HOME menu can draw it: as it is when it already is a 256 x
// 256 JPEG (util/nro.hpp: icon_usable), else made one. Empty when it cannot
// be decoded.
std::vector<uint8_t> normalize(const std::vector<uint8_t>& image);

// RGBA pixels (`w` x `h`) scaled to 256 x 256 (averaged down, interpolated
// up; transparency over white), as a JPEG. Empty when it cannot be written.
std::vector<uint8_t> jpeg_from_rgba(const uint8_t* rgba, int w, int h);

// The square of an image an icon keeps, as RGBA pixels `px` x `px`: as wide
// as its shorter side, at `x` along the longer one (0 its start, 100 its end,
// 50 its middle). Empty when it cannot be decoded.
std::vector<uint8_t> square_of(const std::vector<uint8_t>& image, int x, int px);
// The same of RGBA pixels already decoded (`w` x `h`).
std::vector<uint8_t> square_rgba(const uint8_t* rgba, int w, int h, int x, int px);
// An image decoded as RGBA pixels, its size in `w` and `h`; empty (0 x 0)
// when it cannot be.
std::vector<uint8_t> rgba_of(const std::vector<uint8_t>& image, int& w, int& h);

// A screenshot (1280 x 720, any size) as an icon: its square at `x`
// (square_of), scaled to 256 x 256, as a JPEG. Empty when it cannot be
// decoded.
std::vector<uint8_t> from_screenshot(const std::vector<uint8_t>& image, int x = 50);

// The small copy of a picture a JPEG carries in its Exif data (the album's
// screenshots: 320 x 180), as the JPEG it is; empty when it has none. Read
// in place of the picture where a small one will do: it decodes in a
// fraction of the time.
std::vector<uint8_t> exif_thumbnail(const std::vector<uint8_t>& jpeg);

// `jpeg` with `thumbnail` as its Exif one (an APP1 segment after its start),
// as the console writes its screenshots: the desktop simulator's album and
// the host tests.
std::vector<uint8_t> with_thumbnail(const std::vector<uint8_t>& jpeg, const std::vector<uint8_t>& thumbnail);

// `over` (RGBA, `w` x `h`, scaled to `px`) drawn over `rgba` (RGBA, `px` x
// `px`) as its transparency says: the badge of a protected forwarder over
// the icon the icon sheet shows. Nothing for a bad size.
void draw_over(std::vector<uint8_t>& rgba, int px, const uint8_t* over, int w, int h);

// `icon` (an image, as the HOME menu's 256 x 256 JPEG) with `badge` (a PNG,
// transparent but for itself: forwarder_library::BADGE) drawn over it, as a
// JPEG; `icon` as it is when either cannot be decoded.
std::vector<uint8_t> with_badge(const std::vector<uint8_t>& icon, const std::vector<uint8_t>& badge);

// The icon of a homebrew that has none: `png` (an emoji of img/emoji/)
// drawn large in the middle of a pale square. Empty when `png` cannot be
// decoded.
std::vector<uint8_t> placeholder(const std::vector<uint8_t>& png);

}   // namespace forwarder_icon
