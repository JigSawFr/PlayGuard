// libFuzzer target: the file of source/util/game_cache.cpp, read back from
// the SD card (a damaged or hand-edited one included), then judged by the
// rules of core/gamecheck_rules.c as the screen does with what it shows at
// once. Beyond the sanitizers: what decode() accepts encodes to a file that
// decodes to the same, strings end inside their arrays, the content count is
// bounded, and to_check() keeps only listed games.
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>

#include "fuzz.h"
#include "util/game_cache.hpp"

// gamecheck_rules.c's gamecheck_scan() reads the console through these
// (core/gamecheck.c, not linked here); nothing here scans.
extern "C" Result gamecheck_records(GameRecord*, u32, u32*, bool*) { abort(); }
extern "C" u32 gamecheck_check(const GameRecord*, u32, GameFacts*, bool (*)(void*, const GameFacts*, u32), void*)
{
    abort();
}

static void judge(const GameFacts& g, uint32_t hos)
{
    FUZZ_ASSERT(std::memchr(g.name, 0, sizeof g.name) != nullptr);
    FUZZ_ASSERT(std::memchr(g.display_version, 0, sizeof g.display_version) != nullptr);
    FUZZ_ASSERT(g.content_count <= GAMECHECK_MAX_CONTENTS);
    const GameIssue issue = gamecheck_classify(&g, hos);
    gamecheck_actions(&g, issue);
    gamecheck_recommended(&g, issue, hos);
    FUZZ_ASSERT(gamecheck_category(&g, issue, hos) < GC_CAT_COUNT);
    gamecheck_installed_version(&g);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string bytes(reinterpret_cast<const char*>(data), size);
    game_cache::Cache c;
    if (!game_cache::decode(bytes, &c)) return 0;

    const std::string again = game_cache::encode(c);
    game_cache::Cache back;
    FUZZ_ASSERT(game_cache::decode(again, &back));
    FUZZ_ASSERT(game_cache::encode(back) == again);

    const uint32_t hos = MAKEHOSVERSION(22, 1, 0);
    for (const GameFacts& g : c.games) judge(g, hos);

    // Every other game still listed, one entry changed.
    std::vector<GameRecord> records;
    std::set<uint64_t> listed;
    for (size_t i = 0; i < c.games.size(); i += 2) {
        GameRecord r;
        std::memset(&r, 0, sizeof r);
        r.app_id  = c.games[i].app_id;
        r.updated = c.games[i].record_updated + (i == 0);
        r.event   = c.games[i].record_event;
        records.push_back(r);
        listed.insert(r.app_id);
    }
    // A minute after the check, unless the file's time leaves no room for it.
    const int64_t now = c.checked_at > INT64_MAX - 60 ? c.checked_at : c.checked_at + 60;
    game_cache::expired(c, now, hos);
    game_cache::Cache shown = c;
    game_cache::to_check(&shown, records, now, hos);
    for (const GameFacts& g : shown.games) FUZZ_ASSERT(listed.count(g.app_id) == 1);

    if (!c.games.empty()) {
        GameFacts g = c.games.front();
        g.verified = GC_VERIFIED_NONE;
        game_cache::put(&shown, g);
        game_cache::set_verified(&shown, g.app_id, GC_VERIFIED_CORRUPT, now, 0x1234);
        game_cache::carry_verified(c, &g);
    }
    game_cache::encode(shown);
    return 0;
}
