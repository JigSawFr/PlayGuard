// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/play_timer_tab.hpp"

#include <algorithm>
#include <array>
#include <fmt/format.h>
#include <vector>

#include "action/console_lock.hpp"
#include "action/outside_watch.hpp"
#include "action/play_data.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "activity/play_timer_advanced_activity.hpp"
#include "activity/play_timer_perday_activity.hpp"
#include "activity/profiles_activity.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"
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

// "2 h", "1 h to 3 h", "no limit": the days `days` of `v`.
std::string range(const uint16_t* v, std::initializer_list<int> days)
{
    uint16_t lo = PT_DAY_NOLIMIT, hi = 0;
    bool none = false;
    for (int d : days) {
        if (v[d] == PT_DAY_NOLIMIT) {
            none = true;
            continue;
        }
        lo = std::min(lo, v[d]);
        hi = std::max(hi, v[d]);
    }
    if (lo == PT_DAY_NOLIMIT) return "playguard/cadran/week/no_limit"_i18n;
    if (lo == hi && !none) return ui::fmt_minutes(lo);
    return brls::getStr("playguard/play_timer/profile_range", ui::fmt_minutes(lo),
                        none ? "playguard/cadran/week/no_limit"_i18n : ui::fmt_minutes(hi));
}

// The week in a sentence: the same every day, or weekdays and weekend.
std::string summary_of(const uint16_t* v)
{
    bool same = true;
    for (int d = 1; d < 7; d++) same = same && v[d] == v[0];
    if (same) return v[0] == PT_DAY_NOLIMIT ? "playguard/play_timer/perday/pick_no_limit"_i18n
                                            : brls::getStr("playguard/cadran/week/every_day", ui::fmt_minutes(v[0]));
    return brls::getStr("playguard/cadran/week/summary", range(v, { 1, 2, 3, 4, 5 }), range(v, { 6, 0 }));
}

std::string limit_text(uint16_t minutes)
{
    return minutes == PT_DAY_NOLIMIT ? "—" : ui::fmt_minutes(minutes);
}
}   // namespace

PlayTimerTab::PlayTimerTab()
    : TabBase("xml/tab/play_timer.xml"), list(this)
{
    namespace kit = cadran::kit;
    static_cast<brls::Box*>(this->getView("pt_week_card"))->addView(brls::View::createFromXMLResource("view/cadran_week_card.xml"));
    // The keys and the bars' key on two lines when the text is larger.
    kit::wrap(this->getView("cpt_keys_idle"));
    kit::wrap(this->getView("pt_chips"));
    this->list.apply({ { "pt_split", "calendar" }, { "pt_today", "today" }, { "pt_console_lock", "gamepad" },
                       { "pt_bedtime_reset", "moon" }, { "pt_extra_amounts", "plus" },
                       { "pt_extra_auto", "refresh" }, { "pt_advanced", "settings" } });

    // The week: edited here, saved together (+ or Save), as in the per-day editor.
    week_editor->set_can_edit([this]() {
        if (ui::refuse_read_only()) return false;
        if (this->pt.valid) return true;
        ui::notify("playguard/play_timer/perday/unavailable"_i18n);
        return false;
    });
    week_editor->set_on_change([this](int day, uint16_t minutes) {
        this->pending[day] = minutes;
        this->rerender();
    });
    week_editor->set_on_pick([this](int day) {
        pt_flow::pick_limit(brls::getStr("playguard/play_timer/perday/pick_title", ui::day_name(day)), this->pending[day],
                            [this, day](uint16_t minutes) {
                                this->pending[day] = minutes;
                                this->rerender();
                            });
    });
    week_editor->set_on_copy([this](int day, bool weekend) {
        for (int d = 0; d < 7; d++)
            if ((d == 0 || d == 6) == weekend) this->pending[d] = this->pending[day];
        this->rerender();
    });
    week_editor->set_on_focus([this](int day) { this->show_played_key(day); });
    week_editor->set_on_mode([this](bool editing) {
        keys_idle->setVisibility(editing ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
        keys_edit->setVisibility(editing ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    });
    // The footer has no room for a sixth hint while a day is edited (+ Save,
    // the bell, language and theme, B, A): Ⓧ still reads the console again
    // (it does every 5 s anyway), out of sight; on the week it opens the
    // day's picker, which the keys under the week say.
    this->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->refresh();
        return true;
    }, true);
    save_button->registerClickAction([this](brls::View*) {
        this->save();
        return true;
    });
    discard_button->registerClickAction([this](brls::View*) {
        this->discard();
        return true;
    });

    // The chips.
    profile_chip->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ProfilesActivity());
        return true;
    });
    uniform_chip->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::choose_uniform_limit(this->pt, [this]() { this->refresh(); });
        return true;
    });
    bed_chip->registerClickAction([this](brls::View*) {
        pt_flow::choose_bedtime(this->pt, [this]() { this->refresh(); });
        return true;
    });
    time_up_chip->registerClickAction([this](brls::View*) {
        pt_flow::choose_time_up(this->pt, [this]() { this->refresh(); });
        return true;
    });
    remove_chip->registerClickAction([this](brls::View*) {
        if (!ui::refuse_read_only()) this->remove_limit();
        return true;
    });

    fw_note->setSingleLine(false);
    bedtime_note->setSingleLine(false);
    limit_note->setSingleLine(false);
    // The "time's up" alarm is a switch of Advanced.
    this->mark(advanced.getView(), { "alarm_off" });
    this->enable_auto_refresh(5000);

    // Extra time and no more play today are Today's: a line to it.
    ui::set_detail(today, ui::opens("playguard/tabs/dashboard"_i18n));
    today->registerClickAction([](brls::View* view) {
        ui::go_to_tab(view, ui::tab::dashboard);
        return true;
    });
    // Every day's limit at 0 (action/console_lock), the previous ones back
    // when it is turned off.
    this->mark(console_lock_cell.getView(), { "console_lock_long" });
    console_lock_note->setSingleLine(false);
    console_lock_cell->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        PtState pt;   // read again: the tab's is up to 5 s old
        pctl_play_timer_query(&pt);
        if (!pt.fw_supported) {
            ui::error("", NXM_RC_FW_UNSUPPORTED);
            return true;
        }
        PctlStatus s;
        pctl_status_fetch(&s);
        if (!(s.pin_length_ok && s.pin_length > 0)) {   // nothing gates play without a PIN
            ui::info("playguard/console_lock/needs_pin"_i18n);
            return true;
        }
        console_lock::set(!console_lock::active(), pt, [this]() { this->refresh(); });
        return true;
    });
    ui::set_detail(split, ui::opens());
    split->registerClickAction([](brls::View*) {
        if (ui::refuse_read_only()) return true;
        brls::Application::pushActivity(PlayTimerPerDayActivity::week_split());
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
    ui::set_detail(advanced, ui::opens());
    advanced->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new PlayTimerAdvancedActivity());
        return true;
    });
}

void PlayTimerTab::refresh()
{
    // The changes not saved yet stay over a new read, as in the per-day editor.
    const bool keep = this->changes();
    perf::Timed timed("Play timer refresh");   // the diagnostic report's timings
    // The time of day, for every helper below (the state, the week chart,
    // the outside watch, the timer health), read once, as on Today.
    ui::SameTime same_time;

    pctl_play_timer_query(&this->pt);
    state_header->show(this->pt);
    outside_watch::observe(this->pt);

    const bool fw_ok    = this->pt.fw_supported;
    const bool writable = fw_ok && !app::read_only();

    ui::note_unlocked(this->pt.temporary_unlocked_valid, this->pt.temporary_unlocked);
    ui::set_visible(fw_note.getView(), !fw_ok);
    for (brls::View* v : { (brls::View*)limit_header.getView(), (brls::View*)split.getView(),
                           (brls::View*)today.getView(), (brls::View*)limit_note.getView(),
                           (brls::View*)lock_header.getView(), (brls::View*)console_lock_cell.getView(),
                           (brls::View*)console_lock_note.getView() })
        ui::set_visible(v, fw_ok);
    // When the time is up: only with something set (pt_configured), the
    // only time the choice means anything.
    ui::set_visible(time_up_chip.getView(), fw_ok && this->pt.valid && pt_configured(this->pt.block));
    const int wd = ui::today_weekday();
    // Read-only: the actions that write stay in sight, greyed (A says why).
    for (brls::DetailCell* c : { (brls::DetailCell*)split.getView(), (brls::DetailCell*)console_lock_cell.getView(),
                                 (brls::DetailCell*)bedtime_reset.getView(),
                                 (brls::DetailCell*)extra_amounts.getView(), (brls::DetailCell*)extra_auto.getView() })
        ui::show_writable(c, writable);
    const bool locked = console_lock::active();
    ui::set_detail(console_lock_cell, locked ? "playguard/common/on"_i18n : "playguard/common/off"_i18n);
    console_lock_cell->setDetailTextColor(locked ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    ui::set_text(console_lock_note, locked ? "playguard/console_lock/note_on"_i18n : "playguard/console_lock/note_off"_i18n);
    for (brls::View* v : { (brls::View*)bedtime_header.getView(), (brls::View*)bedtime_reset.getView(),
                           (brls::View*)bedtime_note.getView() })
        ui::set_visible(v, fw_ok);
    for (brls::View* v : { (brls::View*)extra_header.getView(), (brls::View*)extra_amounts.getView(),
                           (brls::View*)extra_auto.getView(), (brls::View*)advanced.getView() })
        ui::set_visible(v, fw_ok);
    const auto& cfg = config::get();
    ui::set_detail(extra_amounts, amounts_text(config::EXTRA_SETS[extra_set_index(cfg.extra_amounts)]));
    extra_auto->setOn(cfg.extra_auto_restore, false);

    // When play is allowed again after bedtime, every day, once the console's
    // answer confirms where the bedtime is (pt_logic::bedtime_layout_ok);
    // else what the console reports for today.
    const std::string na = "playguard/common/unavailable"_i18n;
    PtBedtime same;
    if (pt_logic::bedtime_layout_ok(this->pt, wd)) {
        const bool uniform = pt_logic::bedtime_uniform(this->pt, &same);
        ui::set_detail(bedtime_reset, uniform && same.on ? fmt::format("{:02d}:{:02d}", same.end_hour, same.end_minute)
                                      : uniform ? "—" : "playguard/play_timer/bedtime_varies"_i18n);
    } else {
        ui::set_detail(bedtime_reset, this->pt.bedtime_reset_valid
            ? fmt::format("{:02d}:{:02d}", this->pt.bedtime_reset_hour, this->pt.bedtime_reset_minute) : na);
    }

    this->list.sync();
    if (this->pt.valid && !keep)
        for (int d = 0; d < 7; d++) this->pending[d] = this->pt.day_min[d];
    ui::set_visible(top.getView(), this->pt.fw_supported);
    if (!this->pt.fw_supported) cadran::set_pill("playguard/play_timer/fw_too_old_short"_i18n, "alert", cadran::Tone::WARN);
    else if (!this->pt.valid || !pt_configured(this->pt.block)) cadran::set_pill("playguard/cadran/pill/timer_none"_i18n, "timer");
    else if (this->pt.enabled_valid && this->pt.enabled) cadran::set_pill("playguard/cadran/pill/timer_on"_i18n, "timer");
    else cadran::set_pill("playguard/cadran/pill/timer_off"_i18n, "pause", cadran::Tone::WARN);
    this->rerender();
    if (this->play_listener < 0) this->play_listener = play_data::listen([this]() { this->show_played(); });
    this->show_played();
}

bool PlayTimerTab::changes() const
{
    if (!this->pt.valid) return false;
    for (int d = 0; d < 7; d++)
        if (this->pending[d] != this->pt.day_min[d]) return true;
    return false;
}

void PlayTimerTab::save()
{
    if (!this->pt.valid) {
        ui::notify("playguard/play_timer/perday/unavailable"_i18n);
        return;
    }
    if (!this->changes()) {
        ui::notify("playguard/play_timer/perday/no_changes"_i18n);
        return;
    }
    if (ui::refuse_read_only()) return;
    // As the per-day editor: no question unless the timer is counting down
    // (then the one dialog explains the temporary unlock).
    std::array<uint16_t, 7> snapshot;
    for (int d = 0; d < 7; d++) snapshot[d] = this->pending[d];
    pt_flow::confirm_write("", "playguard/play_timer/confirm_set"_i18n, [this, snapshot](bool did_unlock) {
        Result rc = pt_flow::write_days(snapshot.data(), "per_day");
        pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/written_days"_i18n,
                              "playguard/play_timer/write_err"_i18n, [this]() { this->refresh(); });
    }, snapshot.data(), false, "playguard/cadran/week/save_title"_i18n);
}

void PlayTimerTab::discard()
{
    if (!this->changes()) {
        ui::notify("playguard/play_timer/perday/nothing_to_discard"_i18n);
        return;
    }
    ui::confirm("playguard/play_timer/perday/discard_body"_i18n, "playguard/play_timer/perday/discard_confirm"_i18n, [this]() {
        for (int d = 0; d < 7; d++) this->pending[d] = this->pt.day_min[d];
        this->rerender();
    }, nullptr, true);
}

PlayTimerTab::~PlayTimerTab()
{
    play_data::unlisten(this->play_listener);
}

void PlayTimerTab::show_played()
{
    // Every account's play, the last seven days (today's included), on their
    // weekdays: what the week's limits held, as Activity counts it.
    const auto stats = play_data::latest("");
    uint32_t minutes[7] = {};
    const bool ok = stats && stats->windows_ok;
    if (ok)
        for (int k = 0; k < 7; k++) {
            uint64_t s = 0;
            for (uint32_t g = 0; g < stats->count; g++) s += stats->games[g].day_s[k];
            if (stats->day_wday[k] < 7) minutes[stats->day_wday[k]] = (uint32_t)(s / 60);
        }
    for (int d = 0; d < 7; d++) this->played_min[d] = minutes[d];
    this->played_ok = ok;
    this->week_editor->show_played(minutes, ok);
    this->show_played_key(this->week_editor->focused_day());
    if (!play_data::fresh("", std::chrono::minutes(5)) && !play_data::busy("")) play_data::fetch(nullptr);
}

void PlayTimerTab::show_played_key(int day)
{
    // A day focused: its figures (played, against the limit shown); else
    // what the bars' fill is.
    // No day with a limit: nothing is filled, no key.
    bool limits = false;
    for (int d = 0; d < 7; d++) limits = limits || this->pending[d] != PT_DAY_NOLIMIT;
    ui::set_visible(played_key.getView(), this->played_ok && (limits || day >= 0));
    // A day played past its limit: its bar in coral, said by the key.
    bool over = false;
    for (int d = 0; d < 7; d++)
        over = over || (this->pending[d] != PT_DAY_NOLIMIT && this->pending[d] > 0 && this->played_min[d] >= this->pending[d]);
    ui::set_visible(over_key.getView(), this->played_ok && over && day < 0);
    if (!this->played_ok) return;
    std::string text = "playguard/cadran/week/played_key"_i18n;
    if (day >= 0 && day < 7) {
        const std::string played = ui::fmt_played((uint16_t)std::min<uint32_t>(this->played_min[day], 24 * 60));
        // A day after today: what was played on it last week, said so.
        const std::string name = CadranWeekView::last_week(day)
                                     ? brls::getStr("playguard/cadran/week/last_week_day", ui::day_name(day))
                                     : ui::day_name(day);
        text = this->pending[day] == PT_DAY_NOLIMIT
                   ? brls::getStr("playguard/cadran/week/played_day_free", name, played)
                   : brls::getStr("playguard/cadran/week/played_day", name, played, ui::fmt_played(this->pending[day]));
    }
    ui::set_text(played_text, text);
}

void PlayTimerTab::rerender()
{
    brls::Theme theme = brls::Application::getTheme();
    const PtState& pt = this->pt;
    cadran::kit::set_kicker(kicker, "playguard/play_timer/week_title"_i18n);
    summary->setText(pt.valid ? summary_of(this->pending) : "playguard/common/unavailable"_i18n);
    if (pt.valid) week_editor->show(this->pending, pt.day_min);

    // The profile these limits are, else the one the console's are or how
    // many are saved.
    const std::string profile = pt.valid ? profiles::match(this->pending) : "";
    const std::string current = pt.valid ? profiles::match(pt.day_min) : "";
    const size_t count = current.empty() ? profiles::count() : 0;
    const std::string saved = !current.empty() ? current
                              : count == 1 ? "playguard/play_timer/profiles_count_one"_i18n
                              : count ? brls::getStr("playguard/play_timer/profiles_count", (int)count) : "";
    if (!profile.empty()) profile_name->setText(brls::getStr("playguard/cadran/week/profile", profile));
    else if (!saved.empty() && !this->changes()) profile_name->setText("playguard/play_timer/profiles"_i18n + " · " + saved);
    else profile_name->setText("playguard/play_timer/profiles"_i18n);

    // The changes not saved yet: how many, the first one.
    int changed = 0, first = -1;
    for (int d : { 1, 2, 3, 4, 5, 6, 0 })   // Monday first
        if (pt.valid && this->pending[d] != pt.day_min[d]) {
            changed++;
            if (first < 0) first = d;
        }
    if (changed) {
        pending_text->setText((changed == 1 ? "playguard/play_timer/perday/unsaved_count_one"_i18n : brls::getStr("playguard/play_timer/perday/unsaved_count", changed)) + " · " +
                              brls::getStr("playguard/cadran/week/first_change",
                                           brls::getStr(fmt::format("playguard/days_short/{}", first)),
                                           limit_text(pt.day_min[first]), limit_text(this->pending[first])));
        pending_text->setTextColor(theme["cadran/amber"]);
    } else {
        // A bonus planned raises its day for now: said, so that day's bar is
        // understood. Nothing waiting and no bonus: no line.
        const std::string bonus = pt_flow::bonus_text();
        pending_text->setText(bonus);
        pending_text->setTextColor(theme["cadran/text3"]);
    }
    ui::set_visible(this->getView("pt_pending_row"), changed > 0 || !pt_flow::bonus_text().empty());
    this->show_played_key(this->week_editor->focused_day());   // a day over its limit may no longer be
    pending_dot->setVisibility(changed ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    // The buttons go with the changes; the focus they had to the week.
    brls::View* focus = brls::Application::getCurrentFocus();
    if (!changed && (focus == save_button.getView() || focus == discard_button.getView()))
        brls::Application::giveFocus(week_editor.getView());
    ui::set_visible(save_button.getView(), changed > 0);
    ui::set_visible(discard_button.getView(), changed > 0);
    // + saves from anywhere on the tab while there is something to save.
    if (changed && !this->save_action) {
        this->save_action = this->registerAction("", brls::BUTTON_START, [this](brls::View*) {
            this->save();
            return true;
        });
    } else if (!changed && this->save_action) {
        this->unregisterAction(this->save_action);
        this->save_action = 0;
    }
    if (changed) this->updateActionHint(brls::BUTTON_START, brls::getStr("playguard/play_timer/perday/save_hint", changed));
    brls::Application::getGlobalHintsUpdateEvent()->fire();

    // Bedtime: the time and when play is allowed again, every day.
    PtBedtime same;
    std::string start = "—", end;
    if (pt_logic::bedtime_layout_ok(pt, ui::today_weekday())) {
        if (!pt_logic::bedtime_uniform(pt, &same)) start = pt_flow::bedtime_days_text(pt);
        else if (!same.on) start = "playguard/common/off"_i18n;
        else {
            start = fmt::format("{:02d}:{:02d}", same.hour, same.minute);
            end = fmt::format("{:02d}:{:02d}", same.end_hour, same.end_minute);
        }
    } else if (pt.bedtime_valid) {
        start = pt.bedtime_enabled ? fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute) : "playguard/common/off"_i18n;
    }
    // Each day's under the week, when they differ.
    std::string beds[7];
    if (pt_logic::bedtime_layout_ok(pt, ui::today_weekday()) && pt.valid && !pt_logic::bedtime_uniform(pt, nullptr))
        for (int d = 0; d < 7; d++) beds[d] = pt.bed[d].on ? fmt::format("{:02d}:{:02d}", pt.bed[d].hour, pt.bed[d].minute) : "—";
    week_editor->show_bedtimes(beds);
    bed_value->setText(start);
    bed_end->setText(end);
    bed_arrow->setVisibility(end.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    bed_end->setVisibility(end.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    time_up_value->setText(pt_flow::time_up_text(pt));
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
