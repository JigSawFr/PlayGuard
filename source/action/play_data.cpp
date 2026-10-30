// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/play_data.hpp"

#include <aurore.hpp>
#include <cstdlib>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "util/paths.hpp"
#include "util/play_cache.hpp"
#include "action/title_names.hpp"
#include "util/number.hpp"
#include "util/system_titles.hpp"
#include "util/title_id.hpp"

namespace play_data
{

namespace
{
struct Entry
{
    std::shared_ptr<const PlayStats> stats;
    std::chrono::steady_clock::time_point read_at;   // never fresh when from the SD card
    bool busy = false;
};
std::map<Key, Entry> s_entries;
std::map<int, std::function<void()>> s_listeners;
int s_next_id = 1;

// What each cache file holds, as a hash of its encoding without the time of
// the read (which changes at every read): a read that found the same is not
// written again. Read by save(), off the UI thread.
std::mutex s_saved_lock;
std::map<Key, size_t> s_saved;

size_t content_hash(const PlayStats& stats)
{
    // `now` only moves the day windows at the next start (shift_days): an
    // older one of the same day, with the same windows, shifts them the same.
    auto copy = std::make_unique<PlayStats>(stats);
    copy->now = 0;
    return std::hash<std::string>()(play_cache::encode(*copy));
}

std::string cache_file(const Key& key)
{
    return paths::nand_cache_dir() + "/activity_" + (key.empty() ? std::string("all") : key) + ".bin";
}

// Local days between two user-clock times, or 0 when the time zone cannot say.
int days_between(u64 from, u64 to)
{
    const TimeRule* rule = time_console_rule();
    LocalTime a, b;
    if (!rule || !rule->to_local(rule->ctx, from, &a) || !rule->to_local(rule->ctx, to, &b)) return 0;
    return (int)(calendar_days_from_civil(b.year, b.month, b.day) - calendar_days_from_civil(a.year, a.month, a.day));
}

// `key`'s entry; the first time, the last run's read from the SD card, its
// day windows moved to today. Shown at once, read again as soon as asked.
Entry& entry(const Key& key)
{
    auto it = s_entries.find(key);
    if (it != s_entries.end()) return it->second;
    Entry& e = s_entries[key];
    std::string bytes;
    if (!paths::read_file(cache_file(key), bytes)) return e;
    auto data = std::make_shared<PlayStats>();
    if (!play_cache::decode(bytes, *data)) return e;
    {
        std::lock_guard<std::mutex> lock(s_saved_lock);
        s_saved[key] = content_hash(*data);
    }
    u64 now = 0;
    time_local_now(&now, nullptr);
    if (now) play_cache::shift_days(*data, days_between(data->now, now));
    e.stats   = data;
    e.read_at = std::chrono::steady_clock::time_point::min();
    return e;
}

void save(const Key& key, const PlayStats& stats)
{
    if (R_FAILED(stats.rc)) return;   // nothing worth keeping
    // Read every 5 min: most reads find what the file already holds (unless
    // Clear the cache removed it since).
    const size_t hash = content_hash(stats);
    {
        std::lock_guard<std::mutex> lock(s_saved_lock);
        const auto it = s_saved.find(key);
        if (it != s_saved.end() && it->second == hash && paths::exists_or_pending(cache_file(key))) return;
    }
    std::string error;
    if (!paths::ensure_dir(paths::nand_cache_dir()) ||
        !paths::atomic_write(cache_file(key), play_cache::encode(stats), &error)) {
        aurore::Logger::warning("play_data: cache not saved: {}", error);
        return;
    }
    std::lock_guard<std::mutex> lock(s_saved_lock);
    s_saved[key] = hash;
}
}   // namespace

Key key_of(const PlayAccount* account)
{
    if (!account) return "";
    return title_id::text(account->uid[0]) + title_id::text(account->uid[1]);
}

std::shared_ptr<const PlayStats> latest(const Key& key)
{
    return entry(key).stats;
}

bool fresh(const Key& key, std::chrono::seconds max_age)
{
    const Entry& e = entry(key);
    return e.stats && e.read_at != std::chrono::steady_clock::time_point::min() &&
           std::chrono::steady_clock::now() - e.read_at < max_age;
}

bool busy(const Key& key)
{
    auto it = s_entries.find(key);
    return it != s_entries.end() && it->second.busy;
}

void fetch(const PlayAccount* account)
{
    const Key key = key_of(account);
    Entry& e = entry(key);
    if (e.busy) return;
    e.busy = true;
    const bool one = account != nullptr;
    const PlayAccount who = one ? *account : PlayAccount{};
    // The last read (this run's, or the SD card's): the names it holds are
    // not read from each game again.
    const std::shared_ptr<const PlayStats> known = e.stats;
    aurore::async([key, one, who, known]() {
        auto data = std::make_shared<PlayStats>();
        playstats_remember(known.get());
        playstats_fetch_for(data.get(), one ? &who : nullptr);
        // Quitting: cut short, so neither kept nor shown.
        if (playstats_cancelled()) return;
        if (data->approximate) aurore::Logger::info("play_data: only the newest part of the log was read (clock set back?)");
        // Kept for the next run from here, off the UI thread: a slow SD card
        // must not hold a frame (one file per account, one read at a time).
        save(key, *data);
        aurore::sync([key, data]() {
            title_names::learn(*data);   // the forwarders' names, kept for when they are gone
            Entry& done = s_entries[key];
            done.busy    = false;
            done.stats   = data;
            done.read_at = std::chrono::steady_clock::now();
            notify();
        });
    });
}

void notify()
{
    // A copy: a listener may unlisten (its tab closed) while called.
    const auto listeners = s_listeners;
    for (const auto& l : listeners) l.second();
}

int listen(std::function<void()> on_read)
{
    const int id = s_next_id++;
    s_listeners[id] = std::move(on_read);
    return id;
}

void unlisten(int id)
{
    s_listeners.erase(id);
}

std::string system_name(u64 app_id)
{
    const char* key = system_titles::key(app_id);
    return key ? aurore::getStr("playguard/system_titles/" + std::string(key)) : "";
}

namespace
{
// A title with no name of its own (its control data gone or unreadable): a
// system program's, else the one recorded (a forwarder deleted since), else
// "Homebrew (<ID>)" for a homebrew; "" for a game.
std::string fallback_name(u64 app_id, const std::string& id)
{
    const std::string system = system_name(app_id);
    if (!system.empty()) return system;
    const std::string kept = title_names::recorded(app_id);
    if (!kept.empty()) return kept;
    const SyncTitleKind kind = title_names::kind(app_id);
    if (kind == SyncTitle_PlayGuard) return "PlayGuard";   // its own forwarder
    if (kind == SyncTitle_Homebrew) return aurore::getStr("playguard/games/homebrew_unnamed", id);
    return "";
}
}   // namespace

std::string unnamed(const std::string& id)
{
    const std::string name = fallback_name(number::parse<uint64_t>(id, 16).value_or(0), id);
    return name.empty() ? aurore::getStr("playguard/games/unnamed", id) : name;
}

std::string known_name(const GameStat& g)
{
    if (g.name[0]) return g.name;
    const std::string system = system_name(g.app_id);
    return system.empty() ? title_names::recorded(g.app_id) : system;
}

std::string game_name(const GameStat& g)
{
    if (g.name[0]) return g.name;
    const std::string id = title_id::text(g.app_id);
    const std::string name = fallback_name(g.app_id, id);
    return name.empty() ? aurore::getStr("playguard/activity/deleted_game", id) : name;
}

const std::vector<PlayAccount>& accounts()
{
    static bool read = false;
    static std::vector<PlayAccount> list;
    if (!read) {
        read = true;
        PlayAccount buf[PLAYSTATS_MAX_ACCOUNTS];
        Result rc = 0;
        const size_t n = playstats_accounts(buf, PLAYSTATS_MAX_ACCOUNTS, &rc);
        list.assign(buf, buf + n);
        if (R_FAILED(rc)) aurore::Logger::info("play_data: accounts not listed (0x{:08X})", (unsigned)rc);
    }
    return list;
}

uint64_t today_total_s(const PlayStats& stats)
{
    uint64_t total = 0;
    for (uint32_t i = 0; i < stats.count; i++) total += stats.games[i].today_s;
    return total;
}

}   // namespace play_data
