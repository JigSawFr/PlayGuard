// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_play_timer_tab.hpp"

#include <algorithm>
#include <array>
#include <fmt/format.h>

#include "action/play_data.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/profiles.hpp"

using namespace brls::literals;

namespace
{
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

CadranPlayTimerTab::CadranPlayTimerTab()
    : list(this)
{
    namespace kit = cadran::kit;
    brls::Box* column = this->list.content();
    this->hero = brls::View::createFromXMLResource("view/cadran_play_timer.xml");
    static_cast<brls::Box*>(this->hero->getView("cpt_week_card"))->addView(brls::View::createFromXMLResource("view/cadran_week_card.xml"));
    // Under the read-only note, the unlock banner and the timer's state (its
    // warnings: the limit reached, a timer that does not count).
    int at = 0;
    for (const char* id : { "tab_ro_note", "pt_unlocked_banner", "pt_state_header" }) {
        const auto& lines = column->getChildren();
        for (size_t i = 0; i < lines.size(); i++)
            if (lines[i] == this->getView(id)) at = std::max(at, (int)i + 1);
    }
    column->addView(this->hero, at);
    kit::wrap(this->getView("cpt_chips"));
    this->list.park({ "pt_week_header", "pt_week", "pt_quick", "pt_profiles", "pt_time_up", "pt_remove", "pt_bedtime" });
    this->list.apply({ { "pt_unlocked_banner", "unlock" }, { "pt_extra", "plus" }, { "pt_stop", "stop" },
                       { "pt_split", "calendar" }, { "pt_bedtime_reset", "moon" }, { "pt_extra_amounts", "plus" },
                       { "pt_extra_auto", "refresh" }, { "pt_advanced", "settings" } });
    this->list.banners({ "pt_unlocked_banner" });
    // The timer's state is the header's pill and the week's summary now; its
    // warnings stay.
    if (brls::View* state = this->getView("pt_state_header")->getView("pt_summary")) state->setVisibility(brls::Visibility::GONE);

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

    // The chips: the tab's own lines, from where they are shown now.
    const std::pair<brls::Box*, const char*> chips[] = {
        { profile_chip, "pt_profiles" }, { uniform_chip, "pt_quick" }, { bed_chip, "pt_bedtime" },
        { time_up_chip, "pt_time_up" }, { remove_chip, "pt_remove" },
    };
    for (const auto& [chip, line] : chips) {
        const std::string id = line;
        chip->registerClickAction([this, id](brls::View*) {
            kit::press(this->getView(id));
            return true;
        });
    }
}

bool CadranPlayTimerTab::changes() const
{
    if (!this->pt.valid) return false;
    for (int d = 0; d < 7; d++)
        if (this->pending[d] != this->pt.day_min[d]) return true;
    return false;
}

void CadranPlayTimerTab::save()
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

void CadranPlayTimerTab::discard()
{
    if (!this->changes()) {
        ui::notify("playguard/play_timer/perday/nothing_to_discard"_i18n);
        return;
    }
    ui::confirm("playguard/play_timer/perday/discard_body"_i18n, "playguard/play_timer/perday/discard_confirm"_i18n, [this]() {
        for (int d = 0; d < 7; d++) this->pending[d] = this->pt.day_min[d];
        this->rerender();
    });
}

CadranPlayTimerTab::~CadranPlayTimerTab()
{
    play_data::unlisten(this->play_listener);
}

void CadranPlayTimerTab::show_played()
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

void CadranPlayTimerTab::show_played_key(int day)
{
    // A day focused: its figures (played, against the limit shown); else
    // what the bars' fill is.
    // No day with a limit: nothing is filled, no key.
    bool limits = false;
    for (int d = 0; d < 7; d++) limits = limits || this->pending[d] != PT_DAY_NOLIMIT;
    ui::set_visible(played_key.getView(), this->played_ok && (limits || day >= 0));
    if (!this->played_ok) return;
    std::string text = "playguard/cadran/week/played_key"_i18n;
    if (day >= 0 && day < 7) {
        const std::string played = ui::fmt_played((uint16_t)std::min<uint32_t>(this->played_min[day], 24 * 60));
        text = this->pending[day] == PT_DAY_NOLIMIT
                   ? brls::getStr("playguard/cadran/week/played_day_free", ui::day_name(day), played)
                   : brls::getStr("playguard/cadran/week/played_day", ui::day_name(day), played, ui::fmt_played(this->pending[day]));
    }
    ui::set_text(played_text, text);
}

void CadranPlayTimerTab::refresh()
{
    // The changes not saved yet stay over a new read, as in the per-day editor.
    const bool keep = this->changes();
    PlayTimerTab::refresh();
    this->list.sync();
    if (this->pt.valid && !keep)
        for (int d = 0; d < 7; d++) this->pending[d] = this->pt.day_min[d];
    ui::set_visible(this->hero, this->pt.fw_supported);
    if (!this->pt.fw_supported) cadran::set_pill("playguard/play_timer/fw_too_old_short"_i18n, "alert", true);
    else if (!this->pt.valid || !pt_configured(this->pt.block)) cadran::set_pill("playguard/cadran/pill/timer_none"_i18n, "timer");
    else if (this->pt.enabled_valid && this->pt.enabled) cadran::set_pill("playguard/cadran/pill/timer_on"_i18n, "timer");
    else cadran::set_pill("playguard/cadran/pill/timer_off"_i18n, "pause", true);
    for (const auto& [chip, line] : { std::pair<brls::View*, const char*>{ profile_chip, "pt_profiles" },
                                      { uniform_chip, "pt_quick" }, { bed_chip, "pt_bedtime" },
                                      { time_up_chip, "pt_time_up" }, { remove_chip, "pt_remove" } })
        cadran::kit::follow(chip, this->getView(line));
    this->rerender();
    if (this->play_listener < 0) this->play_listener = play_data::listen([this]() { this->show_played(); });
    this->show_played();
}

void CadranPlayTimerTab::rerender()
{
    brls::Theme theme = brls::Application::getTheme();
    const PtState& pt = this->pt;
    kicker->setText(cadran::upper("playguard/play_timer/week_title"_i18n));
    summary->setText(pt.valid ? summary_of(this->pending) : "playguard/common/unavailable"_i18n);
    if (pt.valid) week_editor->show(this->pending, pt.day_min);

    // The profile these limits are, else how many are saved.
    const std::string profile = pt.valid ? profiles::match(this->pending) : "";
    auto* profiles_line = dynamic_cast<brls::DetailCell*>(this->getView("pt_profiles"));
    const std::string saved = profiles_line ? cadran::kit::without_chevron(profiles_line->detail->getFullText()) : "";
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
        pending_text->setText(brls::getStr("playguard/play_timer/perday/unsaved_count", changed) + " · " +
                              brls::getStr("playguard/cadran/week/first_change",
                                           brls::getStr(fmt::format("playguard/days_short/{}", first)),
                                           limit_text(pt.day_min[first]), limit_text(this->pending[first])));
        pending_text->setTextColor(theme["cadran/amber"]);
    } else {
        pending_text->setText("playguard/cadran/week/no_changes"_i18n);
        pending_text->setTextColor(theme["cadran/text3"]);
    }
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

brls::View* CadranPlayTimerTab::create() { return new CadranPlayTimerTab(); }
