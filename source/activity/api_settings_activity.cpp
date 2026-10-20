// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/api_settings_activity.hpp"

#include <cstdio>

#include "action/api_flow.hpp"
#include "action/forwarder_flow.hpp"
#include "action/history_flow.hpp"
#include "app.hpp"
#include "core/calendar.h"
#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;
using namespace api_logic;

namespace
{
// "2026-10-20T18:42:07Z" (calls.json) in the console's time,
// "2026-10-20 20:42:07"; "" when it cannot be read.
std::string local_time(const std::string& iso)
{
    int y = 0;
    unsigned mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (iso.size() != 20 || std::sscanf(iso.c_str(), "%4d-%2u-%2uT%2u:%2u:%2uZ", &y, &mo, &d, &h, &mi, &s) != 6 ||
        y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || s > 60)
        return "";
    return ui::time_text((uint64_t)calendar_days_from_civil(y, mo, d) * 86400 + h * 3600 + mi * 60 + s);
}

// A row as a card of its own, added to `box`.
brls::DetailCell* card_cell(brls::Box* box, const std::string& title, const char* icon)
{
    auto* cell = new brls::DetailCell();
    cell->setText(title);
    cell->setDetailTextColor(ui::color_neutral());
    cadran::List::row(cell, icon, true);
    cell->setMarginBottom(8);
    box->addView(cell);
    return cell;
}

std::string who(const LogEntry& e)
{
    return e.caller.empty() ? "playguard/api/unnamed"_i18n : e.caller;
}

// A call's outcome in words: done, allowed (a test), or why not.
std::string outcome(const LogEntry& e)
{
    if (e.status == 200) return e.dry_run ? "playguard/api/outcome/tested"_i18n : "playguard/api/outcome/done"_i18n;
    if (e.status == 403) return "playguard/api/outcome/refused"_i18n;
    if (e.status == 400 || e.status == 404 || e.status == 422) return "playguard/api/outcome/unknown"_i18n;
    if (e.status == 424) return "playguard/api/outcome/skipped"_i18n;
    return "playguard/api/outcome/failed"_i18n;
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
        auto* cell = card_cell(perms, api_flow::group_name(g), g == "clock" ? "globe" : nullptr);
        cell->registerClickAction([this, g](brls::View*) {
            if (!api_flow::settings().enabled) ui::notify("playguard/api/turn_on_first"_i18n, ui::Toast::Refused);
            else this->choose(g);
            return true;
        });
        this->groups.push_back({ g, cell });
    }
    home    = card_cell(actions, "playguard/api/home_icon"_i18n, "home");
    all_off = card_cell(actions, "playguard/api/all_off"_i18n, "stop");
    docs    = card_cell(more, "playguard/api/docs"_i18n, "code");
    home->setDetailText(ui::opens());
    home->registerClickAction([this](brls::View*) {
        this->add_clock_icon();
        return true;
    });
    all_off->registerClickAction([this](brls::View*) {
        this->turn_off_or_back();
        return true;
    });
    docs->setDetailText(ui::opens());
    docs->registerClickAction([](brls::View*) {
        ui::qr_dialog("playguard/api/docs_body"_i18n, std::string(app::repo_url()) + "/blob/main/docs/api.md");
        return true;
    });
    this->refresh();
}

void ApiSettingsActivity::choose(const std::string& group)
{
    const auto levels = levels_for(group);
    std::vector<std::string> labels, details;
    int selected = 0;
    const Level now = api_flow::settings().level(group);
    for (size_t i = 0; i < levels.size(); i++) {
        labels.push_back(api_flow::level_text(levels[i]));
        details.push_back(brls::getStr("playguard/api/level_details/" + group + "/" + level_name(levels[i])));
        if (levels[i] == now) selected = (int)i;
    }
    ui::pick(api_flow::group_name(group), labels, selected, [this, group, levels](int i) {
        config::get().api_permissions[group] = level_name(levels[(size_t)i]);
        ui::save_config();
        this->before_off.clear();
        this->refresh();
    }, true, details);
}

void ApiSettingsActivity::turn_off_or_back()
{
    if (!this->before_off.empty()) {
        config::get().api_permissions = this->before_off;
        this->before_off.clear();
        ui::save_config();
        ui::notify("playguard/api/all_back_done"_i18n, ui::Toast::Done);
        this->refresh();
        return;
    }
    ui::confirm_danger("playguard/api/all_off_confirm"_i18n, "playguard/api/all_off"_i18n, [this]() {
        this->before_off = config::get().api_permissions;
        if (this->before_off.empty()) this->before_off["clock"] = level_name(Level::None);
        api_flow::turn_all_off();
        ui::notify("playguard/api/all_off_done"_i18n, ui::Toast::Done);
        this->refresh();
    });
}

void ApiSettingsActivity::add_clock_icon()
{
    if (ui::refuse_read_only()) return;
    auto create = [this]() {
        forwarder_flow::create_request("playguard/api/home_icon_name"_i18n, "POST /v1/clock/sync",
                                       [this](bool) { brls::sync([this]() { this->refresh(); }); });
    };
    const Settings s = api_flow::settings();
    if (s.enabled && s.level("clock") == Level::ReadWrite) {
        create();
        return;
    }
    // The icon would only be refused: allowed in the same go, said first.
    ui::confirm("playguard/api/home_icon_allow"_i18n, "playguard/api/home_icon_allow_yes"_i18n, [this, create]() {
        config::get().api_enabled = true;
        config::get().api_permissions["clock"] = level_name(Level::ReadWrite);
        ui::save_config();
        this->before_off.clear();
        this->refresh();
        brls::sync(create);
    });
}

void ApiSettingsActivity::refresh()
{
    const Settings s = api_flow::settings();
    enabled->setOn(s.enabled, false);
    // Off, the groups stay in sight, greyed: what turning it on would offer.
    for (auto& [group, cell] : this->groups) {
        cell->setDetailText(api_flow::level_text(s.level(group)));
        ui::show_writable(cell, s.enabled);
    }
    ui::set_visible(home, forwarder_flow::loader_present());
    all_off->setText(this->before_off.empty() ? "playguard/api/all_off"_i18n : "playguard/api/all_back"_i18n);
    ui::set_visible(all_off, s.enabled);
    this->refresh_calls();
}

void ApiSettingsActivity::refresh_calls()
{
    calls->clearViews();
    const auto list = api_flow::calls();
    if (list.empty()) {
        calls->addView(ui::text_label("playguard/api/no_calls"_i18n, 20, ui::color_note()));
        return;
    }
    std::string day;
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        const LogEntry e = *it;
        const std::string when = local_time(e.at);
        const std::string this_day = when.size() >= 10 ? when.substr(0, 10) : "";
        if (this_day != day || it == list.rbegin()) {
            day = this_day;
            auto* header = new brls::Header();
            header->setTitle(history_flow::day_title(day));
            header->setMarginTop(8);
            header->setMarginBottom(4);
            cadran::List::section(header);
            calls->addView(header);
        }
        const std::string key = route_key(e.route);
        const std::string action = key.empty() ? e.route : brls::getStr("playguard/api/routes/" + key);
        auto* cell = card_cell(calls, who(e) + " · " + action, e.status == 200 ? "check" : "alert");
        cell->setDetailText((when.size() >= 16 ? when.substr(11, 5) + " · " : "") + outcome(e));
        cell->setDetailTextColor(e.status == 200 ? ui::color_ok() : ui::color_bad());
        cell->registerClickAction([e, when, action](brls::View*) {
            std::string body = brls::getStr("playguard/api/call_details", who(e), action, e.route,
                                            when.empty() ? e.at : when, std::to_string(e.status));
            if (!e.code.empty()) body += "\n\n" + brls::getStr("playguard/api/errors/" + e.code);
            if (e.dry_run) body += "\n\n" + "playguard/api/outcome/tested"_i18n;
            ui::info(body);
            return true;
        });
    }
    auto* clear = card_cell(calls, "playguard/api/clear_calls"_i18n, "trash");
    clear->registerClickAction([this](brls::View*) {
        ui::confirm_danger("playguard/api/clear_calls_confirm"_i18n, "playguard/api/clear_calls"_i18n, [this]() {
            if (api_flow::clear_calls()) ui::notify("playguard/api/clear_calls_done"_i18n, ui::Toast::Done);
            else ui::error("playguard/api/clear_calls_err"_i18n);
            // The focus leaves the list first: its rows are made again.
            brls::sync([this]() {
                brls::Application::giveFocus(this->enabled.getView());
                this->refresh();
            });
        });
        return true;
    });
}
