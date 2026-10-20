// nro — what a homebrew's .nro says of itself, for its forwarder (core/
// forwarder.h): its icon and control data (the asset section hbmenu reads:
// "ASET" after the program, its icon a JPEG, its NACP), and its name, author
// and version from them. And the .nro files of the SD card's /switch folder,
// the homebrew a forwarder can be made for. Plain C++ (no libnx, no UI) for
// the host tests (tests/nro).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nro
{

constexpr size_t NACP_SIZE = 0x4000;
constexpr size_t ICON_MAX  = 0x20000;   // 128 KiB: the HOME menu's icons are far smaller
constexpr int    ICON_PX   = 256;       // the HOME menu wants a 256 x 256 JPEG

struct Info
{
    std::vector<uint8_t> icon;   // JPEG, as the .nro holds it; empty when it has none
    std::vector<uint8_t> nacp;   // NACP_SIZE bytes; empty when it has none
    std::string name, author, version;   // from the NACP's first entry with a name
};

// The .nro's assets from its bytes. False when it is not an .nro (no "NRO0",
// a size beyond the data); a .nro without an asset section is one, with
// nothing in `out`.
bool parse(const std::vector<uint8_t>& data, Info& out);

// The same from a file: only its header and its asset section are read.
bool read(const std::string& path, Info& out);

// A JPEG's size, from its frame header; false when it is not a JPEG.
bool jpeg_size(const std::vector<uint8_t>& jpeg, int& width, int& height);

// The icon is one the HOME menu draws: a 256 x 256 JPEG, at most ICON_MAX.
bool icon_usable(const std::vector<uint8_t>& icon);

// The .nro files of `root`'s /switch folder and of its sub-folders one level
// down (/switch/x.nro, /switch/x/x.nro), as "/switch/…" paths, sorted. The
// hidden folders (.overlays …) left out.
std::vector<std::string> find(const std::string& root);

// The path hbmenu passes to a .nro ("sdmc:/switch/x.nro") for one under the
// SD card's root ("/switch/x.nro"), and back ("" when it is not one).
std::string sdmc_path(const std::string& sd_path);
std::string sd_path(const std::string& sdmc);

}   // namespace nro
