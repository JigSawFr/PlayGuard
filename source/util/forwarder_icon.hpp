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

// A screenshot (1280 x 720, any size) as an icon: its middle square,
// scaled to 256 x 256, as a JPEG. Empty when it cannot be decoded.
std::vector<uint8_t> from_screenshot(const std::vector<uint8_t>& image);

// The icon of a homebrew that has none: `png` (an emoji of img/emoji/)
// drawn large in the middle of a pale square. Empty when `png` cannot be
// decoded.
std::vector<uint8_t> placeholder(const std::vector<uint8_t>& png);

}   // namespace forwarder_icon
