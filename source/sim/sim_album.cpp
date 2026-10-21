// Simulated core/album.h, and the album PLAYGUARD_SIM_FORWARDERS starts
// with: four screenshots as the console writes them (1280 x 720, their
// 320 x 180 thumbnail in their Exif data), drawn here from the made-up
// homebrew's icons so the icon's square can be seen to move along them. Two
// in Star Kart Racers the same minute, one in Island Builders, one on the
// HOME menu (no game): today twice, yesterday, nine days ago, after the
// console's clock as the album was first written.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "core/album.h"

#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "core/calendar.h"
#include "core/time_ops.h"
#include "sim/sim_games.h"
#include "sim/sim_homebrew.hpp"
#include "sim/sim_homebrew_icons.h"
#include "util/forwarder_icon.hpp"
#include "util/paths.hpp"

namespace
{

struct Shot
{
    int                  days_back, hour, minute, second;   // before the console's day
    const char*          id;     // the name's end, after its time
    u64                  app_id;
    const unsigned char* middle;
    size_t               middle_size;
    const unsigned char* sides;   // the icon left and right of it
    size_t               sides_size;
    unsigned char        top[3], bottom[3];   // the background, top to bottom
};

const Shot SHOTS[] = {
    { 0, 9, 12, 0, "57B4628D2267231D57E0FC1078C0596D", 0x0100A1B2C3D40000ULL, SIM_HB_ICON_ATLAS,
      sizeof(SIM_HB_ICON_ATLAS), SIM_HB_ICON_PAINT, sizeof(SIM_HB_ICON_PAINT), { 0x1B, 0x2A, 0x5C }, { 0x3A, 0x8F, 0xC4 } },
    { 0, 9, 12, 42, "57B4628D2267231D57E0FC1078C0596D", 0x0100A1B2C3D40000ULL, SIM_HB_ICON_PAINT,
      sizeof(SIM_HB_ICON_PAINT), SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS), { 0x4A, 0x1F, 0x6B }, { 0xD0, 0x6A, 0x8C } },
    { 1, 17, 5, 0, "0E1D3C2B4A5968778695A4B3C2D1E0F0", 0x0100A1B2C3D41000ULL, SIM_HB_ICON_NOTES,
      sizeof(SIM_HB_ICON_NOTES), SIM_HB_ICON_KEEPER, sizeof(SIM_HB_ICON_KEEPER), { 0x14, 0x4D, 0x3A }, { 0x8C, 0xC8, 0x6E } },
    { 9, 9, 15, 0, "11223344556677889900AABBCCDDEEFF", 0, SIM_HB_ICON_KEEPER, sizeof(SIM_HB_ICON_KEEPER),
      SIM_HB_ICON_NOTES, sizeof(SIM_HB_ICON_NOTES), { 0x30, 0x34, 0x3C }, { 0x8A, 0x90, 0x9C } },
};

constexpr int W = 1280, H = 720, TW = 320, TH = 180;

// `icon` (a JPEG) drawn `side` px wide at (x, y) on `rgb` (W x H), bilinear.
void draw(std::vector<unsigned char>& rgb, const unsigned char* icon, size_t size, int x, int y, int side)
{
    int w = 0, h = 0, comp = 0;
    unsigned char* px = stbi_load_from_memory(icon, (int)size, &w, &h, &comp, 3);
    if (!px) return;
    for (int ty = 0; ty < side; ty++) {
        for (int tx = 0; tx < side; tx++) {
            if (x + tx < 0 || x + tx >= W || y + ty < 0 || y + ty >= H) continue;
            const float fx = std::clamp((tx + 0.5f) * w / side - 0.5f, 0.0f, w - 1.0f);
            const float fy = std::clamp((ty + 0.5f) * h / side - 0.5f, 0.0f, h - 1.0f);
            const int x0 = (int)fx, y0 = (int)fy, x1 = std::min(x0 + 1, w - 1), y1 = std::min(y0 + 1, h - 1);
            const float ax = fx - x0, ay = fy - y0;
            for (int c = 0; c < 3; c++) {
                const float top = px[(y0 * w + x0) * 3 + c] * (1 - ax) + px[(y0 * w + x1) * 3 + c] * ax;
                const float bottom = px[(y1 * w + x0) * 3 + c] * (1 - ax) + px[(y1 * w + x1) * 3 + c] * ax;
                rgb[((size_t)(y + ty) * W + x + tx) * 3 + c] = (unsigned char)(top * (1 - ay) + bottom * ay);
            }
        }
    }
    stbi_image_free(px);
}

void append(void* ctx, void* data, int size)
{
    auto* out = static_cast<std::vector<unsigned char>*>(ctx);
    out->insert(out->end(), (unsigned char*)data, (unsigned char*)data + size);
}

std::vector<unsigned char> jpeg(const std::vector<unsigned char>& rgb, int w, int h)
{
    std::vector<unsigned char> out;
    if (!stbi_write_jpg_to_func(append, &out, w, h, 3, rgb.data(), 85)) out.clear();
    return out;
}

// The screenshot: its background, an icon large in the middle, another
// smaller at each side; then its thumbnail in its Exif data.
std::vector<unsigned char> picture(const Shot& s)
{
    std::vector<unsigned char> rgb((size_t)W * H * 3);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            for (int c = 0; c < 3; c++)
                rgb[((size_t)y * W + x) * 3 + c] = (unsigned char)(s.top[c] + (s.bottom[c] - s.top[c]) * y / (H - 1));
    draw(rgb, s.sides, s.sides_size, 40, 220, 280);
    draw(rgb, s.sides, s.sides_size, W - 40 - 280, 220, 280);
    draw(rgb, s.middle, s.middle_size, (W - 440) / 2, (H - 440) / 2, 440);
    std::vector<unsigned char> small((size_t)TW * TH * 3);
    for (int y = 0; y < TH; y++)
        for (int x = 0; x < TW; x++)
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                for (int k = 0; k < 16; k++) sum += rgb[((size_t)(y * 4 + k / 4) * W + x * 4 + k % 4) * 3 + c];
                small[((size_t)y * TW + x) * 3 + c] = (unsigned char)(sum / 16);
            }
    const std::vector<unsigned char> big = jpeg(rgb, W, H), thumb = jpeg(small, TW, TH);
    if (big.empty() || thumb.empty()) return big;
    return forwarder_icon::with_thumbnail(big, thumb);
}

}   // namespace

namespace sim_homebrew
{

// Its file name, by the console's day now: "YYYYMMDDHHMMSS00-<id>.jpg".
std::string name_of(const Shot& s)
{
    u64 posix = 0;
    LocalTime day = {};
    time_local_now(&posix, &day);
    calendar_add_days(&day, -s.days_back);
    char out[64];
    std::snprintf(out, sizeof(out), "%04d%02d%02d%02d%02d%02d00-%s.jpg", (int)day.year, (int)day.month, (int)day.day,
                  s.hour, s.minute, s.second, s.id);
    return out;
}

}   // namespace

namespace sim_homebrew
{

void seed_album(const std::string& sd_root)
{
    // Once: an album there already keeps its days.
    if (!paths::list_dirs(sd_root + "/Nintendo/Album").empty()) return;
    for (const Shot& s : SHOTS) {
        const std::string n = name_of(s);
        const std::string dir = sd_root + "/Nintendo/Album/" + n.substr(0, 4) + "/" + n.substr(4, 2) + "/" + n.substr(6, 2);
        if (!paths::ensure_dir(dir)) continue;
        const std::vector<unsigned char> b = picture(s);
        if (!b.empty()) paths::atomic_write(dir + "/" + n, std::string(b.begin(), b.end()));
    }
}

}   // namespace sim_homebrew

// The games of the album's screenshots: those of SHOTS by their time and
// number, whichever day they were written.
size_t album_shots_of_games(AlbumShot* out, size_t max)
{
    size_t n = 0;
    std::vector<std::string> files;
    const std::string root = paths::sd_root() + "/Nintendo/Album";
    for (const std::string& y : paths::list_dirs(root))
        for (const std::string& m : paths::list_dirs(root + "/" + y))
            for (const std::string& d : paths::list_dirs(root + "/" + y + "/" + m))
                for (const std::string& f : paths::list_files(root + "/" + y + "/" + m + "/" + d, ".jpg")) files.push_back(f);
    for (const std::string& f : files) {
        if (!out || n >= max || f.size() < 17 + 32) break;
        for (const Shot& s : SHOTS) {
            char time[7];
            std::snprintf(time, sizeof(time), "%02d%02d%02d", s.hour, s.minute, s.second);
            if (f.compare(8, 6, time) != 0 || f.compare(17, 32, s.id) != 0) continue;
            std::memcpy(out[n].stamp, f.data(), 16);
            out[n].stamp[16] = '\0';
            out[n++].app_id = s.app_id;
            break;
        }
    }
    return n;
}

bool album_game_name(u64 app_id, char* out, size_t out_size)
{
    if (!out || !out_size) return false;
    out[0] = '\0';
    const SimGame* g = app_id ? sim_game(app_id) : nullptr;
    if (!g || !g->name || !g->name[0]) return false;
    std::strncpy(out, g->name, out_size - 1);
    out[out_size - 1] = '\0';
    return true;
}
