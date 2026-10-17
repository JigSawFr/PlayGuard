// notices — the notification bell of the main screen: what needs the parent,
// gathered from the parts of PlayGuard that see it (notice_logic has the
// rules). Y on the main screen opens the list (activity/notices_activity);
// the bell in the header counts what is to do. Each entry can carry an
// action: the fix itself, or the screen where it is made. UI thread only.
//
// The sources: the Overview's readings and a sweep of the main screen every
// minute (refresh_status), the network clock check at start-up, the update
// check, PlayGuard's own damaged files at start-up (data_notice).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "action/notice_logic.hpp"
#include "util/pctl_ops_c.hpp"

namespace notices
{

using Level = notice_logic::Level;

struct Entry
{
    notice_logic::Item     item;
    std::string            action_label;   // "" when there is nothing to do but read it
    std::function<void()>  action;
};

// Adds or replaces the entry `id`. `action` runs from the list, after it closed.
// `print`: what an entry the parent may ignore is about (the PIN's length for
// pin_weak): ignored for one, it is asked again for another.
void set(const std::string& id, Level level, const std::string& title, const std::string& detail,
         const std::string& action_label = "", std::function<void()> action = nullptr,
         const std::string& print = "");
void clear(const std::string& id);
// What `id` says is fixed, as read (not only unknown for now): cleared, and
// no longer ignored, so it is told again if it comes back.
void resolved(const std::string& id);
// Puts away an Info entry (see notice_logic::Store::dismiss), and remembers
// it in config.json (notices_read): read at the next start too, until its
// text changes.
void dismiss(const std::string& id);
// Every entry for information put away (dismiss).
void dismiss_all();
// Ignores an entry to do the parent may ignore (notice_logic::ackable), and
// remembers it in config.json (notices_acked): no longer counted, listed
// apart, for 30 days (notices_ack_remind) or until what it says is fixed.
// The caller asks for the PIN first.
void acknowledge(const std::string& id);
// Counted again.
void reactivate(const std::string& id);
// Preferences › Notifications changed: entries of a kind turned off go, the
// state is read again.
void apply_preferences();

std::vector<Entry> list();
// The entries ignored (acknowledge), the bell's order.
std::vector<Entry> ignored();
// The day `id`, ignored, counts again ("" with no reminder).
std::string ignored_until(const std::string& id);
int action_count();    // the parental controls' entries to do: the bell's number
int info_count();      // the parental controls' entries for information
int console_count();   // the console's entries to do
int tab_count(const std::string& tab);   // the entries to do of a tab (notice_logic::tab_of): its sidebar badge

// Called (UI thread) after every change of what the bell shows.
int  listen(std::function<void()> fn);
void unlisten(int id);

// The Overview's readings, and the main screen's sweep: the state of the
// parental controls, the play timer, the clock and the system, as entries.
void refresh_status(const PctlStatus& s, const PtState& pt, const SysInfo& si, Result accuracy_rc, bool accurate);
// What timer_health said, from the screens that ask it (they show the play
// timer): the console not counting play time.
void note_timer_stalled(bool stalled);
// The PIN anyone would try first (pctl_pin_weak): read by the sweep once per
// start, and again when the PIN's length changes.
void note_pin_strength(const PctlStatus& s);
// The main screen's own sweep (a minute apart, only while it is on top and
// in focus): reads what refresh_status needs, in one pctl session.
void sweep();
// The same, unless the Overview gave refresh_status its readings in the last
// seconds: the main screen's minute timer.
void sweep_if_stale();

// MainActivity, once per start: a crash file newer than the last one told
// about (PlayGuard closed unexpectedly since). The first start with this
// only remembers the newest, so an old crash is not told as new.
void note_crash();

// A file PlayGuard could not write to the SD card (`what`: "config",
// "history"), or wrote again after that (`ok`): an entry to do until then.
void note_write(const char* what, bool ok, const std::string& error = "");
// A backup is being written: the SD card's free space is read again at the
// next refresh_status, not at most once a minute as otherwise.
void sd_written();

// Opens tab `position` (ui::tab) of the main screen, under the list.
void open_tab(int position);

}   // namespace notices
