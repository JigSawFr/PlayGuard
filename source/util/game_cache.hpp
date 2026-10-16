// game_cache — the last check of the installed games (core/gamecheck.h),
// kept on the SD card so Tools › Check the installed games shows it at once,
// then only checks again what changed since: a game whose HOME menu entry
// changed (installed, updated, archived, removed) or every game once the
// check is a day old, from another firmware, or asked for (Ⓧ). Pure: the
// file format and what to check again; the screen does the reading and
// writing. Host-tested (tests/game_cache).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "util/pctl_ops_c.hpp"

namespace game_cache
{

// Every game is checked again once the last full check is this old: what no
// entry says (files lost on the SD card, a ticket deleted) shows by then.
constexpr int64_t MAX_AGE_S = 24 * 3600;

struct Cache
{
    int64_t  checked_at = 0;   // the last full check, POSIX (user clock); 0: none
    uint32_t hos = 0;          // the firmware it was made on (MAKEHOSVERSION)
    bool     truncated = false;
    std::vector<GameFacts> games;   // in the console's order
};

// A header (magic, version, sizeof(GameFacts): another build's layout is
// refused), then the games. Strings are cut at their size when read.
std::string encode(const Cache& cache);
bool decode(const std::string& bytes, Cache* out);

// Whether every game must be checked: no full check yet, one from another
// firmware, MAX_AGE_S old, or from the future (the clock went back).
bool expired(const Cache& cache, int64_t now, uint32_t hos);

// Brings `cache` in line with the console's entries: games no longer listed
// go, the rest follow the records' order, and returns the records to check:
// the new ones and those whose entry changed (LastUpdated or LastEvent).
// Every record when expired().
std::vector<GameRecord> to_check(Cache* cache, const std::vector<GameRecord>& records, int64_t now, uint32_t hos);

// `game` in place of the cached one with its ID, added when there is none.
void put(Cache* cache, const GameFacts& game);

}   // namespace game_cache
