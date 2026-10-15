// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/notices.hpp"

#include <borealis.hpp>
#include <map>

#include "action/clock_flow.hpp"
#include "action/outside_watch.hpp"
#include "action/pt_flow.hpp"
#include "activity/main_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "action/history_flow.hpp"
#include "action/upload_flow.hpp"
#include "app.hpp"
#include "util/config.hpp"
#include "util/log_files.hpp"
#include "util/paths.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace notices
{

namespace
{
notice_logic::Store s_store;

struct Act
{
    std::string           label;
    std::function<void()> run;
};
std::map<std::string, Act> s_actions;

std::map<int, std::function<void()>> s_listeners;
int s_next_listener = 1;

void changed()
{
    // A copy: a listener may unlisten (a screen closing).
    const auto listeners = s_listeners;
    for (const auto& l : listeners) l.second();
}

std::string open_label(const char* tab)
{
    return brls::getStr("playguard/notices/open_tab", brls::getStr(std::string("playguard/tabs/") + tab));
}

// One entry that is there while `on`, gone otherwise.
void toggle(bool on, const std::string& id, Level level, const std::string& title, const std::string& detail,
            const std::string& action_label = "", std::function<void()> action = nullptr)
{
    if (on) set(id, level, title, detail, action_label, action);
    else clear(id);
}
}   // namespace

void set(const std::string& id, Level level, const std::string& title, const std::string& detail,
         const std::string& action_label, std::function<void()> action)
{
    s_actions[id] = Act{ action ? action_label : "", action };
    if (s_store.set(id, level, title, detail)) changed();
}

void clear(const std::string& id)
{
    s_actions.erase(id);
    if (s_store.clear(id)) changed();
}

void dismiss(const std::string& id)
{
    if (s_store.dismiss(id)) changed();
}

std::vector<Entry> list()
{
    std::vector<Entry> out;
    for (const auto& item : s_store.visible()) {
        Entry e;
        e.item = item;
        auto it = s_actions.find(item.id);
        if (it != s_actions.end()) {
            e.action_label = it->second.label;
            e.action       = it->second.run;
        }
        out.push_back(e);
    }
    return out;
}

int action_count()
{
    return s_store.action_count();
}

int info_count()
{
    return s_store.info_count();
}

int listen(std::function<void()> fn)
{
    const int id = s_next_listener++;
    s_listeners[id] = std::move(fn);
    return id;
}

void unlisten(int id)
{
    s_listeners.erase(id);
}

void refresh_status(const PctlStatus& s, const PtState& pt, const SysInfo& si, Result accuracy_rc, bool accurate)
{
    const bool writable = !app::read_only();

    // Nothing set up yet: no PIN, so nothing is protected.
    toggle(s.pin_length_ok && s.pin_length == 0, "pin_missing", Level::Action, "playguard/notices/pin_missing"_i18n,
           "playguard/notices/pin_missing_detail"_i18n, "playguard/onboarding/open"_i18n,
           []() { brls::Application::pushActivity(new OnboardingActivity()); });

    // Unlocked temporarily: no limit at all until locked again.
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    toggle(unlocked, "unlocked", Level::Action, "playguard/notices/unlocked"_i18n, "playguard/notices/unlocked_detail"_i18n,
           writable ? "playguard/security/banner_action"_i18n : "", []() {
               const Result rc = pctl_relock();
               if (R_SUCCEEDED(rc)) history_flow::record_event("relock");
               ui::notify_result(rc, "playguard/toast/relocked"_i18n, "playguard/toast/relock_err"_i18n);
               sweep();
               TabBase::refresh_shown();
           });

    // Extra time added on an earlier day still raises a limit.
    toggle(pt_flow::restore_pending(pt), "extra_restore", Level::Action, pt_flow::restore_label(),
           "playguard/notices/extra_restore_detail"_i18n, "playguard/dashboard/extra_restore"_i18n, []() {
               pt_flow::offer_extra_time_restore([]() {
                   sweep();
                   TabBase::refresh_shown();
               });
           });

    // The "Time's up" alarm off: nothing says when time is up.
    toggle(pt_flow::alarm_off(pt), "alarm_off", Level::Action, "playguard/notices/alarm_off"_i18n,
           "playguard/notices/alarm_off_detail"_i18n, writable ? "playguard/notices/alarm_on"_i18n : open_label("play_timer"),
           [writable]() {
               if (writable) pt_flow::turn_alarm_on("notices", []() { sweep(); });
               else open_tab(ui::tab::play_timer);
           });

    // Changes seen outside PlayGuard today (the Overview says them too).
    const std::vector<std::string> outside = outside_watch::notice_lines();
    std::string outside_text;
    for (const auto& l : outside) outside_text += (outside_text.empty() ? "" : "\n") + l;
    toggle(!outside.empty(), "outside", Level::Action, "playguard/notices/outside"_i18n, outside_text,
           "playguard/dashboard/outside_dismiss"_i18n, []() {
               outside_watch::dismiss();
               clear("outside");
               TabBase::refresh_shown();   // the Overview's notice goes too
           });

    // The network clock the play timer counts on.
    // (clock_check's measure at start-up is an entry of its own, clock_off.)
    if (R_SUCCEEDED(accuracy_rc))
        toggle(!accurate, "clock", Level::Action, "playguard/notices/clock"_i18n, "playguard/notices/clock_detail"_i18n,
               writable ? "playguard/notices/clock_fix"_i18n : open_label("clock"), [writable]() {
                   if (writable) clock_flow::guided([]() { sweep(); });
                   else open_tab(ui::tab::clock);
               });

    // A user account never seen before (flagged in the Activity tab too).
    const std::vector<std::string> accounts = outside_watch::new_account_lines();
    std::string accounts_text;
    for (const auto& l : accounts) accounts_text += (accounts_text.empty() ? "" : "\n") + l;
    toggle(!accounts.empty(), "new_account", Level::Info, "playguard/notices/new_account"_i18n, accounts_text,
           open_label("activity"), []() { open_tab(ui::tab::activity); });

    // A linked phone app overwrites what is set here at its next sync.
    toggle(s.pairing_active_ok && s.pairing_active, "pairing", Level::Info, "playguard/notices/pairing"_i18n,
           "playguard/notices/pairing_detail"_i18n, open_label("security"), []() { open_tab(ui::tab::security); });

    // The system: what the Overview shows in amber.
    toggle(sysinfo_compat(&si) != SysCompat_Ok, "compat", Level::Info, "playguard/notices/compat"_i18n,
           ui::fw_text(si) + " · " + ui::compat_text(si), open_label("tools"), []() { open_tab(ui::tab::tools); });
    toggle(ui::serial_warning(si), "serial", Level::Info, "playguard/notices/serial"_i18n,
           "playguard/notices/serial_detail"_i18n, open_label("tools"), []() { open_tab(ui::tab::tools); });
    const patches::Report& report = ui::patch_report();
    bool warn = false;
    toggle(ui::patches_warning(report), "patches", Level::Action, "playguard/notices/patches"_i18n,
           ui::patches_text(report) + "\n" + ui::patches_note(report, si, &warn), open_label("tools"),
           []() { open_tab(ui::tab::tools); });
}

void note_timer_stalled(bool stalled)
{
    toggle(stalled, "not_counting", Level::Action, "playguard/notices/not_counting"_i18n,
           "playguard/play_timer/not_counting"_i18n, open_label("play_timer"), []() { open_tab(ui::tab::play_timer); });
}

void note_crash()
{
    const std::vector<std::string> crashes = log_files::list(paths::crashes_dir(), log_files::CRASH);
    if (crashes.empty()) return;
    auto& cfg = config::get();
    const std::string newest = crashes.front();
    if (newest == cfg.crash_seen) return;
    const bool first = cfg.crash_seen.empty();
    cfg.crash_seen = newest;
    ui::save_config();
    if (first) return;
    // The file's line: when, the version, what was thrown.
    std::string text;
    if (paths::read_file(paths::crashes_dir() + "/" + newest, text)) text = text.substr(0, text.find('\n'));
    set("crash", Level::Info, "playguard/notices/crash"_i18n, text, "playguard/notices/send_report"_i18n, []() {
        dismiss("crash");
        upload_flow::choose();
    });
}

void note_write(const char* what, bool ok, const std::string& error)
{
    const std::string id = std::string("sd_") + what;
    if (ok) {
        clear(id);
        return;
    }
    const std::string file = std::string(what) == "config" ? "config.json" : std::string(what) + ".json";
    set(id, Level::Action, "playguard/notices/sd_write"_i18n,
        brls::getStr("playguard/notices/sd_write_detail", file) + (error.empty() ? "" : " (" + error + ")"));
}

void sweep()
{
    PctlStatus s;
    PtState    pt;
    pctl_overview_fetch(&s, &pt);
    bool accurate = false;
    const Result accuracy_rc = time_network_accuracy(&accurate);
    SysInfo si;
    sysinfo_get(&si);
    outside_watch::observe(pt);
    refresh_status(s, pt, si, accuracy_rc, accurate);
}

void open_tab(int position)
{
    for (brls::Activity* a : brls::Application::getActivitiesStack())
        if (auto* main = dynamic_cast<MainActivity*>(a)) {
            main->open_tab(position);
            return;
        }
}

}   // namespace notices
