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
#include "action/console_actions.hpp"
#include "action/nintendo_dns.hpp"
#include "action/outside_watch.hpp"
#include "action/pairing_state.hpp"
#include "action/pin_lock.hpp"
#include "action/play_data.hpp"
#include "action/pt_flow.hpp"
#include "action/upload_flow.hpp"
#include "action/week_qr.hpp"
#include "activity/main_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "app.hpp"
#include "core/sim_env.h"
#include "tab/clock_tab.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/backup.hpp"
#include "util/config.hpp"
#include "util/log_files.hpp"
#include "util/paths.hpp"
#include "util/text.hpp"

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
// What each entry the parent may ignore is about (set's `print`).
std::map<std::string, std::string> s_prints;

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

// Preferences › Notifications: whether the kind of entry `id` is shown.
bool shown_kind(const std::string& id)
{
    const auto& cfg = config::get();
    notice_logic::Shown shown;
    shown.system = cfg.notices_system;
    shown.summary = cfg.notices_summary;
    shown.reminders = cfg.notices_reminders;
    return notice_logic::shown_kind(id, shown);
}

bool was_read(const std::string& key)
{
    const auto& read = config::get().notices_read;
    return std::find(read.begin(), read.end(), key) != read.end();
}

// Ignored or counted, as config.json says today (30 days gone by, the PIN
// changed). True when what the bell shows changed.
bool recheck_ack(const std::string& id)
{
    if (!notice_logic::ackable(id)) return false;
    const auto& cfg = config::get();
    auto it = s_prints.find(id);
    const std::string print = it == s_prints.end() ? "" : it->second;
    if (notice_logic::ack_holds(cfg.notices_acked, id, print, ui::today_date(), cfg.notices_ack_remind))
        return s_store.ack(id);
    return s_store.unack(id);
}

void recheck_acks()
{
    bool moved = false;
    for (const char* id : { "pin_weak", "pin_lock_off", "alarm_off", "sd_space" }) moved |= recheck_ack(id);
    if (moved) changed();
}

// The entry's in s_store.visible() and ignored(), with its action.
Entry entry_of(const notice_logic::Item& item)
{
    Entry e;
    e.item = item;
    auto it = s_actions.find(item.id);
    if (it != s_actions.end()) {
        e.action_label = it->second.label;
        e.action       = it->second.run;
    }
    return e;
}

// Free space on the SD card, bytes; false when it cannot be read. On the
// desktop build, PLAYGUARD_SIM_SD_FREE_MB only (the computer's disk says
// nothing about a console).
bool sd_free(uint64_t* bytes)
{
#ifndef __SWITCH__
    const long long mb = sim_env_int("PLAYGUARD_SIM_SD_FREE_MB", -1, 0, 1LL << 40);
    if (mb < 0) return false;
    *bytes = (uint64_t)mb << 20;
    return true;
#else
    struct statvfs st;
    if (statvfs(paths::sd_root().c_str(), &st) != 0) return false;
    *bytes = (uint64_t)st.f_bavail * st.f_frsize;
    return true;
#endif
}

// sd_free(), asked again at most once a minute, or after a backup written
// (sd_written()): Today refreshes every 5 s.
notice_logic::Every s_free_every(60 * 1000);
bool                s_free_known = false;
uint64_t            s_free_bytes = 0;

bool sd_free_kept(uint64_t* bytes)
{
    const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now().time_since_epoch()).count();
    if (s_free_every.due(now)) {
        s_free_known = sd_free(&s_free_bytes);
        s_free_every.taken(now);
    }
    *bytes = s_free_bytes;
    return s_free_known;
}

// One entry that is there while `on`, gone otherwise.
void toggle(bool on, const std::string& id, Level level, const std::string& title, const std::string& detail,
            const std::string& action_label = "", std::function<void()> action = nullptr,
            const std::string& print = "")
{
    if (on) set(id, level, title, detail, action_label, action, print);
    else clear(id);
}

// The same for an entry the parent may ignore: when what it is about was
// read (`known`) and is fine, it is fixed (resolved(): its "ignored" goes,
// so it is told again if it comes back); otherwise toggle().
void toggle_or_resolve(bool known, bool on, const std::string& id, Level level, const std::string& title,
                       const std::string& detail, const std::string& action_label = "",
                       std::function<void()> action = nullptr, const std::string& print = "")
{
    if (known && !on) resolved(id);
    else toggle(on, id, level, title, detail, action_label, action, print);
}
}   // namespace

void set(const std::string& id, Level level, const std::string& title, const std::string& detail,
         const std::string& action_label, std::function<void()> action, const std::string& print)
{
    if (!shown_kind(id)) {
        clear(id);
        return;
    }
    s_actions[id] = Act{ action ? action_label : "", action };
    s_prints[id] = print;
    bool moved = s_store.set(id, level, title, detail);
    // Read at an earlier start, with the same text: still read.
    if (level == Level::Info && s_store.has(id) && was_read(notice_logic::read_key(id, title, detail))) moved |= s_store.dismiss(id);
    // Ignored at an earlier start, about the same: still ignored.
    if (level == Level::Action) moved |= recheck_ack(id);
    if (moved) changed();
}

void clear(const std::string& id)
{
    s_actions.erase(id);
    s_prints.erase(id);
    if (s_store.clear(id)) changed();
}

void resolved(const std::string& id)
{
    clear(id);
    if (notice_logic::ack_forget(config::get().notices_acked, id)) ui::save_config();
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
                              [&id](const std::string& k) { return text::starts_with(k, id + ":"); }),
               read.end());
    read.push_back(notice_logic::read_key(id, title, detail));
    if (read.size() > 48) read.erase(read.begin(), read.end() - 48);
    ui::save_config();
    changed();
}

void dismiss_all()
{
    for (const auto& item : s_store.visible())
        if (item.level == Level::Info) dismiss(item.id);
}

void acknowledge(const std::string& id)
{
    if (!s_store.ack(id)) return;
    auto& acked = config::get().notices_acked;
    notice_logic::ack_forget(acked, id);
    acked.push_back(notice_logic::ack_key(id, s_prints[id], ui::today_date()));
    if (acked.size() > 16) acked.erase(acked.begin(), acked.end() - 16);
    ui::save_config();
    brls::Logger::info("notices: {} ignored", id);
    changed();
}

void reactivate(const std::string& id)
{
    if (!s_store.unack(id)) return;
    notice_logic::ack_forget(config::get().notices_acked, id);
    ui::save_config();
    brls::Logger::info("notices: {} counted again", id);
    changed();
}

void apply_preferences()
{
    for (const auto& item : s_store.visible())
        if (!shown_kind(item.id)) clear(item.id);
    recheck_acks();   // the reminder after 30 days turned on or off
    sweep();
}

std::vector<Entry> list()
{
    std::vector<Entry> out;
    for (const auto& item : s_store.visible()) out.push_back(entry_of(item));
    return out;
}

std::vector<Entry> ignored()
{
    std::vector<Entry> out;
    for (const auto& item : s_store.ignored()) out.push_back(entry_of(item));
    return out;
}

std::string ignored_until(const std::string& id)
{
    const auto& cfg = config::get();
    auto it = s_prints.find(id);
    return notice_logic::ack_until(cfg.notices_acked, id, it == s_prints.end() ? "" : it->second,
                                   cfg.notices_ack_remind);
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

int tab_count(const std::string& tab)
{
    return s_store.tab_count(tab);
}

int count_of(const std::vector<std::string>& ids)
{
    return s_store.count_of(ids);
}

std::string reason_of(const std::vector<std::string>& ids)
{
    const std::string id = s_store.first_of(ids);
    if (id.empty()) return "";
    return brls::getStr("playguard/notices/short/" + id);
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

namespace
{
// What leaves the parental controls open: no PIN, a temporary unlock,
// PlayGuard itself without its PIN, the phone app linked, the console lock
// left on.
void note_security(const PctlStatus& s, bool writable)
{
    // Nothing set up yet: no PIN, so nothing is protected.
    toggle(s.pin_length_ok && s.pin_length == 0, "pin_missing", Level::Action, "playguard/notices/pin_missing"_i18n,
           "playguard/notices/pin_missing_detail"_i18n, "playguard/onboarding/open"_i18n,
           []() { brls::Application::pushActivity(new OnboardingActivity()); });

    // Unlocked temporarily: no limit at all until locked again.
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    toggle(unlocked, "unlocked", Level::Action, "playguard/notices/unlocked"_i18n, "playguard/notices/unlocked_detail"_i18n,
           writable ? "playguard/security/banner_action"_i18n : "", []() {
               console_actions::relock("", []() {
                   sweep();
                   TabBase::refresh_shown();
               });
           });

    // A linked phone app overwrites what is set here at its next sync (not
    // when Atmosphère's DNS blocks its server).
    toggle(pairing_state::syncs(s), "pairing", Level::Info, "playguard/notices/pairing"_i18n,
           "playguard/notices/pairing_detail"_i18n, open_label("security"), []() { open_tab(ui::tab::security); });

    const auto& cfg = config::get();
    const bool has_pin = s.pin_length_ok && s.pin_length > 0;

    // A PIN on the console, but PlayGuard itself opens and changes without it.
    toggle_or_resolve(s.pin_length_ok, has_pin && cfg.pin_lock == "off", "pin_lock_off", Level::Action,
                      "playguard/notices/pin_lock_off"_i18n, "playguard/notices/pin_lock_off_detail"_i18n,
                      "playguard/notices/pin_lock_choose"_i18n, []() { pin_lock::choose([]() { sweep(); }); });

    // The console lock on for a week or more.
    toggle(notice_logic::console_lock_long(cfg.console_lock, cfg.console_lock_since, ui::today_date()),
           "console_lock_long", Level::Info, brls::getStr("playguard/notices/console_lock_long", cfg.console_lock_since),
           "playguard/notices/console_lock_long_detail"_i18n, open_label("play_timer"),
           []() { open_tab(ui::tab::play_timer, "pt_console_lock"); });
}

// What the play timer counts on: its limits back to usual, its alarm, the
// changes seen outside PlayGuard, the network clock.
void note_timer(const PtState& pt, Result accuracy_rc, bool accurate, bool writable)
{
    // Extra time added on an earlier day still raises a limit: to do, the
    // child has more than usual. Or "No more play today" still lowers one
    // (the preference to put it back with no question off): only said, the
    // limit being stricter than usual protects the child no less.
    const bool raised = pt_flow::restore_lowers();
    toggle(pt_flow::restore_pending(pt), "extra_restore", raised ? Level::Action : Level::Info, pt_flow::restore_label(),
           raised ? "playguard/notices/extra_restore_detail"_i18n
                  : brls::getStr("playguard/notices/stop_restore_detail", "playguard/dashboard/stop_title"_i18n),
           "playguard/dashboard/extra_restore"_i18n, []() {
               pt_flow::offer_extra_time_restore([]() {
                   sweep();
                   TabBase::refresh_shown();
               }, true);
           });

    // The "Time's up" alarm off: nothing says when time is up. Read as on
    // (or the timer off): fixed, told again if it goes off again.
    const bool alarm_known = pt.fw_supported && pt.enabled_valid && (!pt.enabled || pt.alarm_disabled_valid);
    toggle_or_resolve(alarm_known, pt_flow::alarm_off(pt), "alarm_off", Level::Action, "playguard/notices/alarm_off"_i18n,
                      "playguard/notices/alarm_off_detail"_i18n,
                      writable ? "playguard/notices/alarm_on"_i18n : open_label("play_timer"), [writable]() {
                          if (writable) pt_flow::turn_alarm_on("notices", []() { sweep(); });
                          else open_tab(ui::tab::play_timer);
                      });

    // Changes seen outside PlayGuard today (Today says them too).
    const std::vector<std::string> outside = outside_watch::notice_lines();
    toggle(!outside.empty(), "outside", Level::Action, "playguard/notices/outside"_i18n, text::join(outside, "\n"),
           "playguard/dashboard/outside_dismiss"_i18n, []() {
               outside_watch::dismiss();
               clear("outside");
               TabBase::refresh_shown();   // Today's notice goes too
           });

    // The network clock the play timer counts on.
    // (clock_check's measure at start-up is an entry of its own, clock_off.)
    if (R_SUCCEEDED(accuracy_rc))
        toggle(!accurate, "clock", Level::Action, "playguard/notices/clock"_i18n,
               nintendo_dns::time_blocked() ? "playguard/notices/clock_detail_dns"_i18n : "playguard/notices/clock_detail"_i18n,
               writable ? "playguard/notices/clock_fix"_i18n : open_label("clock"), [writable]() {
                   if (writable) clock_flow::guided([]() { sweep(); });
                   else ClockTab::open_screen();
               });
}

// The system: what Today shows in amber, and the other system's
// parental controls.
void note_system(const SysInfo& si)
{
    toggle(sysinfo_compat(&si) != SysCompat_Ok, "compat", Level::Info, "playguard/notices/compat"_i18n,
           ui::fw_text(si) + " · " + ui::compat_text(si), open_label("console"), []() { open_tab(ui::tab::console); });
    toggle(ui::serial_warning(si), "serial", Level::Info, "playguard/notices/serial"_i18n,
           "playguard/notices/serial_detail"_i18n, open_label("console"), []() { open_tab(ui::tab::console); });
    const patches::Report& report = ui::patch_report();
    bool warn = false;
    toggle(ui::patches_warning(report), "patches", Level::Action, "playguard/notices/patches"_i18n,
           ui::patches_text(report) + "\n" + ui::patches_note(report, si, &warn), open_label("console"),
           []() { open_tab(ui::tab::console); });

    // The other system (sysMMC / emuMMC) has parental controls of its own.
    const std::string other = ui::other_storage_note(si);
    toggle(!other.empty(), "other_system", Level::Info, "playguard/notices/other_system"_i18n, other, open_label("console"),
           []() { open_tab(ui::tab::console); });
}

// The SD card: the settings backups, its free space.
void note_storage()
{
    // No settings backup yet, or none for 30 days (listed once, then after
    // each backup saved: backup_flow).
    const std::vector<std::string>& backups = backup_flow::names();
    const std::string last = backups.empty() ? "" : notice_logic::backup_date(backups.front());
    toggle(notice_logic::backup_due(backups, ui::today_date()), "backup", Level::Info,
           backups.empty() ? "playguard/notices/backup_none"_i18n : brls::getStr("playguard/notices/backup_old", last),
           "playguard/notices/backup_detail"_i18n, "playguard/notices/backup_now"_i18n, []() {
               backup_flow::save_now();
               sweep();
           });

    // The SD card nearly full: PlayGuard's own writes start failing.
    uint64_t free_b = 0;
    const bool free_known = sd_free_kept(&free_b);   // before the text below, which shows it
    toggle_or_resolve(free_known, notice_logic::sd_space_low(free_known, free_b), "sd_space", Level::Action,
                      "playguard/notices/sd_space"_i18n,
                      brls::getStr("playguard/notices/sd_space_detail", (int)(free_b >> 20)));
}

// Who plays and how much: an account never seen before, the week's summary,
// the limit passed while the console only warns.
void note_play_data(const PtState& pt)
{
    // A user account never seen before (flagged in the Activity tab too).
    const std::vector<std::string> accounts = outside_watch::new_account_lines();
    toggle(!accounts.empty(), "new_account", Level::Info, "playguard/notices/new_account"_i18n, text::join(accounts, "\n"),
           open_label("activity"), []() { open_tab(ui::tab::activity); });

    // The play data (read in the background for Today).
    const auto stats = play_data::latest("");
    if (!stats || !stats->windows_ok) return;
    // The week, once every 7 days until read.
    const auto& cfg = config::get();
    const activity_summary::Summary week = activity_summary::summarize(*stats, activity_summary::Week);
    if (notice_logic::week_due(week.known, week.total_s, cfg.summary_seen, ui::today_date())) {
        std::string detail = brls::getStr("playguard/notices/week_detail", ui::fmt_play_time(week.per_day_s));
        // Against the 7 days before, as on Activity.
        int64_t diff = 0;
        if (activity_summary::week_trend_min(*stats, diff)) {
            const int64_t same = activity_summary::TREND_SAME_MIN;
            const uint16_t amount = (uint16_t)std::min<int64_t>(24 * 60, diff < 0 ? -diff : diff);
            detail += " " + (diff >= same    ? brls::getStr("playguard/notices/week_more", ui::fmt_minutes(amount))
                             : diff <= -same ? brls::getStr("playguard/notices/week_less", ui::fmt_minutes(amount))
                                             : "playguard/notices/week_same"_i18n);
        }
        if (week.top >= 0 && stats->games[week.top].name[0])
            detail += "\n" + brls::getStr("playguard/notices/week_top", std::string(stats->games[week.top].name),
                                          ui::fmt_play_time(week.top_s));
        set("week", Level::Info, brls::getStr("playguard/notices/week", ui::fmt_play_time(week.total_s)), detail,
            "playguard/notices/week_qr"_i18n, []() {
                config::get().summary_seen = ui::today_date();
                ui::save_config();
                clear("week");
                // On Activity, its QR code over it: the week on a phone.
                open_tab(ui::tab::activity);
                if (const auto stats = play_data::latest(""))
                    week_qr::show(*stats, "playguard/activity/account_all"_i18n);
            });
    } else {
        clear("week");
    }
    // The console only warning at the limit, and the limit passed on
    // several of the last 7 days.
    const int over = notice_logic::over_limit_days(pt, *stats);
    toggle(over >= 2, "over_limit", Level::Info, brls::getStr("playguard/notices/over_limit", over),
           "playguard/notices/over_limit_detail"_i18n, open_label("play_timer"),
           []() { open_tab(ui::tab::play_timer); });
}
}   // namespace

void refresh_status(const PctlStatus& s, const PtState& pt, const SysInfo& si, Result accuracy_rc, bool accurate)
{
    const bool writable = !app::read_only();
    s_status = s;
    s_status_at = std::chrono::steady_clock::now();
    s_status_known = true;
    ui::SameTime same;   // the date and the weekday of every entry below, read once

    // By topic; the bell lists them in its own order (notice_logic's ranks).
    note_security(s, writable);
    note_timer(pt, accuracy_rc, accurate, writable);
    note_system(si);
    note_storage();
    note_play_data(pt);

    // An entry ignored 30 days ago counts again (the PIN's, set once per start, too).
    recheck_acks();
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
    // A PIN harder to guess, or none: fixed, told again if a weak one comes back.
    // Ignored, it stays ignored for a PIN of the same length (`print`, never the PIN).
    toggle_or_resolve(known || s.pin_length == 0, known && weak, "pin_weak", Level::Action, "playguard/notices/pin_weak"_i18n,
                      "playguard/notices/pin_weak_detail"_i18n, open_label("security"),
                      []() { open_tab(ui::tab::security); }, std::to_string(s.pin_length));
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

void sd_written()
{
    s_free_every.forget();
}

void sweep_if_stale()
{
    // Today read the same a moment ago (every 5 s while it is shown):
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

void open_tab(int position, const std::string& row)
{
    if (MainActivity* main = ui::main_activity()) main->open_tab(position, row);
}

}   // namespace notices
