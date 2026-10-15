// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_notice.hpp"

#include <borealis.hpp>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <vector>

#include "action/notices.hpp"
#include "activity/game_check_activity.hpp"
#include "ui/ui.hpp"

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
    ui::in_background("game check", []() {
        auto check = std::make_shared<GameCheck>();
        gamecheck_scan(check.get());
        if (ui::quitting()) return;
        brls::sync([check]() { note(*check); });
    });
}

void note(const GameCheck& check)
{
    if (R_FAILED(check.rc)) return;
    SysInfo si;
    sysinfo_get(&si);
    std::vector<std::string> names;
    for (u32 i = 0; i < check.count; i++) {
        const GameFacts& g = check.games[i];
        if (gamecheck_classify(&g, si.hos_version) == GC_OK) continue;
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
