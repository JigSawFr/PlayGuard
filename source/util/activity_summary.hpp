// activity_summary — the Activity tab's summary of one account's play data
// (core/playstats.h) over the chosen period: the average per day, the most
// played game, the days played and the busiest one, the average session; the
// list's order and the table its export writes. Pure (the words are the
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

// Month, added later, keeps the numbers config's activity_period saved:
// PERIODS is the order they are offered in.
enum Period { Today = 0, Week = 1, AllTime = 2, Month = 3 };
constexpr int PERIODS[] = { Today, Week, Month, AllTime };
constexpr int PERIOD_COUNT = 4;

// A game's play time over `period` (all time: 0 when its totals were not read).
uint64_t value(const GameStat& g, int period);
// The period's figures were read: the log for today, the week and the month,
// the statistics for all time.
bool period_read(const PlayStats& stats, int period);

struct Summary
{
    bool     known = false;   // the period's figures were read (nothing below is meaningful otherwise)
    uint64_t total_s = 0;     // every game added up over the period
    // Per day: the last 7 days over 7, the last 30 over 30, all time over
    // the days since the first game was first played (today counting as one). 0 days: no average
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

// When the last 7 days' play happened (PlayStats.hours): the sessions, the
// longest one and its day (k days ago), the two hours in a row with the most
// play, every day together (busy_from: the first, -1 with no play), and the
// play between LATE_FROM and LATE_TO (21:00 to 06:00).
constexpr int LATE_FROM = 21, LATE_TO = 6;
struct Hours
{
    bool     known = false;   // nothing below is meaningful otherwise
    uint32_t sessions = 0;
    uint64_t average_s = 0;
    uint64_t longest_s = 0;
    int      longest_day = -1;
    int      busy_from = -1;
    uint64_t busy_s = 0;
    uint64_t late_s = 0;
};
Hours hours(const PlayStats& stats);

// The last 7 days against the 7 before, in minutes a day (fewer: negative),
// in `per_day_min`; false when either week was not read (or the days moved
// on since). Within TREND_SAME_MIN either way, it is about the same.
constexpr int64_t TREND_SAME_MIN = 5;
bool week_trend_min(const PlayStats& stats, int64_t& per_day_min);

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

// The account Activity starts on, as an index in `keys` (each account's
// play_data key, in play_data::accounts() order), -1 every account. `setting`
// is config's activity_account: "last" takes `last`, the one shown last; ""
// or a key no account has any more (deleted, or the other system's), every
// account.
int start_account(const std::string& setting, const std::string& last, const std::vector<std::string>& keys);

// What the export's table says, in the language shown: the tab's words.
struct ExportText
{
    std::string title, subtitle, sheet;
    // The column headers, but the six days before today's.
    std::string game, id, today, week, month, total, launches, first, last;
    std::function<std::string(int wday)> day;           // a day's column ("Sat (min)"), 0 = Sunday
    std::function<std::string(const GameStat&)> name;   // a game's name, a deleted one's too
    std::function<std::string(uint64_t posix)> time;    // a date and time, never given 0
};

// Every game of `stats`, in the export's order (sort_by_play by total), with
// its play time in whole minutes (rounded, so a spreadsheet can add them up)
// today, on each of the six days before (most recent first), this week, the
// last 30 days and in all; its launches and first and last plays. A figure that was not read
// is an empty cell.
table_export::Table export_table(const PlayStats& stats, int period, const ExportText& text);

}   // namespace activity_summary
