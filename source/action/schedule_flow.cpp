// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/schedule_flow.hpp"

#include <algorithm>
#include <array>
#include <borealis.hpp>
#include <cstdlib>
#include <fmt/format.h>
#include <vector>

#include "action/profile_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/schedule_logic.hpp"
#include "app.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/profiles.hpp"

using namespace brls::literals;

namespace schedule_flow
{

namespace
{
constexpr int START_DAYS  = 60;   // the first day: today and the 59 after
constexpr int PERIOD_DAYS = 60;   // the last day: from the first, 60 days

std::string s_asked_on;   // the day the question was asked this run: once a day

schedule_logic::Schedule load()
{
    const auto& c = config::get();
    schedule_logic::Schedule s;
    s.file    = c.schedule_file;
    s.name    = c.schedule_name;
    s.from    = c.schedule_from;
    s.to      = c.schedule_to;
    s.applied = c.schedule_applied;
    for (size_t d = 0; d < 7 && d < c.schedule_prev.size(); d++) s.prev[d] = (uint16_t)c.schedule_prev[d];
    return s;
}

void clear()
{
    auto& c = config::get();
    c.schedule_file.clear();
    c.schedule_name.clear();
    c.schedule_from.clear();
    c.schedule_to.clear();
    c.schedule_applied = false;
    c.schedule_prev.clear();
    ui::save_config();
}

// "October 19" in the language shown.
std::string day_text(const std::string& ymd)
{
    if (ymd.size() != 10) return ymd;
    const int month = std::atoi(ymd.substr(5, 2).c_str()), day = std::atoi(ymd.substr(8, 2).c_str());
    if (month < 1 || month > 12 || day < 1) return ymd;
    return brls::getStr("playguard/schedule/date", day, brls::getStr(fmt::format("playguard/cadran/months/{}", month)));
}

bool find_profile(const std::string& file, profiles::Profile* out)
{
    for (const profiles::Profile& p : profiles::list())
        if (p.file == file) {
            *out = p;
            return true;
        }
    return false;
}

// The period begins: the profile put in force (its own confirmation, the
// days drawn under it). The limits it replaces are saved first, to be
// offered back afterwards.
void apply(std::function<void()> after)
{
    const schedule_logic::Schedule s = load();
    profiles::Profile p;
    if (!find_profile(s.file, &p)) {
        ui::info(brls::getStr("playguard/schedule/profile_gone", s.name));
        clear();
        return;
    }
    PtState live;
    pctl_play_timer_query(&live);
    if (!live.valid) {
        ui::refuse(NXM_RC_STATE_UNKNOWN);
        return;
    }
    auto& c = config::get();
    c.schedule_prev.assign(live.day_min, live.day_min + 7);
    // Already the profile's limits: in force with nothing to write.
    if (std::equal(p.days.begin(), p.days.end(), live.day_min)) {
        c.schedule_applied = true;
        ui::save_config();
        ui::notify(brls::getStr("playguard/play_timer/profile_already", p.name));
        if (after) after();
        return;
    }
    if (!ui::save_config_now()) {
        ui::error("playguard/play_timer/write_err"_i18n, NXM_RC_NOT_SAVED);
        return;
    }
    profile_flow::apply(p, [after]() {
        config::get().schedule_applied = true;
        ui::save_config();
        if (after) after();
    });
}

// The period is over: the limits from before back (the days drawn under the
// question), or kept.
void put_back(std::function<void()> after)
{
    const schedule_logic::Schedule s = load();
    const std::array<uint16_t, 7> prev = s.prev;
    pt_flow::confirm_write(brls::getStr("playguard/schedule/put_back_body", s.name), "playguard/play_timer/confirm_set"_i18n,
                           [prev, after](bool did_unlock) {
                               const Result rc = pt_flow::write_days(prev.data(), "schedule_end");
                               if (R_SUCCEEDED(rc)) clear();
                               pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/written_days"_i18n,
                                                     "playguard/play_timer/write_err"_i18n, after);
                           }, prev.data());
}

void ask_apply(const schedule_logic::Schedule& s, std::function<void()> after)
{
    auto* dialog = ui::dialog(brls::getStr("playguard/schedule/apply_body", day_text(s.from), day_text(s.to), s.name, day_text(s.to)));
    dialog->addButton("playguard/schedule/not_now"_i18n, []() {});
    dialog->addButton("playguard/schedule/apply"_i18n, [after]() { brls::sync([after]() { apply(after); }); });
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    dialog->open();
}

void ask_put_back(const schedule_logic::Schedule& s, std::function<void()> after)
{
    auto* dialog = ui::dialog(brls::getStr("playguard/schedule/over_body", s.name, day_text(s.to), ui::days_summary(s.prev.data())));
    dialog->addButton("playguard/schedule/keep"_i18n, [after]() {
        clear();
        ui::notify("playguard/schedule/removed"_i18n);
        if (after) after();
    });
    dialog->addButton("playguard/schedule/put_back"_i18n, [after]() { brls::sync([after]() { put_back(after); }); });
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    dialog->open();
}

// The period: its first day, then its last, then saved (not in force yet:
// the main screen offers it when the first day comes, at once when today).
void pick_period(const profiles::Profile& p, std::function<void()> after)
{
    const std::vector<std::string> starts = schedule_logic::dates_from(ui::today_date(), START_DAYS);
    if (starts.empty()) return;
    std::vector<std::string> labels;
    for (const std::string& d : starts) labels.push_back(cadran::kit::long_date(d));
    ui::pick("playguard/schedule/pick_from"_i18n, labels, 0, [p, starts, after](int i) {
        const std::string from = starts[(size_t)i];
        const std::vector<std::string> ends = schedule_logic::dates_from(from, PERIOD_DAYS);
        std::vector<std::string> labels;
        for (const std::string& d : ends) labels.push_back(cadran::kit::long_date(d));
        ui::pick("playguard/schedule/pick_to"_i18n, labels, std::min(6, (int)ends.size() - 1), [p, from, ends, after](int j) {
            auto& c = config::get();
            c.schedule_file    = p.file;
            c.schedule_name    = p.name;
            c.schedule_from    = from;
            c.schedule_to      = ends[(size_t)j];
            c.schedule_applied = false;
            c.schedule_prev.clear();
            ui::save_config();
            ui::notify(brls::getStr("playguard/schedule/saved", p.name, day_text(from), day_text(ends[(size_t)j])));
            if (after) after();
            // Today already: asked now.
            if (schedule_logic::step(load(), ui::today_date()) == schedule_logic::Step::Apply) {
                s_asked_on = ui::today_date();
                ask_apply(load(), after);
            }
        });
    });
}

void pick_profile(std::function<void()> after)
{
    const std::vector<profiles::Profile> all = profiles::list();
    if (all.empty()) {
        ui::info("playguard/schedule/no_profile"_i18n);
        return;
    }
    std::vector<std::string> labels, details;
    for (const profiles::Profile& p : all) {
        labels.push_back(p.name);
        details.push_back(ui::days_summary(p.days.data()));
    }
    ui::pick("playguard/schedule/pick_profile"_i18n, labels, 0, [all, after](int i) {
        pick_period(all[(size_t)i], after);
    }, false, details);
}
}   // namespace

void check(bool at_start)
{
    if (s_asked_on == ui::today_date() || app::read_only()) return;
    const schedule_logic::Schedule s = load();
    const schedule_logic::Step step = schedule_logic::step(s, ui::today_date());
    if (step == schedule_logic::Step::None) return;
    if (step == schedule_logic::Step::Drop) {   // its period went by unseen
        clear();
        return;
    }
    if (at_start && !ui::claim_start_screen("the scheduled profile")) return;   // the next tick asks
    s_asked_on = ui::today_date();
    if (step == schedule_logic::Step::Apply) ask_apply(s, nullptr);
    else ask_put_back(s, nullptr);
}

void choose(std::function<void()> after)
{
    if (ui::refuse_read_only()) return;
    const schedule_logic::Schedule s = load();
    if (s.from.empty()) {
        pick_profile(after);
        return;
    }
    if (s.applied) {
        // In force: its end now, the limits from before back or kept.
        ui::pick(summary(), { "playguard/schedule/put_back_now"_i18n, "playguard/schedule/keep_remove"_i18n }, 0,
                 [after](int i) {
            if (i == 0) put_back(after);
            else {
                clear();
                ui::notify("playguard/schedule/removed"_i18n);
                if (after) after();
            }
        }, false);
        return;
    }
    ui::pick(summary(), { "playguard/schedule/change"_i18n, "playguard/schedule/remove"_i18n }, 0, [after](int i) {
        if (i == 0) pick_profile(after);
        else {
            clear();
            ui::notify("playguard/schedule/removed"_i18n);
            if (after) after();
        }
    }, false);
}

std::string summary()
{
    const auto& c = config::get();
    if (c.schedule_from.empty()) return "";
    return brls::getStr("playguard/schedule/value", c.schedule_name, day_text(c.schedule_from), day_text(c.schedule_to));
}

}   // namespace schedule_flow
