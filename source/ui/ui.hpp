// ui — shared helpers for dialogs, toasts, keyboards and formatting.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "ui/format_text.hpp"
#include "ui/cadran_text.hpp"
#include "util/patches.hpp"
#include "util/pctl_ops_c.hpp"

class MainActivity;

namespace ui
{

// The status colours below are the boards' inks (cadran::install_theme(),
// which must run before the first lookup: borealis aborts on an unknown
// theme key), one shade per theme (>= 4.5:1).
// On the console: the default font of the labels made from now on, for the
// strings' language `locale`: the system's Latin / Japanese font, the Chinese
// one only as a fallback; for Chinese, the Chinese one. Right after
// createWindow(), before the first label exists, then again when the language
// changes while PlayGuard runs (live_prefs, which builds the screens again).
void     use_font_for(const std::string& locale);
NVGcolor color_ok();
NVGcolor color_warn();
NVGcolor color_bad();
NVGcolor color_track();     // empty part of a progress bar, a missing cover
NVGcolor color_note();      // explanatory notes under a section
NVGcolor color_neutral();   // DetailCell value colour
NVGcolor color_text();      // plain label text colour
NVGcolor color_accent();    // the theme's highlight (the sidebar's active item)
// A label of `size` in `color` that wraps (not single-line).
brls::Label* text_label(const std::string& text, float size, NVGcolor color);
// The same, centred, wrapping at the width of the box it is in (a dialog's).
brls::Label* centered_label(const std::string& text, float size, NVGcolor color);

// "error 0x00001234" followed by a human explanation when one is known.
// PlayGuard's own results (read-only, invalid value …) are just the sentence:
// there is no code to look up. A system code gets, in a paragraph of its own,
// what to do if it happens again (send a report); `next_step` false leaves it
// out (the start-up error and recovery screens say what to do themselves).
std::string rc_text(Result rc, bool next_step = true);

// Runs `task` on a thread of its own, rather than brls::async: that single
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
// uses (brls::sync's queue, curl, statics), joining the threads of those
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

// Deferred to the next frame (toasts fired right after a system applet returns
// were dropped on fw 22.1.0).
// What a toast says: news (plain), something done (a check), or something
// not done because of how things are (a warning); the icon in the Cadran look.
enum class Toast
{
    Info,
    Done,
    Refused,
};
void notify(const std::string& text, Toast kind = Toast::Info);
// A failure the user asked for something and did not get: a dialog, not a
// toast (a toast is gone before "error 0x…" is read). Next frame, so it can
// be called from a dialog button. Logged as "error: " for the smoke test.
void error(const std::string& text);
// `what` failed with `rc`: `what` as the dialog's title, the error's hint as
// its text, its code small, and Send a report for a console code ("" for
// `what`: the hint alone). `before`: what else happened, said first (what
// went through, or another failure).
void error(const std::string& what, Result rc, const std::string& before = "");
// `what` failed because of `why` (a path, a message from the network…).
void error(const std::string& what, const std::string& why);
// The dialog the three open, now: the title, the reason, the code small, and
// Send a report (OK focused) when `f.report`.
void show_failure(const cadran_text::Failure& f);
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
// `next` in place of the current screen. Borealis never pops the first
// activity (the loading screen, under every start screen): a screen that
// cannot be popped gets `next` pushed over it instead.
void replace_screen(brls::Activity* next);
// Success: `ok_text` as a toast. Failure: `error_prefix — reason` as a dialog.
void notify_result(Result rc, const std::string& ok_text, const std::string& error_prefix);

// Two-button dialog. Cancel (left) has the focus when it opens;
// `confirm_label` (right) runs `on_yes`, Cancel or B runs `on_no`.
// `danger`: the confirm button is drawn in the "bad" colour (see below).
void confirm(const std::string& body, const std::string& confirm_label,
             std::function<void()> on_yes, std::function<void()> on_no = nullptr, bool danger = false);
// Same, for actions that delete something: the confirm button is drawn in the
// "bad" colour so it never looks like an ordinary OK.
void confirm_danger(const std::string& body, const std::string& confirm_label, std::function<void()> on_yes);
void info(const std::string& body);
// A web page to open on a phone: `body`, the QR code of `url`, then `url`
// itself, shortened (text::short_url), and OK.
void qr_dialog(const std::string& body, const std::string& url);
// The same for a text to read on a phone (not a link): larger, without the
// line under the code.
void qr_text_dialog(const std::string& body, const std::string& text);

// A list of checks in a dialog: each line with its mark (a teal check for
// what is ready, an amber or coral triangle for what is not, a grey dot for
// what is only said), under `title`, `note` small at the end (may be empty).
enum class Check
{
    Ok,
    Warn,
    Bad,
    Info,
};
struct CheckLine
{
    Check       state = Check::Info;
    std::string text;
};
// One of them: the mark, then the text (`size` px).
brls::Box* check_row(const CheckLine& line, float size = 17);
brls::Box* checklist_body(const std::string& title, const std::vector<CheckLine>& lines, const std::string& note = "");
// `body` with Cancel and `hold_label`: A held on it for 2 s, a bar under
// the body filling meanwhile, runs `on_yes`; let go sooner, the bar empties
// and nothing happens. For what is hard to undo from the console.
void hold_confirm(brls::Box* body, const std::string& hold_label, std::function<void()> on_yes,
                  std::function<void()> on_no = nullptr);

// Reads the stored PIN (pctl_get_pin) and shows it in large digits, or a
// dialog with the error. The app's copies are wiped as soon as the label
// holds the text. True when it was shown; `on_close` then runs once it is
// closed (OK or B). Used by PIN › Show the PIN.
bool show_pin_dialog(std::function<void()> on_close = nullptr);

// A dialog showing `text`. borealis' own text dialog does not scroll: a text
// taller than the screen collapses into one cut line. A long one is laid out
// with a smaller font and narrower margins instead, so it fits.
brls::Dialog* dialog(const std::string& text);

// B closes `dialog` and runs `on_cancel` (borealis' own B only closes it).
void on_cancel(brls::Dialog* dialog, std::function<void()> on_cancel);
// A Cancel button and B, both closing `dialog` and running `cancel`: a
// dialog that waits for something (a check, an answer from the network).
void add_cancel(brls::Dialog* dialog, std::function<void()> cancel);

// Dropdown list; `on_pick(index)` runs after the list has closed. The row
// `selected` has the focus and is checked as the current value, unless
// `checked` is false (a list of things to do, not of values). `details`, when
// given, says more about each row, after its label and quieter.
void pick(const std::string& title, const std::vector<std::string>& values, int selected,
          std::function<void(int)> on_pick, bool checked = true, const std::vector<std::string>& details = {});
// The same with several rows checked: those `marks` sets (true). Choosing a
// row closes the list as pick() does: to toggle one, the caller opens it
// again with the row's mark changed and the focus on it (`focus`).
void pick_marked(const std::string& title, const std::vector<std::string>& values, const std::vector<bool>& marks,
                 int focus, std::function<void(int)> on_pick, const std::vector<std::string>& details = {});
// A list of things to do: each line's label, and what it runs once the
// list has closed. The first line is selected; none, no list.
struct Choice
{
    std::string           label;
    std::function<void()> run;
};
void pick_actions(const std::string& title, const std::vector<Choice>& choices);

// A duration set with the stick (cadran::prompt_minutes: 15 min a step, 5
// with L and R, up to 24:00; "no limit" starts at 1 h), and the system
// keyboard for a text. The callback only on confirm.
void prompt_minutes(const std::string& header, uint16_t current, std::function<void(uint16_t)> on_value);
void prompt_text(const std::string& header, const std::string& initial, int max_len,
                 std::function<void(std::string)> on_value);

// Limits, play time, day, level and age names as text: ui/format_text.hpp.
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

// System / compatibility strings shared by the dashboard and the About section.
std::string hos_text(u32 version);                        // "23.0.1" (MAKEHOSVERSION), "?" for 0
std::string fw_text(const SysInfo& info);                 // "23.0.1 · Atmosphère 1.12.0"
std::string compat_text(const SysInfo& info, NVGcolor* color = nullptr);

// The Console tab's and Today's warnings.
std::string storage_text(const SysInfo& info);             // "emuMMC (Atmosphère)"
std::string storage_short(const SysInfo& info);            // "emuMMC" / "sysMMC" / "—"
// The other system (sysMMC from emuMMC, or an emuMMC set up on the card from
// sysMMC) has its own parental controls, out of PlayGuard's sight; "" if none.
std::string other_storage_note(const SysInfo& info);
// The serial-number warning only applies on emuMMC: on sysMMC the real serial
// is normal (online play), blanking it would cut Nintendo's services.
bool        serial_warning(const SysInfo& info);
std::string blank_text(const SysInfo& info, NVGcolor* color = nullptr);
// As the system sees it; `reveal` false masks the middle ("XAW1000•••••01").
std::string serial_text(const SysInfo& info, bool reveal);
bool        patches_warning(const patches::Report& report);
// The game-patch report of this boot, read from the SD card once per run (it
// only changes with a reboot): Today and the Console tab no longer rescan it
// each time they are opened.
const patches::Report& patch_report();
std::string patches_text(const patches::Report& report, NVGcolor* color = nullptr);
// Explanation under the Game patches line ("" when there is nothing to say);
// *warn tells whether it is a warning (amber) or a plain note.
std::string patches_note(const patches::Report& report, const SysInfo& info, bool* warn);
std::string time_text(uint64_t posix);                     // local time, or "—" for 0

// Read-only mode: a toast says so and this returns true (the caller then
// does nothing). Else false.
bool refuse_read_only();
// An action that writes stays visible in read-only mode, greyed (title and
// value), so the parent sees what exists; its handler calls
// refuse_read_only(). `title` / `detail`: the colours when writable. A
// BooleanCell gets its own colours back from its state.
void show_writable(brls::DetailCell* cell, bool writable, NVGcolor title, NVGcolor detail);
void show_writable(brls::DetailCell* cell, bool writable);   // plain text, neutral value
// A BooleanCell flips before telling its listener: in read-only mode the
// switch would flip, the write be refused and the switch flip back. This
// checks first, then flips as BooleanCell does.
void guard_switch(brls::BooleanCell* cell);

// Hides a view when `visible` is false (Visibility::GONE frees its space).
// When the hidden view had the focus, the focus moves to the nearest visible
// focusable sibling (the next one on a tie), so it never lands on the list
// container or jumps far away.
void set_visible(brls::View* view, bool visible);

// Applies several visibility changes, showing views before hiding others, so a
// cell that replaces the focused one (Unlock -> Lock now) is there to take
// the focus.
void set_visible_all(std::initializer_list<std::pair<brls::View*, bool>> changes);

// A label's text, or a cell's detail, set only when it changes: borealis lays
// the whole screen out again on every setText, and the tabs that refresh
// themselves every few seconds mostly write the same values back.
void set_text(brls::Label* label, const std::string& text);
void set_detail(brls::DetailCell* cell, const std::string& text);

// Runs `fill`, which adds many views to `box` (or changes them), with the box
// out of the screen's layout, then lays the screen out once. borealis lays
// the whole screen out again on each addView and setText: a list of n rows
// built in place cost n layouts of the whole screen, n of them on screen.
void batch_layout(brls::Box* box, const std::function<void()>& fill);

// For an activity's onResume(): borealis then gives the focus back to the
// view that opened the dialog (or screen) just closed, and gives none when
// that view was hidden meanwhile; the focus would stay on the closed
// dialog's button, deleted after its animation, and no button answer any
// more. In that case only, `fallback` (else the activity's first focusable
// view) gets it first.
void keep_focus(brls::Activity* activity, brls::View* fallback = nullptr);

// Whether `activity` has the focus: nothing (a dialog, another screen) is
// open over it. Unlike the top of borealis' stack, true again as soon as a
// dialog over it starts closing.
bool has_focus(brls::Activity* activity);
// Whether `activity` is the top of borealis' stack: no screen or dialog
// pushed over it, one closing included.
bool on_top(brls::Activity* activity);
// The main screen, wherever it is in the stack; nullptr before it opens.
MainActivity* main_activity();

// Sidebar positions of the main screen tabs, in the order of
// resources/xml/activity/main.xml (separators count as positions).
namespace tab
{
constexpr int dashboard    = 0;
constexpr int play_timer   = 2;
constexpr int activity     = 3;   // play time per game
constexpr int restrictions = 4;
constexpr int security     = 5;   // the PIN tab: PIN, unlock, blocking games, companion app, delete
constexpr int console      = 7;   // the network clock, installed games, firmware, storage, serial, patches
constexpr int history      = 9;   // changes and settings backups
constexpr int preferences  = 10;
constexpr int about        = 11;  // version, credits, changelog
// The n-th tab (0 = Today) as a sidebar position (separators count).
constexpr int of(int n)
{
    constexpr int positions[] = { dashboard, play_timer, activity, restrictions, security, console, history, preferences, about };
    return n >= 0 && n < (int)(sizeof(positions) / sizeof(positions[0])) ? positions[n] : dashboard;
}
}   // namespace tab

// Opens another tab of the main screen and focuses its first item. `from` is
// any view inside the current tab; the switch happens on the next frame
// because the current tab is deleted by it.
void go_to_tab(brls::View* from, int position);
// `content` (a tab's view, such as the network clock) on a screen of its own,
// over the current one, titled `title`; B closes it.
void open_screen(brls::View* content, const std::string& title);

// "Parental controls are temporarily unlocked — Lock now" cell shown at the
// top of the Play timer tab (no "Lock now" in read-only mode).
// `after` runs once it relocked.
void init_unlock_banner(brls::DetailCell* cell, std::function<void()> after);
void show_unlock_banner(brls::DetailCell* cell, bool unlocked);

// The language and theme pickers, shared by Preferences and the main
// screen's header: save the choice and apply it at once (a language or a
// theme makes every screen again, live_prefs; the text size runs `after`, a
// refresh). language_name / theme_name: the choice saved, as text.
void        pick_language(std::function<void()> after = nullptr);
void        pick_theme(std::function<void()> after = nullptr);
void        pick_text_size(std::function<void()> after = nullptr);
std::string text_size_name();
std::string language_name();
std::string theme_name();
// − on the main screen: a list of the two, each opening its picker.
void        quick_settings(std::function<void()> after = nullptr);

// Temporary unlock as last read by any tab (`valid` false: the read failed,
// keep the previous value). The main screen's title says it while unlocked.
void note_unlocked(bool valid, bool unlocked);
bool known_unlocked();

// After app::set_read_only / set_dev_mode: updates the main screen title,
// starts or stops the play-timer recorder (action/pt_log_flow) and re-reads
// the tab on screen.
void on_mode_changed();

}   // namespace ui
