// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/session_pause_logic.hpp"

namespace session_pause_logic
{

bool wanted(const Inputs& in)
{
    // From the album, the time does not count and a game may be left open
    // behind: nothing to pause, and it would be that game's.
    if (!in.application || in.read_only || in.quitting) return false;
    return in.pin_holds && in.in_focus && in.holds == 0;
}

}   // namespace session_pause_logic
