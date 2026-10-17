// activity_summary — the Activity tab's summary of one account's play data
// (core/playstats.h) over the chosen period: the average per day, the most
// played game, the days played and the busiest one, the average session; and
// the games to rediscover (installed, little played, left aside for a while);
// the list's order and the table its export writes. Pure (the words are the
// tab's, handed in), so the host tests can run it (tests/activity_summary).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "util/pctl_ops_c.hpp"
#include "util/table_export.hpp"

namespace activity_summary
{

enum Period { Today = 0, Week = 1, AllTime = 2 };

// A game's play time over `period` (all time: 0 when its totals were not read).
uint64_t value(const GameStat& g, int period);

struct Summary
{
    bool     known = false;   // the period's figures were read (nothing below is meaningful otherwise)
    uint64_t total_s = 0;     // every game added up over the period
    // Per day: the last 7 days over 7, all time over the days since the first
    // game was first played (today counting as one). 0 days: no average
    // (today, or all time with no first play known).
    uint64_t per_day_s = 0;
    int      days = 0;
    int      top = -1;        // index in PlayStats.games of the most played game, -1 none
    uint64_t top_s = 0;
    // Last 7 days only (-1 otherwise): the days with any play, and the day
    // (k days ago, see PlayStats.day_s) with the most.
    int      days_played = -1;
    int      busiest = -1;
    uint64_t busiest_s = 0;
    // All time only (0 otherwise, or with no launch counted): play time over launches.
    uint64_t session_s = 0;
};

Summary summarize(const PlayStats& stats, int period);

// Games to rediscover: still installed (a name), under 3 h in all and not
// played for 30 days or more, the least played first, at most `max`. Indices
// in PlayStats.games.
constexpr uint64_t REDISCOVER_MAX_S   = 3 * 3600;
constexpr uint64_t REDISCOVER_AFTER_S = 30 * 86400;
std::vector<int> rediscover(const PlayStats& stats, size_t max);

// A game's all-time play time that no user account holds (`total_s` is the
// game's, every account): played without choosing an account (some games
// never ask) or by an account deleted since. Never negative: several accounts
// open together add up to more than the game. 0 under UNATTRIBUTED_MIN_S
// (the two counts round apart by a few seconds).
constexpr uint64_t UNATTRIBUTED_MIN_S = 60;
uint64_t unattributed_s(uint64_t total_s, const AccountPlay* accounts, size_t count);

// Most played in `period` first, a tie to the most recently played. The
// export (`by_total`) breaks a tie by all-time play first (raw total_s), so
// its unplayed games keep a useful order too.
void sort_by_play(std::vector<const GameStat*>& games, int period, bool by_total);

// What the export's table says, in the language shown: the tab's words.
struct ExportText
{
    std::string title, subtitle, sheet;
    // The column headers, but the six days before today's.
    std::string game, id, today, week, total, launches, first, last;
    std::function<std::string(int wday)> day;           // a day's column ("Sat (min)"), 0 = Sunday
    std::function<std::string(const GameStat&)> name;   // a game's name, a deleted one's too
    std::function<std::string(uint64_t posix)> time;    // a date and time, never given 0
};

// Every game of `stats`, in the export's order (sort_by_play by total), with
// its play time in whole minutes (rounded, so a spreadsheet can add them up)
// today, on each of the six days before (most recent first), this week and
// in all; its launches and first and last plays. A figure that was not read
// is an empty cell.
table_export::Table export_table(const PlayStats& stats, int period, const ExportText& text);

}   // namespace activity_summary
