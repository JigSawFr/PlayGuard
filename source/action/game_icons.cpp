// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_icons.hpp"

#include <borealis.hpp>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <stb_image.h>

#include "action/icon_logic.hpp"

namespace game_icons
{
namespace
{
// Decoded icons kept for the run, at most this many: past it, a cell decodes
// its own and frees it (about 80 KB of texture each).
constexpr size_t TEXTURES_KEPT = 48;

// Icons' bytes kept, for this many games at most (up to 128 KiB each): one
// pushed out is read again from the console when its cell needs it.
constexpr size_t ICON_BYTES_KEPT = 64;

// An icon decoded and shrunk to ICON_PX, ready to become a texture.
struct Pixels
{
    int w = 0, h = 0;
    std::vector<unsigned char> rgba;
};

struct
{
    bool checked = false;
    bool wanted = true;
    icon_logic::Lru icons{ ICON_BYTES_KEPT };
    std::map<u64, int> textures;   // decoded once, small (NVG images), for the run
    std::map<u64, Pixels> pixels;  // decoded off the UI thread, not a texture yet
    std::set<u64> no_icon;         // games known to have none (icons do not change)
    bool busy = false;
    std::set<u64> queued;          // asked while a read ran
    std::function<void()> done;
} s;

// An icon decoded and shrunk to ICON_PX at most (empty when it cannot be
// decoded): what nvgCreateImageMem does, before it reaches the GPU. Plain
// CPU work, done off the UI thread with the read (a 256 x 256 JPEG takes a
// few ms on the console: 42 of them at once held a frame for 200 ms).
Pixels decode(const unsigned char* bytes, size_t size)
{
    Pixels out;
    int w = 0, h = 0, comp = 0;
    unsigned char* rgba = stbi_load_from_memory(bytes, (int)size, &w, &h, &comp, 4);
    if (!rgba) return out;
    const auto fit = icon_logic::fit(w, h, ICON_PX);
    if (fit.first == w && fit.second == h) out.rgba.assign(rgba, rgba + (size_t)w * h * 4);
    else out.rgba = icon_logic::shrink(rgba, w, h, fit.first, fit.second);
    stbi_image_free(rgba);
    if (!out.rgba.empty()) {
        out.w = fit.first;
        out.h = fit.second;
    }
    return out;
}

// The texture of decoded pixels (0 when there are none): UI thread only.
int upload(const Pixels& p)
{
    if (p.rgba.empty()) return 0;
    return nvgCreateImageRGBA(brls::Application::getNVGContext(), p.w, p.h, 0, p.rgba.data());
}

void read_queued()
{
    if (s.busy || s.queued.empty()) return;
    auto wanted = std::make_shared<std::vector<PlayIcon>>();
    for (u64 id : s.queued) wanted->push_back(PlayIcon{ id, nullptr, 0 });
    s.queued.clear();
    s.busy = true;
    brls::async([wanted]() {
        playstats_icons(wanted->data(), wanted->size());
        auto decoded = std::make_shared<std::vector<Pixels>>();
        for (const PlayIcon& icon : *wanted) {
            if (playstats_cancelled()) break;
            decoded->push_back(icon.jpeg && icon.size ? decode(icon.jpeg, icon.size) : Pixels{});
        }
        if (playstats_cancelled()) {   // quitting: nobody to show them
            for (PlayIcon& icon : *wanted) std::free(icon.jpeg);
            return;
        }
        brls::sync([wanted, decoded]() {
            s.busy = false;
            for (size_t i = 0; i < wanted->size(); i++) {
                PlayIcon& icon = (*wanted)[i];
                if (icon.jpeg && icon.size) {
                    s.icons.put(icon.app_id, std::vector<unsigned char>(icon.jpeg, icon.jpeg + icon.size));
                    // Kept until it becomes a texture; a bound for a list
                    // scrolled past before its icons are shown.
                    if (!(*decoded)[i].rgba.empty() && !s.textures.count(icon.app_id)) {
                        if (s.pixels.size() >= ICON_BYTES_KEPT) s.pixels.erase(s.pixels.begin());
                        s.pixels[icon.app_id] = std::move((*decoded)[i]);
                    }
                } else {
                    s.no_icon.insert(icon.app_id);
                }
                std::free(icon.jpeg);
                icon.jpeg = nullptr;
            }
            if (s.done) s.done();
            read_queued();
        });
    });
}
}   // namespace

bool wanted()
{
    if (!s.checked) {
        SysInfo si;
        sysinfo_get(&si);
        s.wanted  = !si.applet_mode;
        s.checked = true;
    }
    return s.wanted;
}

const std::vector<unsigned char>* bytes(u64 app_id)
{
    return s.icons.get(app_id);
}

int texture(u64 app_id, bool& owned)
{
    owned = false;
    auto tex = s.textures.find(app_id);
    if (tex != s.textures.end()) {
        s.icons.get(app_id);   // kept as recently used
        return tex->second;
    }
    const std::vector<unsigned char>* b = s.icons.get(app_id);
    if (!b || b->empty()) return 0;
    // Decoded with the read; else (its pixels dropped since) here.
    auto ready = s.pixels.find(app_id);
    const int t = ready != s.pixels.end() ? upload(ready->second) : upload(decode(b->data(), b->size()));
    if (t && s.textures.size() < TEXTURES_KEPT) {
        s.textures.emplace(app_id, t);
        if (ready != s.pixels.end()) s.pixels.erase(ready);
        return t;
    }
    // A texture the cell frees: its pixels stay for the next one.
    owned = t != 0;
    return t;
}

bool known(u64 app_id)
{
    return s.textures.count(app_id) || s.icons.contains(app_id) || s.no_icon.count(app_id);
}

void load(const std::vector<u64>& ids, std::function<void()> done)
{
    if (!wanted()) return;
    s.done = std::move(done);
    for (u64 id : ids)
        if (!known(id)) s.queued.insert(id);
    read_queued();
}

}   // namespace game_icons
