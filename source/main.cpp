// PlayGuard — borealis entry point: loads the preferences, registers the
// custom XML views, probes the pctl service and shows the tabbed main screen
// (or the init-error screen).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  This program is free software under the GNU
// General Public License v3 or later; it comes with NO WARRANTY. See the
// LICENSE file or <https://www.gnu.org/licenses/gpl-3.0.html> for details.
#include <borealis.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fcntl.h>
#include <typeinfo>
#include <unistd.h>

#include "action/fw_gate.hpp"
#include "action/log_prune_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_log_flow.hpp"
#include "action/rescue.hpp"
#include "activity/init_error_activity.hpp"
#include "activity/loading_activity.hpp"
#include "activity/lock_activity.hpp"
#include "activity/main_activity.hpp"
#include "activity/rescue_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "tab/about_tab.hpp"
#include "tab/activity_tab.hpp"
#include "tab/clock_tab.hpp"
#include "tab/dashboard_tab.hpp"
#include "tab/play_timer_tab.hpp"
#include "tab/preferences_tab.hpp"
#include "tab/restrictions_tab.hpp"
#include "tab/security_tab.hpp"
#include "tab/tools_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/http.hpp"
#include "util/data_move.hpp"
#include "util/log_files.hpp"
#include "util/nand.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/startup.h"
#include "view/loading_bar.hpp"
#include "view/loading_mark.hpp"
#include "view/made_in_france.hpp"
#include "view/scroll_view.hpp"
#include "view/play_days.hpp"
#include "view/pt_gauge.hpp"
#include "view/pt_state_header.hpp"
#include "view/pt_week.hpp"

using namespace brls::literals;

namespace
{
// An exception nothing caught: what it said goes to a file of its own,
// logs/crashes/crash_<date>.txt (util/log_files.hpp), then
// PlayGuard ends as a crash, so Atmosphère's report shows where it was
// thrown (the stack is not unwound yet). Left to std::terminate, it would
// abort(), which hbloader takes as a normal exit: back in hbmenu, borealis'
// task thread, still running, crashed hbmenu instead.
[[noreturn]] void on_terminate()
{
    std::string what = "no exception (std::terminate called directly)";
    if (std::exception_ptr e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception& x) {
            what = std::string(typeid(x).name()) + ": " + x.what();
        } catch (...) {
            what = "an exception not derived from std::exception";
        }
    }
    char when[32] = "?";
    const std::time_t now = std::time(nullptr);
    std::tm tmv{};
    if (localtime_r(&now, &tmv)) std::strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", &tmv);
    if (paths::ensure_dir(paths::crashes_dir())) {
        // Created 0644 (as util/pt_log.cpp), not fopen's 0666. Two crashes in
        // the same second (another thread) share the file, one line each.
        const std::string file = paths::crashes_dir() + "/" + log_files::name(log_files::CRASH, log_files::stamp(now));
        const int fd = ::open(file.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (std::FILE* f = fd < 0 ? nullptr : ::fdopen(fd, "a")) {
            std::fprintf(f, "%s  PlayGuard %s (%s): uncaught %s\n", when, app::version().c_str(),
                         app::commit().empty() ? "?" : app::commit().c_str(), what.c_str());
            std::fclose(f);
        } else if (fd >= 0) {
            ::close(fd);
        }
    }
    std::fprintf(stderr, "uncaught %s\n", what.c_str());
    platform_crash();
    std::abort();
}

// The start-up after the loading screen, one step per frame so the loading
// screen moves along (brls::sync runs a task after the frame is drawn, and
// one queued from a task waits for the next frame). It took a moment on the
// console (the service probe, the SD-card reads, the main screen's build),
// and the screen stayed black for it. Each step is timed for the diagnostic
// report (util/startup.h).

// The last step: `first` built and shown over the loading screen, which stays
// under it (borealis' Application::clear() is private). It holds the focus
// the first screen hands back, so nothing focused is ever freed under the
// stack (ui::replace_screen pops a lock or recovery screen).
void show_first(brls::Activity* first)
{
    brls::Application::pushActivity(first, brls::TransitionAnimation::NONE);
    pt_log_flow::apply();   // Developer › Record the play timer, when on (after fw_gate::prepare)
    log_prune_flow::run();  // the reports and crashes past the limits of Tools › Diagnostics
    startup_mark(StartupStep_FirstScreen);
    brls::Logger::info("start-up: first screen ready after {} ms", startup_elapsed_ms(StartupStep_Main, StartupStep_FirstScreen));
}

// The first screen once the intro is over: checked once a frame.
void after_intro(LoadingActivity* loading, brls::Activity* first)
{
    if (loading->inIntro()) {
        brls::sync([loading, first]() { after_intro(loading, first); });
        return;
    }
    startup_mark(StartupStep_Intro);
    show_first(first);
}

// Which first screen, once the console is probed: the recovery report, the
// lock screen or the main screen. Constructed only: show_first builds it.
brls::Activity* choose_first()
{
    // Untested firmware: read-only (or the remembered choice) before any tab is built.
    fw_gate::prepare();
    // Security › Ask for the PIN: checked before every change from now on;
    // "To open PlayGuard" starts on the lock screen.
    pin_lock::install();
    // ... and again after a while away, over whatever is open.
    pin_lock::watch_focus([]() { LockActivity::lock_again(); });
    // The playguard-rescue sysmodule acted on a RESCUE file: show what it
    // did and let the parent finish, before (and instead of) the lock
    // screen — they are here because they forgot the PIN. Only when the
    // console confirms it: anyone can write the report file, and an
    // unconfirmed one leads to the lock screen like any start.
    auto report = rescue::take();
    const bool confirmed = report && rescue::confirmed(*report);
    // Stopped in the middle of a change last time: lock again before any
    // screen, so a lock screen left with B does not leave the console
    // unlocked. Not after a confirmed recovery unlock: that one is wanted.
    if (!(confirmed && report->mode == RescueMode_Unlock)) pt_flow::relock_if_interrupted();
    if (report) return new RescueActivity(*report, confirmed);
    if (pin_lock::at_start()) return new LockActivity();
    return new MainActivity();
}

void start(LoadingActivity* loading)
{
    startup_mark(StartupStep_LoadingDrawn);
    brls::Logger::info("start-up: loading screen drawn after {} ms", startup_elapsed_ms(StartupStep_Main, StartupStep_LoadingDrawn));
    if (config::get().intro) {
        loading->startIntro();
        brls::Logger::info("start-up: intro, {} ms at least", LoadingActivity::INTRO_US / 1000);
    }
    const bool probed = app::init();
    startup_mark(StartupStep_Probed);
    if (!probed) {
        brls::Logger::error("pctl probe failed (0x{:08X}) — showing InitErrorActivity", app::pctl_init_result());
        show_first(new InitErrorActivity());
        return;
    }
    loading->show(LoadingActivity::Step::Settings);
    brls::sync([loading]() {
        brls::Activity* first = choose_first();
        startup_mark(StartupStep_Settings);
        loading->show(LoadingActivity::Step::Screen);
#ifndef __SWITCH__
        // Desktop simulation, for a screenshot of the loading screen.
        if (std::getenv("PLAYGUARD_SIM_HOLD_LOADING")) {
            brls::Logger::info("start-up: held on the loading screen (PLAYGUARD_SIM_HOLD_LOADING)");
            delete first;
            return;
        }
#endif
        brls::sync([loading, first]() { after_intro(loading, first); });
    });
}
}   // namespace

int main(int argc, char* argv[])
{
    startup_mark(StartupStep_Main);
    std::set_terminate(on_terminate);
    app::set_self_path(argc > 0 ? argv[0] : nullptr);

#ifdef __SWITCH__
    // The data folder of the versions that kept it next to the .nro.
    if (const size_t moved = data_move::run(data_move::old_dir(), paths::data_dir()))
        brls::Logger::info("start-up: {} entries moved from {} to {}", moved, data_move::old_dir(), paths::data_dir());
#endif
    // logs/ before its subfolders (reports/, crashes/, uploads/, play_timer/).
    log_files::migrate(paths::logs_dir());
    // The system this is (sysMMC or emuMMC): its own folder, nand/<key>,
    // before config::load() reads its state.json. What the versions before
    // it kept for every system goes to the first one started.
    {
        SysInfo si;
        sysinfo_get(&si);
        const SysStorage storage = sysinfo_storage(&si);
        paths::set_nand(nand::key(storage == SysStorage_SysMMC   ? nand::Storage::SysMMC
                                  : storage == SysStorage_EmuMMC ? nand::Storage::EmuMMC
                                                                 : nand::Storage::Unknown));
        if (const size_t moved = data_move::to_nand(paths::data_dir(), paths::nand_dir()))
            brls::Logger::info("start-up: {} entries moved to {}", moved, paths::nand_dir());
    }

    // Preferences first: the locale must be chosen before borealis loads i18n.
    config::load();
    startup_mark(StartupStep_Preferences);
    const auto& cfg = config::get();
    // config::sanitize() keeps the language to config::LANGUAGES.
    if (cfg.language != "system")
        brls::Platform::APP_LOCALE_DEFAULT = cfg.language;
    else
        brls::Platform::APP_LOCALE_DEFAULT = brls::LOCALE_AUTO;

    if (!brls::Application::init()) {
        brls::Logger::error("Unable to init borealis Application");
        return EXIT_FAILURE;
    }
    startup_mark(StartupStep_Borealis);

    brls::Application::createWindow("playguard/title"_i18n);
    ui::use_latin_font();
    startup_mark(StartupStep_Window);

    // Follows the console theme unless a preference says otherwise.
    if (cfg.theme == "dark")
        brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);
    else if (cfg.theme == "light")
        brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::LIGHT);

    // No cross-fade when a screen opens or closes: on the console's LCD the
    // two screens blended for 200 ms read as ghosting. 2 ms, not 0: borealis
    // still runs (and ends) its animations, in whole milliseconds, and a
    // picker's takes half of it.
    brls::getStyle().addMetric("brls/animations/show", 2.0f);

    // PlayGuard status colours (light / dark variants), before any view exists.
    ui::register_theme_colors();

    // We own the quit path (B on the sidebar / error screen).
    brls::Application::setGlobalQuit(false);

    brls::Application::registerXMLView("ScrollView",      ScrollView::create);
    brls::Application::registerXMLView("PtStateHeader",   PtStateHeader::create);
    brls::Application::registerXMLView("PtGauge",         PtGauge::create);
    brls::Application::registerXMLView("PtWeekView",      PtWeekView::create);
    brls::Application::registerXMLView("PlayDaysView",    PlayDaysView::create);
    brls::Application::registerXMLView("MadeInFrance",    MadeInFrance::create);
    brls::Application::registerXMLView("DashboardTab",    DashboardTab::create);
    brls::Application::registerXMLView("PlayTimerTab",    PlayTimerTab::create);
    brls::Application::registerXMLView("ActivityTab",     ActivityTab::create);
    brls::Application::registerXMLView("RestrictionsTab", RestrictionsTab::create);
    brls::Application::registerXMLView("ClockTab",        ClockTab::create);
    brls::Application::registerXMLView("SecurityTab",     SecurityTab::create);
    brls::Application::registerXMLView("PreferencesTab",  PreferencesTab::create);
    brls::Application::registerXMLView("ToolsTab",        ToolsTab::create);
    brls::Application::registerXMLView("AboutTab",        AboutTab::create);

    brls::Application::registerXMLView("LoadingMark",     LoadingMark::create);
    brls::Application::registerXMLView("LoadingBar",      LoadingBar::create);

    // The loading screen first, with nothing slow before it, and drawn at once
    // (no fade: a frame at alpha 0 is a black one). The rest of the start-up
    // runs from the end of that first frame.
    auto* loading = new LoadingActivity();
    brls::Application::pushActivity(loading, brls::TransitionAnimation::NONE);
    brls::sync([loading]() { start(loading); });

    // Game icons kept between runs, next to the play data cache.
    playstats_set_icon_dir((paths::data_dir() + "/cache/icons").c_str());
    // borealis joins its task thread before mainLoop() returns: a play-data
    // or icon read running there stops now, not after every game is read.
    brls::Application::getExitEvent()->subscribe([]() { playstats_cancel(); });

    while (brls::Application::mainLoop())
        ;

    // Network tasks still running (an update check, a sign-in waiting for its
    // code): stopped, and waited for a moment, before what they use goes away.
    // The SD-card writes still queued (ui::write_later) are done first.
    http::abort_all();
    ui::finish_background(std::chrono::seconds(3));
    pt_log_flow::stop();
    app::shutdown();
    http::cleanup();
    return EXIT_SUCCESS;
}
