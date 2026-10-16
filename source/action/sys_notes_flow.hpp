// sys_notes_flow — the release notes of Atmosphère, hekate and the firmware
// (util/sys_notes.hpp): Tools › Console opens them (A on a line), the screen
// showing what was kept on the SD card at once and fetching them again in
// the background; the daily update check (sys_update_flow) refreshes the
// lists too, so Tools can say how far behind each one is and when the
// newest firmware is not supported yet.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "util/sys_notes.hpp"

namespace sys_notes_flow
{

// What the console runs: "1.12.0", "6.5.4" (bootloader/update.bin), "22.0.0";
// "" when unknown (no Atmosphère, no hekate on the SD card).
std::string installed(sys_notes::Which which);
// The newest release the update check found (sys_updates), "" none.
std::string latest(sys_notes::Which which);

// The lists kept (read from the SD card on the first call).
const sys_notes::Cache& cache();

// Opens the notes of `which` (activity/sys_notes_activity.hpp) and fetches
// them again unless they were today (or `force`, X on the screen).
void open(sys_notes::Which which);
void refresh(sys_notes::Which which, bool force);
// Whether a fetch of `which` is running.
bool fetching(sys_notes::Which which);

// After the daily update check: the three lists again, without the
// firmware's pages (one request each). Blocking: from a background task.
// Returns the fresh cache to keep(), through brls::sync.
sys_notes::Cache fetch_lists();
// Keeps `fresh` (merged into what was kept) and saves it.
void keep(const sys_notes::Cache& fresh);

// "22.0.0 · 23.0.1 available (4 versions behind)" for a line of Tools ›
// Console; *newer when a newer one is out.
std::string status_text(sys_notes::Which which, bool* newer);

// What updating to the newest firmware needs first, from the notes of
// Atmosphère and hekate (one line each, "" when nothing: no newer firmware,
// supported already, or nothing said in the notes).
std::string firmware_advice();

}   // namespace sys_notes_flow
