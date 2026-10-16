// sys_update_flow — newer Atmosphère, hekate and firmware releases in the
// notification bell (util/sys_updates.hpp): checked with PlayGuard's own
// updates (once a day with "Check for updates every day", and by "Check for
// updates"), remembered in config.json, compared at each start with what
// the console runs. For information only: PlayGuard updates none of them,
// and a firmware is worth installing only once Atmosphère (and hekate)
// support it, which the firmware's entry says.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace sys_update_flow
{

// Start-up: the entries from the last check, then a new check in the
// background when Check for updates every day is on and none ran today.
void at_start();

// Check for updates: a new check now, in the background.
void check_now();

// The bell's entries again, from what the last check found (Preferences ›
// Notifications turned them back on).
void refresh_entries();

// hekate as installed on the SD card (bootloader/update.bin); "" when absent.
std::string installed_hekate();
// Tools › Console's value: `running`, then the newer release the last check
// found ("22.0.0 · 23.0.1 available"); *newer says whether there is one.
// `which`: "atmosphere", "hekate" or "firmware".
std::string with_latest(const char* which, const std::string& running, bool* newer);

}   // namespace sys_update_flow
