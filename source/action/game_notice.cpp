// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_notice.hpp"

#include <borealis.hpp>
#include <string>

#include "action/game_check_text.hpp"
#include "action/game_scan.hpp"
#include "action/notices.hpp"
#include "action/play_data.hpp"
#include "activity/game_check_activity.hpp"

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
    game_scan::at_start(hos, [hos](const game_cache::Cache& cache) { note(cache.games, hos); });
}

void note(const std::vector<GameFacts>& games, u32 hos)
{
    std::vector<std::string> names;
    for (const GameFacts& g : games) {
        if (gamecheck_classify(&g, hos) == GC_OK) continue;
        names.push_back(game_check_text::game_name(g, play_data::unnamed));
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
