// Simulated core/forwarder.h, Console › Forwarders: the simulated HOME menu
// starts with one forwarder another tool made; what PlayGuard installs or
// deletes stays until the app quits, or from one run to the next with
// PLAYGUARD_SIM_KEEP. Nothing is built: the contents are the console's part
// (forwarder_pack.c, which the host tests cover), the IDs the same.
// PLAYGUARD_SIM_FAIL=forwarder makes installing and deleting fail. And the
// made-up homebrew a forwarder can be made for, as .nro files on the
// simulated SD card (sim_homebrew.hpp). PLAYGUARD_SIM_FORWARDERS=1 starts
// with a few made already (PlayGuard's own, sphaira protected and seen
// working, Star Atlas without the PIN), the signature patches forwarders
// need, and two screenshots in the album: Console › Forwarders as it looks
// once in use (the web simulator's "homebrew" preset).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "core/forwarder.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

#include "core/forwarder_pack.h"
#include "core/sim_env.h"
#include "core/write_guard.h"
#include "sim/sim_games.h"
#include "sim/sim_homebrew.hpp"
#include "sim/sim_homebrew_icons.h"
#include "sim/sim_state.h"
#include "util/forwarders.hpp"
#include "util/paths.hpp"
#include "util/sha256.hpp"

namespace
{

// The icons installed in this run, for the HOME menu's icon of each
// (sim_game_icon): too large for the state PLAYGUARD_SIM_KEEP keeps.
std::mutex g_icons_lock;
std::map<u64, std::vector<unsigned char>> g_icons;

void copy(char* dst, size_t size, const char* src)
{
    std::strncpy(dst, src ? src : "", size - 1);
    dst[size - 1] = '\0';
}

// The made-up homebrew on the simulated SD card (the fourth has the
// forwarder another tool made), a homebrew menu (sphaira, by its file name:
// forwarders_logic::is_menu), and PlayGuard's own .nro where the console
// keeps it, without an icon (the app's own stands in, forwarder_flow.cpp).
struct Homebrew
{
    const char *path, *name, *author, *version;
    const unsigned char* icon;
    size_t icon_size;
};
const Homebrew HOMEBREW[] = {
    { "/switch/PixelPaint/PixelPaint.nro", "Pixel Paint", "Studio Lune", "2.1.0", SIM_HB_ICON_PAINT, sizeof(SIM_HB_ICON_PAINT) },
    { "/switch/StarAtlas.nro", "Star Atlas", "Orbit Lab", "0.9.4", SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS) },
    { "/switch/QuickNotes/QuickNotes.nro", "Quick Notes", "Paper Fox", "1.3.0", SIM_HB_ICON_NOTES, sizeof(SIM_HB_ICON_NOTES) },
    { "/switch/SaveKeeper/SaveKeeper.nro", "Save Keeper", "Hollow Tools", "3.10.1", SIM_HB_ICON_KEEPER, sizeof(SIM_HB_ICON_KEEPER) },
    { "/switch/playguard/playguard.nro", "PlayGuard", "JigSawFr", "", nullptr, 0 },
    { "/switch/sphaira/sphaira.nro", "sphaira", "ITotalJustice", "1.0.0", SIM_HB_ICON_NOTES, sizeof(SIM_HB_ICON_NOTES) },
};
const Homebrew& KEPT = HOMEBREW[3];
// PLAYGUARD_SIM_FORWARDERS's: PlayGuard's own, sphaira, Star Atlas.
const Homebrew* const SAMPLE[] = { &HOMEBREW[4], &HOMEBREW[5], &HOMEBREW[1] };

u64 id_of(const Homebrew& h)
{
    const std::string p = std::string("sdmc:") + h.path;
    return forwarder_id(p.c_str(), p.c_str());
}

u64 kept_id()
{
    const std::string p = std::string("sdmc:") + KEPT.path;
    return forwarder_id(p.c_str(), p.c_str());
}

void put32(std::vector<unsigned char>& b, size_t at, uint64_t v, int bytes = 4)
{
    for (int i = 0; i < bytes; i++) b[at + i] = (unsigned char)(v >> (8 * i));
}

// An .nro as elf2nro lays one out: a program of 0x100 bytes (its header at
// 0x10), then the asset section, its icon and its NACP.
std::vector<unsigned char> make_nro(const Homebrew& h)
{
    std::vector<unsigned char> b(0x100 + 0x38, 0);
    std::memcpy(b.data() + 0x10, "NRO0", 4);
    put32(b, 0x18, 0x100);
    std::memcpy(b.data() + 0x100, "ASET", 4);
    put32(b, 0x108, 0x38, 8);
    put32(b, 0x110, h.icon_size, 8);
    put32(b, 0x118, 0x38 + h.icon_size, 8);
    put32(b, 0x120, FWD_NACP_SIZE, 8);
    if (h.icon) b.insert(b.end(), h.icon, h.icon + h.icon_size);
    std::vector<unsigned char> nacp(FWD_NACP_SIZE, 0);
    for (int lang = 0; lang < 16; lang++) {
        std::memcpy(nacp.data() + lang * 0x300, h.name, std::strlen(h.name));
        std::memcpy(nacp.data() + lang * 0x300 + 0x200, h.author, std::strlen(h.author));
    }
    std::memcpy(nacp.data() + 0x3060, h.version, std::strlen(h.version));
    b.insert(b.end(), nacp.begin(), nacp.end());
    return b;
}

}   // namespace

namespace sim_homebrew
{

namespace
{
void write_file(const std::string& path, const unsigned char* data, size_t size)
{
    if (paths::file_exists(path)) return;
    const size_t slash = path.find_last_of('/');
    if (paths::ensure_dir(path.substr(0, slash))) paths::atomic_write(path, std::string((const char*)data, size));
}

// PLAYGUARD_SIM_FORWARDERS: what PlayGuard remembers making (once), the
// patches, the album.
void seed_sample(const std::string& sd_root)
{
    static const unsigned char PATCH[] = { 'P', 'A', 'T', 'C', 'H', 'E', 'O', 'F' };
    for (const char* kind : { "fs_patches", "loader_patches" })
        write_file(sd_root + "/atmosphere/kip_patches/" + kind + "/0000000000000000.ips", PATCH, sizeof(PATCH));
    write_file(sd_root + "/Nintendo/Album/2026/10/20/2026102018300000-57B4628D2267231D57E0FC1078C0596D.jpg",
               SIM_HB_ICON_ATLAS, sizeof(SIM_HB_ICON_ATLAS));
    write_file(sd_root + "/Nintendo/Album/2026/10/19/2026101917050000-57B4628D2267231D57E0FC1078C0596D.jpg",
               SIM_HB_ICON_PAINT, sizeof(SIM_HB_ICON_PAINT));
    if (!forwarders::list().empty()) return;
    for (const Homebrew* h : SAMPLE) {
        forwarders::Made m;
        m.id = id_of(*h);
        m.nro = m.argv = std::string("sdmc:") + h->path;
        m.name = h->name;
        m.author = h->author;
        // sphaira protected, its gate PlayGuard's own .nro (the desktop's).
        if (h == SAMPLE[1]) {
            m.pin = true;
            m.gate = std::string("sdmc:") + SAMPLE[0]->path;
            m.verified = true;
            m.carried = true;
        }
        forwarders::save(m);
    }
}
}   // namespace

void seed(const std::string& sd_root)
{
    for (const Homebrew& h : HOMEBREW) {
        const std::string path = sd_root + h.path;
        if (paths::file_exists(path)) continue;
        const size_t slash = path.find_last_of('/');
        if (!paths::ensure_dir(path.substr(0, slash))) continue;
        const std::vector<unsigned char> nro = make_nro(h);
        paths::atomic_write(path, std::string(nro.begin(), nro.end()));
    }
    if (sim_env_flag("PLAYGUARD_SIM_FORWARDERS")) seed_sample(sd_root);
}

}   // namespace sim_homebrew

extern "C" void forwarder_sha256(const void* data, size_t size, uint8_t out[32])
{
    Sha256 h;
    h.update(data, size);
    const auto d = h.digest();
    std::memcpy(out, d.data(), d.size());
}

extern "C" void sim_forwarder_defaults(SimForwarders* f)
{
    // A save manager's forwarder, as sphaira would have made it.
    SimForwarder& s = f->list[0];
    s.id = kept_id();
    copy(s.name, sizeof(s.name), KEPT.name);
    copy(s.author, sizeof(s.author), KEPT.author);
    copy(s.version, sizeof(s.version), KEPT.version);
    f->count = 1;
    if (!sim_env_flag("PLAYGUARD_SIM_FORWARDERS")) return;
    for (const Homebrew* h : SAMPLE) {
        SimForwarder& x = f->list[f->count++];
        x.id = id_of(*h);
        copy(x.name, sizeof(x.name), h->name);
        copy(x.author, sizeof(x.author), h->author);
        copy(x.version, sizeof(x.version), h->version);
    }
}

extern "C" unsigned char* sim_forwarder_icon(u64 id, size_t* size)
{
    *size = 0;
    std::lock_guard<std::mutex> hold(g_icons_lock);
    const auto it = g_icons.find(id);
    std::vector<unsigned char> icon;
    if (it != g_icons.end()) icon = it->second;
    else if (id == kept_id()) icon.assign(KEPT.icon, KEPT.icon + KEPT.icon_size);
    else
        for (const Homebrew* h : SAMPLE)
            if (id == id_of(*h) && h->icon) icon.assign(h->icon, h->icon + h->icon_size);
    if (icon.empty()) return nullptr;
    auto* out = static_cast<unsigned char*>(std::malloc(icon.size()));
    if (!out) return nullptr;
    std::memcpy(out, icon.data(), icon.size());
    *size = icon.size();
    return out;
}

bool forwarder_is_one(u64 id)
{
    return (id >> 56) == FWD_ID_PREFIX;
}

Result forwarder_list(ForwarderRecord* out, u32 max, u32* count)
{
    const SimState st = sim_state();
    *count = 0;
    for (u32 i = 0; i < st.forwarders.count && *count < max; i++) {
        const SimForwarder& s = st.forwarders.list[i];
        ForwarderRecord& f = out[(*count)++];
        std::memset(&f, 0, sizeof(f));
        f.id = s.id;
        copy(f.name, sizeof(f.name), s.name);
        copy(f.author, sizeof(f.author), s.author);
        copy(f.version, sizeof(f.version), s.version);
    }
    return 0;
}

Result forwarder_install(const ForwarderSpec* spec, u64* id)
{
    *id = 0;
    if (!spec->nro_path || !spec->argv || !spec->icon || !spec->icon_size) return NXM_RC_INVALID_ARGUMENT;
    if (std::strlen(spec->nro_path) > FWD_PATH_MAX || std::strlen(spec->argv) > FWD_ARGV_MAX)
        return NXM_RC_INVALID_ARGUMENT;
    if (spec->gate_path && (!spec->gate_argv || std::strlen(spec->gate_path) > FWD_PATH_MAX ||
                            std::strlen(spec->gate_argv) > FWD_ARGV_MAX))
        return NXM_RC_INVALID_ARGUMENT;
    *id = forwarder_id(spec->nro_path, spec->argv);
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    if (sim_fails("forwarder")) return SIM_FAIL_RC;
    // The NACP's version, as the console would read it back (0x3060).
    char version[0x10] = "";
    if (spec->nacp) copy(version, sizeof(version), static_cast<const char*>(spec->nacp) + 0x3060);
    SimState* st = sim_begin();
    SimForwarders& f = st->forwarders;
    u32 at = f.count;
    for (u32 i = 0; i < f.count; i++)
        if (f.list[i].id == *id) at = i;
    if (at < sizeof(f.list) / sizeof(f.list[0])) {
        SimForwarder& s = f.list[at];
        s.id = *id;
        copy(s.name, sizeof(s.name), spec->name);
        copy(s.author, sizeof(s.author), spec->author);
        copy(s.version, sizeof(s.version), version);
        if (at == f.count) f.count++;
    }
    const bool full = at >= sizeof(f.list) / sizeof(f.list[0]);
    sim_end();
    if (!full) {
        const auto* icon = static_cast<const unsigned char*>(spec->icon);
        std::lock_guard<std::mutex> hold(g_icons_lock);
        g_icons[*id].assign(icon, icon + spec->icon_size);
    }
    // The SD card's database full, as close as the simulation comes.
    return full ? SIM_FAIL_RC : 0;
}

Result forwarder_remove(u64 id)
{
    if (!forwarder_is_one(id)) return NXM_RC_INVALID_ARGUMENT;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    if (sim_fails("forwarder")) return SIM_FAIL_RC;
    SimState* st = sim_begin();
    SimForwarders& f = st->forwarders;
    bool found = false;
    for (u32 i = 0; i < f.count; i++) {
        if (f.list[i].id != id) continue;
        std::memmove(&f.list[i], &f.list[i + 1], (f.count - i - 1) * sizeof(f.list[0]));
        std::memset(&f.list[--f.count], 0, sizeof(f.list[0]));
        found = true;
        break;
    }
    sim_end();
    {
        std::lock_guard<std::mutex> hold(g_icons_lock);
        g_icons.erase(id);
    }
    // ns' "not found" (2016-0002), as nsDeleteApplicationCompletely answers.
    return found ? 0 : (Result)0x410;
}
