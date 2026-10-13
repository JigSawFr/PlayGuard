// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_play_timer_tab.hpp"

#include <algorithm>
#include <array>
#include <fmt/format.h>

#include "action/pt_flow.hpp"
#include "activity/profiles_activity.hpp"
#include "app.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"
#include "util/profiles.hpp"

using namespace brls::literals;

namespace
{
std::string range(const uint16_t* v, int n)
{
    uint16_t lo = PT_DAY_NOLIMIT, hi = 0;
    bool none = false, same = true;
    for (int i = 0; i < n; i++) {
        if (v[i] == PT_DAY_NOLIMIT) { none = true; continue; }
        lo = std::min(lo, v[i]);
        hi = std::max(hi, v[i]);
        if (v[i] != v[0]) same = false;
    }
    if (lo == PT_DAY_NOLIMIT) return "pas de limite";
    std::string text = same && !none ? ui::fmt_minutes(v[0]) : ui::fmt_minutes(lo) + " à " + (none ? "pas de limite" : ui::fmt_minutes(hi));
    return text;
}
}   // namespace

CadranPlayTimerTab::CadranPlayTimerTab()
    : TabBase("xml/tab/cadran_play_timer.xml")
{
    week->set_on_change([this](int day, uint16_t minutes) {
        this->pending[day] = minutes;
        this->rerender();
    });
    week->set_on_pick([this](int day) {
        pt_flow::pick_limit(ui::day_name(day) + " : limite", this->pending[day], [this, day](uint16_t v) {
            this->pending[day] = v;
            this->rerender();
        });
    });
    week->set_on_copy([this](int day, bool weekend) {
        const uint16_t v = this->pending[day];
        for (int d = 0; d < 7; d++) {
            const bool is_weekend = d == 0 || d == 6;
            if (is_weekend == weekend) this->pending[d] = v;
        }
        this->rerender();
    });

    profile_chip->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ProfilesActivity());
        return true;
    });
    uniform_chip->registerClickAction([this](brls::View*) {
        pt_flow::choose_uniform_limit(this->live, [this]() { this->refresh(); });
        return true;
    });
    bed_chip->registerClickAction([this](brls::View*) {
        pt_flow::choose_bedtime(this->live, [this]() { this->refresh(); });
        return true;
    });
    pause_chip->registerClickAction([this](brls::View*) {
        pt_flow::set_countdown(!(this->live.enabled_valid && this->live.enabled), [this]() { this->refresh(); });
        return true;
    });
    alarm_chip->registerClickAction([this](brls::View*) {
        pt_flow::set_alarm(pt_flow::alarm_off(this->live), "", [this]() { this->refresh(); });
        return true;
    });
    remove_chip->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::confirm_write("playguard/play_timer/remove_body"_i18n, "playguard/play_timer/remove_confirm"_i18n,
                               [this](bool did_unlock) {
                                   Result rc = pt_flow::clear_days("remove");
                                   pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/removed"_i18n,
                                                         "playguard/play_timer/remove_err"_i18n, [this]() { this->refresh(); });
                               }, nullptr, true);
        return true;
    });
    for (brls::Box* b : { bed_chip.getView(), pause_chip.getView(), alarm_chip.getView(), remove_chip.getView(), profile_chip.getView(), uniform_chip.getView() }) {
        b->setHideHighlightBackground(true);
        for (brls::View* v : b->getChildren())
            if (auto* l = dynamic_cast<brls::Label*>(v)) l->setSingleLine(true);
    }
    // + saves from anywhere on the screen.
    this->registerAction("Enregistrer", brls::BUTTON_START, [this](brls::View*) {
        this->save();
        return true;
    });
}

bool CadranPlayTimerTab::has_changes() const
{
    if (!this->live.valid) return false;
    for (int d = 0; d < 7; d++)
        if (this->pending[d] != this->live.day_min[d]) return true;
    return false;
}

void CadranPlayTimerTab::save()
{
    if (!this->live.valid) {
        ui::notify("playguard/play_timer/perday/unavailable"_i18n);
        return;
    }
    if (!this->has_changes()) {
        ui::notify("playguard/play_timer/perday/no_changes"_i18n);
        return;
    }
    std::array<uint16_t, 7> snapshot;
    for (int d = 0; d < 7; d++) snapshot[d] = this->pending[d];
    pt_flow::confirm_write("", "playguard/play_timer/confirm_set"_i18n, [this, snapshot](bool did_unlock) {
        Result rc = pt_flow::write_days(snapshot.data(), "per_day");
        pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/written_days"_i18n,
                              "playguard/play_timer/write_err"_i18n, [this]() { this->refresh(); });
    }, snapshot.data());
}

void CadranPlayTimerTab::refresh()
{
    // The console's state again; edits not saved yet stay, as in the editor.
    const bool keep = this->has_changes();
    pctl_play_timer_query(&this->live);
    if (this->live.valid && !keep)
        for (int d = 0; d < 7; d++) this->pending[d] = this->live.day_min[d];
    if (this->live.enabled_valid && this->live.enabled) cadran::set_pill("Minuteur actif", "timer");
    else cadran::set_pill("Minuteur en pause", "pause", true);
    this->rerender();
}

void CadranPlayTimerTab::rerender()
{
    brls::Theme theme = brls::Application::getTheme();
    const PtState& pt = this->live;
    if (pt.valid) week->show(this->pending, pt.day_min);

    // The summary: weekdays, then the weekend.
    const uint16_t weekdays[5] = { this->pending[1], this->pending[2], this->pending[3], this->pending[4], this->pending[5] };
    const uint16_t weekend[2] = { this->pending[6], this->pending[0] };
    summary->setText(!pt.valid ? "Limites indisponibles" : range(weekdays, 5) + " en semaine · " + range(weekend, 2) + " le week-end");
    const std::string profile = pt.valid ? profiles::match(this->pending) : "";
    profile_name->setText(profile.empty() ? "Profil · aucun" : "Profil · " + profile);

    // Unsaved edits.
    int changed = 0, first = -1;
    for (int d = 0; d < 7; d++)
        if (pt.valid && this->pending[d] != pt.day_min[d]) { changed++; if (first < 0) first = d; }
    if (changed) {
        pending_text->setText(fmt::format("{} modification{} non enregistrée{} · {} {} → {}", changed, changed > 1 ? "s" : "",
                                          changed > 1 ? "s" : "", ui::day_name_in_text(first).substr(0, 3),
                                          ui::fmt_minutes(pt.day_min[first]), ui::fmt_minutes(this->pending[first])));
        pending_text->setTextColor(theme["cadran/amber"]);
    } else {
        pending_text->setText("Aucune modification en attente · + enregistre");
        pending_text->setTextColor(theme["cadran/text3"]);
    }
    ui::set_visible(pending_dot.getView(), changed > 0);

    // The chips.
    const bool bed = pt.bedtime_valid && pt.bedtime_enabled;
    bed_value->setText(bed ? fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute) : "désactivé");
    bed_end->setText(bed && pt.bedtime_reset_valid ? fmt::format("{:02d}:{:02d}", pt.bedtime_reset_hour, pt.bedtime_reset_minute) : "");
    ui::set_visible(bed_arrow.getView(), bed && pt.bedtime_reset_valid);
    ui::set_visible(bed_end.getView(), bed && pt.bedtime_reset_valid);
    const bool running = pt.enabled_valid && pt.enabled;
    pause_text->setText(running ? "Pause du décompte" : "Reprendre le décompte");
    const bool alarm_on = !pt_flow::alarm_off(pt);
    alarm_state->setText(alarm_on ? "activée" : "désactivée");
    alarm_state->setTextColor(alarm_on ? theme["cadran/teal"] : theme["cadran/amber"]);
}

brls::View* CadranPlayTimerTab::create() { return new CadranPlayTimerTab(); }
