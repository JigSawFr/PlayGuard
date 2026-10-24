// session_pause_logic — whether PlayGuard's own pause of parental controls
// (action/session_pause) should be on now, without any UI or console call,
// so the host tests can run it (tests/session_pause_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace session_pause_logic
{

struct Inputs
{
    bool enabled = false;      // the parent asked for it (config pin_pauses, off by default)
    bool pin_holds = false;    // the parent's PIN entered in this run, and it still holds (pin_lock)
    bool in_focus = false;     // PlayGuard has the screen (not the HOME menu, nor sleep)
    int holds = 0;             // screens open during which play counts as usual (the child's, the lock screen)
    bool application = false;  // run as an application (over a game, or from its HOME icon), not from the album
    bool read_only = false;    // the read-only mode: no write at all
    bool quitting = false;     // the main loop is over
};

// The pause is wanted: the parent is in PlayGuard, PIN entered.
bool wanted(const Inputs& in);

// A remote order (Console › Remote access: Home Assistant) PlayGuard carries
// out while its pause is on. The temporary unlock it asks for makes the
// pause's unlock the parent's own (Adopt: it stays as it is once PlayGuard
// is left, as Security › Unlock temporarily does); turning parental controls
// back on ends the pause as the parent's own does (End); any other order
// leaves it as it is (Keep).
enum class Remote { Keep, Adopt, End };
Remote on_remote(bool paused, bool unlock_order, bool relock_order);

}   // namespace session_pause_logic
