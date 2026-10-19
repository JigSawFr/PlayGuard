// rating_logic — what an age restriction means for the installed games: the
// age each game is rated for by the console's rating organisation (its
// control data, read by the game check: GameFacts.rating_age), and the games
// a level's age blocks. Plain C++ over GameFacts, so the host tests can run
// it (tests/rating_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "util/pctl_ops_c.hpp"

namespace rating_logic
{

// The age `org` (pctl_rating_org_name's order) rates the game for; -1 when
// it does not rate it, or its control data was not read.
int age_of(const GameFacts& g, uint32_t org);

// The game cannot be started under an age restriction of `age` (0: none):
// `org` rates it above that age. A game `org` does not rate is not counted
// as blocked: the console looks at that organisation's rating only.
bool blocked(const GameFacts& g, uint32_t org, uint8_t age);

// The games of `games` that `age` blocks, in their order, and how many of
// them `org` rates at all (the others cannot be told).
struct Blocked
{
    std::vector<const GameFacts*> games;
    size_t rated = 0;
};
Blocked blocked_games(const std::vector<GameFacts>& games, uint32_t org, uint8_t age);

}   // namespace rating_logic
