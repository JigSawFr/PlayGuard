// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/api_settings_activity.hpp"

#include <cstdio>

#include "action/api_flow.hpp"
#include "core/calendar.h"
#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;
using namespace api_logic;

namespace
{
// "2026-10-20T18:42:07Z" (calls.json) in the console's time.
std::string when(const std::string& iso)
{
    int y = 0;
    unsigned mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (iso.size() != 20 || std::sscanf(iso.c_str(), "%4d-%2u-%2uT%2u:%2u:%2uZ", &y, &mo, &d, &h, &mi, &s) != 6 ||
        y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || s > 60)
        return iso;
    return ui::time_text((uint64_t)calendar_days_from_civil(y, mo, d) * 86400 + h * 3600 + mi * 60 + s);
}
}   // namespace

void ApiSettingsActivity::onContentAvailable()
{
    note->setSingleLine(false);
    enabled->init("playguard/api/enabled"_i18n, config::get().api_enabled, [this](bool on) {
        config::get().api_enabled = on;
        ui::save_config();
        this->refresh();
    });
    for (const auto& g : groups_in_build()) {
        auto* cell = new brls::DetailCell();
        cell->setText(api_flow::group_name(g));
        cell->setDetailTextColor(ui::color_neutral());
        cadran::List::row(cell, g == "clock" ? "globe" : nullptr, true);
        cell->setMarginBottom(8);
        cell->registerClickAction([this, g](brls::View*) {
            this->choose(g);
            return true;
        });
        perms->addView(cell);
        this->groups.push_back({ g, cell });
    }
    cadran::List::row(all_off, "stop", true);
    all_off->registerClickAction([this](brls::View*) {
        api_flow::turn_all_off();
        ui::notify("playguard/api/all_off_done"_i18n, ui::Toast::Done);
        this->refresh();
        return true;
    });
    this->refresh();
}

void ApiSettingsActivity::choose(const std::string& group)
{
    const auto levels = levels_for(group);
    std::vector<std::string> labels;
    int selected = 0;
    const Level now = api_flow::settings().level(group);
    for (size_t i = 0; i < levels.size(); i++) {
        labels.push_back(api_flow::level_text(levels[i]));
        if (levels[i] == now) selected = (int)i;
    }
    ui::pick(api_flow::group_name(group), labels, selected, [this, group, levels](int i) {
        config::get().api_permissions[group] = level_name(levels[(size_t)i]);
        ui::save_config();
        this->refresh();
    });
}

void ApiSettingsActivity::refresh()
{
    const Settings s = api_flow::settings();
    enabled->setOn(s.enabled, false);
    for (auto& [group, cell] : this->groups) {
        cell->setDetailText(api_flow::level_text(s.level(group)));
        ui::set_visible(cell, s.enabled);
    }
    ui::set_visible(all_off.getView(), s.enabled);

    calls->clearViews();
    const auto list = api_flow::calls();
    if (list.empty()) calls->addView(ui::text_label("playguard/api/no_calls"_i18n, 20, ui::color_note()));
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        const std::string who = it->caller.empty() ? "playguard/api/unnamed"_i18n : it->caller;
        std::string line = brls::getStr("playguard/api/call_line", when(it->at), who, it->route, std::to_string(it->status));
        if (it->dry_run) line += "playguard/api/call_dry_run"_i18n;
        calls->addView(ui::text_label(line, 20, it->status == 200 ? ui::color_text() : ui::color_bad()));
    }
}
