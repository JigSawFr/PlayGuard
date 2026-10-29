// ui — PlayGuard's own helpers (background tasks, the SD card's writes,
// config.json, result codes, the console's clock and system), beside Aurore's
// Cadran helpers (Aurore/cadran/ui/ui.hpp: dialogs, toasts, pickers…) in
// the same namespace.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iterator>
#include <string>

#include <aurore/cadran/ui/ui.hpp>

#include "core/calendar.h"
#include "core/nx_types.h"
#include "core/sysinfo.h"
#include "ui/play_text.hpp"
#include "util/patches.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/prodinfo.hpp"

namespace ui
{

// "error 0x00001234" followed by a human explanation when one is known.
// PlayGuard's own results (read-only, invalid value …) are just the sentence:
// there is no code to look up. A system code gets, in a paragraph of its own,
// what to do if it happens again (send a report); `next_step` false leaves it
// out (the start-up error and recovery screens say what to do themselves).
std::string rc_text(Result rc, bool next_step = true);

// Runs `task` on a thread of its own, rather than aurore::async: that single
// queue runs one task after the other, so a network request queued there
// waits behind reading the play log and loading game icons (minutes, on a
// console with many games), or behind another request's timeout. Every
// network task goes here; the play log and the icons stay on the queue, one
// at a time on the console's services. Queued there when no thread can be
// had. `what` names it in the log. The thread is never detached (the console
// cannot: util/worker_threads.hpp); it is joined once the task is over.
void in_background(const char* what, std::function<void()> task);
// At exit, after the main loop: says the app is quitting (quitting() turns
// true, for a task that waits or polls) and waits up to `max` for the tasks
// of in_background still running, so none of them outlives the objects it
// uses (aurore::sync's queue, curl, statics), joining the threads of those
// that ended. The writes of write_later are finished first, and any asked
// for later run at once. False when some are still running.
bool finish_background(std::chrono::milliseconds max);
bool quitting();

// One screen of its own at start-up, the first to ask: the firmware screen,
// then First steps (both as the main screen opens), the update screen (once
// the check answers), "What's new" or the support reminder (a few seconds
// in). True for that one; the others wait for the next start, as when
// something else is in front of the main screen. `what` names it in the log.
bool claim_start_screen(const char* what);

// An SD-card write off the UI thread (a slow card must not hold a frame), on
// one queue run by a task of in_background: the writes happen one at a time,
// in the order asked. `key` names a file written whole: a write of it still
// waiting is dropped for the newer one. `job` reports its own result (ui::
// notify, ui::error). finish_background runs what is still queued at exit.
void write_later(const char* what, std::function<void()> job, const std::string& key = "");
// The same for a file added to (history.json, the play-timer log): `key`
// names it, and none of its writes is dropped.
void append_later(const char* what, const std::string& key, std::function<void()> job);
// Until every write asked so far is on the SD card, the waiting ones run on
// this thread: off the UI thread only (a report collecting every file).
void settle_writes();
// Until every write of the file `key` asked so far is on the SD card: before
// reading it back (history.json). Its writes still waiting run here; those
// of other files are left to the queue, so a long export does not hold the
// UI thread.
void settle_writes(const std::string& key);

void refuse(Result rc);   // its rc_text
// `what` failed with `rc`: `what` as the dialog's title, the error's hint as
// its text, its code small, and Send a report for a console code ("" for
// `what`: the hint alone). `before`: what else happened, said first (what
// went through, or another failure).
void error(const std::string& what, Result rc, const std::string& before = "");
// Saves the settings (config::snapshot, then config::write off the UI
// thread: write_later): `saved_text` as a toast once it is on the SD card,
// `failed_text` (default: the config_err toast) when the card refused it.
// False only when it cannot be saved at all (an unreadable config.json kept:
// config::snapshot refuses to overwrite it), said at once.
bool save_config(const std::string& saved_text = "", const std::string& failed_text = "");
// The same, waiting for the write: for a record that must be on the SD card
// before the console change it describes. False when it is not. Only for
// config.json: it goes ahead of the other writes still waiting, and runs here
// when the queue is on another file's.
bool save_config_now();
// Success: `ok_text` as a toast. Failure: `error_prefix — reason` as a dialog.
void notify_result(Result rc, const std::string& ok_text, const std::string& error_prefix);

// Reads the stored PIN (pctl_get_pin) and shows it in large digits, or a
// dialog with the error. The app's copies are wiped as soon as the label
// holds the text. True when it was shown; `on_close` then runs once it is
// closed (OK or B). Used by PIN › Show the PIN.
bool show_pin_dialog(std::function<void()> on_close = nullptr);

// A duration set with the stick (15 min a step, 5 with L and R, up to
// 24:00; "no limit" starts at 1 h). The callback only on confirm.
void prompt_minutes(const std::string& header, uint16_t current, std::function<void(uint16_t)> on_value);

// "Now" on the console: its user clock read live and its time-zone rule
// (calendar.h says why not time()/localtime()).
LocalTime   local_now();
int         today_weekday();                    // 0 = Sunday
std::string today_date();                       // "2026-10-07"
std::string now_hms();                          // "14:03:12"
std::string now_stamp();                        // "2026-10-07 14:03"
// While one lives, local_now() and the four above read the console's clock
// once on the thread that made it (two IPCs each time otherwise): a screen's
// refresh asks the time of day a dozen times, through the helpers it calls.
// Other threads read it live. Nested ones keep the outer one's time.
class SameTime
{
public:
    SameTime();
    ~SameTime();
    SameTime(const SameTime&) = delete;
    SameTime& operator=(const SameTime&) = delete;

private:
    bool outer_;
};

// System / compatibility strings shared by the dashboard and the About section;
// `color`, when given, the theme key of the ink to show them in.
std::string hos_text(u32 version);                        // "23.0.1" (MAKEHOSVERSION), "?" for 0
std::string fw_text(const SysInfo& info);                 // "23.0.1 · Atmosphère 1.12.0"
std::string compat_text(const SysInfo& info, const char** color = nullptr);

// The Console tab's and Today's warnings.
std::string storage_text(const SysInfo& info);             // "emuMMC (Atmosphère)"
std::string storage_short(const SysInfo& info);            // "emuMMC" / "sysMMC" / "—"
// The other system (sysMMC from emuMMC, or an emuMMC set up on the card from
// sysMMC) has its own parental controls, out of PlayGuard's sight; "" if none.
std::string other_storage_note(const SysInfo& info);
// The serial-number warning only applies on emuMMC: on sysMMC the real serial
// is normal (online play), blanking it would cut Nintendo's services.
bool        serial_warning(const SysInfo& info);
std::string blank_text(const SysInfo& info, const char** color = nullptr);
// The console's real serial number: the system's when Atmosphère does not
// blank it, else read from its PRODINFO backup on the SD card
// (util/prodinfo.hpp). `note` says where it comes from, the whole number in
// it ("" serial: not found, and why). Looked up once per run.
struct RealSerial
{
    std::string      serial;
    std::string      note;
    prodinfo::Source source = prodinfo::Source::None;   // when blanked
};
const RealSerial& real_serial(const SysInfo& info);
// real_serial() with the middle masked ("XAW1000•••••01"), or "Unavailable".
std::string serial_text(const SysInfo& info);
bool        patches_warning(const patches::Report& report);
// The game-patch report of this boot, read from the SD card once per run (it
// only changes with a reboot): Today and the Console tab no longer rescan it
// each time they are opened.
const patches::Report& patch_report();
// After PlayGuard itself moved patch files (sigpatch_flow): read again.
void                   reread_patch_report();
std::string patches_text(const patches::Report& report, const char** color = nullptr);
// Explanation under the Game patches line ("" when there is nothing to say);
// *warn tells whether it is a warning (amber) or a plain note.
std::string patches_note(const patches::Report& report, const SysInfo& info, bool* warn);
std::string time_text(uint64_t posix);                     // local time, or "—" for 0

// Read-only mode: a toast says so and this returns true (the caller then
// does nothing). Else false.
bool refuse_read_only();
// A BooleanCell flips before telling its listener: in read-only mode the
// switch would flip, the write be refused and the switch flip back. This
// checks first, then flips as BooleanCell does.
void guard_switch(aurore::BooleanCell* cell);

// Sidebar positions of the main screen tabs, in the order of
// resources/xml/activity/main.xml (separators count as positions).
namespace tab
{
constexpr int dashboard    = 0;
constexpr int play_timer   = 2;
constexpr int activity     = 3;   // play time per game
constexpr int restrictions = 4;
constexpr int security     = 5;   // PIN, unlock, protecting PlayGuard, companion app, delete
constexpr int history      = 6;   // the change history and settings backups
constexpr int console      = 8;   // the network clock, installed games, forwarders, firmware, storage, serial, patches
constexpr int preferences  = 10;  // the app's settings and its updates
constexpr int about        = 11;  // version, help, credits
// The n-th tab (0 = Today) as a sidebar position (separators count).
constexpr int of(int n)
{
    constexpr int positions[] = { dashboard, play_timer, activity, restrictions, security, history, console, preferences, about };
    return n >= 0 && n < (int)std::size(positions) ? positions[n] : dashboard;
}
}   // namespace tab

// Temporary unlock as last read by any tab (`valid` false: the read failed,
// keep the previous value). The main screen's title says it while unlocked.
void note_unlocked(bool valid, bool unlocked);
bool known_unlocked();
// That unlock is PlayGuard's own pause while the parent is in it
// (action/session_pause), not one to turn back on: said calmly.
void note_paused(bool paused);
bool known_paused();

// After app::set_read_only / set_dev_mode: updates the main screen title,
// starts or stops the play-timer recorder (action/pt_log_flow) and re-reads
// the tab on screen.
void on_mode_changed();

// PlayGuard's own theme keys (ui/palette.hpp) in both variants, with the
// pairs they keep readable: before the window, whose theme file then
// changes them like any other key.
void install_palette();

}   // namespace ui
