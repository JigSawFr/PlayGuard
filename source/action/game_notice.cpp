// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_notice.hpp"

#include <borealis.hpp>
#include <fmt/format.h>
#include <memory>
#include <string>

#include "action/notices.hpp"
#include "activity/game_check_activity.hpp"
#include "ui/ui.hpp"
#include "util/game_cache.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace game_notice
{

namespace
{
// Names in the entry's text; the others are counted.
constexpr size_t NAMES = 3;
}   // namespace

void at_start()
{
    static bool started = false;
    if (started) return;
    started = true;
    SysInfo si;
    sysinfo_get(&si);
    if (si.applet_mode) return;
    const u32 hos = si.hos_version;
    ui::in_background("game check", [hos]() {
        // The last check, then what changed since: what the screen does.
        auto cache = std::make_shared<game_cache::Cache>();
        std::string bytes;
        if (paths::read_file(GameCheckActivity::cache_file(), bytes)) game_cache::decode(bytes, cache.get());
        std::vector<GameRecord> records(GAMECHECK_MAX);
        u32 count = 0;
        bool truncated = false;
        if (R_FAILED(gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated))) return;
        records.resize(count);
        cache->truncated = truncated;
        u64 now = 0;
        time_local_now(&now, nullptr);
        const bool full = game_cache::expired(*cache, (int64_t)now, hos);
        const std::vector<GameRecord> check = game_cache::to_check(cache.get(), records, (int64_t)now, hos);
        bool complete = true;
        if (!check.empty()) {
            std::vector<GameFacts> facts(check.size());
            const u32 done = gamecheck_check(check.data(), (u32)check.size(), facts.data(), nullptr, nullptr);
            for (u32 i = 0; i < done; i++) game_cache::put(cache.get(), facts[i]);
            complete = done == check.size();
        }
        if (complete && full) {
            cache->checked_at = (int64_t)now;
            cache->hos = hos;
        }
        if (ui::quitting()) return;
        const std::string encoded = game_cache::encode(*cache);
        std::string error;
        if (!paths::ensure_dir(paths::nand_dir() + "/cache") ||
            !paths::atomic_write(GameCheckActivity::cache_file(), encoded, &error))
            brls::Logger::warning("games cache: not saved ({})", error);
        brls::sync([cache, hos]() {
            brls::Logger::info("games: start-up check done ({} games)", cache->games.size());
            note(cache->games, hos);
        });
    });
}

void note(const std::vector<GameFacts>& games, u32 hos)
{
    std::vector<std::string> names;
    for (const GameFacts& g : games) {
        if (gamecheck_classify(&g, hos) == GC_OK) continue;
        names.push_back(g.name[0] ? std::string(g.name)
                                  : brls::getStr("playguard/games/unnamed", fmt::format("{:016X}", (unsigned long long)g.app_id)));
    }
    if (names.empty()) {
        notices::clear("games");
        return;
    }
    std::string list;
    for (size_t i = 0; i < names.size() && i < NAMES; i++) list += (list.empty() ? "" : ", ") + names[i];
    if (names.size() > NAMES) list = brls::getStr("playguard/notices/games_more", list, (int)(names.size() - NAMES));
    const std::string title = names.size() == 1 ? "playguard/notices/games_one"_i18n
                                                : brls::getStr("playguard/notices/games", (int)names.size());
    notices::set("games", notices::Level::Action, title, list, "playguard/games/open"_i18n,
                 []() { brls::Application::pushActivity(new GameCheckActivity()); });
}

}   // namespace game_notice
