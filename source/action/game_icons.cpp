// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_icons.hpp"

#include <borealis.hpp>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>

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

struct
{
    bool checked = false;
    bool wanted = true;
    icon_logic::Lru icons{ ICON_BYTES_KEPT };
    std::map<u64, int> textures;   // decoded once, small (NVG images), for the run
    std::set<u64> no_icon;         // games known to have none (icons do not change)
    bool busy = false;
    std::set<u64> queued;          // asked while a read ran
    std::function<void()> done;
} s;

// An icon as a texture no larger than ICON_PX (0 when it cannot be decoded):
// what nvgCreateImageMem does, shrunk before it reaches the GPU.
int decode(const std::vector<unsigned char>& bytes)
{
    int w = 0, h = 0, comp = 0;
    unsigned char* rgba = stbi_load_from_memory(bytes.data(), (int)bytes.size(), &w, &h, &comp, 4);
    if (!rgba) return 0;
    const auto size = icon_logic::fit(w, h, ICON_PX);
    NVGcontext* vg = brls::Application::getNVGContext();
    int tex = 0;
    if (size.first == w && size.second == h) {
        tex = nvgCreateImageRGBA(vg, w, h, 0, rgba);
    } else {
        const std::vector<unsigned char> small = icon_logic::shrink(rgba, w, h, size.first, size.second);
        if (!small.empty()) tex = nvgCreateImageRGBA(vg, size.first, size.second, 0, small.data());
    }
    stbi_image_free(rgba);
    return tex;
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
        if (playstats_cancelled()) {   // quitting: nobody to show them
            for (PlayIcon& icon : *wanted) std::free(icon.jpeg);
            return;
        }
        brls::sync([wanted]() {
            s.busy = false;
            for (PlayIcon& icon : *wanted) {
                if (icon.jpeg && icon.size) s.icons.put(icon.app_id, std::vector<unsigned char>(icon.jpeg, icon.jpeg + icon.size));
                else s.no_icon.insert(icon.app_id);
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
    const int t = decode(*b);
    if (t && s.textures.size() < TEXTURES_KEPT) {
        s.textures.emplace(app_id, t);
        return t;
    }
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
