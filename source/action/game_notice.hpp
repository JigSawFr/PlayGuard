// game_notice — the notification bell's entry (action/notices) for the
// installed games the HOME menu cannot start or draw (core/gamecheck.h):
// how many, and which. At start-up, off the main thread, action/game_scan
// brings the last check (util/game_cache.hpp) up to date the way Console ›
// Check the installed games does (only the games whose entry changed, every
// game once a day or on another firmware) and saves it; then every check on
// that screen, repairs included, updates it, so the entry goes once the last
// one is fixed. A on it opens that screen.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "util/pctl_ops_c.hpp"

namespace game_notice
{

// MainActivity, once per start. Not opened from the album (applet mode):
// little memory there.
void at_start();

// What a check found (UI thread), on firmware `hos`.
void note(const std::vector<GameFacts>& games, u32 hos);

}   // namespace game_notice
