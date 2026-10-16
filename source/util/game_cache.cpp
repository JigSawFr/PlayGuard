// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/game_cache.hpp"

#include <cstddef>
#include <cstring>
#include <map>

namespace game_cache
{

namespace
{
constexpr char MAGIC[4] = { 'P', 'G', 'G', 'C' };
constexpr uint32_t VERSION = 2;   // 2: what the integrity check found
constexpr uint32_t MAX_GAMES = GAMECHECK_MAX;

struct Header
{
    char     magic[4];
    uint32_t version;
    uint32_t game_size;   // sizeof(GameFacts): another layout is not read
    uint32_t hos;
    int64_t  checked_at;
    uint32_t count;
    uint32_t truncated;
};

// A damaged file can hold a bool that is neither 0 nor 1, and reading one is
// undefined: its byte is read and written back as one.
void fix_bool(bool& b)
{
    unsigned char v;
    std::memcpy(&v, &b, 1);
    v = v != 0;
    std::memcpy(&b, &v, 1);
}

// gamecheck_installed_version(), which lives with the console calls.
uint32_t installed_version(const GameFacts& g)
{
    uint32_t v = g.has_base ? g.base_version : 0;
    if (g.has_patch && g.patch_version > v) v = g.patch_version;
    return v;
}
}   // namespace

std::string encode(const Cache& cache)
{
    const uint32_t count = cache.games.size() > MAX_GAMES ? MAX_GAMES : (uint32_t)cache.games.size();
    Header h;
    std::memset(&h, 0, sizeof h);
    std::memcpy(h.magic, MAGIC, sizeof MAGIC);
    h.version    = VERSION;
    h.game_size  = sizeof(GameFacts);
    h.hos        = cache.hos;
    h.checked_at = cache.checked_at;
    h.count      = count;
    h.truncated  = cache.truncated ? 1 : 0;
    std::string out;
    out.append(reinterpret_cast<const char*>(&h), sizeof h);
    if (count) out.append(reinterpret_cast<const char*>(cache.games.data()), count * sizeof(GameFacts));
    return out;
}

bool decode(const std::string& bytes, Cache* out)
{
    Header h;
    if (bytes.size() < sizeof h) return false;
    bytes.copy(reinterpret_cast<char*>(&h), sizeof h, 0);
    if (std::memcmp(h.magic, MAGIC, sizeof MAGIC) != 0 || h.version != VERSION || h.game_size != sizeof(GameFacts) ||
        h.count > MAX_GAMES || bytes.size() != sizeof h + (size_t)h.count * sizeof(GameFacts))
        return false;
    Cache c;
    c.checked_at = h.checked_at;
    c.hos        = h.hos;
    c.truncated  = h.truncated != 0;
    c.games.resize(h.count);
    if (h.count) bytes.copy(reinterpret_cast<char*>(c.games.data()), h.count * sizeof(GameFacts), sizeof h);
    for (GameFacts& g : c.games) {
        g.name[sizeof g.name - 1] = '\0';
        g.display_version[sizeof g.display_version - 1] = '\0';
        if (g.content_count > GAMECHECK_MAX_CONTENTS) g.content_count = GAMECHECK_MAX_CONTENTS;
        for (bool* b : { &g.control_ok, &g.has_base, &g.has_patch, &g.card_only, &g.storage_unavailable,
                         &g.meta_missing, &g.files_missing, &g.needs_ticket, &g.ticket_missing, &g.view_ok,
                         &g.launch_checked })
            fix_bool(*b);
    }
    *out = std::move(c);
    return true;
}

bool expired(const Cache& cache, int64_t now, uint32_t hos)
{
    return cache.checked_at <= 0 || cache.hos != hos || now < cache.checked_at || now - cache.checked_at >= MAX_AGE_S;
}

std::vector<GameRecord> to_check(Cache* cache, const std::vector<GameRecord>& records, int64_t now, uint32_t hos)
{
    const bool all = expired(*cache, now, hos);
    std::map<uint64_t, GameFacts> known;
    for (const GameFacts& g : cache->games) known[g.app_id] = g;
    std::vector<GameFacts> kept;
    std::vector<GameRecord> check;
    for (const GameRecord& r : records) {
        auto it = known.find(r.app_id);
        const bool changed = it == known.end() || it->second.record_updated != r.updated || it->second.record_event != r.event;
        if (all || changed) check.push_back(r);
        // Kept meanwhile, so the list does not empty while it is checked.
        if (it != known.end()) kept.push_back(it->second);
    }
    cache->games = std::move(kept);
    return check;
}

void carry_verified(const Cache& cache, GameFacts* game)
{
    if (game->verified != GC_VERIFIED_NONE) return;
    for (const GameFacts& g : cache.games) {
        if (g.app_id != game->app_id) continue;
        if (g.verified == GC_VERIFIED_NONE || g.verified_version != installed_version(*game)) return;
        game->verified         = g.verified;
        game->verified_version = g.verified_version;
        game->verified_at      = g.verified_at;
        game->verified_code    = g.verified_code;
        return;
    }
}

void put(Cache* cache, const GameFacts& game)
{
    GameFacts kept = game;
    carry_verified(*cache, &kept);
    for (GameFacts& g : cache->games)
        if (g.app_id == game.app_id) {
            g = kept;
            return;
        }
    cache->games.push_back(kept);
}

bool set_verified(Cache* cache, uint64_t app_id, uint8_t result, int64_t at, Result code)
{
    for (GameFacts& g : cache->games) {
        if (g.app_id != app_id) continue;
        g.verified         = result;
        g.verified_version = installed_version(g);
        g.verified_at      = at > 0 ? (u64)at : 0;
        g.verified_code    = result == GC_VERIFIED_CORRUPT ? code : 0;
        return true;
    }
    return false;
}

}   // namespace game_cache
