// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/nro.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "util/text.hpp"
#include "util/utf8.hpp"

namespace nro
{

namespace
{

// The .nro's layout (switchbrew NRO): its header at 0x10 ("NRO0", its size
// at 0x18), then the asset section at that size: "ASET", a version, and the
// icon, the NACP and the RomFS as (offset, size) from the section's start.
constexpr size_t HEADER_END = 0x80;
constexpr size_t ASSET_SIZE = 0x38;

uint32_t u32_at(const uint8_t* p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

uint64_t u64_at(const uint8_t* p)
{
    return (uint64_t)u32_at(p) | (uint64_t)u32_at(p + 4) << 32;
}

bool valid_utf8(const std::string& s)
{
    uint32_t cp = 0;
    for (size_t i = 0, n = 0; i < s.size(); i += n)
        if ((n = utf8::decode(s, i, &cp)) == 0) return false;
    return true;
}

// A NACP text field: up to its terminator or its end, valid UTF-8 only.
std::string field(const uint8_t* p, size_t size)
{
    size_t n = 0;
    while (n < size && p[n]) n++;
    const std::string s(reinterpret_cast<const char*>(p), n);
    return valid_utf8(s) ? text::trim_space(s) : std::string();
}

void names_from_nacp(Info& out)
{
    if (out.nacp.size() < NACP_SIZE) return;
    const uint8_t* n = out.nacp.data();
    // A 21.0.0+ compressed title block is not one an .nro carries: its
    // entries are read as they are, and come out empty if it were.
    for (int lang = 0; lang < 16; lang++) {
        const uint8_t* e = n + lang * 0x300;
        const std::string name = field(e, 0x200);
        if (name.empty()) continue;
        out.name   = name;
        out.author = field(e + 0x200, 0x100);
        break;
    }
    out.version = field(n + 0x3060, 0x10);
}

// The asset section's icon and NACP, from its `size` bytes read from its
// start.
bool assets(const uint8_t* section, size_t size, Info& out)
{
    if (size < ASSET_SIZE || std::memcmp(section, "ASET", 4) != 0) return true;   // none: still an .nro
    const uint64_t icon_off = u64_at(section + 8), icon_size = u64_at(section + 0x10);
    const uint64_t nacp_off = u64_at(section + 0x18), nacp_size = u64_at(section + 0x20);
    if (icon_size && icon_size <= ICON_MAX && icon_off <= size && icon_size <= size - icon_off)
        out.icon.assign(section + icon_off, section + icon_off + icon_size);
    if (nacp_size >= NACP_SIZE && nacp_off <= size && NACP_SIZE <= size - nacp_off) {
        out.nacp.assign(section + nacp_off, section + nacp_off + NACP_SIZE);
        names_from_nacp(out);
    }
    return true;
}

}   // namespace

bool parse(const std::vector<uint8_t>& data, Info& out)
{
    out = Info();
    if (data.size() < HEADER_END || std::memcmp(data.data() + 0x10, "NRO0", 4) != 0) return false;
    const uint32_t size = u32_at(data.data() + 0x18);
    if (size < HEADER_END || size > data.size()) return false;
    return assets(data.data() + size, data.size() - size, out);
}

bool read(const std::string& path, Info& out)
{
    out = Info();
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    uint8_t header[HEADER_END];
    bool ok = std::fread(header, 1, sizeof(header), f) == sizeof(header) && std::memcmp(header + 0x10, "NRO0", 4) == 0;
    const uint32_t size = ok ? u32_at(header + 0x18) : 0;
    ok = ok && size >= HEADER_END && std::fseek(f, size, SEEK_SET) == 0;
    if (ok) {
        // The asset header, then as far as its icon and NACP go (an .nro's
        // RomFS, after them, is not read).
        uint8_t asset[ASSET_SIZE];
        const size_t got = std::fread(asset, 1, sizeof(asset), f);
        if (got == sizeof(asset) && std::memcmp(asset, "ASET", 4) == 0) {
            const uint64_t end = std::max(u64_at(asset + 8) + u64_at(asset + 0x10), u64_at(asset + 0x18) + u64_at(asset + 0x20));
            if (end <= ASSET_SIZE + ICON_MAX + NACP_SIZE + 0x1000) {
                std::vector<uint8_t> section(asset, asset + sizeof(asset));
                section.resize((size_t)std::max<uint64_t>(end, ASSET_SIZE));
                const size_t more = section.size() - ASSET_SIZE;
                section.resize(ASSET_SIZE + std::fread(section.data() + ASSET_SIZE, 1, more, f));
                assets(section.data(), section.size(), out);
            }
        }
    }
    std::fclose(f);
    return ok;
}

bool jpeg_size(const std::vector<uint8_t>& jpeg, int& width, int& height)
{
    width = height = 0;
    if (jpeg.size() < 4 || jpeg[0] != 0xFF || jpeg[1] != 0xD8) return false;
    size_t i = 2;
    while (i + 4 <= jpeg.size()) {
        if (jpeg[i] != 0xFF) return false;
        const uint8_t marker = jpeg[i + 1];
        if (marker == 0xFF) {   // fill bytes
            i++;
            continue;
        }
        if (marker == 0xD8 || (marker >= 0xD0 && marker <= 0xD7) || marker == 0x01) {
            i += 2;
            continue;
        }
        const size_t len = (size_t)jpeg[i + 2] << 8 | jpeg[i + 3];
        if (len < 2 || i + 2 + len > jpeg.size()) return false;
        // A frame header (SOF0 to SOF15, but DHT, JPG and DAC): its height,
        // then its width.
        if (marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
            if (len < 7) return false;
            height = jpeg[i + 5] << 8 | jpeg[i + 6];
            width  = jpeg[i + 7] << 8 | jpeg[i + 8];
            return width > 0 && height > 0;
        }
        if (marker == 0xDA || marker == 0xD9) return false;   // the image data before any frame
        i += 2 + len;
    }
    return false;
}

bool jpeg_plain(const std::vector<uint8_t>& jpeg)
{
    if (jpeg.size() < 4 || jpeg[0] != 0xFF || jpeg[1] != 0xD8) return false;
    bool frame = false;
    for (size_t i = 2; i + 4 <= jpeg.size();) {
        if (jpeg[i] != 0xFF) return false;
        const uint8_t marker = jpeg[i + 1];
        if (marker == 0xFF) {
            i++;
            continue;
        }
        if (marker == 0xDA) return frame;   // the image data: all seen
        const size_t len = (size_t)jpeg[i + 2] << 8 | jpeg[i + 3];
        if (len < 2 || i + 2 + len > jpeg.size()) return false;
        // JFIF, the tables, a comment, and a baseline or extended frame:
        // nothing else (no EXIF, no progressive or arithmetic coding).
        if (marker == 0xC0 || marker == 0xC1) frame = true;
        else if (marker != 0xE0 && marker != 0xDB && marker != 0xC4 && marker != 0xDD && marker != 0xFE) return false;
        i += 2 + len;
    }
    return false;
}

bool icon_usable(const std::vector<uint8_t>& icon)
{
    int w = 0, h = 0;
    return !icon.empty() && icon.size() <= ICON_MAX && jpeg_size(icon, w, h) && w == ICON_PX && h == ICON_PX &&
           jpeg_plain(icon);
}

std::vector<std::string> find(const std::string& root)
{
    std::vector<std::string> out;
    const std::string base = root == "/" ? std::string() : root;
    const auto is_nro = [](const std::string& name) {
        return name.size() > 4 && text::lower_ascii(name.substr(name.size() - 4)) == ".nro";
    };
    const auto is_dir = [](const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    };
    const auto scan = [&](const std::string& rel, bool descend) {
        DIR* d = opendir((base + rel).c_str());
        if (!d) return std::vector<std::string>();
        std::vector<std::string> dirs;
        while (const dirent* e = readdir(d)) {
            const std::string name = e->d_name;
            if (name.empty() || name[0] == '.') continue;
            const std::string path = rel + "/" + name;
            if (is_nro(name) && !is_dir(base + path)) out.push_back(path);
            else if (descend && is_dir(base + path)) dirs.push_back(path);
        }
        closedir(d);
        return dirs;
    };
    for (const std::string& dir : scan("/switch", true)) scan(dir, false);
    std::sort(out.begin(), out.end(), [](const std::string& a, const std::string& b) {
        const std::string la = text::lower_ascii(a), lb = text::lower_ascii(b);
        return la != lb ? la < lb : a < b;
    });
    return out;
}

std::string sdmc_path(const std::string& sd_path)
{
    return sd_path.empty() || sd_path[0] != '/' ? std::string() : "sdmc:" + sd_path;
}

std::string sd_path(const std::string& sdmc)
{
    return text::starts_with(sdmc, "sdmc:/") ? sdmc.substr(5) : std::string();
}

}   // namespace nro
