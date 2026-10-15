// game_notice — the notification bell's entry (action/notices) for the
// installed games the HOME menu cannot start or draw (core/gamecheck.h):
// how many, and which. Read once in the background at start-up, then again
// by every read of Tools › Check the installed games (after a repair too),
// so the entry goes once the last one is fixed. A on it opens that screen.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "util/pctl_ops_c.hpp"

namespace game_notice
{

// MainActivity, once per start: the games read off the main thread. Not
// opened from the album (applet mode): little memory there.
void at_start();

// What one read found (UI thread). A read that failed leaves the entry as is.
void note(const GameCheck& check);

}   // namespace game_notice
