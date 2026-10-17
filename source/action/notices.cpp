// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/notices.hpp"

#include <algorithm>
#include <borealis.hpp>
#include <chrono>
#include <cstdio>
#include <map>
#include <sys/statvfs.h>

#include "action/backup_flow.hpp"
#include "action/clock_flow.hpp"
#include "action/history_flow.hpp"
#include "action/outside_watch.hpp"
#include "action/pin_lock.hpp"
#include "action/play_data.hpp"
#include "action/pt_flow.hpp"
#include "action/upload_flow.hpp"
#include "activity/main_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "app.hpp"
#include "tab/clock_tab.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/backup.hpp"
#include "util/config.hpp"
#include "util/log_files.hpp"
#include "util/paths.hpp"
#include "util/update.hpp"

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

// Preferences › Notifications: which kind an entry is, and whether that kind
// is shown. Every other entry always is.
bool shown_kind(const std::string& id)
{
    const auto& cfg = config::get();
    if (id.rfind("sys_", 0) == 0) return cfg.notices_system;
    if (id == "week" || id == "over_limit") return cfg.notices_summary;
    if (id == "backup" || id == "other_system" || id == "console_lock_long") return cfg.notices_reminders;
    return true;
}

// An entry for information marked as read: its id and a hash of its text,
// remembered in config.json, so it stays read at the next start until its
// text changes.
std::string read_key(const std::string& id, const std::string& title, const std::string& detail)
{
    uint32_t h = 2166136261u;   // FNV-1a
    for (const std::string* part : { &title, &detail })
        for (unsigned char c : *part) h = (h ^ c) * 16777619u;
    char hex[9];
    std::snprintf(hex, sizeof(hex), "%08x", (unsigned)h);
    return id + ":" + hex;
}

bool was_read(const std::string& key)
{
    const auto& read = config::get().notices_read;
    return std::find(read.begin(), read.end(), key) != read.end();
}

// "2026-10-08" from a backup's name ("20261008_142530.json"); "" otherwise.
std::string backup_date(const std::string& name)
{
    if (name.size() < 8) return "";
    for (int i = 0; i < 8; i++)
        if (name[i] < '0' || name[i] > '9') return "";
    return name.substr(0, 4) + "-" + name.substr(4, 2) + "-" + name.substr(6, 2);
}

// `date` ("YYYY-MM-DD") is `days` days ago or more.
bool days_ago(const std::string& date, int days)
{
    const std::string then = update::add_days(date, days);
    return !then.empty() && then <= ui::today_date();
}

// Free space on the SD card, bytes; false when it cannot be read. On the
// desktop build, PLAYGUARD_SIM_SD_FREE_MB only (the computer's disk says
// nothing about a console).
bool sd_free(uint64_t* bytes)
{
#ifndef __SWITCH__
    const char* sim = std::getenv("PLAYGUARD_SIM_SD_FREE_MB");
    if (!sim) return false;
    *bytes = (uint64_t)std::strtoull(sim, nullptr, 10) << 20;
    return true;
#else
    struct statvfs st;
    if (statvfs(paths::sd_root().c_str(), &st) != 0) return false;
    *bytes = (uint64_t)st.f_bavail * st.f_frsize;
    return true;
#endif
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
    if (!shown_kind(id)) {
        clear(id);
        return;
    }
    s_actions[id] = Act{ action ? action_label : "", action };
    bool moved = s_store.set(id, level, title, detail);
    // Read at an earlier start, with the same text: still read.
    if (level == Level::Info && s_store.has(id) && was_read(read_key(id, title, detail))) moved |= s_store.dismiss(id);
    if (moved) changed();
}

void clear(const std::string& id)
{
    s_actions.erase(id);
    if (s_store.clear(id)) changed();
}

void dismiss(const std::string& id)
{
    // Its text, for the key remembered in config.json.
    std::string title, detail;
    for (const auto& item : s_store.visible())
        if (item.id == id) title = item.title, detail = item.detail;
    if (!s_store.dismiss(id)) return;
    auto& read = config::get().notices_read;
    // One key per entry: an older text of it goes.
    read.erase(std::remove_if(read.begin(), read.end(),
                              [&id](const std::string& k) { return k.rfind(id + ":", 0) == 0; }),
               read.end());
    read.push_back(read_key(id, title, detail));
    if (read.size() > 48) read.erase(read.begin(), read.end() - 48);
    ui::save_config();
    changed();
}

void apply_preferences()
{
    for (const auto& item : s_store.visible())
        if (!shown_kind(item.id)) clear(item.id);
    sweep();
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

int console_count()
{
    return s_store.console_count();
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

namespace
{
// The last readings refresh_status() was given, for the minute's sweep.
PctlStatus                            s_status{};
std::chrono::steady_clock::time_point s_status_at;
bool                                  s_status_known = false;
}   // namespace

void refresh_status(const PctlStatus& s, const PtState& pt, const SysInfo& si, Result accuracy_rc, bool accurate)
{
    const bool writable = !app::read_only();
    s_status = s;
    s_status_at = std::chrono::steady_clock::now();
    s_status_known = true;
    ui::SameTime same;   // the date and the weekday of every entry below, read once

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
                   else ClockTab::open_screen();
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
           ui::fw_text(si) + " · " + ui::compat_text(si), open_label("console"), []() { open_tab(ui::tab::console); });
    toggle(ui::serial_warning(si), "serial", Level::Info, "playguard/notices/serial"_i18n,
           "playguard/notices/serial_detail"_i18n, open_label("console"), []() { open_tab(ui::tab::console); });
    const patches::Report& report = ui::patch_report();
    bool warn = false;
    toggle(ui::patches_warning(report), "patches", Level::Action, "playguard/notices/patches"_i18n,
           ui::patches_text(report) + "\n" + ui::patches_note(report, si, &warn), open_label("console"),
           []() { open_tab(ui::tab::console); });

    const auto& cfg = config::get();
    const bool has_pin = s.pin_length_ok && s.pin_length > 0;

    // A PIN on the console, but PlayGuard itself opens and changes without it.
    toggle(has_pin && cfg.pin_lock == "off", "pin_lock_off", Level::Action, "playguard/notices/pin_lock_off"_i18n,
           "playguard/notices/pin_lock_off_detail"_i18n, "playguard/notices/pin_lock_choose"_i18n,
           []() { pin_lock::choose([]() { sweep(); }); });

    // The other system (sysMMC / emuMMC) has parental controls of its own.
    const std::string other = ui::other_storage_note(si);
    toggle(!other.empty(), "other_system", Level::Info, "playguard/notices/other_system"_i18n, other, open_label("console"),
           []() { open_tab(ui::tab::console); });

    // The console lock on for a week or more.
    toggle(cfg.console_lock && !cfg.console_lock_since.empty() && days_ago(cfg.console_lock_since, 7),
           "console_lock_long", Level::Info, brls::getStr("playguard/notices/console_lock_long", cfg.console_lock_since),
           "playguard/notices/console_lock_long_detail"_i18n, open_label("security"),
           []() { open_tab(ui::tab::security); });

    // No settings backup yet, or none for 30 days.
    const std::vector<std::string> backups = backup::list();
    const std::string last = backups.empty() ? "" : backup_date(backups.front());
    toggle(backups.empty() || (!last.empty() && days_ago(last, 30)), "backup", Level::Info,
           backups.empty() ? "playguard/notices/backup_none"_i18n : brls::getStr("playguard/notices/backup_old", last),
           "playguard/notices/backup_detail"_i18n, "playguard/notices/backup_now"_i18n, []() {
               backup_flow::save_now();
               sweep();
           });

    // The SD card nearly full: PlayGuard's own writes start failing.
    uint64_t free_b = 0;
    const bool free_known = sd_free(&free_b);   // before the text below, which shows it
    toggle(free_known && free_b < (500ULL << 20), "sd_space", Level::Action, "playguard/notices/sd_space"_i18n,
           brls::getStr("playguard/notices/sd_space_detail", (int)(free_b >> 20)));

    // The play data (read in the background for the Overview).
    const auto stats = play_data::latest("");
    if (stats && stats->windows_ok) {
        // The week, once every 7 days until read.
        const activity_summary::Summary week = activity_summary::summarize(*stats, activity_summary::Week);
        if (week.known && week.total_s > 0 && (cfg.summary_seen.empty() || days_ago(cfg.summary_seen, 7))) {
            std::string detail = brls::getStr("playguard/notices/week_detail", ui::fmt_play_time(week.per_day_s));
            if (week.top >= 0 && stats->games[week.top].name[0])
                detail += "\n" + brls::getStr("playguard/notices/week_top", std::string(stats->games[week.top].name),
                                              ui::fmt_play_time(week.top_s));
            set("week", Level::Info, brls::getStr("playguard/notices/week", ui::fmt_play_time(week.total_s)), detail,
                open_label("activity"), []() {
                    config::get().summary_seen = ui::today_date();
                    ui::save_config();
                    clear("week");
                    open_tab(ui::tab::activity);
                });
        } else {
            clear("week");
        }
        // The console only warning at the limit, and the limit passed on
        // several of the last 7 days.
        int over = 0;
        if (pt.valid && pt.alarm_only)
            for (int k = 0; k < 7; k++) {
                const uint16_t limit = pt.day_min[stats->day_wday[k] % 7];
                if (limit == PT_DAY_NOLIMIT) continue;
                uint64_t played = 0;
                for (uint32_t g = 0; g < stats->count; g++) played += stats->games[g].day_s[k];
                if (played > (uint64_t)limit * 60) over++;
            }
        toggle(over >= 2, "over_limit", Level::Info, brls::getStr("playguard/notices/over_limit", over),
               "playguard/notices/over_limit_detail"_i18n, open_label("play_timer"),
               []() { open_tab(ui::tab::play_timer); });
    }
}

void note_pin_strength(const PctlStatus& s)
{
    // Read once per start, and again when the PIN's length changes (a new PIN).
    static int checked_len = -1;
    if (!s.pin_length_ok) return;
    if ((int)s.pin_length == checked_len) return;
    checked_len = (int)s.pin_length;
    bool weak = false;
    const bool known = s.pin_length > 0 && R_SUCCEEDED(pctl_pin_weak(&weak));
    toggle(known && weak, "pin_weak", Level::Action, "playguard/notices/pin_weak"_i18n,
           "playguard/notices/pin_weak_detail"_i18n, open_label("security"), []() { open_tab(ui::tab::security); });
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

void sweep_if_stale()
{
    // The Overview read the same a moment ago (every 5 s while it is shown):
    // nothing new to ask the console.
    if (s_status_known && std::chrono::steady_clock::now() - s_status_at < std::chrono::seconds(10)) {
        note_pin_strength(s_status);
        return;
    }
    sweep();
}

void sweep()
{
    ui::SameTime same;
    PctlStatus s;
    PtState    pt;
    pctl_overview_fetch(&s, &pt);
    bool accurate = false;
    const Result accuracy_rc = time_network_accuracy(&accurate);
    SysInfo si;
    sysinfo_get(&si);
    outside_watch::observe(pt);
    refresh_status(s, pt, si, accuracy_rc, accurate);
    note_pin_strength(s);
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
