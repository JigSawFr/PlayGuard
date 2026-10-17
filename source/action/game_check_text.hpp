// game_check_text — the rules behind what Console › Check the installed games
// says of a game, without borealis: the section order, the i18n key of each
// problem and section, which problems are warnings rather than failures,
// the firmware a game asks for, which fix text applies, and the IDs and
// console codes as the screen writes them. The texts themselves (and the
// cause, whose values are worded too) stay in GameCheckActivity. Plain C++
// so the host tests can run it (tests/game_check_text).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <string>

#include "core/gamecheck.h"

namespace game_check_text
{

// The screen's sections, in the order they are shown.
extern const GameCategory GROUPS[GC_CAT_COUNT];

// From this many broken games, the cause is more likely the SD card or the
// storage the console started on than each game.
constexpr size_t MANY = 3;

// A section's part of its i18n keys ("fix", "verify", "reinstall", "system").
const char* category_key(GameCategory category);
// A problem's part of its i18n keys ("archived", "firmware"…); "" for GC_OK.
const char* issue_key(GameIssue issue);
// "playguard/games/issues/<issue_key>/<part>": "short", "cause" or "fix".
std::string issue_text_key(GameIssue issue, const char* part);
// The problems shown as a warning (the game is there, something around it
// is not) rather than as a failure.
bool warn_only(GameIssue issue);

// The system version the game or its update asks for, the higher of the two
// (MAKEHOSVERSION, 0 when unknown).
u32 required_hos(const GameFacts& g);
// The i18n key of what to do about `issue` on a console at `current_hos`:
// only the update too new names playing without it, a key too new reads as
// a firmware too old, the rest their own.
std::string fix_key(const GameFacts& g, GameIssue issue, u32 current_hos);

// "0100ABCD12345000": a game's or a content's ID.
std::string id_text(u64 id);
// What the change history keeps: "Name (0100…)", the ID alone when unnamed.
std::string history_label(const GameFacts& g);
// A system result as the console shows it: "2016-0601".
std::string console_code(Result rc);

}   // namespace game_check_text
