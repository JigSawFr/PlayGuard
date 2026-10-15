// history_logic — the decisions behind history_flow's undo, without any UI:
// what an undoable entry's dialog offers given the console's value now, and
// what the undo records. Whether an entry can be undone at all is
// history::undoable (tests/history). Host-tested (tests/history_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
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

// When the History screen dates an entry (`when`: "2026-10-08 18:30")
// relative to `today` and `yesterday` ("2026-10-08").
enum class Day { Today, Yesterday, Other };
Day day_of(const std::string& when, const std::string& today, const std::string& yesterday);

}   // namespace history_logic
