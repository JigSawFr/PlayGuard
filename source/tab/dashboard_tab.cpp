// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <algorithm>
#include <yoga/Yoga.h>

#include "action/clock_flow.hpp"
#include "action/console_actions.hpp"
#include "action/notices.hpp"
#include "action/outside_watch.hpp"
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
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace brls::literals;

namespace
{
// "Turn back on": the primary button, in the warning colour of the unlock it ends.
const brls::ButtonStyle RELOCK = {
    .shadowType              = brls::ShadowType::GENERIC,
    .hideHighlightBackground = true,
    .highlightPadding        = "brls/button/primary_highlight_padding",
    .borderThickness         = "",
    .enabledBackgroundColor  = "cadran/amber_fill",
    .enabledLabelColor       = "cadran/navy",
    .enabledBorderColor      = "",
    .disabledBackgroundColor = "brls/button/primary_disabled_background",
    .disabledLabelColor      = "brls/button/primary_disabled_text",
    .disabledBorderColor     = "",
};
}   // namespace

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
        pt_flow::offer_extra_time_restore(this->refresher(), true);
        return true;
    });
    // Shown only while the alarm is off: A turns it back on (the dialog says why).
    ui::set_detail(alarm, "playguard/dashboard/alarm_off"_i18n);
    alarm->setDetailTextColor(cadran::kit::color("cadran/amber"));
    alarm->registerClickAction([this](brls::View*) {
        pt_flow::turn_alarm_on("overview", this->refresher());
        return true;
    });
    this->enable_auto_refresh(5000);
    // Only while shown: replaced by another tab and waiting to be freed, its
    // pill would cover the new tab's.
    this->listener = play_data::listen([this]() { if (this->shown()) this->refresh(); });
    this->build_pills();

    // The buttons' icons, before their label.
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
    relock->setStyle(&RELOCK);
    relock->addView(lock, 0);
    relock->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* week = kit::icon("timer", 20, "cadran/navy");
    week->setMarginRight(10);
    setup->addView(week, 0);
    setup->setAlignItems(brls::AlignItems::CENTER);
    // No limit yet: the Play timer, on its week, where the days are set.
    setup->registerClickAction([](brls::View* view) {
        if (ui::main_screen()) ui::open_tab(ui::tab::play_timer, "cpt_week");
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
        profile_flow::pick(this->refresher());
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
        console_actions::relock("", this->refresher());
        return true;
    });
    limit->registerClickAction([this](brls::View*) {
        this->choose_today_limit();
        return true;
    });
    // Extra time: its words, the amounts, then "…" for any other (a
    // duration, another day: the picker). One row, one way in.
    {
        brls::Label* words = kit::label("playguard/cadran/today/extra"_i18n, kit::TEXT_LEAD, "cadran/text2");
        words->setSingleLine(true);
        words->setMarginRight(16);
        chips->addView(words);
    }
    for (int minutes : config::get().extra_amounts) {
        brls::Box* amount = kit::chip("+" + ui::fmt_minutes((uint16_t)minutes));
        amount->setMarginRight(10);
        amount->registerClickAction([this, minutes](brls::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_minutes(this->pt, (uint16_t)minutes, this->refresher());
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
    {
        brls::Box* other = kit::chip("\u2026");
        other->registerClickAction([this](brls::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_time(this->pt, this->refresher());
            return true;
        });
        chips->addView(other);
    }
    stop->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        pt_flow::stop_today(this->pt, this->refresher());
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
    else if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", cadran::Tone::WARN);
    else if (s.restriction_enabled) cadran::set_pill("playguard/cadran/pill/pc_on"_i18n);
    else cadran::set_pill("playguard/cadran/pill/pc_off"_i18n, "alert", cadran::Tone::DANGER);
    this->show_ring(unlocked);
    this->show_next(unlocked);
    this->show_cards(unlocked, has_pin);
    this->show_pills();
    // The actions while they can do something.
    const bool can_extra = pt_logic::can_add_extra_time(pt, today, false);
    ui::set_visible(chips.getView(), can_extra);
    ui::set_visible(stop.getView(), pt_logic::can_stop_today(pt, today, false));
    // Read-only: the actions that write stay in sight, greyed (A says why
    // and where it is turned off), as on the other tabs.
    const float alpha = app::read_only() ? 0.45f : 1.0f;
    for (brls::View* v : { (brls::View*)chips.getView(), (brls::View*)stop.getView(), (brls::View*)profile.getView() })
        v->setAlpha(alpha);
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
