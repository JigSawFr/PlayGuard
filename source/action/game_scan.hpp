// game_scan — the last check of the installed games (util/game_cache.hpp)
// for its two users: the start-up check (action/game_notice), which brings it
// up to date off the main thread, and Console › Check the installed games
// (activity/game_check_activity). Where it is kept, its saves off the UI
// thread through ui::write_later (one file, so a save still waiting gives way
// to a newer one), and one scan at a time: the screen's waits for the
// start-up one, which stops at its next game when asked and saves what it
// checked for the screen to go on from.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

#include "util/game_cache.hpp"

namespace game_scan
{

// games.bin, in the console's memory with the other caches.
std::string cache_file();

// The last check, once the saves already asked for are on the SD card (most
// often none: no wait). False, `out` left as it is, when there is none or it
// cannot be read.
bool load(game_cache::Cache* out);

// `cache` on the SD card, off the UI thread.
void save(const game_cache::Cache& cache);

// MainActivity, once per start (game_notice::at_start): off the main thread,
// the last check brought up to date as the screen does (only the games whose
// entry changed, every game once a day or on firmware `hos`), saved (what a
// screen open meanwhile found of a game's data kept), then `done` with it on
// the main thread. Nothing once the app is quitting.
void at_start(u32 hos, std::function<void(const game_cache::Cache& cache)> done);

// The screen, before it lists and checks the games (main thread): `go` at
// once when the start-up check is not under way; else that check is asked to
// stop after its current game, and `go` runs (main thread) once it saved what
// it checked, `waited` true: load() then has it.
void when_free(std::function<void(bool waited)> go);

}   // namespace game_scan
