// sys_update_flow — newer Atmosphère, hekate and firmware releases in the
// notification bell (util/sys_updates.hpp): checked with PlayGuard's own
// updates (once a day with "Check for updates every day", and by "Check for
// updates"), remembered in config.json, compared at each start with what
// the console runs. For information only: PlayGuard updates none of them,
// and a firmware is worth installing only once Atmosphère (and hekate)
// support it, which the firmware's entry says.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace sys_update_flow
{

// Start-up: the entries from the last check, then a new check in the
// background when Check for updates every day is on and none ran today.
void at_start();

// Check for updates: a new check now, in the background.
void check_now();

}   // namespace sys_update_flow
