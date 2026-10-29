// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rating_flow.hpp"

#include <aurore.hpp>
#include <memory>
#include <vector>

#include "action/game_check_text.hpp"
#include "action/game_scan.hpp"
#include "action/play_data.hpp"
#include "action/rating_logic.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

namespace rating_flow
{

int level_age(const PctlStatus& st, uint32_t level)
{
    if (level == PctlSafetyLevel_None) return 0;
    if (level == PctlSafetyLevel_Custom) return st.settings_ok ? st.settings.rating_age : -1;
    PctlCustomSettings s = {};
    return R_SUCCEEDED(pctl_get_level_settings(level, &s)) ? s.rating_age : -1;
}

std::string game_rating(const PctlStatus& st, const GameFacts& g)
{
    if (!st.rating_org_ok) return "";
    const int age = rating_logic::age_of(g, st.rating_org);
    return age < 0 ? "" : std::string(pctl_rating_org_name(st.rating_org)) + " " + std::to_string(age);
}

const GameFacts* facts_of(uint64_t app_id)
{
    // The cache lives as long as the check it holds is the latest: kept here
    // for the pointer handed out.
    static std::shared_ptr<const game_cache::Cache> held;
    held = game_scan::latest();
    if (!held) return nullptr;
    for (const GameFacts& g : held->games)
        if (g.app_id == app_id) return &g;
    return nullptr;
}

bool count_blocked(const PctlStatus& st, int age, size_t* blocked, size_t* games)
{
    const auto cache = game_scan::latest();
    if (!cache || !st.rating_org_ok || age < 0) return false;
    const rating_logic::Blocked b = rating_logic::blocked_games(cache->games, st.rating_org, (uint8_t)age);
    *blocked = b.games.size();
    *games = cache->games.size();
    return true;
}

void show_blocked(const PctlStatus& st, int age)
{
    const auto cache = game_scan::latest();
    if (!cache || !st.rating_org_ok || age < 0) return;
    const rating_logic::Blocked b = rating_logic::blocked_games(cache->games, st.rating_org, (uint8_t)age);
    const std::string org = pctl_rating_org_name(st.rating_org);
    if (b.games.empty()) {
        ui::info(age ? aurore::getStr("playguard/restrictions/blocked_none", org, age)
                     : "playguard/restrictions/blocked_no_age"_i18n);
        return;
    }
    std::vector<std::string> names, ratings;
    for (const GameFacts* g : b.games) {
        names.push_back(game_check_text::game_name(*g, play_data::unnamed));
        ratings.push_back(game_rating(st, *g));
    }
    ui::pick(aurore::getStr("playguard/restrictions/blocked_title", org, age), names, 0, [](int) {}, false, ratings);
}

}   // namespace rating_flow
