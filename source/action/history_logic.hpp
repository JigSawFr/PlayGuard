// history_logic — the decisions behind history_flow's undo, without any UI:
// what an undoable entry's dialog offers given the console's value now, and
// what the undo records. Whether an entry can be undone at all is
// history::undoable (tests/history). Also the History screen's categories
// and filters, and the kinds and sources it names. Host-tested
// (tests/history_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "util/history.hpp"

namespace history_logic
{

enum class Undo
{
    AlreadyBack,     // the console already holds `before`
    ReadOnly,        // read-only mode: nothing is written
    NeedsAdvanced,   // the alarm is changed only in advanced mode
    Offer,           // offer to put `before` back
};
// For an undoable entry `e`; `now` is the console's value of its kind ({}
// when it could not be read: the undo is still offered).
Undo undo_action(const history::Entry& e, const std::vector<int>& now, bool read_only, bool advanced);

// The value changed again since the entry: the dialog says so.
bool changed_since(const history::Entry& e, const std::vector<int>& now);

// What an undo records as the value it replaced: the console's now, or the
// entry's `after` when that could not be read.
std::vector<int> undo_replaces(const history::Entry& e, const std::vector<int>& now);

// The days (0 = Sunday) a "limits" entry changed, in week order; {} for
// another kind or a damaged entry.
std::vector<int> changed_days(const history::Entry& e);

// What an entry is about, for the History screen's filter.
enum class Category
{
    PlayTime,       // the limits, the alarms, the bedtime, the countdown, the console lock
    Restrictions,   // the level, the rating organisation, custom settings, VR
    Security,       // the PIN, unlocks, the companion app, deleting, recovery, the optional modules
    Outside,        // what PlayGuard saw changed outside it, a new account
    Homebrew,       // the forwarders, their PIN gate's tries, the homebrew menu's lock
    Other,          // the network clock, a backup restored, settings reset, a kind unknown
};
Category category(const history::Entry& e);

// The History screen's filter: everything, one category, or the changes
// the console refused.
enum class Filter
{
    All,
    PlayTime,
    Restrictions,
    Security,
    Outside,
    Failed,
    Homebrew,
    Count
};
bool matches(const history::Entry& e, Filter f);

// The day an entry belongs to ("2026-10-08"), "" when its stamp has none:
// the History screen starts a group at each new one.
std::string day_key(const history::Entry& e);

// The day of the week of `date` ("2026-10-08"): 0 = Sunday; -1 when it is
// not a date.
int weekday_of(const std::string& date);

// When the History screen dates an entry (`when`: "2026-10-08 18:30")
// relative to `today` and `yesterday` ("2026-10-08").
enum class Day { Today, Yesterday, Other };
Day day_of(const std::string& when, const std::string& today, const std::string& yesterday);

// The kinds of change this version records, each named under
// playguard/history/kinds/ (tools/check_resources.py checks the catalog
// against the list): one not in it was written by a newer version.
std::span<const std::string_view> known_kinds();
bool                              known_kind(std::string_view kind);
// What made a change, each named under playguard/history/sources/ ("day" is
// no longer written, but older histories hold it). "profile" is said apart,
// with the profile's name; any other source is not said.
std::span<const std::string_view> known_sources();
bool                              known_source(std::string_view source);

}   // namespace history_logic
