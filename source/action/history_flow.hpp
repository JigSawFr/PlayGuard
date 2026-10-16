// history_flow — records what PlayGuard changes on the console in the change
// history (util/history.hpp), words it for the History screen, and puts a
// recorded value back (through the same gates as any other change: the
// play-timer unlock, the PIN asked before a change, read-only mode).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "action/history_logic.hpp"
#include "util/history.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/table_export.hpp"

namespace history_flow
{

std::vector<int> days_values(const uint16_t days[7]);
std::vector<int> custom_values(const PctlCustomSettings& s);

// A value change ("limits", "level", "custom", "org", "vr", "alarm",
// "time_up"); nothing is recorded when `before` equals `after`. Written off
// the UI thread (ui::write_later); a failed SD write is only logged: the
// change itself went through.
void record_values(const char* kind, std::vector<int> before, std::vector<int> after,
                   const std::string& source = "", const std::string& detail = "");
// An action with no value to put back ("pin", "unlock", "unlink",
// "remove_game" / "remove_update" / "remove_addon" / "launch_version" with
// the game in `detail`, …).
void record_event(const char* kind, const std::string& source = "", const std::string& detail = "");
// A change the console refused with `rc`: what was tried (`after`, from
// `before` when known), never undoable. Nothing changed, so the outside
// watch is not told of a change of PlayGuard's.
void record_failure(const char* kind, Result rc, std::vector<int> before = {}, std::vector<int> after = {},
                    const std::string& source = "", const std::string& detail = "");

// history::load(), once the entries still being written are on the SD card.
std::vector<history::Entry> load();

// The History screen's wording: one line, and the details for its dialog.
std::string title(const history::Entry& e);
std::string details(const history::Entry& e);
// When it was made: "Today, 21:29", "Yesterday, 01:25", else the stamp.
std::string when_text(const history::Entry& e);
// Its time of day, "21:29" (the stamp when it has none).
std::string time_of(const history::Entry& e);
// A day's group title on the History screen: "Today", "Yesterday", else
// "Monday · 2026-10-12" (`day`: history_logic::day_key).
std::string day_title(const std::string& day);
// The filter's name: "Everything", "Play time"…
std::string filter_label(history_logic::Filter f);
// The entries as a table to export (util/table_export.hpp): when, the
// change, from where, its details, before, after, the result, the kind's
// code and the version.
table_export::Table table(const std::vector<history::Entry>& entries);

// A on a History line: its details, and when the value can be put back, the
// offer to (through the same gates as the change itself), recorded as an
// "undo". Says why not when it cannot be.
void open(const history::Entry& e, std::function<void()> refresh);

}   // namespace history_flow
