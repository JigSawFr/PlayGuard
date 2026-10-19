// rating_flow — the installed games' age ratings against the console's
// restriction (action/rating_logic over the last game check,
// game_scan::latest): the age a level restricts to, a game's rating as the
// screens write it ("PEGI 16"), and the list of the games a level blocks.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include "util/pctl_ops_c.hpp"

namespace rating_flow
{

// The age `level` restricts software to (0: none), as the console has it
// (Custom: its own setting); -1 when it cannot be read.
int level_age(const PctlStatus& st, uint32_t level);

// "PEGI 16" (the organisation `st` has, its age for the game), "" when the
// organisation is unknown or does not rate the game.
std::string game_rating(const PctlStatus& st, const GameFacts& g);

// The last game check's facts of `app_id`, nullptr when it has none.
const GameFacts* facts_of(uint64_t app_id);

// How many installed games an age restriction of `age` (level_age) blocks,
// out of how many installed; false when there is no game check yet, or the
// organisation or the age (-1) is unknown.
bool count_blocked(const PctlStatus& st, int age, size_t* blocked, size_t* games);

// The games `age` blocks, as a list (their ratings beside them), or a line
// saying there are none.
void show_blocked(const PctlStatus& st, int age);

}   // namespace rating_flow
