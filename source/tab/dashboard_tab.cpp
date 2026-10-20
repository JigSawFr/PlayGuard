// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fmt/format.h>
#include <utility>
#include <yoga/Yoga.h>

#include "action/clock_flow.hpp"
#include "action/console_actions.hpp"
#include "action/nintendo_dns.hpp"
#include "action/notices.hpp"
#include "action/outside_watch.hpp"
#include "action/pairing_state.hpp"
#include "action/play_data.hpp"
#include "action/profile_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "action/timer_health.hpp"
#include "activity/cadran_child_activity.hpp"
#include "activity/main_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "tab/clock_tab.hpp"
#include "ui/cadran_text.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"
#include "util/profiles.hpp"
#include "util/text.hpp"

using namespace brls::literals;

DashboardTab::DashboardTab()
    : TabBase("xml/tab/dashboard.xml"), list(this)
{
    namespace kit = cadran::kit;
    brls::Box* column = this->list.content();
    applet->setSingleLine(false);
    not_counting->setSingleLine(false);
    // Above the top, what needs the parent first: the first steps, then what
    // changed outside PlayGuard.
    const auto& lines = column->getChildren();
    const int at = (int)(std::find(lines.begin(), lines.end(), this->getView("dash_top")) - lines.begin());
    YGNodeStyleSetFlexWrap(pills->getYGNode(), YGWrapWrap);
    this->build_outside_card(column, at);
    this->build_steps_card(column, at);

    // What stops play first, under the time played.
    this->next = new brls::Box(brls::Axis::ROW);
    this->next->setAlignItems(brls::AlignItems::FLEX_START);
    this->next->setAlignSelf(brls::AlignSelf::STRETCH);   // the column's width, for its text to wrap in
    this->next->setMarginTop(8);
    this->next_icon = kit::icon("timer", 18, "cadran/teal");
    this->next_icon->setMarginRight(8);
    this->next_icon->setMarginTop(2);
    this->next_icon->setShrink(0);
    this->next->addView(this->next_icon);
    // A second line when it has to (the unlocked case says why at length).
    this->next_text = kit::label("", 16.5f, "cadran/text2");
    this->next_text->setSingleLine(false);
    this->next_text->setGrow(1.0f);
    this->next_text->setWidth(0);
    this->next->addView(this->next_text);
    this->next->setVisibility(brls::Visibility::GONE);
    today_text->addView(this->next, 2);
    // Or, before the time up or the bedtime alarm, the rest of the day as a line.
    this->timeline = new CadranTimeline();
    this->timeline->setAlignSelf(brls::AlignSelf::STRETCH);
    this->timeline->setMarginTop(14);
    this->timeline->setMarginBottom(4);
    this->timeline->setVisibility(brls::Visibility::GONE);
    today_text->addView(this->timeline, 3);
    this->list.apply({ { "dash_extra_pending", "refresh" }, { "dash_alarm", "bell" } });
    extra_pending->registerClickAction([this](brls::View*) {
        pt_flow::offer_extra_time_restore([this]() { this->refresh(); }, true);
        return true;
    });
    // Shown only while the alarm is off: A turns it back on (the dialog says why).
    ui::set_detail(alarm, "playguard/dashboard/alarm_off"_i18n);
    alarm->setDetailTextColor(ui::color_warn());
    alarm->registerClickAction([this](brls::View*) {
        pt_flow::turn_alarm_on("overview", [this]() { this->refresh(); });
        return true;
    });
    this->enable_auto_refresh(5000);
    this->listener = play_data::listen([this]() { this->refresh(); });
    this->build_pills();

    // The buttons' icons, before their label.
    CadranIcon* plus = kit::icon("plus", 20, "cadran/navy");
    plus->setMarginRight(10);
    extra->addView(plus, 0);
    extra->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* square = kit::icon("stop", 18, "cadran/coral");
    square->setMarginRight(10);
    stop->addView(square, 0);
    stop->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* eye = kit::icon("eye", 18, "cadran/text2");
    eye->setMarginRight(10);
    child->addView(eye, 0);
    child->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* lock = kit::icon("lock", 20, "cadran/navy");
    lock->setMarginRight(10);
    relock->addView(lock, 0);
    relock->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* week = kit::icon("timer", 20, "cadran/navy");
    week->setMarginRight(10);
    setup->addView(week, 0);
    setup->setAlignItems(brls::AlignItems::CENTER);
    // No limit yet: the Play timer, on its week, where the days are set.
    setup->registerClickAction([](brls::View* view) {
        if (MainActivity* main = ui::main_activity()) main->open_tab(ui::tab::play_timer, "cpt_week");
        else ui::go_to_tab(view, ui::tab::play_timer);
        return true;
    });
    CadranIcon* timer = kit::icon("timer", 18, "cadran/text2");
    timer->setMarginRight(10);
    limit->addView(timer, 0);
    limit->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* list_icon = kit::icon("list", 18, "cadran/text2");
    list_icon->setMarginRight(10);
    profile->addView(list_icon, 0);
    profile->setAlignItems(brls::AlignItems::CENTER);
    if (auto* text = dynamic_cast<brls::Label*>(profile->getView("brls/button/label"))) text->setSingleLine(true);
    // The saved profiles, the one in force checked: one press to change week.
    profile->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        profile_flow::pick([this]() { this->refresh(); });
        return true;
    });
    // Stop, the child screen, today's limit, the profile: on a second line
    // when a language needs the room.
    kit::wrap(actions2.getView());
    YGNodeStyleSetGap(actions2->getYGNode(), YGGutterColumn, 12);
    YGNodeStyleSetGap(actions2->getYGNode(), YGGutterRow, 10);
    // Wrapped, the row's buttons are one line to borealis: ↓ would leave it
    // past the last button, on the line under. Between its lines, ↑ and ↓ go
    // to the nearest button of the next line.
    const std::vector<brls::View*> row2 = { stop.getView(), child.getView(), limit.getView(), profile.getView() };
    const auto line_move = [row2](brls::View* from, bool down) {
        brls::View* best = nullptr;
        float best_line = 0, best_dx = 0;
        const float cx = from->getX() + from->getWidth() / 2;
        for (brls::View* to : row2) {
            if (to == from || to->getVisibility() != brls::Visibility::VISIBLE) continue;
            const float dy = to->getY() - from->getY();
            if (down ? dy <= 1 : dy >= -1) continue;
            const float dx = std::abs(to->getX() + to->getWidth() / 2 - cx);
            const float line = std::abs(dy);
            if (!best || line < best_line - 1 || (std::abs(line - best_line) <= 1 && dx < best_dx)) {
                best = to;
                best_line = line;
                best_dx = dx;
            }
        }
        if (!best) return false;
        brls::Application::giveFocus(best);
        return true;
    };
    for (brls::View* button : row2) {
        button->registerAction("", brls::BUTTON_NAV_DOWN, [line_move](brls::View* view) { return line_move(view, true); }, true);
        button->registerAction("", brls::BUTTON_NAV_UP, [line_move](brls::View* view) { return line_move(view, false); }, true);
    }

    relock->registerClickAction([this](brls::View*) {
        console_actions::relock("", [this]() { this->refresh(); });
        return true;
    });
    limit->registerClickAction([this](brls::View*) {
        this->choose_today_limit();
        return true;
    });
    extra->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::add_extra_time(this->pt, [this]() { this->refresh(); });
        return true;
    });
    for (int minutes : config::get().extra_amounts) {
        brls::Box* amount = kit::chip("+" + ui::fmt_minutes((uint16_t)minutes));
        amount->setMarginRight(10);
        amount->registerClickAction([this, minutes](brls::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_minutes(this->pt, (uint16_t)minutes, [this]() { this->refresh(); });
            return true;
        });
        // Focused, the next stop's line says when play would end with it.
        amount->getFocusEvent()->subscribe([this, minutes](brls::View*) {
            this->preview_extra = minutes;
            this->show_next(this->unlocked_now);
        });
        amount->getFocusLostEvent()->subscribe([this](brls::View*) {
            this->preview_extra = 0;
            this->show_next(this->unlocked_now);
        });
        chips->addView(amount);
        this->amount_chips.push_back(amount);
    }
    stop->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::stop_today(this->pt, [this]() { this->refresh(); });
        return true;
    });
    child->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new CadranChildActivity());
        return true;
    });
    // The cards: where each is set.
    bed_card->registerClickAction([](brls::View* view) {
        ui::go_to_tab(view, ui::tab::play_timer);
        return true;
    });
    pc_card->registerClickAction([](brls::View* view) {
        ui::go_to_tab(view, ui::tab::security);
        return true;
    });
    // Inaccurate and writable: the guided measure-and-set path; else the clock's screen.
    clock_card->registerClickAction([this](brls::View*) {
        if (this->clock_inaccurate && !app::read_only()) clock_flow::guided([this]() { this->refresh(); });
        else ClockTab::open_screen();
        return true;
    });
}

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
        const uint16_t limit = pt.day_min[today];
        const int over = pt_logic::over_limit_min(pt, today);
        if (limit == 0 && (unlocked || pt.enabled)) {
            // A day without play in words ("0 min (no play)" said it twice).
            ui::set_text(headline, unlocked ? "playguard/cadran/today/head_unlocked_no_play"_i18n
                                            : "playguard/cadran/today/head_no_play"_i18n);
        } else if (limit != PT_DAY_NOLIMIT && unlocked) {
            // The countdown may be off while unlocked: the limit is on hold.
            ui::set_text(headline, brls::getStr("playguard/cadran/today/head_unlocked", ui::fmt_minutes(limit)));
        } else if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            ui::set_text(headline, log.ok ? brls::getStr("playguard/dashboard/gauge_none_played", ui::fmt_play_time(log.played_s))
                                          : "playguard/dashboard/gauge_none"_i18n);
        } else if (pt.restricted_valid && pt.restricted) {
            ui::set_text(headline, "playguard/dashboard/gauge_reached"_i18n);
        } else if (over >= 0) {
            // Past the limit, the console only warning: the time over it.
            ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_over", ui::fmt_played((uint16_t)(limit + over)),
                                                ui::fmt_minutes(limit), ui::fmt_played((uint16_t)over)));
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left = (uint64_t)pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left >= limit ? 0 : (uint16_t)(limit - left);
            ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_known", ui::fmt_played(used), ui::fmt_minutes(limit)));
        } else if (log.ok) {
            // No game counted yet by the timer: the log's figure, marked as an
            // estimate (it counts PlayGuard opened over a game, for instance).
            ui::set_text(headline, brls::getStr("playguard/dashboard/gauge_log", ui::fmt_play_time(log.played_s),
                                                ui::fmt_minutes(limit)));
        } else {
            ui::set_text(headline, "playguard/dashboard/gauge_idle"_i18n);
        }
    }
}

void DashboardTab::show_ring(bool unlocked)
{
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    kicker->setText(cadran::upper(brls::getStr("playguard/cadran/today/kicker", ui::day_name_in_text(today))));
    float frac = -1.0f;
    std::string left = "—", caption;
    if (pt.fw_supported && pt.valid && pt.enabled_valid) {
        const uint16_t limit = pt.day_min[today];
        const int over = pt_logic::over_limit_min(pt, today);
        // Unlocked first: the console reports the timer off while
        // unlocked, though today's limit still stands.
        if (limit == 0) {
            // A day without play: said in words, not as "0 min".
            frac = unlocked ? -1.0f : 1.0f;
            left = "playguard/cadran/today/no_play"_i18n;
            caption = unlocked ? "playguard/cadran/today/unlocked"_i18n : "playguard/cadran/today/today"_i18n;
        } else if (unlocked && limit != PT_DAY_NOLIMIT) {
            left = ui::fmt_minutes(limit);
            caption = "playguard/cadran/today/unlocked"_i18n;
        } else if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            left = "playguard/cadran/today/free"_i18n;
            caption = "playguard/cadran/today/no_limit"_i18n;
        } else if (over >= 0 || (pt.restricted_valid && pt.restricted)) {
            frac = 1.0f;
            left = ui::fmt_played(0);
            caption = "playguard/cadran/today/time_up"_i18n;
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left_min = (uint64_t)pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left_min >= limit ? 0 : (uint16_t)(limit - left_min);
            frac = (float)used / (float)limit;
            left = ui::fmt_duration_ns((uint64_t)pt.remaining_ns);
            caption = "playguard/cadran/today/left"_i18n;
        } else {
            left = ui::fmt_minutes(limit);
            caption = "playguard/cadran/today/today"_i18n;
        }
    }
    ring->setFraction(frac);
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
    // goes (the headline and the line under it say the day's limit), the
    // buttons and the cards are a little tighter.
    const bool larger = brls::Label::getFontScale() > 1.0f;
    ui::set_visible(kicker.getView(), !larger);
    actions->setMarginTop(larger ? 10 : 14);
    actions->setMarginBottom(larger ? 8 : 12);
    for (brls::View* button : { stop.getView(), child.getView(), limit.getView(), profile.getView() })
        button->setHeight(larger ? 44 : 48);
    for (brls::View* card : { bed_card.getView(), pc_card.getView(), clock_card.getView() }) {
        static_cast<brls::Box*>(card)->setPaddingTop(larger ? 14 : 20);
        static_cast<brls::Box*>(card)->setPaddingBottom(larger ? 14 : 20);
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

void DashboardTab::build_outside_card(brls::Box* column, int at)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    // What changed outside PlayGuard, as one card above the ring: its title,
    // the changes one under the other, Ⓐ to dismiss it.
    auto* card = kit::card(true, false);
    card->setAxis(brls::Axis::ROW);
    card->setAlignItems(brls::AlignItems::FLEX_START);
    card->setBackgroundColor(theme["cadran/amber_surf"]);
    card->setBorderColor(theme["cadran/amber_line"]);
    card->setPadding(18, 24, 18, 20);
    card->setMarginBottom(16);
    card->setVisibility(brls::Visibility::GONE);
    CadranIcon* icon = kit::icon("alert", 22, "cadran/amber");
    icon->setMarginTop(1);
    icon->setMarginRight(14);
    icon->setShrink(0);
    card->addView(icon);
    auto* text = new brls::Box(brls::Axis::COLUMN);
    text->setGrow(1.0f);
    text->setWidth(0);
    text->addView(kit::label("playguard/notices/outside"_i18n, 18, "cadran/amber"));
    this->outside_lines = new brls::Box(brls::Axis::COLUMN);
    this->outside_lines->setMarginTop(6);
    text->addView(this->outside_lines);
    // Ⓐ and what it does, as on the network clock's card.
    auto* hint = new brls::Box(brls::Axis::ROW);
    hint->setAlignItems(brls::AlignItems::CENTER);
    hint->setMarginTop(10);
    brls::Box* key = kit::key("A", 22);
    key->setMarginRight(8);
    hint->addView(key);
    hint->addView(kit::label("playguard/dashboard/outside_dismiss"_i18n, 15, "cadran/text2"));
    text->addView(hint);
    card->addView(text);
    card->registerClickAction([this](brls::View*) {
        outside_watch::dismiss();
        this->refresh();
        return true;
    });
    column->addView(card, at);
    this->outside_card = card;
}

void DashboardTab::build_steps_card(brls::Box* column, int at)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    auto* card = kit::card(false, false);
    card->setPadding(16, 16, 10, 16);
    card->setMarginBottom(16);
    card->setVisibility(brls::Visibility::GONE);
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    head->setPadding(0, 8, 6, 8);
    CadranIcon* icon = kit::icon("sparkle", 22, "cadran/teal");
    icon->setMarginRight(12);
    head->addView(icon);
    brls::Label* title = kit::label("playguard/onboarding/title"_i18n, 18);
    title->setGrow(1.0f);
    head->addView(title);
    this->steps_count = kit::label("", 15, "cadran/text3");
    head->addView(this->steps_count);
    card->addView(head);

    // The onboarding's steps, their words; Ⓐ does each one as it does.
    const char* words[3] = { "playguard/onboarding/step_pin", "playguard/onboarding/step_limit",
                             "playguard/onboarding/step_clock" };
    for (int i = 0; i < 3; i++) {
        Step& s = this->steps[i];
        s.row = new brls::Box(brls::Axis::ROW);
        s.row->setAlignItems(brls::AlignItems::CENTER);
        s.row->setPadding(10, 8, 10, 8);
        s.row->setFocusable(true);
        s.row->setHideHighlightBackground(true);
        s.row->setHighlightCornerRadius(12);
        s.done = kit::icon("check", 20, "cadran/teal");
        s.done->setMarginRight(12);
        s.row->addView(s.done);
        s.todo = new brls::Box();
        s.todo->setWidth(20);
        s.todo->setHeight(20);
        s.todo->setCornerRadius(10);
        s.todo->setBorderColor(theme["cadran/amber"]);
        s.todo->setBorderThickness(2);
        s.todo->setMarginRight(12);
        s.row->addView(s.todo);
        brls::Label* text = kit::label(brls::getStr(words[i]), 17);
        text->setGrow(1.0f);
        text->setWidth(0);
        s.row->addView(text);
        s.state = kit::label("", 15, "cadran/text3");
        s.state->setMarginLeft(12);
        s.state->setSingleLine(true);
        s.row->addView(s.state);
        card->addView(s.row);
    }
    steps[0].row->registerClickAction([this](brls::View*) {
        console_actions::set_pin("first_steps", [this]() { this->refresh(); });
        return true;
    });
    steps[1].row->registerClickAction([this](brls::View*) {
        if (!this->pt.fw_supported) ui::error("", NXM_RC_FW_UNSUPPORTED);
        else pt_flow::choose_uniform_limit(this->pt, [this]() { this->refresh(); });
        return true;
    });
    steps[2].row->registerClickAction([this](brls::View*) {
        if (this->clock_inaccurate) clock_flow::guided([this]() { this->refresh(); });
        else ui::info("playguard/clock/guided_fine"_i18n);
        return true;
    });
    column->addView(card, at);
    this->steps_card = card;
}

void DashboardTab::show_steps_card(bool has_pin)
{
    // Until the PIN and the limits are set (the clock is Today's own card
    // afterwards); not in read-only mode, where none can be done.
    const bool limits = this->pt.valid && pt_configured(this->pt.block);
    const bool shown = !app::read_only() && this->status.pin_length_ok && this->pt.fw_supported && this->pt.valid &&
                       (!has_pin || !limits);
    ui::set_visible(this->steps_card, shown);
    if (!shown) return;
    brls::Theme theme = brls::Application::getTheme();
    const bool clock_ok = R_SUCCEEDED(this->accuracy_rc) && this->accurate;
    const bool done[3] = { has_pin, limits, clock_ok };
    const std::string state[3] = {
        has_pin ? "playguard/dashboard/pin_set"_i18n : "playguard/onboarding/todo"_i18n,
        limits ? ui::days_summary(this->pt.day_min) : "playguard/onboarding/todo"_i18n,
        clock_ok ? "playguard/dashboard/clock_ok"_i18n : "playguard/dashboard/clock_bad"_i18n,
    };
    int count = 0;
    for (int i = 0; i < 3; i++) {
        Step& s = this->steps[i];
        count += done[i];
        ui::set_visible(s.done, done[i]);
        ui::set_visible(s.todo, !done[i]);
        ui::set_text(s.state, state[i]);
        s.state->setTextColor(theme[done[i] ? "cadran/teal" : "cadran/amber"]);
    }
    ui::set_text(this->steps_count, brls::getStr("playguard/cadran/today/steps_count", count, 3));
}

void DashboardTab::show_outside_card(const std::vector<std::string>& lines)
{
    namespace kit = cadran::kit;
    const bool shown = !lines.empty();
    const std::string text = text::join(lines, "\n");
    if (text != this->outside_shown) {
        this->outside_shown = text;
        while (!this->outside_lines->getChildren().empty())
            this->outside_lines->removeView(this->outside_lines->getChildren().back());
        brls::Theme theme = brls::Application::getTheme();
        size_t from = 0;
        while (from < text.size()) {
            size_t to = text.find('\n', from);
            if (to == std::string::npos) to = text.size();
            auto* row = new brls::Box(brls::Axis::ROW);
            row->setAlignItems(brls::AlignItems::FLEX_START);
            row->setMarginTop(4);
            auto* dot = new brls::Rectangle(theme["cadran/amber_fill"]);
            dot->setWidth(6);
            dot->setHeight(6);
            dot->setCornerRadius(3);
            dot->setMarginTop(7);
            dot->setMarginRight(10);
            dot->setShrink(0);
            row->addView(dot);
            brls::Label* label = kit::label(text.substr(from, to - from), 16, "cadran/text2");
            label->setGrow(1.0f);
            label->setWidth(0);
            row->addView(label);
            this->outside_lines->addView(row);
            from = to + 1;
        }
    }
    ui::set_visible(this->outside_card, shown);
}

void DashboardTab::show_low(bool unlocked)
{
    // Little or no time left today: the first amount stands out, the one
    // most likely wanted: ringed in teal on the soft fill, not filled (a
    // filled pill is the screen's main action, and a chosen chip).
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    bool low = false;
    if (!unlocked && pt.fw_supported && pt.valid && pt.enabled_valid && pt.enabled && pt.day_min[today] != PT_DAY_NOLIMIT) {
        constexpr int64_t QUARTER_NS = 15LL * 60 * 1000000000LL;
        const bool up = pt_logic::over_limit_min(pt, today) >= 0 || (pt.restricted_valid && pt.restricted);
        low = up || (pt.remaining_valid && pt.remaining_ns > 0 && pt.remaining_ns <= QUARTER_NS);
    }
    brls::Theme theme = brls::Application::getTheme();
    for (size_t i = 0; i < this->amount_chips.size(); i++) {
        brls::Box* chip = this->amount_chips[i];
        const bool lit = low && i == 0;
        chip->setBackgroundColor(lit ? theme["cadran/teal_soft"] : theme["cadran/surf2"]);
        chip->setBorderColor(lit ? theme["cadran/teal_fill"] : theme["cadran/line"]);
        chip->setBorderThickness(lit ? 2 : 1);
        for (brls::View* v : chip->getChildren())
            if (auto* digits = dynamic_cast<CadranDigits*>(v)) digits->setTextColor(lit ? theme["cadran/teal"] : theme["cadran/text"]);
    }
}

void DashboardTab::show_cards(bool unlocked, bool has_pin)
{
    brls::Theme theme = brls::Application::getTheme();
    const std::string na = "playguard/common/unavailable"_i18n;
    const PtState& pt = this->pt;
    const PctlStatus& s = this->status;

    // Bedtime. Off, the card says so in large rather than an alarm time that
    // does not ring, and A sets it.
    const bool bed_off = pt.bedtime_valid && !pt.bedtime_enabled;
    ui::set_visible(bed_key.getView(), bed_off && !app::read_only());
    bed_value->setTextColor(theme[bed_off ? "cadran/text2" : "cadran/text"]);
    if (bed_off) {
        bed_value->setText("playguard/cadran/today/bed_off_value"_i18n);
        bed_state->setText("");
        bed_sub->setText(app::read_only() ? "" : "playguard/cadran/today/bed_set"_i18n);
    } else if (pt.bedtime_valid) {
        bed_value->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
        bed_state->setText("playguard/cadran/today/on"_i18n);
        bed_state->setTextColor(theme["cadran/teal"]);
        std::string sub;
        if (pt.bedtime_reset_valid) {
            // The time kept with the word before it: "again at / 06:00" never
            // breaks there (patches/borealis/0010).
            const std::string at = fmt::format("{:02d}:{:02d}", pt.bedtime_reset_hour, pt.bedtime_reset_minute);
            sub = brls::getStr("playguard/cadran/today/bed_sub", at);
            const size_t pos = sub.find(" " + at);
            if (pos != std::string::npos) sub.replace(pos, 1, "\u00A0");
        } else if (pt.bedtime_enabled) {
            // No morning time from the console: in how long the alarm rings.
            const LocalTime now = ui::local_now();
            const int in = (pt.bedtime_hour * 60 + pt.bedtime_minute) - (now.hour * 60 + now.minute);
            if (in > 0) sub = brls::getStr("playguard/cadran/today/bed_in", ui::fmt_minutes((uint16_t)in));
        }
        bed_sub->setText(sub);
    } else {
        bed_value->setText("—");
        bed_state->setText("");
        bed_sub->setText(pt.fw_supported ? na : "");
    }

    // Parental controls: the level, the PIN, the overall state.
    pc_value->setText(s.safety_level_ok ? ui::level_name(s.safety_level) : na);
    if (unlocked) {
        pc_state->setText("playguard/cadran/today/pc_unlocked"_i18n);
        pc_state->setTextColor(theme["cadran/amber"]);
    } else {
        pc_state->setText(has_pin ? "playguard/cadran/today/pin_set"_i18n : "playguard/cadran/today/no_pin"_i18n);
        pc_state->setTextColor(has_pin ? theme["cadran/teal"] : theme["cadran/amber"]);
    }
    pc_sub->setText(!s.restriction_enabled_ok ? na
                    : unlocked                ? "playguard/cadran/today/pc_lifted"_i18n
                    : s.restriction_enabled   ? "playguard/dashboard/pc_active"_i18n
                    : has_pin                 ? "playguard/dashboard/pc_pin_only"_i18n
                                              : "playguard/dashboard/pc_off"_i18n);

    // Network clock: amber, and a call to action, only when it needs one.
    const bool bad = this->clock_inaccurate;
    clock_value->setText(R_FAILED(this->accuracy_rc) ? na
                         : this->accurate              ? "playguard/cadran/today/clock_ok"_i18n
                                                       : "playguard/cadran/today/clock_bad"_i18n);
    clock_value->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text"]);
    clock_title->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text3"]);
    clock_state->setText(bad ? "playguard/cadran/today/to_set"_i18n : "playguard/cadran/today/ok"_i18n);
    clock_state->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/teal"]);
    // Nintendo's time server blocked by the DNS: why it went wrong.
    clock_sub->setText(!bad                         ? "playguard/cadran/today/clock_fine"_i18n
                       : nintendo_dns::time_blocked() ? "playguard/cadran/today/clock_fix_dns"_i18n
                                                    : "playguard/cadran/today/clock_fix"_i18n);
    clock_sub->setTextColor(bad ? theme["cadran/text"] : theme["cadran/text2"]);
    ui::set_visible(clock_key.getView(), bad && !app::read_only());
    clock_card->setBackgroundColor(bad ? theme["cadran/amber_surf"] : theme["cadran/surf"]);
    clock_card->setBorderColor(bad ? theme["cadran/amber_line"] : theme["cadran/line"]);
}

// The console's state as pills, in the colours of what each says; A opens
// where each is set.
DashboardTab::Pill DashboardTab::make_pill(const std::string& text, const char* dot)
{
    namespace kit = cadran::kit;
    Pill p;
    p.box = kit::pill(text, dot);
    p.box->setFocusable(true);
    p.box->setHideHighlightBackground(true);
    p.dot = dynamic_cast<kit::Mark*>(p.box->getChildren()[0]);
    p.text = dynamic_cast<brls::Label*>(p.box->getChildren()[1]);
    pills->addView(p.box);
    return p;
}

void DashboardTab::build_pills()
{
    const std::pair<Pill*, int> to_tab[] = { { &this->compat_pill, ui::tab::console }, { &this->pairing_pill, ui::tab::security },
                                             { &this->serial_pill, ui::tab::console } };
    for (const auto& [pill, tab] : to_tab) {
        *pill = this->make_pill("", "cadran/teal_fill");
        const int position = tab;
        pill->box->registerClickAction([position](brls::View* view) {
            ui::go_to_tab(view, position);
            return true;
        });
    }
    // Ⓐ: what the DNS blocks, and what that changes here.
    this->dns_pill = this->make_pill("", "cadran/teal_fill");
    this->dns_pill.box->registerClickAction([](brls::View*) {
        // The dialogs' layout (cadran::dialog_body): a title, the two
        // servers as a table, what that changes as notes.
        const bool sync = nintendo_dns::pctl_blocked(), time = nintendo_dns::time_blocked();
        const auto state = [](bool blocked) {
            return blocked ? "playguard/cadran/today/dns_blocked"_i18n : "playguard/cadran/today/dns_reachable"_i18n;
        };
        std::string body = "playguard/cadran/today/dns_title"_i18n + "\n\n" +
                           (sync || time ? "playguard/cadran/today/dns_intro"_i18n : "playguard/cadran/today/dns_intro_open"_i18n) +
                           "\n\n" + cadran_text::table({ { "playguard/cadran/today/dns_row_sync"_i18n, state(sync) },
                                                            { "playguard/cadran/today/dns_row_time"_i18n, state(time) } });
        if (sync) body += "\n\n" + "playguard/cadran/today/dns_sync_note"_i18n;
        if (time)
            body += "\n\n" + brls::getStr("playguard/cadran/today/dns_time_note",
                                         "playguard/tabs/console"_i18n + " › " + "playguard/tabs/clock"_i18n);
        ui::info(body);
        return true;
    });
    this->patches_pill = this->make_pill("", "cadran/teal_fill");
    this->patches_pill.box->registerClickAction([](brls::View* view) {
        ui::go_to_tab(view, ui::tab::console);
        return true;
    });
    // Opened over a game, PlayGuard's own time counts: said at length on Ⓐ.
    this->counted_pill = this->make_pill("playguard/cadran/today/counted_pill"_i18n, "cadran/amber_fill");
    this->counted_pill.box->registerClickAction([this](brls::View*) {
        std::string body = "playguard/cadran/today/counted_title"_i18n + "\n\n" + "playguard/dashboard/counted"_i18n;
        if (this->home_counts()) body += "\n\n" + "playguard/dashboard/home_counts"_i18n;
        ui::info(body);
        return true;
    });
    // A game left open on HOME keeps counting (the opened-over-a-game pill
    // says it too, when shown).
    this->home_pill = this->make_pill("playguard/cadran/today/home_pill"_i18n, "cadran/amber_fill");
    this->home_pill.box->registerClickAction([](brls::View*) {
        ui::info("playguard/cadran/today/home_title"_i18n + "\n\n" + "playguard/dashboard/home_counts"_i18n);
        return true;
    });
}

void DashboardTab::show_pills()
{
    namespace kit = cadran::kit;
    const SysInfo& si = this->sys;
    // Something to look at: its title and what it says, in its colour; else
    // the all-fine words, or nothing.
    const auto show = [](Pill& p, bool issue, const std::string& title, const std::string& value, NVGcolor color,
                         const std::string& fine) {
        const std::string text = issue ? title + " · " + value : fine;
        ui::set_text(p.text, text);
        const std::string dot = issue ? kit::dot_of(color) : "cadran/teal_fill";
        p.dot->set(dot.c_str());
        p.rank = dot == "cadran/teal_fill" ? 1 : dot == "cadran/text3" ? 2 : 0;
        ui::set_visible(p.box, !text.empty());
    };
    // The firmware's compatibility: the firmware with it when all is fine.
    NVGcolor c = ui::color_neutral();
    const std::string compat = ui::compat_text(si, &c);
    show(this->compat_pill, sysinfo_compat(&si) != SysCompat_Ok, "playguard/dashboard/compat"_i18n, compat, c,
         ui::fw_text(si) + " · " + compat);
    // The phone app only while it can overwrite what is set here.
    const PctlStatus& s = this->status;
    show(this->pairing_pill, pairing_state::syncs(s), "playguard/dashboard/pairing"_i18n,
         ui::bool_text(s.pairing_active_ok, s.pairing_active, "playguard/dashboard/pairing_on"_i18n,
                       "playguard/dashboard/pairing_off"_i18n),
         ui::color_warn(), "");
    // The serial number visible on emuMMC.
    show(this->serial_pill, ui::serial_warning(si), "playguard/dashboard/serial"_i18n, "playguard/dashboard/serial_visible"_i18n,
         ui::color_warn(), "playguard/cadran/today/serial_hidden"_i18n);
    // Sigpatch files only, or sys-patch incomplete.
    const patches::Report& report = ui::patch_report();
    c = ui::color_neutral();
    const std::string patches_text = ui::patches_text(report, &c);
    show(this->patches_pill, ui::patches_warning(report), "playguard/dashboard/patches"_i18n, patches_text, c, "");
    const bool sync = nintendo_dns::pctl_blocked(), time = nintendo_dns::time_blocked();
    // Blocking nothing, said too: the parent who set up the redirection
    // sees whether it holds.
    ui::set_text(this->dns_pill.text, sync && time ? "playguard/cadran/today/dns_nintendo"_i18n
                                      : sync       ? "playguard/cadran/today/dns_sync"_i18n
                                      : time       ? "playguard/cadran/today/dns_time"_i18n
                                                   : "playguard/cadran/today/dns_open"_i18n);
    this->dns_pill.dot->set(sync || time ? "cadran/teal_fill" : "cadran/text3");
    this->dns_pill.rank = sync || time ? 1 : 2;
    this->counted_pill.rank = 0;
    this->home_pill.rank = 0;
    ui::set_visible(this->counted_pill.box, !si.applet_mode);
    ui::set_visible(this->home_pill.box, this->home_counts() && si.applet_mode);
    this->order_pills();
}

void DashboardTab::order_pills()
{
    // What needs the parent first (amber, coral), then what is fine (teal),
    // then what is only said (grey); in their usual order within each.
    std::vector<Pill*> all = { &this->compat_pill, &this->pairing_pill, &this->serial_pill, &this->dns_pill,
                               &this->patches_pill, &this->counted_pill, &this->home_pill };
    std::stable_sort(all.begin(), all.end(), [](const Pill* a, const Pill* b) { return a->rank < b->rank; });
    std::vector<brls::View*> wanted;
    for (const Pill* p : all)
        if (p->box) wanted.push_back(p->box);
    if (wanted == pills->getChildren()) return;
    // Not under the focus: moved, a focused pill would lose it.
    for (brls::View* v : wanted)
        if (v->isFocused()) return;
    for (brls::View* v : wanted) pills->removeView(v, false);
    for (brls::View* v : wanted) pills->addView(v);
}

void DashboardTab::refresh()
{
    perf::Timed timed("Overview refresh");   // the diagnostic report's timings
    // The time of day, for every helper below (today's limit, the outside
    // watch, the timer health, the bell), read once.
    ui::SameTime same;

    // Every 5 s: one pctl session with only what this screen shows, and the
    // network clock's accuracy flag alone.
    PctlStatus s;
    pctl_overview_fetch(&s, &this->pt);
    const PtState& pt = this->pt;
    bool accurate = false;
    const Result accuracy_rc = time_network_accuracy(&accurate);
    SysInfo si;
    sysinfo_get(&si);

    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);

    const TodayLog log = read_today_log();

    // Changes seen outside PlayGuard, and whether the console counts at all.
    outside_watch::observe(pt);
    outside_watch::observe_accounts();   // once per run; flagged in the Activity tab
    const std::vector<std::string> outside_lines = outside_watch::notice_lines();
    const bool stalled = timer_health::observe(pt);
    // The same readings for the notification bell.
    notices::note_timer_stalled(stalled);
    notices::refresh_status(s, pt, si, accuracy_rc, accurate);

    this->status = s;
    this->sys = si;
    this->accuracy_rc = accuracy_rc;
    this->accurate = accurate;
    this->clock_inaccurate = R_SUCCEEDED(accuracy_rc) && !accurate;
    this->unlocked_now = unlocked;

    const int today = ui::today_weekday();
    this->show_headline(today, unlocked, log);

    // Visibility last, so a vanished focused view hands the focus to a neighbour.
    const bool pending = pt_flow::restore_pending(pt);
    if (pending) extra_pending->setText(pt_flow::restore_label());
    ui::set_visible_all({ { extra_pending.getView(), pending },
                          { alarm.getView(), pt_flow::alarm_off(pt) },
                          { applet.getView(), si.applet_mode },
                          { not_counting.getView(), stalled } });

    this->list.sync();
    if (!s.restriction_enabled_ok) cadran::set_pill("");
    else if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", true);
    else if (s.restriction_enabled) cadran::set_pill("playguard/cadran/pill/pc_on"_i18n);
    else cadran::set_pill("playguard/cadran/pill/pc_off"_i18n, "alert", true);
    this->show_ring(unlocked);
    this->show_next(unlocked);
    this->show_cards(unlocked, has_pin);
    this->show_pills();
    // The actions while they can do something.
    const bool can_extra = pt_logic::can_add_extra_time(pt, today, false);
    ui::set_visible(extra.getView(), can_extra);
    ui::set_visible(chips.getView(), can_extra);
    ui::set_visible(stop.getView(), pt_logic::can_stop_today(pt, today, false));
    ui::set_visible(relock.getView(), unlocked && !app::read_only());
    ui::set_visible(setup.getView(), !unlocked && !app::read_only() && pt.fw_supported && pt.valid &&
                                         !pt_configured(pt.block));
    this->show_profile();
    this->fit_text_size();
    this->show_low(unlocked);
    this->show_outside_card(outside_lines);
    this->show_steps_card(has_pin);
}

bool DashboardTab::home_counts() const
{
    return platform_game_open(nullptr) && !this->unlocked_now;
}

DashboardTab::~DashboardTab()
{
    play_data::unlisten(this->listener);
}

brls::View* DashboardTab::create()
{
    return new DashboardTab();
}
