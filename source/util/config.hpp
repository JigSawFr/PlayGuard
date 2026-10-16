// config — small persistent settings in sd:/config/playguard/config.json, and
// the part that belongs to one system in nand/<system>/state.json (Snapshot).
// Every key, its values and default: docs/config.md (keep it in step).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

namespace config
{

struct Config
{
    std::string language = "system";   // "system" or one of LANGUAGES
    std::string theme    = "system";   // "system", "light", "dark"
    std::string ntp_server;            // empty: pick from the console region
    std::vector<std::string> custom_servers;
    bool advanced = false;             // show debug-class play-timer actions
    bool auto_relock = true;           // lock again right after a change that needed an unlock
    bool dev_mode = false;             // developer tools (7 presses on About > Version)
    bool pt_log = false;               // Developer › record the play timer (action/pt_log_flow)
    // How an update is installed: "" (not chosen yet: asked on the first
    // update), "inplace" (PlayGuard downloads it and replaces itself),
    // "auto", "sphaira", "appstore", "manual".
    std::string update_via;
    bool update_daily = false;         // check for updates at start-up, once a day
    std::string update_checked;        // "YYYY-MM-DD" of the last check, any kind
    // The latest release the last check found ("1.2.0"): a newer one than the
    // running version is offered at each start-up (action/update_flow) until
    // installed or postponed. update_snooze_version / _until: "Later" on that
    // screen, for that version until that day ("YYYY-MM-DD", 30 days at most).
    std::string update_latest;
    std::string update_snooze_version;
    std::string update_snooze_until;
    // What the console runs on (action/sys_update_flow): the latest Atmosphère,
    // hekate and firmware the last check found (util/sys_updates.hpp's text),
    // and the day of that check ("YYYY-MM-DD").
    std::string sys_latest;
    std::string sys_checked;
    // PlayGuard's Home Assistant integration (action/ha_update_flow): its
    // latest release the last check found, and the day of that check.
    std::string ha_latest;
    std::string ha_checked;
    std::string start_tab = "dashboard";   // tab shown at start-up (main.xml order: START_TABS)
    std::vector<int> extra_amounts = { 15, 30, 60 };   // "Extra time today" choices, minutes
    int  activity_period = 1;          // Activity's period: 0 today, 1 last 7 days, 2 all time
    int  export_format   = 0;          // last export format (table_export::Format)
    int  backup_keep     = 0;          // backups kept after a new one (0: all of them)
    // The saved reports and the crash files kept, each folder on its own
    // (util/log_files.hpp; 0: no limit): the newest logs_keep, those of the
    // last logs_keep_days days, the newest that fit in logs_keep_mb MB.
    int  logs_keep       = 20;
    int  logs_keep_days  = 0;
    int  logs_keep_mb    = 0;
    bool clock_check_at_start = false; // measure the network clock at start-up, say when it is off
    bool intro = true;                 // the loading screen stays 2 s at start-up (activity/loading_activity.hpp)
    std::string dev_builds_order = "newest";   // Install another build's pull requests: "newest" or "number"
    // Security › Ask for the PIN: "off", "changes" (before the first change,
    // then not for 5 min) or "open" (to open PlayGuard). See pin_lock.hpp.
    // "changes" by default, and when the value is missing or unknown: a
    // damaged file must not turn the prompt off.
    std::string pin_lock = "changes";
    // First steps opens by itself at start-up while no PIN is set, unless
    // its "Show at start-up" switch was turned off.
    bool onboarding_at_start = true;

    // "Support PlayGuard" once a month at start-up (Preferences; off stays
    // off, updates included), and "What's new" once after each update. See
    // util/support.hpp. support_reminded: "YYYY-MM-DD" of the last reminder
    // (or of when the month started); seen_version: the version whose
    // "What's new" was handled.
    bool        support_reminder = true;
    std::string support_reminded;
    std::string seen_version;
    // The agent sysmodule PlayGuard carries (its SHA-256) that the parent
    // answered "Later" to at start-up: not offered again at start (Tools ›
    // Optional modules still offers it).
    std::string agent_update_skipped;

    // The newest crash file (logs/crashes/) the notification bell told about:
    // a newer one means PlayGuard closed unexpectedly since.
    std::string crash_seen;

    // The notification bell (action/notices): which kinds of entries it
    // shows (Preferences › Notifications), the entries for information read
    // ("id:hash" of their text, the newest last, at most 48: a read one comes
    // back when its text changes), and the day the weekly summary was read.
    bool notices_system    = true;   // newer Atmosphère, hekate, firmware
    bool notices_summary   = true;   // the week's play time, a limit passed several days
    bool notices_reminders = true;   // a backup to make, the other system, a long console lock
    std::vector<std::string> notices_read;
    std::string summary_seen;

    // Console lock (Security › Console lock): every day's limit set to 0, so a
    // PIN is needed to play. console_lock is whether it is on; console_lock_prev
    // keeps the seven limits it replaced (Sun..Sat minutes, 65535 no limit), to
    // put back when it is turned off. Empty: nothing to put back (the timer was
    // off), so turning it off clears the limit.
    bool             console_lock = false;
    std::string      console_lock_since;   // "YYYY-MM-DD" it was turned on; "" unknown
    std::vector<int> console_lock_prev;

    // Choice made on the "firmware not supported yet" screen, remembered for
    // one firmware with one app version: "read_only", "probe" or "risk".
    std::string fw_gate_fw;
    std::string fw_gate_app;
    std::string fw_gate_choice;

    // "Extra time today" or "No more play today": the weekday limit changed
    // on extra_date (YYYY-MM-DD) from extra_base to extra_value minutes, for
    // that day only (extra_base may be 0xFFFF, no limit). extra_weekday < 0:
    // none pending.
    int         extra_weekday = -1;
    std::string extra_date;
    int         extra_base  = 0;
    int         extra_value = 0;
    // Put that limit back by itself the next day (when the timer needs the
    // temporary unlock, that is still asked) instead of asking keep / put back.
    bool        extra_auto_restore = false;

    // Set right before PlayGuard unlocks parental controls for a change it
    // locks again afterwards, cleared once it has. Still set at the next start:
    // the app stopped in between, so it locks again then.
    bool relock_pending = false;
};

constexpr int    SCHEMA             = 1;    // "schema" in config.json
// The languages PlayGuard is translated into, after "system" (the console's
// own): each has resources/i18n/<code>/playguard.json and a name under
// playguard/tools/languages/. tools/check_resources.py checks both; README
// "Translating PlayGuard" lists the steps to add one.
constexpr const char* LANGUAGES[] = { "system", "en-US", "fr", "fr-CA", "de", "es", "es-419", "it", "nl", "pt", "pt-BR", "ru", "ja", "ko", "zh-Hans", "zh-Hant" };
// What start_tab may be, in the sidebar's order.
constexpr const char* START_TABS[] = { "dashboard", "play_timer", "activity", "restrictions",
                                       "clock", "security", "preferences", "tools", "about" };
// What extra_amounts may be (the Preferences picker offers these sets).
constexpr int EXTRA_SETS[][3] = { { 15, 30, 60 }, { 10, 20, 30 }, { 30, 60, 90 }, { 5, 10, 15 } };
constexpr int BACKUP_KEEP[]   = { 0, 5, 10, 20 };
// What the Tools tab offers for the reports and crash files kept.
constexpr int LOGS_KEEP[]      = { 0, 5, 10, 20, 50 };
constexpr int LOGS_KEEP_DAYS[] = { 0, 7, 30, 90, 365 };
constexpr int LOGS_KEEP_MB[]   = { 0, 1, 5, 20 };
constexpr size_t MAX_HOST           = 253;  // longest DNS name
constexpr size_t MAX_CUSTOM_SERVERS = 10;

// What the last load() found. A config.json that is there but cannot be read
// or parsed (not one wrong field: those keep their default alone) is put
// aside as config.json.bad (state.json the same: state.json.bad, with the
// same notice) (paths::put_aside: the previous .bad is kept as
// .bad.1), and the defaults are used. When it cannot be moved, save() refuses
// to write for the rest of the session rather than replace it.
enum class Loaded { Missing, Read, PutAside, Stuck };

Config& get();
void    load();          // never throws; missing / broken file => defaults, per field
Loaded  loaded();
bool    save();          // atomic write; false on failure (or Loaded::Stuck)
// save() in two steps, so the write can be left to another thread: the
// files' content now (false: Loaded::Stuck, nothing may be written), then
// writing them. write() touches no config state: callable from any thread.
// `shared` is config.json; `system` is this system's state.json
// (paths::state_file(): the console lock, the extra-time record, a pending
// relock, the firmware choice), so that one system's state never acts on
// another (util/nand.hpp).
struct Snapshot
{
    std::string shared, system;
};
bool    snapshot(Snapshot& snap);
bool    write(const Snapshot& snap);
// Runs at the end of every successful snapshot(), on its thread (the remote
// link mirrors what the agent must know into sync/nro_state.txt). nullptr:
// nothing.
void    set_saved_hook(void (*hook)());
// Puts every out-of-range value back to its default (load() calls it).
void    sanitize(Config& c);

}   // namespace config
