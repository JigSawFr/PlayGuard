// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/screen_hooks.hpp"

#include <borealis.hpp>

#include <string>
#include <utility>
#include <vector>

#include <borealis/cadran/ui/cadran_modal.hpp>
#include <borealis/cadran/ui/hooks.hpp>
#include <borealis/cadran/ui/prefs.hpp>

#include "action/frame_stats_flow.hpp"
#include "action/notices.hpp"
#include "action/pt_log_flow.hpp"
#include "action/sync_flow.hpp"
#include "action/upload_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/lock_activity.hpp"
#include "activity/main_activity.hpp"
#include "activity/notices_activity.hpp"
#include "tab/help_tab.hpp"
#include "app.hpp"
#include "core/sim_env.h"
#include "core/sysinfo.h"
#include "tab/tab_base.hpp"
#include "ui/app_hooks.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/sync_files.hpp"
#include "view/title_state.hpp"

using namespace brls::literals;

namespace
{
// PlayGuard's logo (branding/mark.svg), Cadran's mark in the header: a play
// button inside the daily play-time ring (75 %).
void mark(NVGcontext* vg, float w, float h)
{
    nvgScale(vg, w / 256.0f, h / 256.0f);
    nvgBeginPath(vg);
    nvgCircle(vg, 128, 128, 104);
    nvgFillColor(vg, nvgRGB(0x0B, 0x14, 0x24));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgCircle(vg, 128, 128, 74);
    nvgStrokeWidth(vg, 20);
    nvgStrokeColor(vg, nvgRGB(0x1F, 0x33, 0x55));
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgArc(vg, 128, 128, 74, -NVG_PI / 2.0f, -NVG_PI / 2.0f + 0.75f * 2.0f * NVG_PI, NVG_CW);
    nvgLineCap(vg, NVG_ROUND);
    nvgStrokeColor(vg, nvgRGB(0x2E, 0xC4, 0xA6));
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 110, 98);
    nvgLineTo(vg, 162, 128);
    nvgLineTo(vg, 110, 158);
    nvgClosePath(vg);
    nvgFillColor(vg, nvgRGB(0xE8, 0xEE, 0xF7));
    nvgFill(vg);
    nvgStrokeWidth(vg, 10);
    nvgLineJoin(vg, NVG_ROUND);
    nvgStrokeColor(vg, nvgRGB(0xE8, 0xEE, 0xF7));
    nvgStroke(vg);
}

// The header's pill over every screen: PlayGuard's own pause while the
// parent is in it (said calmly, as a state of the tab would be: it ends by
// itself), parental controls unlocked, then read-only; a tap goes where that
// is undone, or says why (view/title_state).
bool pill(cadran::PillMode& mode)
{
    using Kind = TitleState::Kind;
    const bool paused = ui::known_paused();
    const bool unlocked = ui::known_unlocked() && !paused;
    Kind kind;
    if (paused) {
        mode.text = "playguard/cadran/pill/paused"_i18n;
        mode.icon = "pause";
        mode.tone = cadran::Tone::OK;
        kind = Kind::PAUSED;
    } else if (unlocked) {
        mode.text = "playguard/cadran/pill/unlocked"_i18n;
        mode.icon = "unlock";
        kind = Kind::UNLOCKED;
    } else if (app::read_only()) {
        mode.text = "cadran/pill/read_only"_i18n;
        mode.icon = "pencil-off";
        kind = Kind::READ_ONLY;
    } else {
        return false;
    }
    mode.on_tap = [kind]() { TitleState::open(kind); };
    return true;
}

// The screen names of the main screen's tabs, and their sidebar positions.
const std::pair<const char*, int> TABS[] = {
    { "tab/dashboard", ui::tab::dashboard },       { "tab/play_timer", ui::tab::play_timer },
    { "tab/activity", ui::tab::activity },         { "tab/restrictions", ui::tab::restrictions },
    { "tab/security", ui::tab::security },         { "tab/history", ui::tab::history },
    { "tab/console", ui::tab::console },           { "tab/preferences", ui::tab::preferences },
    { "tab/about", ui::tab::about },
};
}   // namespace

void install_screen_hooks()
{
    ui::hooks::Hooks& h = ui::hooks::get();

    h.is_main = [](brls::Activity* a) { return dynamic_cast<MainActivity*>(a) != nullptr; };
    h.open_tab = [](brls::Activity* main, int position, const std::string& row) {
        static_cast<MainActivity*>(main)->open_tab(position, row);
    };
    h.update_title = [](brls::Activity* main) { static_cast<MainActivity*>(main)->update_title(); };
    h.push_main = [](int position, const std::string& row) {
        auto* main = new MainActivity();
        brls::Application::pushActivity(main, brls::TransitionAnimation::NONE);
        main->open_tab(position, row);
    };

    h.refresh_shown = []() { TabBase::refresh_shown(); };
    h.shown_screen = []() { return TabBase::shown_screen(); };
    h.main_screen = []() { return TabBase::main_screen(); };
    h.is_tab = [](brls::View* v) { return dynamic_cast<TabBase*>(v) != nullptr; };
    h.tab_position = [](const std::string& screen) {
        for (const auto& [name, position] : TABS)
            if (screen == name) return position;
        return -1;
    };
    h.settings_tab = []() { return ui::tab::preferences; };
    // main.xml's order: Today | timer, activity, restrictions, security, history | console | preferences, about.
    h.rail_icons = []() {
        return std::vector<std::string>{ "today", "", "timer", "activity", "shield", "lock",
                                         "history", "", "chip", "", "settings", "info" };
    };
    // The lines of PlayGuard's own screens that Cadran dresses, and their icons.
    h.screen_icons = []() {
        return std::vector<std::pair<const char*, const char*>>{
            // First steps
            { "ob_pin", "key" }, { "ob_limit", "timer" }, { "ob_clock", "globe" }, { "ob_unlink", "phone" },
            { "ob_alarm", "bell" }, { "ob_protect", "lock" }, { "ob_at_start", "sparkle" }, { "ob_close", "check" },
            { "ob_support", "heart" },
            // A firmware not checked yet
            { "fg_update", "download" }, { "fg_read_only", "eye" }, { "fg_probe", "tools" }, { "fg_risk", "alert" },
            { "fg_remember", "save" }, { "fg_quit", "stop" },
            // Recovery
            { "rc_reset_pin", "key" }, { "rc_delete", "trash" }, { "rc_continue", "chevron-right" },
            // The play timer's Advanced
            { "pa_show", "eye" }, { "pa_alarm", "bell" }, { "pa_pause", "pause" }, { "pa_resume", "today" },
            { "pa_diag", "code" },
            // A game
            { "ga_today", "today" }, { "ga_week", "calendar" }, { "ga_total", "history" },
            { "ga_launches", "gamepad" }, { "ga_first", "flag" }, { "ga_last", "timer" }, { "ga_rating", "shield" },
        };
    };

    h.send_report = []() { upload_flow::choose(); };

    // PlayGuard itself, for the header: its version and commit, its logo,
    // its pill's states, the console's clock, a tab's name.
    h.version = []() { return app::version(); };
    h.commit = []() { return app::commit(); };
    h.is_release = []() { return app::is_release(); };
    h.read_only = []() { return app::read_only(); };
    h.pill = pill;
    h.today = []() { return ui::today_date(); };
    h.mark = mark;
    h.tab_title = [](const std::string& screen) {
        return screen.rfind("tab/", 0) == 0 ? brls::getStr("playguard/tabs/" + screen.substr(4)) : std::string();
    };
    // The look's preferences live in config.json; the simulators' flags are
    // PLAYGUARD_SIM_* (core/sim_env.h).
    h.save_prefs = []() { return ui::save_config(); };
    h.env_flag = [](const char* name) { return sim_env_flag((std::string("PLAYGUARD_SIM_") + name).c_str()); };
    ui::use_prefs(config::get().look);

    // The notes PlayGuard adds to its dialogs, drawn small at the bottom with
    // their icon: the linked phone app and its help before unlinking, what
    // the DNS's blocking changes, the unlock, a game about to be suspended.
    cadran::add_note("playguard/pairing/note", "phone");
    cadran::add_note("playguard/pairing/decode_note", "code");
    cadran::add_note("playguard/cadran/today/dns_sync_note", "phone");
    cadran::add_note("playguard/cadran/today/dns_time_note", "globe");
    cadran::add_note("playguard/play_timer/gate/body", "unlock");
    cadran::add_note("playguard/cadran/dialog/unlock_short", "unlock");
    cadran::add_note("playguard/play_timer/suspend_warning", "alert", true);

    // What PlayGuard's own ui/ and view/ ask (ui/app_hooks.hpp).
    ui::app_hooks::Hooks& a = ui::app_hooks::get();
    a.open_notices = []() { NoticesActivity::open(); };
    a.open_firmware_screen = []() { brls::Application::pushActivity(new FirmwareGateActivity()); };
    a.make_help_tab = []() -> brls::View* { return new HelpTab(); };
    a.note_write = [](const char* what, bool ok) { notices::note_write(what, ok); };
    a.lock_screen_up = []() {
        for (brls::Activity* act : brls::Application::getActivitiesStack())
            if (dynamic_cast<LockActivity*>(act)) return true;
        return false;
    };
    a.mode_changed = []() {
        pt_log_flow::apply();
        frame_stats_flow::apply();
        sync_flow::changed();   // read-only mode is in the remote link's state
        SysInfo si;             // and in what the agent may do
        sysinfo_get(&si);
        char fw[16];
        sysinfo_version_string(si.hos_version, fw, sizeof(fw));
        sync_files::set_console(fw, app::read_only());
        sync_files::export_nro_state();
    };
}
