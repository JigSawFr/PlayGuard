// pin_lock — PIN › "Protect PlayGuard with the PIN": the parental-control PIN (in the
// system's own PIN screen) before PlayGuard changes anything, or before it
// opens at all.
//
//   off      never, except before "Show the PIN" (the PIN opens everything);
//   changes  before the first change: anyone can look, only the parent can
//            change (the default, also when config.json is missing, damaged
//            or holds an unknown value);
//   open     to open PlayGuard (a lock screen until it is entered).
//
// Once entered, the PIN holds until PlayGuard closes. "Ask for the PIN
// again" (off by default) makes it hold 5 minutes instead: "changes" asks
// again after them, "open" locks again after 5 minutes or more out of focus.
//
// "Show the PIN" asks every time, whatever the mode (the reveal check,
// write_guard.h).
//
// The check sits in the service layer (write_guard.h), so no change can skip
// it; locking again never asks. Without a PIN on the console nothing is asked
// (and the setting cannot be turned on). The setting itself is on the SD
// card: it keeps a child out of PlayGuard, not someone who edits the card.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

namespace pin_lock
{

// At start-up, once: installs the checks in the service layer.
void install();

// While one lives, the change check lets every change through without the
// PIN, as it does locking again (read-only mode still refuses them). Only for
// the few calls of a change that tightens a limit and that nobody chose
// (pt_flow: extra time put back the next day), made at once on the UI thread,
// where every change check runs: no question, no dialog, no wait in between.
class Tightening
{
  public:
    Tightening();
    ~Tightening();
    Tightening(const Tightening&) = delete;
    Tightening& operator=(const Tightening&) = delete;
};

// At start-up, once: runs `lock` (on the UI thread) when PlayGuard comes back
// into focus and "open" wants the lock screen again (pin_lock_logic::lock_again).
void watch_focus(std::function<void()> lock);

// The setting is "open": start on the lock screen.
bool at_start();

// Asks for the PIN now. True when it was entered, or when no PIN is set.
// Entered, it pauses parental controls while PlayGuard is open, when
// pin_pauses asks for it (session_pause).
bool ask();
// The PIN was entered in this run and still holds (as confirm() lets through).
bool holds();
// Asks for the PIN unless it still holds (entered since PlayGuard opened, or
// in the last 5 minutes with "Ask for the PIN again"), whatever the mode.
bool confirm();
// Before PlayGuard hands the console over to a homebrew store (sphaira, the
// App Store), which starts any homebrew and changes the SD card: the change
// check as before a change ("Before a change" asks; the read-only mode does
// not refuse it, an update being its way out), and the PIN whatever the mode
// while the homebrew menu is locked (`menu_locked`), the store being a way
// around the lock. On a refusal it says why.
bool before_handing_over(bool menu_locked);
// Before showing the PIN: asks for it now, in every mode, ignoring the grace
// and the lock screen. True when it may be shown (the read right after does
// not ask again). On a refusal it says why.
bool before_show_pin();
// What to tell the user after ask() said no: "the PIN was not entered", with
// the result the PIN screen gave (its code tells a cancel from a refusal).
std::string refusal_text();

// "Never" / "Before a change" / "To open PlayGuard".
std::string mode_text();
// One sentence on what the chosen mode means (shown under the setting).
std::string note_text();

// The PIN tab's picker. Turning it on needs a PIN on the console; turning it
// down asks for the PIN first. `done` runs after a change.
void choose(std::function<void()> done);

// "Ask for the PIN again" (config pin_ask_again). Turning it off asks for
// the PIN first (confirm()). `done` runs after a change.
void set_ask_again(bool on, std::function<void()> done);

// "Pause parental controls while I'm in PlayGuard" (config pin_pauses,
// session_pause). Turning it on asks for the PIN first (confirm()). `done`
// runs after a change.
void set_pauses(bool on, std::function<void()> done);

}   // namespace pin_lock
