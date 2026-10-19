// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_scan.hpp"

#include <atomic>
#include <borealis.hpp>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "ui/ui.hpp"
#include "util/paths.hpp"

namespace game_scan
{

namespace
{
// The last check saved, and the order the saves are queued in: the start-up
// thread and the main thread both save.
std::mutex s_mutex;
std::shared_ptr<const game_cache::Cache> s_saved;

// Main thread only: the start-up check under way, and the screens waiting
// for it.
bool s_starting = false;
std::vector<std::function<void(bool)>> s_waiting;
// Read by the start-up thread after each game: a screen is waiting.
std::atomic<bool> s_stop{ false };

// `cache` queued for the SD card (ui::write_later runs one write at a time,
// in the order asked). `keep_verified`: what the integrity check found of a
// game since `cache` was read (a screen open during the start-up check) is
// kept.
void store(game_cache::Cache cache, bool keep_verified)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (keep_verified && s_saved)
        for (GameFacts& g : cache.games) game_cache::carry_verified(*s_saved, &g);
    std::string encoded = game_cache::encode(cache);
    s_saved = std::make_shared<const game_cache::Cache>(std::move(cache));
    ui::write_later("games cache", [encoded = std::move(encoded)]() {
        std::string error;
        if (!paths::ensure_dir(paths::nand_cache_dir()) || !paths::atomic_write(cache_file(), encoded, &error))
            brls::Logger::warning("games cache: not saved ({})", error);
    }, "games");
}

// Main thread: the start-up check is over, the screens waiting for it go on.
void finished()
{
    s_starting = false;
    std::vector<std::function<void(bool)>> waiting = std::move(s_waiting);
    s_waiting.clear();
    for (auto& go : waiting) go(true);
}

// The SD card's check, as it is now.
bool read(game_cache::Cache* out)
{
    std::string bytes;
    game_cache::Cache cache;
    if (!paths::read_file(cache_file(), bytes) || !game_cache::decode(bytes, &cache)) return false;
    *out = std::move(cache);
    return true;
}

bool keep_going(void*, const GameFacts*, u32)
{
    return !ui::quitting() && !s_stop.load();
}
}   // namespace

std::string cache_file()
{
    return paths::nand_cache_dir() + "/games.bin";
}

bool load(game_cache::Cache* out)
{
    ui::settle_writes("games");   // a save of it still queued (most often none)
    return read(out);
}

std::shared_ptr<const game_cache::Cache> latest()
{
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_saved) return s_saved;
    }
    auto cache = std::make_shared<game_cache::Cache>();
    if (!load(cache.get())) return nullptr;
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_saved) s_saved = cache;   // a save meanwhile is newer
    return s_saved;
}

void save(const game_cache::Cache& cache)
{
    store(cache, false);
}

void at_start(u32 hos, std::function<void(const game_cache::Cache&)> done)
{
    s_starting = true;
    s_stop = false;
    ui::in_background("game check", [hos, done]() {
        // The last check, then what changed since: what the screen does.
        auto cache = std::make_shared<game_cache::Cache>();
        read(cache.get());   // at start-up: no save of it queued yet
        std::vector<GameRecord> records(GAMECHECK_MAX);
        u32 count = 0;
        bool truncated = false;
        const Result rc = gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated);
        if (ui::quitting()) return;
        if (R_FAILED(rc)) {
            brls::sync([]() { finished(); });
            return;
        }
        records.resize(count);
        cache->truncated = truncated;
        u64 now = 0;
        time_local_now(&now, nullptr);
        const bool full = game_cache::expired(*cache, (int64_t)now, hos);
        const std::vector<GameRecord> check = game_cache::to_check(cache.get(), records, (int64_t)now, hos);
        bool complete = true;
        if (!check.empty()) {
            // Stopped at the next game when the app quits or a screen waits.
            std::vector<GameFacts> facts(check.size());
            const u32 checked = gamecheck_check(check.data(), (u32)check.size(), facts.data(), keep_going, nullptr);
            for (u32 i = 0; i < checked; i++) game_cache::put(cache.get(), facts[i]);
            complete = checked == check.size();
        }
        if (complete && full) {
            cache->checked_at = (int64_t)now;
            cache->hos = hos;
        }
        if (ui::quitting()) return;
        store(*cache, true);
        if (ui::quitting()) return;
        brls::sync([cache, done]() {
            brls::Logger::info("games: start-up check done ({} games)", cache->games.size());
            done(*cache);
            finished();
        });
    });
}

void when_free(std::function<void(bool waited)> go)
{
    if (!s_starting) {
        go(false);
        return;
    }
    s_stop = true;
    s_waiting.push_back(std::move(go));
}

}   // namespace game_scan
