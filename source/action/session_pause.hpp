// session_pause — parental controls paused while the parent is in PlayGuard.
//
// Only with Security › Pause parental controls while I'm in PlayGuard on
// (config pin_pauses, off by default: the PIN alone then changes nothing).
// Once the parent's PIN is entered (pin_lock::ask: the lock screen, the
// first change, the child screen's way back…), PlayGuard unlocks parental
// controls temporarily (pt_flow::unlock_temporarily, the same unlock as
// Security › Unlock temporarily), so the time spent in it does not take
// from the child's play time. It locks them again as soon as PlayGuard is
// left: the HOME menu or sleep (out of focus), closing it (from the HOME
// menu too: the exit waits), and while a screen meant for the child is open
// (Hold: the child screen, the lock screen). Back in PlayGuard, the PIN
// still holding, it pauses again.
//
// Only as an application (over a game or from its HOME icon: from the
// album the time does not count), not in read-only mode, never without the
// PIN, and never over an unlock it did not make (Security › Unlock
// temporarily stays as it is). The unlock is recorded as any other
// (config relock_pending): should PlayGuard stop without locking, its next
// start locks again, and a restart of the console does it by itself.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "util/pctl_ops_c.hpp"

namespace session_pause
{

// At start-up, once, after pin_lock::watch_focus: follows the focus and the exit.
void install();

// pin_lock::ask: the PIN was just entered.
void pin_entered();

// Pause parental controls while I'm in PlayGuard (config pin_pauses) was
// just turned on or off: pauses now, the PIN holding, or locks again.
void setting_changed();

// PlayGuard's own pause is on: parental controls unlocked by it, until it is left.
bool active();

// The parent turned parental controls back on by hand: no pause until the
// PIN is entered again.
void stop();

// While active(): what the screens show, as the console reads once locked
// again: not temporarily unlocked, the play timer on (1453 reads true
// whenever locked). The day as it is for the child, not "on hold". The
// writes keep what the console reads. Either pointer may be null.
void shown_as_locked(PctlStatus* status, PtState* pt);

// Locks again now, for good (the main loop is over).
void end();

// While one lives, the pause is off: a screen during which play counts as
// usual (the child screen, the lock screen). Back on when the last one goes.
class Hold
{
  public:
    Hold();
    ~Hold();
    Hold(const Hold&) = delete;
    Hold& operator=(const Hold&) = delete;
};

}   // namespace session_pause
