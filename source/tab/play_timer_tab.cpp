// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/play_timer_tab.hpp"

#include <array>
#include <fmt/format.h>
#include <vector>

#include "action/outside_watch.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "activity/play_timer_advanced_activity.hpp"
#include "activity/play_timer_perday_activity.hpp"
#include "activity/profiles_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/profiles.hpp"

using namespace brls::literals;

namespace
{
// "+15 min / +30 min / +1 h"
std::string amounts_text(const int (&set)[3])
{
    std::string text;
    for (int m : set) text += (text.empty() ? "+" : " / +") + ui::fmt_minutes((uint16_t)m);
    return text;
}

int extra_set_index(const std::vector<int>& amounts)
{
    for (size_t i = 0; i < sizeof(config::EXTRA_SETS) / sizeof(config::EXTRA_SETS[0]); i++)
        if (amounts == std::vector<int>(std::begin(config::EXTRA_SETS[i]), std::end(config::EXTRA_SETS[i]))) return (int)i;
    return 0;
}
}   // namespace

PlayTimerTab::PlayTimerTab()
    : TabBase("xml/tab/play_timer.xml")
{
    fw_note->setSingleLine(false);
    bedtime_note->setSingleLine(false);
    limit_note->setSingleLine(false);
    ui::init_unlock_banner(unlocked_banner, [this]() { this->refresh(); });
    this->enable_auto_refresh(5000);

    // The week chart is the editor: ←/→ pick a day, A changes its limit.
    week->set_on_pick([this](int day) {
        if (!this->pt.valid) return;
        pt_flow::change_day_limit(day, this->pt.day_min[day], [this]() { this->refresh(); });
    });

    quick->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::choose_uniform_limit(this->pt, [this]() { this->refresh(); });
        return true;
    });
    extra->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::add_extra_time(this->pt, [this]() { this->refresh(); });
        return true;
    });
    stop->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::stop_today(this->pt, [this]() { this->refresh(); });
        return true;
    });
    per_day->setDetailText(ui::opens());
    per_day->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new PlayTimerPerDayActivity());
        return true;
    });
    remove->registerClickAction([this](brls::View*) {
        if (!ui::refuse_read_only()) this->remove_limit();
        return true;
    });
    profiles_cell->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ProfilesActivity());
        return true;
    });
    time_up->registerClickAction([this](brls::View*) {
        pt_flow::choose_time_up(this->pt, [this]() { this->refresh(); });
        return true;
    });
    bedtime->registerClickAction([this](brls::View*) {
        pt_flow::choose_bedtime(this->pt, [this]() { this->refresh(); });
        return true;
    });
    bedtime_reset->registerClickAction([this](brls::View*) {
        pt_flow::choose_bedtime_end(this->pt, [this]() { this->refresh(); });
        return true;
    });

    // The extra-time settings: PlayGuard's own, kept with the actions they shape.
    ui::guard_switch(extra_auto);
    extra_amounts->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        std::vector<std::string> labels;
        for (const auto& set : config::EXTRA_SETS) labels.push_back(amounts_text(set));
        ui::pick("playguard/tools/extra_amounts"_i18n, labels, extra_set_index(config::get().extra_amounts), [this](int i) {
            config::get().extra_amounts.assign(std::begin(config::EXTRA_SETS[i]), std::end(config::EXTRA_SETS[i]));
            ui::save_config();
            this->refresh();
        });
        return true;
    });
    extra_auto->init("playguard/tools/extra_auto"_i18n, config::get().extra_auto_restore, [](bool on) {
        config::get().extra_auto_restore = on;
        ui::save_config();
    });
    advanced->setDetailText(ui::opens());
    advanced->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new PlayTimerAdvancedActivity());
        return true;
    });
}

void PlayTimerTab::refresh()
{
    pctl_play_timer_query(&this->pt);
    state_header->show(this->pt);
    outside_watch::observe(this->pt);

    const bool fw_ok    = this->pt.fw_supported;
    const bool writable = fw_ok && !app::read_only();

    ui::note_unlocked(this->pt.temporary_unlocked_valid, this->pt.temporary_unlocked);
    ui::show_unlock_banner(unlocked_banner, this->pt.temporary_unlocked_valid && this->pt.temporary_unlocked);
    ui::set_visible(fw_note.getView(), !fw_ok);
    // Read-only: the actions that write stay in sight, greyed (A says why);
    // the per-day editor and the profiles can still be looked at.
    for (brls::View* v : { (brls::View*)limit_header.getView(), (brls::View*)quick.getView(),
                           (brls::View*)per_day.getView(), (brls::View*)remove.getView(),
                           (brls::View*)profiles_cell.getView(), (brls::View*)limit_note.getView() })
        ui::set_visible(v, fw_ok);
    // When the time is up: only with something set (pt_configured), the
    // only time the choice means anything.
    ui::set_visible(time_up.getView(), fw_ok && this->pt.valid && pt_configured(this->pt.block));
    const int wd = ui::today_weekday();
    ui::set_visible(extra.getView(), pt_logic::can_add_extra_time(this->pt, wd, false));
    ui::set_visible(stop.getView(), pt_logic::can_stop_today(this->pt, wd, false));
    for (brls::DetailCell* c : { (brls::DetailCell*)quick.getView(), (brls::DetailCell*)extra.getView(),
                                 (brls::DetailCell*)stop.getView(), (brls::DetailCell*)remove.getView(),
                                 (brls::DetailCell*)bedtime.getView(), (brls::DetailCell*)bedtime_reset.getView(),
                                 (brls::DetailCell*)time_up.getView(), (brls::DetailCell*)extra_amounts.getView(),
                                 (brls::DetailCell*)extra_auto.getView() })
        ui::show_writable(c, writable);
    extra->setDetailText(pt_flow::extra_today_text(this->pt));
    for (brls::View* v : { (brls::View*)bedtime_header.getView(), (brls::View*)bedtime.getView(),
                           (brls::View*)bedtime_reset.getView(), (brls::View*)bedtime_note.getView() })
        ui::set_visible(v, fw_ok);
    for (brls::View* v : { (brls::View*)extra_header.getView(), (brls::View*)extra_amounts.getView(),
                           (brls::View*)extra_auto.getView(), (brls::View*)advanced.getView() })
        ui::set_visible(v, fw_ok);
    const auto& cfg = config::get();
    extra_amounts->setDetailText(amounts_text(config::EXTRA_SETS[extra_set_index(cfg.extra_amounts)]));
    extra_auto->setOn(cfg.extra_auto_restore, false);

    ui::set_visible(week.getView(), fw_ok && this->pt.valid);
    ui::set_visible(week_header.getView(), fw_ok && this->pt.valid);
    if (!fw_ok) return;
    if (this->pt.valid) {
        week->show(this->pt);
        week->set_editable(writable);
        week_header->setTitle(writable ? "playguard/play_timer/week_title_edit"_i18n : "playguard/play_timer/week_title"_i18n);
    }

    // Profiles…: the profile applied now, else how many are saved.
    const std::string current = this->pt.valid ? profiles::match(this->pt.day_min) : "";
    const size_t saved = current.empty() ? profiles::count() : 0;
    profiles_cell->setDetailText(ui::opens(!current.empty() ? current
                                           : saved ? brls::getStr("playguard/play_timer/profiles_count", (int)saved) : ""));
    time_up->setDetailText(pt_flow::time_up_text(this->pt));

    // "Same limit every day": the value when all days agree, else that they
    // differ, as the bedtime row says it (a range such as "1 h to 3 h" read
    // as a limit that is set; a dash would read as "unavailable").
    bool uniform = this->pt.valid;
    for (int i = 1; i < 7 && uniform; i++) uniform = this->pt.day_min[i] == this->pt.day_min[0];
    quick->setDetailText(uniform ? ui::fmt_minutes(this->pt.day_min[0]) : "playguard/play_timer/bedtime_varies"_i18n);

    // The bedtime as the block holds it, every day, once the console's answer
    // confirms where it is (pt_logic::bedtime_layout_ok); else what the
    // console reports for today.
    const std::string na = "playguard/common/unavailable"_i18n;
    PtBedtime same;
    if (pt_logic::bedtime_layout_ok(this->pt, wd)) {
        const bool uniform = pt_logic::bedtime_uniform(this->pt, &same);
        bedtime->setDetailText(!uniform ? "playguard/play_timer/bedtime_varies"_i18n
                               : same.on ? fmt::format("{:02d}:{:02d}", same.hour, same.minute)
                                         : "playguard/common/off"_i18n);
        bedtime_reset->setDetailText(uniform && same.on ? fmt::format("{:02d}:{:02d}", same.end_hour, same.end_minute)
                                     : uniform ? "—" : "playguard/play_timer/bedtime_varies"_i18n);
    } else {
        if (this->pt.bedtime_valid) {
            std::string hm = fmt::format("{:02d}:{:02d}", this->pt.bedtime_hour, this->pt.bedtime_minute);
            bedtime->setDetailText(brls::getStr(this->pt.bedtime_enabled ? "playguard/play_timer/bedtime_value_on"
                                                                         : "playguard/play_timer/bedtime_value_off", hm));
        } else {
            bedtime->setDetailText(na);
        }
        bedtime_reset->setDetailText(this->pt.bedtime_reset_valid
            ? fmt::format("{:02d}:{:02d}", this->pt.bedtime_reset_hour, this->pt.bedtime_reset_minute) : na);
    }

}

void PlayTimerTab::remove_limit()
{
    pt_flow::confirm_write("playguard/play_timer/remove_body"_i18n, "playguard/play_timer/remove_confirm"_i18n,
                           [this](bool did_unlock) {
                               Result rc = pt_flow::clear_days("remove");
                               pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/removed"_i18n,
                                                     "playguard/play_timer/remove_err"_i18n,
                                                     [this]() { this->refresh(); });
                           }, nullptr, true);
}

brls::View* PlayTimerTab::create()
{
    return new PlayTimerTab();
}
