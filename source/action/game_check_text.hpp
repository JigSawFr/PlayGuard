// game_check_text — the rules behind what Console › Check the installed games
// says of a game, without Aurore: the section order, the i18n key of each
// problem and section, which problems are warnings rather than failures,
// the firmware a game asks for, which fix text applies, and the IDs and
// console codes as the screen writes them; then the list itself: the games
// with a problem by section, sorted, their names told apart, the line above
// the list, the list to install again as text, the names a question lists
// and what a batch of integrity checks found. The words come from
// GameCheckActivity (Words, as the catalogs have them); the cause, whose
// values are worded too, stays there. Plain C++ so the host tests can run it
// (tests/game_check_text).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

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

// What the change history keeps: "Name (0100…)", the ID alone when unnamed.
std::string history_label(const GameFacts& g);
// A system result as the console shows it: "2016-0601".
std::string console_code(Result rc);

// The words of the screen the helpers below put together (GameCheckActivity
// reads them from the catalogs).
struct Words
{
    std::function<std::string(const std::string& id)> unnamed;   // a game without a name, from its ID
    std::function<std::string(const std::string& v)>  version;   // "version 1.4.0", from the game's own
    std::function<std::string(const GameFacts&, GameIssue)> problem;   // the cell's value: what the HOME menu shows
    std::string list_title;                                      // the list to install again's
    std::string checked_now;                                     // "Checked just now."
    std::function<std::string(int)> checked_min, checked_h;      // "Checked 12 min ago.", "… 3 h ago."
};

// The game's name, or `unnamed` of its ID.
std::string game_name(const GameFacts& g, const std::function<std::string(const std::string& id)>& unnamed);

// A game with a problem.
struct Broken
{
    GameFacts game;
    GameIssue issue;
};
// The games with a problem on a console at `hos`: all of them in the
// cache's order, and the same in a section each (gamecheck_category), the
// same problem together, then by name (case aside).
struct Sorted
{
    std::vector<Broken> all;
    std::vector<Broken> by_group[GC_CAT_COUNT];
};
Sorted sort_broken(const std::vector<GameFacts>& games, u32 hos, const Words& w);
// Each game's line in the list, by its ID: its name, and when another game
// of `broken` has the same one (two regions, a demo), " · " and its version,
// or its ID when the versions do not tell them apart.
std::map<u64, std::string> line_names(const std::vector<Broken>& broken, const Words& w);
// The games of `selected` still in `broken`: the others left the list.
std::set<u64> still_selected(const std::set<u64>& selected, const std::vector<Broken>& broken);

// How long ago the last check was, as of `now` (POSIX seconds, local): ""
// when never; under a minute, in minutes, else in hours.
std::string age_text(int64_t checked_at, int64_t now, const Words& w);
// The line above the list once checked: what was `found` (`none`: nothing),
// then `age` after a space when nothing was found, " · " otherwise; then
// each of `notes` on a line of its own. The empty ones are left out.
std::string result_line(const std::string& found, bool none, const std::string& age,
                        const std::vector<std::string>& notes);

// The games to install again as plain text: `w.list_title`, a blank line,
// then a line each with its ID, its name (as an installer searches it), its
// version and its problem on a console at `hos`.
std::string list_text(const std::vector<GameFacts>& games, u32 hos, const Words& w);

// A question names this many games at most.
constexpr size_t NAMES_SHOWN = 6;
// The games' names, each after a line break, the first NAMES_SHOWN only,
// then "…" on a line of its own when there are more.
std::string name_lines(const std::vector<GameFacts>& games, const Words& w);

// What the check of one game of a batch gave.
struct Verified
{
    GameFacts game;
    Result rc = 0;
    bool corrupt = false;
    Result found = 0;
};
// A batch of them counted: the corrupt games, the intact ones, the failed checks.
struct Tally
{
    std::vector<GameFacts> corrupt;
    size_t intact = 0, failed = 0;
};
Tally tally(const std::vector<Verified>& results);

}   // namespace game_check_text
