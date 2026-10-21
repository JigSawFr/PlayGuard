// sys_notes_flow — the release notes of Atmosphère, hekate, the firmware and sys-patch
// (util/sys_notes.hpp): the Console tab opens them (A on a line), About
// PlayGuard's, the screen
// showing what was kept on the SD card at once and fetching them again in
// the background; the daily update check (sys_update_flow) refreshes the
// lists too, so the Console tab can say how far behind each one is and when the
// newest firmware is not supported yet.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "util/sys_notes.hpp"

namespace sys_notes_flow
{

// What the console runs: "1.12.0", "6.5.4" (bootloader/update.bin), "22.0.0",
// PlayGuard's own version, "1.6.3.0" (sys-patch's log.ini); "" when unknown
// (no Atmosphère, no hekate on the SD card, no sys-patch or no log).
std::string installed(sys_notes::Which which);
// The newest release the update check found (sys_updates; PlayGuard's
// update check for PlayGuard), "" none.
std::string latest(sys_notes::Which which);

// Reads the lists kept on the SD card, off the UI thread (at start-up; any
// call below starts it too). Once read, what asked meanwhile (a refresh, a
// keep) is done and what shows them is refreshed: the release notes screen,
// the firmware's entry of the notification bell, the tab on screen.
void load();
// Whether they are read (starts reading them).
bool ready();

// The lists kept (empty until ready()).
const sys_notes::Cache& cache();

// Opens the notes of `which` (activity/sys_notes_activity.hpp) and fetches
// them again unless they were today (or `force`, X on the screen).
void open(sys_notes::Which which);
void refresh(sys_notes::Which which, bool force);
// Whether a fetch of `which` is running, or the lists kept are still being
// read (a refresh asked meanwhile waits for them).
bool fetching(sys_notes::Which which);

// After the daily update check: the four lists again, without the
// firmware's pages (one request each). Blocking: from a background task.
// Returns the fresh cache to keep(), through brls::sync.
sys_notes::Cache fetch_lists();
// Keeps `fresh` (merged into what was kept, once that is read) and saves it
// (encoded off the UI thread).
void keep(const sys_notes::Cache& fresh);

// "22.0.0 · 23.0.1 available (4 versions behind)" for a line of the
// Console tab (or About's, for PlayGuard); *newer when a newer one is out.
std::string status_text(sys_notes::Which which, bool* newer);

// What updating to the newest firmware needs first, from the notes of
// Atmosphère and hekate (one line each, "" when nothing: no newer firmware,
// supported already, nothing said in the notes, or not read yet).
std::string firmware_advice();

}   // namespace sys_notes_flow
