// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "activity/main_activity.hpp"

#include <cstdlib>

#include <borealis/cadran/ui/cadran_frame.hpp>
#include <borealis/cadran/ui/cadran_modal.hpp>
#include <borealis/cadran/view/tab_badge.hpp>

#include "action/agent_update.hpp"
#include "action/bundled_modules.hpp"
#include "action/clock_check.hpp"
#include "action/data_notice.hpp"
#include "action/fw_gate.hpp"
#include "action/game_notice.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/nintendo_dns.hpp"
#include "action/notices.hpp"
#include "action/pt_flow.hpp"
#include "action/rescue.hpp"
#include "action/rollback_flow.hpp"
#include "action/schedule_flow.hpp"
#include "action/session_pause.hpp"
#include "action/support_flow.hpp"
#include "action/update_flow.hpp"
#include "activity/forwarders_activity.hpp"
#include "activity/loading_activity.hpp"
#include "activity/modules_activity.hpp"
#include "activity/notices_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "app.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/title_state.hpp"

using namespace brls::literals;

// Ten tabs and three separators do not fit the 720p sidebar at borealis'
// sizes (70 px items, 30 px separators): the last tabs fell below the edge.
// Tighter items there only; detail cells keep the shared 70 px.
static void compact_sidebar(brls::View* tab_frame)
{
    auto* sidebar = tab_frame ? dynamic_cast<brls::Sidebar*>(tab_frame->getView("brls/tab_frame/sidebar")) : nullptr;
    brls::SidebarItem* first = sidebar ? sidebar->getItem(0) : nullptr;
    brls::Box* list = first ? first->getParent() : nullptr;
    if (!list) return;
    list->setPaddingTop(10);
    list->setPaddingBottom(10);
    for (brls::View* v : list->getChildren()) {
        if (dynamic_cast<brls::SidebarItem*>(v)) v->setHeight(54);
        else v->setHeight(16);   // a separator: its line is drawn at mid-height
    }
}

// Quits, but first asks to lock again when parental controls are still
// temporarily unlocked (PIN › Unlock temporarily): leaving them so means
// no limit at all until a restart, which is easy to forget on the way out.
// Locking again waits for the screen to be gone, and closes the game PlayGuard
// runs over too (pt_flow::lock_at_exit): back to the HOME menu, locked. Until
// then, the closing screen.
void MainActivity::quit_or_relock()
{
    bool unlocked = false;
    // PlayGuard's own pause locks again by itself as it closes (session_pause::end).
    if (app::read_only() || session_pause::active() || R_FAILED(pctl_lock_state(nullptr, &unlocked)) || !unlocked) {
        brls::Application::quit();
        return;
    }
    brls::Logger::info("quitting while unlocked: asking to lock again");
    // Turning them back on first: borealis focuses a dialog's first button, so
    // a quick A on the way out never leaves the console unlocked.
    auto* dialog = ui::dialog("playguard/hints/exit_unlocked"_i18n);
    dialog->addButton("playguard/hints/exit_lock"_i18n, []() {
        pt_flow::lock_at_exit();
        // Drawn before quitting: the last frame, the one the console keeps
        // behind its own screens, is this one rather than PlayGuard's.
        brls::Application::pushActivity(new LoadingActivity(LoadingActivity::Mode::Closing),
                                        brls::TransitionAnimation::NONE);
        brls::sync([]() { brls::sync([]() { brls::sync([]() { brls::Application::quit(); }); }); });
    });
    dialog->addButton("playguard/hints/exit_leave"_i18n, []() {
        pt_flow::keep_unlocked();   // chosen: the next start does not lock again
        brls::Application::quit();
    });
    // The risky choice in coral, as ui::confirm_danger draws it.
    cadran::style_dialog(dialog, cadran::Emphasis::DANGER);
    dialog->open();
}

brls::View* MainActivity::createContentView()
{
    return brls::View::createFromXMLResource("activity/main.xml");
}

void MainActivity::onContentAvailable()
{
    // For the desktop smoke test: the start screens (lock, rescue) lead here.
    brls::Logger::info("main screen opened");
    this->update_title();
    compact_sidebar(this->getView("main_tabs"));
    // The frame after the design boards.
    cadran::style_frame(dynamic_cast<brls::AppletFrame*>(this->getContentView()), this->getView("main_tabs"));

    // B on the sidebar quits (the tabs themselves send B back to the sidebar),
    // but only when pressed twice within 2 s: one stray B never closes the app.
    this->getContentView()->registerAction("hints/exit"_i18n, brls::BUTTON_B, [](brls::View*) {
        static brls::Time last_press = 0;
        const brls::Time now = brls::getCPUTimeUsec();
        if (last_press && now - last_press < 2000000) {
            last_press = 0;
            quit_or_relock();
            return true;
        }
        last_press = now;
        ui::notify("playguard/hints/exit_again"_i18n);
        return true;
    });

    // Y: the notification bell's list, from any tab, its count in the footer.
    // A tab with its own Ⓨ shows that one instead (the focused view's wins).
    // With − below, the footer's optional keys: left out of it, − first, when
    // a screen's own keys need their room (Aurore, patch 0020).
    brls::setHintsOptionalOwner(this->getContentView());
    this->getContentView()->registerAction("playguard/notices/hint"_i18n, brls::BUTTON_Y, [](brls::View*) {
        NoticesActivity::open();
        return true;
    });
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    // The desktop build: the keyboard has no Ⓨ, so the Y key (smoke test). Not in
    // a browser, where the page presses Ⓨ itself for the Y key (web/README.md).
    this->getContentView()->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Y), [](brls::View*) {
        NoticesActivity::open();
        return true;
    });
#endif
    // −: the language and the theme, in the footer (also a tap on them in
    // the header, which shows them).
    this->getContentView()->registerAction("playguard/quick/hint"_i18n, brls::BUTTON_BACK, [](brls::View*) {
        ui::quick_settings(TabBase::refresh_shown);
        return true;
    });
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    this->getContentView()->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_MINUS), [](brls::View*) {
        ui::quick_settings(TabBase::refresh_shown);
        return true;
    });
#endif
    this->notice_listener = notices::listen([this]() { this->update_notice_hint(); });
    this->update_notice_hint();

    // Preferences › Start on: that tab rather than Today (first, so the
    // questions below keep the focus they take).
    int start = 0;
    for (const char* t : config::START_TABS) {
        if (config::get().start_tab == t) break;
        start++;
    }
    if (start > 0 && start < (int)(sizeof(config::START_TABS) / sizeof(config::START_TABS[0]))) {   // ui::tab::of knows them all
        brls::sync([this, start]() {
            auto* frame = dynamic_cast<brls::TabFrame*>(this->getView("main_tabs"));
            if (frame && ui::on_top(this)) frame->focusTab(ui::tab::of(start));
        });
    }

    // Stopped in the middle of a change last time: lock again first. Then, for
    // a limit changed for an earlier day, put the usual one back (extra time:
    // by itself; no more play today: asked, or as the preference says).
    brls::sync([]() {
        pt_flow::relock_if_interrupted();
        pt_flow::offer_extra_time_restore();
        schedule_flow::check(true);   // a profile scheduled for a period: its start or its end
    });
    // Background checks: a newer version (daily when opted in, and while one
    // is known and not installed: its screen), a network clock off (opt-in).
    update_flow::at_start(this);
    clock_check::at_start();
    game_notice::at_start();   // the installed games, off the main thread: the bell says which cannot start

    // Firmware newer than the verified one: the firmware screen (or the remembered choice).
    fw_gate::on_main_screen();
    // Nothing set up yet (no PIN): the first steps, once per start, unless the
    // firmware screen opens (one screen per start: ui::claim_start_screen).
    if (OnboardingActivity::wanted_at_start() && ui::claim_start_screen("first steps"))
        brls::sync([]() { brls::Application::pushActivity(new OnboardingActivity()); });
    // Settings, history or a recovery report found damaged, a build gone back
    // from: in the notification bell.
    data_notice::at_start();
    rollback_flow::note_went_back();
    notices::note_crash();
    rescue::note_version();
    // The homebrew menu's lock and the forwarders made here (the bell).
    hbmenu_lock::at_start([]() { brls::Application::pushActivity(new ForwardersActivity()); });
    // What Atmosphère's DNS.mitm redirects: the session log and the diagnostic report.
    nintendo_dns::at_start();
    // Then, with nothing else in front: "What's new" after an update, else the
    // monthly "Support PlayGuard" reminder.
    support_flow::at_start(this);
    // The agent sysmodule on the SD card is not the one this PlayGuard
    // carries: the offer to update it.
    agent_update::at_start(this, bundled_modules::dir());

    // What the bell shows, read now and every minute after that: Today
    // reads the same every 5 s while it is shown.
    brls::sync([]() { notices::sweep(); });
    this->notice_timer.setPeriod(60000);
    this->notice_timer.setCallback([this]() {
        if (!app::in_focus() || !ui::on_top(this)) return;
        notices::sweep_if_stale();
    });
    this->notice_timer.start();

    this->day = ui::today_date();
    this->day_timer.setPeriod(30000);
    this->day_timer.setCallback([this]() {
        // Only on the main screen, not behind a dialog: the date is compared
        // again at the next tick otherwise.
        if (!app::in_focus() || !ui::on_top(this)) return;
        const std::string today = ui::today_date();
        schedule_flow::check(false);   // asked once a day, when nothing else was in front
        if (today == this->day) {
            pt_flow::retry_extra_time_restore();   // only after a state that could not be read
            return;
        }
        this->day = today;
        pt_flow::offer_extra_time_restore();
    });
    this->day_timer.start();
}

MainActivity::~MainActivity()
{
    notices::unlisten(this->notice_listener);
    if (brls::getHintsOptionalOwner() == this->getContentView()) brls::setHintsOptionalOwner(nullptr);
}

void MainActivity::update_notice_hint()
{
    const int n = notices::action_count();
    this->getContentView()->updateActionHint(brls::BUTTON_Y, n > 0 ? brls::getStr("playguard/notices/hint_count", n)
                                                                    : "playguard/notices/hint"_i18n);
    brls::Application::getGlobalHintsUpdateEvent()->fire();
    // Each tab counts its own entries to do (notice_logic::tab_of), on a
    // badge by its name: where to look, without opening the list.
    auto* frame   = this->getView("main_tabs");
    auto* sidebar = frame ? dynamic_cast<brls::Sidebar*>(frame->getView("brls/tab_frame/sidebar")) : nullptr;
    if (!sidebar) return;
    static const std::pair<int, const char*> TABS[] = {
        { ui::tab::dashboard, "dashboard" }, { ui::tab::play_timer, "play_timer" }, { ui::tab::activity, "activity" },
        { ui::tab::security, "security" },   { ui::tab::history, "history" },       { ui::tab::console, "console" },
        { ui::tab::preferences, "preferences" }, { ui::tab::about, "about" },
    };
    for (const auto& t : TABS)
        if (TabBadge* badge = TabBadge::of(sidebar->getItem(t.first))) badge->set_count(notices::tab_count(t.second));
}

void MainActivity::open_tab(int position, const std::string& row)
{
    auto* frame = dynamic_cast<brls::TabFrame*>(this->getView("main_tabs"));
    if (!frame) return;
    brls::sync([frame, position, row]() {
        frame->focusTab(position);   // replaces the tab content
        const auto& children = frame->getChildren();
        if (children.size() < 2) return;
        brls::View* focus = children.back();
        if (brls::View* cell = row.empty() ? nullptr : focus->getView(row))
            if (cell->getVisibility() == brls::Visibility::VISIBLE) focus = cell;
        brls::Application::giveFocus(focus);
    });
}

void MainActivity::update_title()
{
    // "PlayGuard"; what the user must know whatever the tab (temporarily
    // unlocked: the play timer is not counting; read-only) is the header's
    // pill, which a tap undoes (view/title_state).
    auto* frame = dynamic_cast<brls::AppletFrame*>(this->getContentView());
    if (!frame) return;
    frame->setTitle("playguard/title"_i18n);
    // The header's pill says when parental controls are unlocked or the app
    // read-only, on every screen.
    cadran::refresh_pill();
}

// Back from a dialog or a screen: a tab may have hidden the view that opened
// it meanwhile (Today's refresh, every 5 s and on new play data).
void MainActivity::onResume()
{
    ui::keep_focus(this);
}
