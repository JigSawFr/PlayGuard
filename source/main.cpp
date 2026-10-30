// PlayGuard — Aurore entry point: loads the preferences, registers the
// custom XML views, probes the pctl service and shows the tabbed main screen
// (or the init-error screen).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  This program is free software under the GNU
// General Public License v3 or later; it comes with NO WARRANTY. See the
// LICENSE file or <https://www.gnu.org/licenses/gpl-3.0.html> for details.
#include <aurore.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <format>
#include <functional>
#include <typeinfo>
#ifndef __SWITCH__
#include <fcntl.h>
#include <unistd.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <aurore/cadran/ui/cut_text.hpp>
#include <aurore/cadran/view/cadran_digits.hpp>
#include <aurore/cadran/view/cadran_icon.hpp>
#include <aurore/cadran/view/loading_bar.hpp>
#include <aurore/cadran/view/scroll_view.hpp>

#include "action/frame_stats_flow.hpp"
#include "action/fw_gate.hpp"
#include "action/log_prune_flow.hpp"
#include "action/migration_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_log_flow.hpp"
#include "action/rescue.hpp"
#include "action/rollback_flow.hpp"
#include "action/session_pause.hpp"
#include "action/sync_flow.hpp"
#include "activity/init_error_activity.hpp"
#include "activity/loading_activity.hpp"
#include "activity/lock_activity.hpp"
#include "activity/main_activity.hpp"
#include "activity/rescue_activity.hpp"
#include "activity/screen_hooks.hpp"
#include "app.hpp"
#include "core/gate_nx.h"
#include "core/platform.h"
#include "core/sim_env.h"
#include "tab/about_tab.hpp"
#include "tab/activity_tab.hpp"
#include "tab/console_tab.hpp"
#include "tab/dashboard_tab.hpp"
#include "tab/history_tab.hpp"
#include "tab/play_timer_tab.hpp"
#include "tab/preferences_tab.hpp"
#include "tab/restrictions_tab.hpp"
#include "tab/security_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/crash_dump.h"
#include "util/crash_report.hpp"
#include "util/diagnostics.hpp"
#include "util/http.hpp"
#include "util/nand.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/session_log.hpp"
#include "util/startup.h"
#include "util/sync_files.hpp"
#include "view/cadran_week.hpp"
#include "view/loading_dial.hpp"
#include "view/loading_mark.hpp"
#include "view/made_in_france.hpp"
#include "view/notice_bell.hpp"
#include "view/play_days.hpp"
#include "view/play_hours.hpp"
#include "view/pt_ring.hpp"
#include "view/pt_state_header.hpp"

using namespace aurore::literals;

namespace
{
// The return addresses from the caller up, for a crash file: "" when
// none could be read (desktop).
std::string backtrace_here()
{
    CrashDump d{};
    if (!platform_code_range(&d.module_base, &d.module_size)) d.module_base = d.module_size = 0;
    d.frames = crash_dump_backtrace((uint64_t)(uintptr_t)__builtin_frame_address(0), platform_read64, nullptr, d.frame,
                                    CRASH_DUMP_FRAMES);
    if (d.frames == 0) return "";
    std::string out = "backtrace (innermost first, from the terminate handler):\n";
    for (size_t i = 0; i < d.frames; i++) {
        char where[40];
        crash_dump_where(&d, d.frame[i], where, sizeof(where));
        out += std::format("  #{:<3} {}\n", i, where);
    }
    return out;
}

// An exception nothing caught: what it said, the backtrace and the last
// lines logged go to a file of its own, logs/crashes/crash_<date>.txt
// (util/crash_report.hpp), then PlayGuard ends as a crash, so Atmosphère's
// report shows where it was thrown (the stack is not unwound yet). Left to
// std::terminate, it would abort(), which hbloader takes as a normal exit:
// back in hbmenu, Aurore's task thread, still running, crashed hbmenu
// instead.
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
    const std::string summary = "uncaught " + what;
    const std::string trace = backtrace_here();
    const char* name = crash_report::write(std::time(nullptr), summary.c_str(), trace.empty() ? nullptr : trace.c_str());
    session_log::end_crash(name);
    std::fprintf(stderr, "%s\n", summary.c_str());
    platform_crash();
    std::abort();
}

// "PlayGuard 1.2.0 (abc1234)" for the session and crash files.
std::string build_name()
{
    return std::format("PlayGuard {} ({})", app::version(), app::commit().empty() ? "?" : app::commit());
}

// Every line logged, also in this run's session file (util/session_log.hpp).
void keep_log_lines()
{
    aurore::Logger::getLogEvent()->subscribe([](aurore::Logger::TimePoint when, aurore::LogLevel level, std::string text) {
        static const char LEVELS[] = "EWIDV";
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(when.time_since_epoch()).count() % 1000;
        const unsigned l = (unsigned)level;
        session_log::add(std::chrono::system_clock::to_time_t(when), (int)ms, l < 5 ? LEVELS[l] : '?', text);
    });
}

// This run's session file, and a crash file for the run before it when it
// never ended (no end line: a crash PlayGuard did not catch, a hang closed
// from the HOME menu, the console turned off).
void start_session()
{
    const std::time_t now = std::time(nullptr);
    std::tm tmv{};
    char when[32] = "?";
    if (localtime_r(&now, &tmv)) std::strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", &tmv);
    const std::string header = std::format("=== PlayGuard session ===\nstarted : {}\nbuild   : {}\n\n", when, build_name());
    const std::string name = session_log::start(paths::sessions_dir(), now, header);
    crash_report::prepare(paths::crashes_dir(), build_name(), "", name);
    if (name.empty()) aurore::Logger::warning("logs: no session file in {}", paths::sessions_dir());

    session_log::Unfinished before;
    if (session_log::unfinished(paths::sessions_dir(), name, &before)) {
        const std::string details = "last lines of logs/sessions/" + before.name + ":\n" +
                                    session_log::last_lines(before.text, 60);
        const char* crash = crash_report::write(
            now, ("the run of " + before.name + " did not end: a crash PlayGuard did not catch, closed from the HOME "
                  "menu while it hung, or the console turned off").c_str(),
            details.c_str(), false);
        aurore::Logger::warning("logs: the last run ({}) did not end; {}", before.name,
                              *crash ? std::string("see logs/crashes/") + crash : std::string("no crash file written"));
    }
}

// The console, for the session and crash files: "firmware 22.5.0,
// Atmosphère 1.11.1, sysMMC, over a game".
std::string system_line(const SysInfo& si)
{
    const std::string fw = ui::hos_text(si.hos_version);
    const char* storage = diagnostic::storage_name(sysinfo_storage(&si));
    return std::format("firmware {}, Atmosphère {}, {}, {}", fw,
                       si.ams_valid ? std::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro) : "unknown",
                       storage ? storage : "storage unknown",
                       si.applet_mode ? "from the Album (applet)"
                       : si.forwarder ? "from its HOME menu icon (forwarder)"
                                      : "over a game");
}

// The start-up after the loading screen, one step per frame so the loading
// screen moves along (aurore::sync runs a task after the frame is drawn, and
// one queued from a task waits for the next frame). It took a moment on the
// console (the service probe, the SD-card reads, the main screen's build),
// and the screen stayed black for it. Each step is timed for the diagnostic
// report (util/startup.h).

// The last step: `first` built and shown over the loading screen, which stays
// under it (Aurore's Application::clear() is private). It holds the focus
// the first screen hands back, so nothing focused is ever freed under the
// stack (ui::replace_screen pops a lock or recovery screen).
void show_first(aurore::Activity* first)
{
    aurore::Application::pushActivity(first, aurore::TransitionAnimation::NONE);
    pt_log_flow::apply();   // Help & diagnostics › Developer › Record the play timer, when on (after fw_gate::prepare)
    frame_stats_flow::apply();  // Help & diagnostics › Developer › Frame statistics, when on
    log_prune_flow::run();  // the reports and crashes past the limits of About › Help & diagnostics
    startup_mark(StartupStep_FirstScreen);
    rollback_flow::started();   // this build starts: nothing to go back from
    aurore::Logger::info("start-up: first screen ready after {} ms", startup_elapsed_ms(StartupStep_Main, StartupStep_FirstScreen));
}

// The first screen once the intro is over: checked once a frame.
void after_intro(LoadingActivity* loading, aurore::Activity* first)
{
    if (loading->inIntro()) {
        aurore::sync([loading, first]() { after_intro(loading, first); });
        return;
    }
    startup_mark(StartupStep_Intro);
    show_first(first);
}

// Which first screen, once the console is probed: the recovery report, the
// lock screen or the main screen. Constructed only: show_first builds it.
aurore::Activity* choose_first()
{
    // Untested firmware: read-only (or the remembered choice) before any tab is built.
    fw_gate::prepare();
    {
        // What the agent sysmodule may do on PlayGuard's behalf (sync/nro_state.txt).
        SysInfo si;
        sysinfo_get(&si);
        char fw[16];
        sysinfo_version_string(si.hos_version, fw, sizeof(fw));
        sync_files::set_console(fw, app::read_only());
        sync_files::export_nro_state();
    }
    // PIN › Protect PlayGuard with the PIN: checked before every change from now on;
    // "To open PlayGuard" starts on the lock screen.
    pin_lock::install();
    // ... and again after a while away, over whatever is open.
    pin_lock::watch_focus([]() { LockActivity::lock_again(); });
    // The PIN entered pauses parental controls while PlayGuard is open; left
    // or closed, they are back on (after the lock screen's own check).
    session_pause::install();
    // The remote link (MQTT, Home Assistant), when sync.conf turns it on.
    // Before the relock below and the first screen: the agent's records
    // (relock pending, console lock, extra time) are adopted first, and what
    // it changed while PlayGuard was closed goes into the history as
    // PlayGuard's own, not as a change made outside it.
    sync_flow::start();
    // The playguard-rescue sysmodule acted on a RESCUE file: show what it
    // did and let the parent finish, before (and instead of) the lock
    // screen — they are here because they forgot the PIN. Only when the
    // console confirms it (and the sysmodule's proof, for an unlock): anyone
    // can write the report file, and an unconfirmed one leads to the lock
    // screen like any start.
    RescueProof proof;
    auto report = rescue::take(&proof);
    bool old_module = false;   // an unlock by a module from before the proof
    const bool confirmed = report && rescue::confirmed(*report, proof, &old_module);
    // Stopped in the middle of a change last time: lock again before any
    // screen, so a lock screen left with B does not leave the console
    // unlocked. Not after a confirmed recovery unlock: that one is wanted.
    if (!(confirmed && report->mode == RescueMode_Unlock)) pt_flow::relock_if_interrupted();
    if (report) return new RescueActivity(*report, confirmed, old_module);
    if (pin_lock::at_start()) return new LockActivity();
    return new MainActivity();
}

void start(LoadingActivity* loading)
{
    startup_mark(StartupStep_LoadingDrawn);
    aurore::Logger::info("start-up: loading screen drawn after {} ms", startup_elapsed_ms(StartupStep_Main, StartupStep_LoadingDrawn));
    if (config::get().intro) {
        loading->startIntro();
        aurore::Logger::info("start-up: intro, {} ms at least", LoadingActivity::INTRO_US / 1000);
    }
    const bool probed = app::init();
    startup_mark(StartupStep_Probed);
    if (!probed) {
        aurore::Logger::error("pctl probe failed (0x{:08X}) — showing InitErrorActivity", app::pctl_init_result());
        show_first(new InitErrorActivity());
        return;
    }
    loading->show(LoadingActivity::Step::Settings);
    aurore::sync([loading]() {
        aurore::Activity* first = choose_first();
        startup_mark(StartupStep_Settings);
        loading->show(LoadingActivity::Step::Screen);
#ifndef __SWITCH__
        // Desktop simulation, for a screenshot of the loading screen.
        if (sim_env_flag("PLAYGUARD_SIM_HOLD_LOADING")) {
            aurore::Logger::info("start-up: held on the loading screen (PLAYGUARD_SIM_HOLD_LOADING)");
            delete first;
            return;
        }
#endif
        aurore::sync([loading, first]() { after_intro(loading, first); });
    });
}

// The system this is (sysMMC or emuMMC): its own folder, nand/<key>,
// before config::load() reads its state.json. What the versions before it
// kept for every system goes to the first one started. Then the system in
// the log and in a crash file.
void select_nand_and_log_system()
{
    SysInfo si;
    sysinfo_get(&si);
    const SysStorage storage = sysinfo_storage(&si);
    paths::set_nand(nand::key(storage == SysStorage_SysMMC   ? nand::Storage::SysMMC
                              : storage == SysStorage_EmuMMC ? nand::Storage::EmuMMC
                                                             : nand::Storage::Unknown));
    migration_flow::for_system();
    const std::string system = system_line(si);
    aurore::Logger::info("start-up: {}, {}", build_name(), system);
    crash_report::prepare(paths::crashes_dir(), build_name(), system, session_log::current_name());
}

// The views the XML layouts name, and the bell in the main screen's header
// (Cadran's own look, the footer's hints and the header's badge and
// settings come with Aurore).
void register_views()
{
    aurore::Application::registerXMLView("ScrollView",      ScrollView::create);
    aurore::Application::registerXMLView("PtStateHeader",   PtStateHeader::create);
    aurore::Application::registerXMLView("PtRing",          PtRing::create);
    aurore::Application::registerXMLView("PlayDaysView",    PlayDaysView::create);
    aurore::Application::registerXMLView("PlayHoursView",   PlayHoursView::create);
    aurore::Application::registerXMLView("MadeInFrance",    MadeInFrance::create);
    aurore::Application::registerXMLView("DashboardTab",    DashboardTab::create);
    aurore::Application::registerXMLView("PlayTimerTab",    PlayTimerTab::create);
    aurore::Application::registerXMLView("ActivityTab",     ActivityTab::create);
    aurore::Application::registerXMLView("RestrictionsTab", RestrictionsTab::create);
    aurore::Application::registerXMLView("SecurityTab",     SecurityTab::create);
    aurore::Application::registerXMLView("HistoryTab",      HistoryTab::create);
    aurore::Application::registerXMLView("ConsoleTab",      ConsoleTab::create);
    aurore::Application::registerXMLView("PreferencesTab",  PreferencesTab::create);
    aurore::Application::registerXMLView("AboutTab",        AboutTab::create);

    aurore::Application::registerXMLView("LoadingMark",     LoadingMark::create);
    aurore::Application::registerXMLView("LoadingDial",     LoadingDial::create);
    aurore::Application::registerXMLView("LoadingBar",      LoadingBar::create);
    aurore::Application::registerXMLView("CadranIcon",      CadranIcon::create);
    aurore::Application::registerXMLView("CadranDigits",    CadranDigits::create);
    aurore::Application::registerXMLView("CadranWeekView",  CadranWeekView::create);

    NoticeBell::install();   // the bell goes in the header's row at the right
}

// After the main loop: the play timer locked again if asked, the tasks and
// writes still running finished, the services closed, the session ended.
void shut_down()
{
    // The pause of parental controls over (closed from the HOME menu too:
    // the exit waits for this), then "Turn back on and quit": locked now
    // that the screen is gone (pt_flow.hpp).
    session_pause::end();
    pt_flow::lock_at_exit_if_asked();

    sync_flow::stop();   // the remote link: "offline" while the network is still up
    // Network tasks still running (an update check, a sign-in waiting for its
    // code): stopped, and waited for a moment, before what they use goes away.
    // The SD-card writes still queued (ui::write_later) are done first.
    http::abort_all();
    ui::finish_background(std::chrono::seconds(3));
    pt_log_flow::stop();
    frame_stats_flow::stop();
    app::shutdown();
    http::cleanup();
    aurore::Logger::info("quit: after {} s", session_log::uptime_s());
    session_log::end("clean exit");
}

#ifndef __SWITCH__
FILE* s_stats = nullptr;   // PLAYGUARD_SIM_STATS's file, when asked (main())
aurore::Time s_stats_last = 0;
#endif

void close_stats()
{
#ifndef __SWITCH__
    if (s_stats) std::fclose(s_stats);
    s_stats = nullptr;
#endif
}

// One frame; false once PlayGuard quits.
bool run_frame()
{
    if (!aurore::Application::mainLoop()) return false;
    // The lines the session file waits for, written off the UI thread
    // once they waited long enough (util/session_log.hpp).
    if (session_log::due()) ui::write_later("session log", []() { session_log::flush(); }, "session_log");
    frame_stats_flow::sample();   // Developer › Frame statistics, when on
    cut_text::sample();           // the desktop's texts cut short (PLAYGUARD_SIM_CUT_TEXT)
#ifndef __SWITCH__
    if (s_stats) {
        int draws = 0, fill = 0, stroke = 0, text = 0;
        nvgDebugGetStats(aurore::Application::getNVGContext(), &draws, &fill, &stroke, &text);
        const aurore::Time now = aurore::getCPUTimeUsec();
        std::fprintf(s_stats, "%lld %lld %d %d %d %d\n", (long long)now, (long long)(now - s_stats_last), draws, fill,
                     stroke, text);
        s_stats_last = now;
    }
#endif
    return true;
}
}   // namespace

int main(int argc, char* argv[])
{
    startup_mark(StartupStep_Main);
#ifdef __SWITCH__
    // Started by a forwarder protected with the PIN as its gate (one made
    // before the gate had its own .nro, core/gate_nx.h): the PIN screen and
    // nothing else of PlayGuard, then the homebrew or the HOME menu.
    if (gate_nx_run()) return EXIT_SUCCESS;
#endif
    keep_log_lines();
    std::set_terminate(on_terminate);
    app::set_self_path(argc > 0 ? argv[0] : nullptr);
    app::set_open_route(argc, argv);

    migration_flow::at_start();
    startup_mark(StartupStep_Migrations);
    start_session();
    startup_mark(StartupStep_Session);
    // A way out of main() that skips the end of this function.
    std::atexit([]() { session_log::end("exit"); });
    // Before anything that could crash: the last start of a build just
    // installed never reached its first screen? Then the previous build is
    // offered (action/rollback_flow.hpp).
    if (rollback_flow::at_start()) {
        session_log::end("exit at start-up (the start-up screen of the previous build)");
        return EXIT_SUCCESS;
    }
    startup_mark(StartupStep_Rollback);
    select_nand_and_log_system();
    startup_mark(StartupStep_System);

    // What ui/ and view/ ask of the screens above them, before any opens.
    install_screen_hooks();

    // Preferences first: their language replaces the console's once Aurore
    // has read it (Aurore's Cadran, in Application::init()).
    config::load();
    startup_mark(StartupStep_Preferences);
    // Remote link: what the agent sysmodule needs of the settings follows
    // every save (nro_state.txt, only once the link was set up), its files
    // written off the UI thread as the settings are.
    sync_files::set_writer([](const std::string& file, std::function<void()> write) {
        ui::write_later("remote link file", std::move(write), file);
    });
    sync_files::set_on_failure([](const std::string& file, const std::string& why) {
        aurore::Logger::error("remote link: {} not saved: {}", file, why);
        aurore::sync([]() { ui::notify("playguard/toast/save_err"_i18n, ui::Toast::Refused); });
    });
    config::set_saved_hook(sync_files::export_nro_state);

    if (!aurore::Application::init()) {
        aurore::Logger::error("Unable to init Aurore Application");
        session_log::end("exit: Aurore could not start");
        return EXIT_FAILURE;
    }
    startup_mark(StartupStep_Borealis);
    // In the Boréale homebrews' registry, for the others to know it is there
    // (written only when its version or place changed).
    ui::write_later("Boréale registry", []() {
        std::string why;
        if (!sync_files::register_app(app::version(), app::self_path(), &why))
            aurore::Logger::error("Boréale registry: {} not saved: {}", sync_files::app_file(), why);
    }, sync_files::app_file());

    // PlayGuard's own colours, before the window reads the theme file that
    // may change them.
    ui::install_palette();

    // Aurore starts Cadran itself: the preferences' language (in init()), then
    // their font and theme, the palettes after the design boards, the text
    // size, the footer's hints, the header's version badge and settings, and
    // every screen's look (in createWindow()).
    aurore::Application::createWindow("playguard/title"_i18n);
    startup_mark(StartupStep_Window);

    // No cross-fade when a screen opens or closes: on the console's LCD the
    // two screens blended for 200 ms read as ghosting. 2 ms, not 0: Aurore
    // still runs (and ends) its animations, in whole milliseconds, and a
    // picker's takes half of it.
    aurore::getStyle().addMetric("aurore/animations/show", 2.0f);

    // We own the quit path (B on the sidebar / error screen).
    aurore::Application::setGlobalQuit(false);

    register_views();

    // The loading screen first, with nothing slow before it, and drawn at once
    // (no fade: a frame at alpha 0 is a black one). The rest of the start-up
    // runs from the end of that first frame.
    auto* loading = new LoadingActivity();
    aurore::Application::pushActivity(loading, aurore::TransitionAnimation::NONE);
    aurore::sync([loading]() { start(loading); });

    // Game icons kept between runs, next to the play data cache.
    playstats_set_icon_dir((paths::cache_dir() + "/icons").c_str());
    // Aurore joins its task thread before mainLoop() returns: a play-data
    // or icon read running there stops now, not after every game is read.
    aurore::Application::getExitEvent()->subscribe([]() { playstats_cancel(); });

#ifndef __SWITCH__
    // Desktop measurement (PLAYGUARD_SIM_STATS=<file>, tools/perf_frames.sh):
    // one line per frame, "<time us> <frame us> <draw calls> <fill tris>
    // <stroke tris> <text tris>", the counters nanovg's own
    // (nvgDebugGetStats). The FPS limiter is turned off so the frame time is
    // the work alone.
    if (const char* path = sim_env("PLAYGUARD_SIM_STATS")) {
        // Created with an explicit mode (CodeQL: fopen's depends on the umask).
        const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd >= 0) {
            s_stats = fdopen(fd, "w");
            if (!s_stats) close(fd);
        }
        aurore::Application::setLimitedFPS(0);
    }
    s_stats_last = aurore::getCPUTimeUsec();
#endif
#ifdef __EMSCRIPTEN__
    // In a browser a frame must hand the thread back to the page (web/README.md):
    // the browser calls one per display refresh, and main() returns at once.
    emscripten_set_main_loop([]() {
        if (run_frame()) return;
        emscripten_cancel_main_loop();
        close_stats();
        shut_down();
    }, 0, false);
    return EXIT_SUCCESS;
#else
    while (run_frame()) {}
    close_stats();

    shut_down();
    return EXIT_SUCCESS;
#endif
}
