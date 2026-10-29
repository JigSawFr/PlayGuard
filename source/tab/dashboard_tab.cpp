// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <algorithm>

#include <aurore/cadran/ui/cadran_frame.hpp>
#include <aurore/cadran/ui/cadran_kit.hpp>

#include "action/clock_flow.hpp"
#include "action/console_actions.hpp"
#include "action/notices.hpp"
#include "action/outside_watch.hpp"
#include "action/play_data.hpp"
#include "action/profile_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "action/session_pause.hpp"
#include "action/timer_health.hpp"
#include "activity/cadran_child_activity.hpp"
#include "activity/main_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "tab/clock_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace aurore::literals;

namespace
{
// "Turn back on": the primary button, in the warning colour of the unlock it ends.
const aurore::ButtonStyle RELOCK = {
    .shadowType              = aurore::ShadowType::GENERIC,
    .hideHighlightBackground = true,
    .highlightPadding        = "aurore/button/primary_highlight_padding",
    .borderThickness         = "",
    .enabledBackgroundColor  = "cadran/amber_fill",
    .enabledLabelColor       = "cadran/navy",
    .enabledBorderColor      = "",
    .disabledBackgroundColor = "aurore/button/primary_disabled_background",
    .disabledLabelColor      = "aurore/button/primary_disabled_text",
    .disabledBorderColor     = "",
};
}   // namespace

DashboardTab::DashboardTab()
    : TabBase("xml/tab/dashboard.xml"), list(this)
{
    namespace kit = cadran::kit;
    aurore::Box* column = this->list.content();
    applet->setSingleLine(false);
    not_counting->setSingleLine(false);
    // Above the top, what needs the parent first: the first steps, then what
    // changed outside PlayGuard.
    const auto& lines = column->getChildren();
    const int at = (int)(std::find(lines.begin(), lines.end(), this->getView("dash_top")) - lines.begin());
    pills->setWrap(aurore::Wrap::WRAP);
    this->build_outside_card(column, at);
    this->build_steps_card(column, at);

    // What stops play first, under the time played.
    this->next = new aurore::Box(aurore::Axis::ROW);
    this->next->setAlignItems(aurore::AlignItems::FLEX_START);
    this->next->setAlignSelf(aurore::AlignSelf::STRETCH);   // the column's width, for its text to wrap in
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
    this->next->setVisibility(aurore::Visibility::GONE);
    today_text->addView(this->next, 2);
    // Or, before the time up or the bedtime alarm, the rest of the day as a line.
    this->timeline = new CadranTimeline();
    this->timeline->setAlignSelf(aurore::AlignSelf::STRETCH);
    this->timeline->setMarginTop(14);
    this->timeline->setMarginBottom(4);
    this->timeline->setVisibility(aurore::Visibility::GONE);
    today_text->addView(this->timeline, 3);
    this->list.apply({ { "dash_extra_pending", "refresh" }, { "dash_alarm", "bell" } });
    extra_pending->registerClickAction([this](aurore::View*) {
        pt_flow::offer_extra_time_restore(this->refresher(), true);
        return true;
    });
    // Shown only while the alarm is off: A turns it back on (the dialog says why).
    ui::set_detail(alarm, "playguard/dashboard/alarm_off"_i18n);
    alarm->detail->setThemeColor("textColor", "cadran/amber");
    alarm->registerClickAction([this](aurore::View*) {
        pt_flow::turn_alarm_on("overview", this->refresher());
        return true;
    });
    ring->setRolls(true);   // the minutes roll as they go
    this->enable_auto_refresh(5000);
    // Only while shown: replaced by another tab and waiting to be freed, its
    // pill would cover the new tab's.
    this->listener = play_data::listen([this]() { if (this->shown()) this->refresh(); });
    this->build_pills();

    // The buttons' icons, before their label.
    CadranIcon* square = kit::icon("stop", 18, "cadran/coral");
    square->setMarginRight(10);
    stop->addView(square, 0);
    stop->setAlignItems(aurore::AlignItems::CENTER);
    CadranIcon* eye = kit::icon("eye", 18, "cadran/text2");
    eye->setMarginRight(10);
    child->addView(eye, 0);
    child->setAlignItems(aurore::AlignItems::CENTER);
    CadranIcon* lock = kit::icon("lock", 20, "cadran/navy");
    lock->setMarginRight(10);
    relock->setStyle(&RELOCK);
    relock->addView(lock, 0);
    relock->setAlignItems(aurore::AlignItems::CENTER);
    CadranIcon* week = kit::icon("timer", 20, "cadran/navy");
    week->setMarginRight(10);
    setup->addView(week, 0);
    setup->setAlignItems(aurore::AlignItems::CENTER);
    // No limit yet: the Play timer, on its week, where the days are set.
    setup->registerClickAction([](aurore::View* view) {
        if (ui::main_screen()) ui::open_tab(ui::tab::play_timer, "cpt_week");
        else ui::go_to_tab(view, ui::tab::play_timer);
        return true;
    });
    CadranIcon* timer = kit::icon("timer", 18, "cadran/text2");
    timer->setMarginRight(10);
    limit->addView(timer, 0);
    limit->setAlignItems(aurore::AlignItems::CENTER);
    CadranIcon* list_icon = kit::icon("list", 18, "cadran/text2");
    list_icon->setMarginRight(10);
    profile->addView(list_icon, 0);
    profile->setAlignItems(aurore::AlignItems::CENTER);
    if (auto* text = dynamic_cast<aurore::Label*>(profile->getView("aurore/button/label"))) text->setSingleLine(true);
    // The saved profiles, the one in force checked: one press to change week.
    profile->registerClickAction([this](aurore::View*) {
        if (ui::refuse_read_only()) return true;
        profile_flow::pick(this->refresher());
        return true;
    });
    // Stop, the child screen, today's limit, the profile: on a second line
    // when a language needs the room.
    kit::wrap(actions2.getView());
    // Broken only where it would be at the normal size: with the larger text,
    // the buttons' words smaller again instead, and the cards' states whole.
    for (aurore::Button* b : { stop.getView(), child.getView(), limit.getView(), profile.getView() }) kit::fit_button(b);
    for (aurore::Box* card : { bed_card.getView(), pc_card.getView(), clock_card.getView() })
        kit::fit_head(dynamic_cast<aurore::Box*>(card->getChildren().front()));
    actions2->setColumnGap(12);
    actions2->setRowGap(10);
    // Wrapped, the row's buttons are one line to Aurore: ↓ would leave it
    // past the last button, on the line under. Between its lines, ↑ and ↓ go
    // to the nearest button of the next line.
    const std::vector<aurore::View*> row2 = { stop.getView(), child.getView(), limit.getView(), profile.getView() };
    const auto line_move = [row2](aurore::View* from, bool down) {
        aurore::View* best = nullptr;
        float best_line = 0, best_dx = 0;
        const float cx = from->getX() + from->getWidth() / 2;
        for (aurore::View* to : row2) {
            if (to == from || to->getVisibility() != aurore::Visibility::VISIBLE) continue;
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
        aurore::Application::giveFocus(best);
        return true;
    };
    for (aurore::View* button : row2) {
        button->registerAction("", aurore::BUTTON_NAV_DOWN, [line_move](aurore::View* view) { return line_move(view, true); }, true);
        button->registerAction("", aurore::BUTTON_NAV_UP, [line_move](aurore::View* view) { return line_move(view, false); }, true);
    }

    relock->registerClickAction([this](aurore::View*) {
        console_actions::relock("", this->refresher());
        return true;
    });
    limit->registerClickAction([this](aurore::View*) {
        this->choose_today_limit();
        return true;
    });
    // Extra time: its words, the amounts, then "…" for any other (a
    // duration, another day: the picker). One row, one way in.
    {
        aurore::Label* words = kit::label("playguard/cadran/today/extra"_i18n, kit::TEXT_LEAD, "cadran/text2");
        words->setSingleLine(true);
        words->setMarginRight(16);
        chips->addView(words);
    }
    for (int minutes : config::get().extra_amounts) {
        aurore::Box* amount = kit::chip("+" + ui::fmt_limit((uint16_t)minutes));
        amount->setMarginRight(10);
        amount->registerClickAction([this, minutes](aurore::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_minutes(this->pt, (uint16_t)minutes, this->refresher());
            return true;
        });
        // Focused, the next stop's line says when play would end with it.
        amount->getFocusEvent()->subscribe([this, minutes](aurore::View*) {
            this->preview_extra = minutes;
            this->show_next(this->unlocked_now);
        });
        amount->getFocusLostEvent()->subscribe([this](aurore::View*) {
            this->preview_extra = 0;
            this->show_next(this->unlocked_now);
        });
        chips->addView(amount);
        this->amount_chips.push_back(amount);
    }
    {
        aurore::Box* other = kit::chip("\u2026");
        other->registerClickAction([this](aurore::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_time(this->pt, this->refresher());
            return true;
        });
        chips->addView(other);
    }
    stop->registerClickAction([this](aurore::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::stop_today(this->pt, this->refresher());
        return true;
    });
    child->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new CadranChildActivity());
        return true;
    });
    // The cards: where each is set.
    bed_card->registerClickAction([](aurore::View* view) {
        ui::go_to_tab(view, ui::tab::play_timer);
        return true;
    });
    pc_card->registerClickAction([](aurore::View* view) {
        ui::go_to_tab(view, ui::tab::security);
        return true;
    });
    // Inaccurate and writable: the guided measure-and-set path; else the clock's screen.
    clock_card->registerClickAction([this](aurore::View*) {
        if (this->clock_inaccurate && !app::read_only()) clock_flow::guided(this->refresher());
        else ClockTab::open_screen();
        return true;
    });
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
    const PtState& timer = this->pt;
    bool network_accurate = false;
    const Result network_rc = time_network_accuracy(&network_accurate);
    SysInfo si;
    sysinfo_get(&si);

    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);
    // Paused for the parent here: the day as it is for the child.
    session_pause::shown_as_locked(&s, &this->pt);
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;

    const TodayLog log = read_today_log();

    // Changes seen outside PlayGuard, and whether the console counts at all.
    outside_watch::observe(timer);
    outside_watch::observe_accounts();   // once per run; flagged in the Activity tab
    const std::vector<std::string> outside_notes = outside_watch::notice_lines();
    const bool stalled = timer_health::observe(timer);
    // The same readings for the notification bell.
    notices::note_timer_stalled(stalled);
    notices::refresh_status(s, timer, si, network_rc, network_accurate);

    this->status = s;
    this->sys = si;
    this->accuracy_rc = network_rc;
    this->accurate = network_accurate;
    this->clock_inaccurate = R_SUCCEEDED(network_rc) && !network_accurate;
    this->unlocked_now = unlocked;

    const int today = ui::today_weekday();
    this->show_headline(today, unlocked, log);

    // Visibility last, so a vanished focused view hands the focus to a neighbour.
    const bool pending = pt_flow::restore_pending(timer);
    if (pending) extra_pending->setText(pt_flow::restore_label());
    ui::set_visible_all({ { extra_pending.getView(), pending },
                          { alarm.getView(), pt_flow::alarm_off(timer) },
                          { applet.getView(), si.applet_mode },
                          { not_counting.getView(), stalled } });

    this->list.sync();
    if (!s.restriction_enabled_ok) cadran::set_pill("");
    else if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", cadran::Tone::WARN);
    else if (s.restriction_enabled) cadran::set_pill("playguard/cadran/pill/pc_on"_i18n);
    else cadran::set_pill("playguard/cadran/pill/pc_off"_i18n, "alert", cadran::Tone::DANGER);
    this->show_ring(unlocked);
    this->show_next(unlocked);
    this->show_cards(unlocked, has_pin);
    this->show_pills();
    // The actions while they can do something.
    const bool can_extra = pt_logic::can_add_extra_time(timer, today, false);
    ui::set_visible(chips.getView(), can_extra);
    ui::set_visible(stop.getView(), pt_logic::can_stop_today(timer, today, false));
    // Read-only: the actions that write stay in sight, greyed (A says why
    // and where it is turned off), as on the other tabs.
    const float actions_alpha = app::read_only() ? 0.45f : 1.0f;
    for (aurore::View* v : { (aurore::View*)chips.getView(), (aurore::View*)stop.getView(), (aurore::View*)profile.getView() })
        v->setAlpha(actions_alpha);
    ui::set_visible(relock.getView(), unlocked && !app::read_only());
    ui::set_visible(setup.getView(), !unlocked && !app::read_only() && timer.fw_supported && timer.valid &&
                                         !pt_configured(timer.block));
    this->show_profile();
    this->fit_text_size();
    this->show_low(unlocked);
    this->show_outside_card(outside_notes);
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

aurore::View* DashboardTab::create()
{
    return new DashboardTab();
}
