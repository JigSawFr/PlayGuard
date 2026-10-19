// schedule_flow — a profile scheduled for a period (holidays, a school
// trip): Play timer › Profiles › Scheduled profile sets it (the profile, the
// first and the last day), and the main screen asks, once a day, to put it
// in force when the period begins and the limits from before back once it
// is over (schedule_logic over config schedule_*). PlayGuard only acts when
// it is opened: nothing happens on a console it is not opened on.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

namespace schedule_flow
{

// The main screen: at start-up (`at_start`: only when no other screen
// claims the start, ui::claim_start_screen) and on its 30-second tick, the
// question the schedule calls for now, once a day.
void check(bool at_start);

// Profiles › Scheduled profile: set one (the profile, the first day, the last
// day), or change, remove, put back the one there is; `after` refreshes.
void choose(std::function<void()> after);

// "Holidays · October 19 to November 2", "" when none is scheduled.
std::string summary();

}   // namespace schedule_flow
