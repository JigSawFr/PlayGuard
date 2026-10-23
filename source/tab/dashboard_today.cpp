// DashboardTab — Today's time: the log read when the timer says nothing, the headline over
// the ring, the ring, what stops play next, the text size, the profile and
// today's limit.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <algorithm>
#include <array>
#include <fmt/format.h>

#include "action/play_data.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/profiles.hpp"

using namespace brls::literals;

DashboardTab::TodayLog DashboardTab::read_today_log()
{
    // Today's play time from the activity log, for when the timer does not
    // say (no game counted yet today, no limit today). Read again in the
    // background when older than a few minutes.
    TodayLog out;
    const auto log = play_data::latest("");
    out.ok = log && log->windows_ok;
    out.played_s = out.ok ? play_data::today_total_s(*log) : 0;
    if (!play_data::fresh("", std::chrono::minutes(5)) && !play_data::busy("")) play_data::fetch(nullptr);
    out.played_min = (uint16_t)std::min<uint64_t>(1440, (out.played_s + 30) / 60);
    return out;
}

// The time played today in words, over the ring's buttons.
void DashboardTab::show_headline(int today, bool unlocked, const TodayLog& log)
{
    const PtState& pt = this->pt;
    if (!pt.fw_supported) {
        ui::set_text(headline, "playguard/play_timer/fw_too_old_short"_i18n);
    } else if (!pt.valid || !pt.enabled_valid) {
        ui::set_text(headline, "playguard/dashboard/gauge_unknown"_i18n);
    } else {
        const pt_logic::RingState st = pt_logic::ring_state(pt, today, unlocked);
        const uint16_t limit = st.limit;
        switch (st.kind) {
            case pt_logic::RingState::NO_PLAY:
                // A day without play in words ("0 min (no play)" said it twice).
                ui::set_text(headline, st.unlocked ? "playguard/cadran/today/head_unlocked_no_play"_i18n
                                                   : "playguard/cadran/today/head_no_play"_i18n);
                break;
            case pt_logic::RingState::FREE:
                ui::set_text(headline, log.ok ? brls::getStr("playguard/dashboard/gauge_none_played", ui::fmt_play_time(log.played_s))
                                              : "playguard/dashboard/gauge_none"_i18n);
                break;
            case pt_logic::RingState::ON_HOLD:
                // The countdown may be off while unlocked: the limit is on hold.
                ui::set_text(headline, brls::getStr("playguard/cadran/today/head_unlocked", ui::fmt_minutes(limit)));
                break;
            case pt_logic::RingState::TIME_UP:
                if (st.over < 0 || (pt.restricted_valid && pt.restricted)) {
                    ui::set_text(headline, "playguard/dashboard/gauge_reached"_i18n);
                } else {
                    // Past the limit, the console only warning: the time over it.
                    ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_over", ui::fmt_played((uint16_t)st.played),
                                                        ui::fmt_minutes(limit), ui::fmt_played((uint16_t)st.over)));
                }
                break;
            case pt_logic::RingState::COUNTING:
                ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_known", ui::fmt_played((uint16_t)st.played),
                                                    ui::fmt_minutes(limit)));
                break;
            case pt_logic::RingState::IDLE:
                if (log.ok) {
                    // No game counted yet by the timer: the log's figure, marked as an
                    // estimate (it counts PlayGuard opened over a game, for instance).
                    ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_log", ui::fmt_play_time(log.played_s),
                                                        ui::fmt_minutes(limit)));
                } else {
                    ui::set_text(headline, "playguard/dashboard/gauge_idle"_i18n);
                }
                break;
            case pt_logic::RingState::UNKNOWN:   // said above
                break;
        }
    }
}

void DashboardTab::show_ring(bool unlocked)
{
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    // Larger text: the day and its limit only ("THURSDAY · 2 h"), on one line.
    if (brls::Label::getFontScale() > 1.0f) {
        const uint16_t limit = pt.valid ? pt.day_min[today] : PT_DAY_NOLIMIT;
        const std::string value = !pt.valid ? "—"
                                  : limit == 0 && (unlocked || pt.enabled) ? "playguard/cadran/today/no_play"_i18n
                                  : limit == PT_DAY_NOLIMIT || !pt.enabled ? "playguard/cadran/today/free"_i18n
                                                                           : ui::fmt_minutes(limit);
        // The day in capitals, as the kicker; the limit as everywhere else.
        kicker->setText(brls::getStr("playguard/cadran/today/kicker_short", cadran::upper(ui::day_name(today)), value));
    } else {
        kicker->setText(cadran::upper(brls::getStr("playguard/cadran/today/kicker", ui::day_name_in_text(today))));
    }
    const pt_logic::RingState st = pt_logic::ring_state(pt, today, unlocked);
    std::string left = "—", caption;
    switch (st.kind) {
        case pt_logic::RingState::NO_PLAY:
            // A day without play: said in words, not as "0 min".
            left = "playguard/cadran/today/no_play"_i18n;
            caption = st.unlocked ? "playguard/cadran/today/unlocked"_i18n : "playguard/cadran/today/today"_i18n;
            break;
        case pt_logic::RingState::ON_HOLD:
            left = ui::fmt_minutes(st.limit);
            caption = "playguard/cadran/today/unlocked"_i18n;
            break;
        case pt_logic::RingState::FREE:
            left = "playguard/cadran/today/free"_i18n;
            caption = "playguard/cadran/today/no_limit"_i18n;
            break;
        case pt_logic::RingState::TIME_UP:
            left = ui::fmt_played(0);
            caption = "playguard/cadran/today/time_up"_i18n;
            break;
        case pt_logic::RingState::COUNTING:
            left = ui::fmt_duration_ns((uint64_t)pt.remaining_ns);
            caption = "playguard/cadran/today/left"_i18n;
            break;
        case pt_logic::RingState::IDLE:
            left = ui::fmt_minutes(st.limit);
            caption = "playguard/cadran/today/today"_i18n;
            break;
        case pt_logic::RingState::UNKNOWN:
            break;
    }
    // Nothing counted yet: the track alone (the child screen's ring shows it
    // whole, as the time left).
    ring->setFraction(st.kind == pt_logic::RingState::IDLE ? -1.0f : st.fraction);
    // Today's extra time, the end of today's limit.
    const uint16_t limit = pt.valid ? pt.day_min[today] : PT_DAY_NOLIMIT;
    const int extra = pt.valid ? pt_flow::extra_today_minutes(pt) : 0;
    ring->setExtra(!unlocked && extra > 0 && limit != PT_DAY_NOLIMIT && limit > 0 ? (float)extra / (float)limit : 0.0f);
    ring->setValue(left);
    ring->setCaption(caption);
}

void DashboardTab::show_next(bool unlocked)
{
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    pt_logic::NextStop n;
    bool with_extra = false;
    int left = -1, now_min = 0;
    bool no_limit = false;
    // Unlocked, or the timer off: nothing stops play.
    if (!unlocked && pt.fw_supported && pt.valid && pt.enabled_valid && pt.enabled) {
        const uint16_t limit = pt.day_min[today];
        no_limit = limit == PT_DAY_NOLIMIT;
        const int over = pt_logic::over_limit_min(pt, today);
        if (limit != PT_DAY_NOLIMIT && pt.remaining_valid && pt.remaining_ns > 0 && over < 0)
            left = (int)((uint64_t)pt.remaining_ns / 60000000000ULL);
        // A chip's amount added to the limit (24 h at most, as add_extra_minutes
        // writes it), from the time left or the time over it.
        if (this->preview_extra > 0 && limit != PT_DAY_NOLIMIT && pt.remaining_valid) {
            const int base  = over >= 0 ? -over : std::max(0, left);
            const int added = std::min(24 * 60, limit + this->preview_extra) - limit;
            if (base + added > 0) {
                left       = base + added;
                with_extra = true;
            }
        }
        const LocalTime now = ui::local_now();
        now_min = now.hour * 60 + now.minute;
        n = pt_logic::next_stop(now_min, left, pt.bed[today]);
    }
    const auto clock = [](int minutes) { return fmt::format("{:02d}:{:02d}", minutes / 60, minutes % 60); };
    const std::string amount = ui::fmt_minutes((uint16_t)this->preview_extra);
    std::string text;
    const char* glyph = n.kind == pt_logic::NextStop::TIME_UP ? "timer" : "moon";
    if (unlocked && pt.valid && pt.day_min[today] != PT_DAY_NOLIMIT) {
        // Nothing stops play while unlocked: the headline says the limit is
        // on hold, this line why.
        text  = "playguard/dashboard/gauge_unlocked"_i18n;
        glyph = "unlock";
    } else if (with_extra && n.kind == pt_logic::NextStop::TIME_UP)
        text = brls::getStr("playguard/cadran/today/next_with_extra", amount, clock(n.at));
    else if (with_extra && n.kind == pt_logic::NextStop::BEDTIME)
        text = brls::getStr("playguard/cadran/today/next_with_extra_bed", amount, clock(n.at));
    else if (n.kind == pt_logic::NextStop::TIME_UP) text = brls::getStr("playguard/cadran/today/next_time_up", clock(n.at));
    else if (n.kind == pt_logic::NextStop::BEDTIME)
        text = brls::getStr("playguard/cadran/today/next_bedtime", clock(n.at), ui::fmt_minutes((uint16_t)n.in));
    else if (n.kind == pt_logic::NextStop::BEDTIME_NOW) text = brls::getStr("playguard/cadran/today/next_bedtime_now", clock(n.at));
    if (!text.empty()) {
        ui::set_text(this->next_text, text);
        this->next_icon->setIcon(glyph);
    }
    // Before the time up or the bedtime alarm, the rest of the day as a line
    // in place of the words: now, then each of them that is today's. Its
    // teal says play is allowed until then: not once the day's time is
    // spent (a warning only, no play today), where the words stay.
    const bool line = n.kind == pt_logic::NextStop::TIME_UP ||
                      (n.kind == pt_logic::NextStop::BEDTIME && (no_limit || left >= 0));
    if (line) {
        const PtBedtime& bed = pt.bed[today];
        const int start = bed.hour * 60 + bed.minute, end = bed.end_hour * 60 + bed.end_minute;
        const int bed_at = bed.on && start > end && now_min < start ? start : -1;
        int up = left >= 0 && now_min + left < 24 * 60 ? now_min + left : -1;
        if (up >= 0 && bed_at >= 0 && up >= bed_at) up = -1;   // the alarm comes first
        // An amount focused that the alarm would cut short: the alarm says so.
        const bool bed_first = with_extra && n.kind == pt_logic::NextStop::BEDTIME;
        this->timeline->show(now_min, up, bed_at,
                             with_extra ? brls::getStr("playguard/cadran/today/tl_with", amount)
                                        : "playguard/cadran/today/tl_time_up"_i18n,
                             bed_first ? "playguard/cadran/today/tl_bed_first"_i18n : "");
    }
    ui::set_visible(this->timeline, line);
    ui::set_visible(this->next, !text.empty() && !line);
}

void DashboardTab::fit_text_size()
{
    // Larger text (on the TV): the same screen in the same height. The kicker
    // shortened to the day and its limit (show_ring()), the buttons and the
    // cards a little tighter.
    const bool larger = brls::Label::getFontScale() > 1.0f;
    kicker->setMarginBottom(larger ? 2 : 10);
    actions->setMarginTop(larger ? 6 : 14);
    actions->setMarginBottom(larger ? 4 : 12);
    for (brls::View* button : { stop.getView(), child.getView(), limit.getView(), profile.getView() })
        button->setHeight(larger ? 40 : 48);
    for (brls::View* card : { bed_card.getView(), pc_card.getView(), clock_card.getView() }) {
        static_cast<brls::Box*>(card)->setPaddingTop(larger ? 10 : 20);
        static_cast<brls::Box*>(card)->setPaddingBottom(larger ? 10 : 20);
    }
}

void DashboardTab::show_profile()
{
    // Once there is a profile: the one in force by its name, else the list.
    const bool any = profiles::count() > 0 && this->pt.fw_supported && this->pt.valid;
    if (any) {
        const std::string current = profiles::match(this->pt.day_min);
        const std::string text = current.empty() ? "playguard/cadran/today/profiles"_i18n : current;
        if (profile->getText() != text) profile->setText(text);
    }
    ui::set_visible(profile.getView(), any);
}

void DashboardTab::choose_today_limit()
{
    // Not writable (read-only, an unknown state): the Play timer, which says why.
    if (app::read_only() || !this->pt.fw_supported || !this->pt.valid) {
        ui::go_to_tab(limit.getView(), ui::tab::play_timer);
        return;
    }
    // Right here, without the per-day editor: the day's picker, then the one
    // confirmation the week's save asks (the day before → after).
    const int today = ui::today_weekday();
    std::array<uint16_t, 7> days;
    for (int d = 0; d < 7; d++) days[d] = this->pt.day_min[d];
    pt_flow::pick_limit(brls::getStr("playguard/play_timer/perday/pick_title", ui::day_name(today)), days[today],
                        [this, today, days](uint16_t v) mutable {
        if (v == days[today]) return;
        days[today] = v;
        pt_flow::confirm_write("", "playguard/play_timer/confirm_set"_i18n, [this, days](bool did_unlock) {
            Result rc = pt_flow::write_days(days.data(), "per_day");
            pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/written_days"_i18n,
                                  "playguard/play_timer/write_err"_i18n, [this]() { this->refresh(); });
        }, days.data(), false, brls::getStr("playguard/cadran/today/limit_title", ui::day_name_in_text(today)));
    });
}
