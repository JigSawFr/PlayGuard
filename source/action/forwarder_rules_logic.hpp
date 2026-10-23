// forwarder_rules_logic — the UI-free part of forwarder_rules (the PIN
// gate's rules, its pause, its log): a protected forwarder's free and closed
// hours as PlayGuard remembers them, the closed hours of the forwarders made
// before they carried their rules, the gate's log folded for the change
// history and the week's tally, and the times and days its lines show.
// Plain C++ for the host tests (tests/forwarder_rules_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "util/forwarders.hpp"
#include "util/gate_tally.hpp"

namespace forwarder_rules_logic
{

// The free hours (when the PIN is not asked) set on `m`: a second span
// (`days2`) only with a first one, nothing at all without `days`.
void set_free(forwarders::Made& m, int days, int from, int to, int days2, int from2, int to2);
// The hours it never opens, nothing without `days`.
void set_closed(forwarders::Made& m, int days, int from, int to);

// GATE_CLOSED_NAME's text: the closed hours of the protected forwarders
// made before they carried their rules (Made::carried false), one line each
// (core/gate.h's gate_rule_format); "" when none has any.
std::string closed_rules(const std::vector<forwarders::Made>& made);

// At most this many of the log's tries are read at a time.
constexpr int LOG_MAX = 200;

// The gate's log (GATE_LOG_NAME) read: one entry per kind, forwarder and
// day, in the order first seen, with the time of each try in it.
struct Seen
{
    const char*              kind;   // the change history's: "gate_open", "gate_refused" …
    std::string              when;   // the first try's time ("" unknown)
    std::string              detail; // the forwarder's name (" (error)" after it for an error)
    std::vector<std::string> times;  // each try's time of day ("07:00:12"), the first's included
};
struct Log
{
    std::vector<Seen> seen;
    // The refusals, each the forwarder's name and its time: for
    // forwarders_logic::repeated_refusals.
    std::vector<std::pair<std::string, std::string>> refused;
    // Indexes in `made` of the protected forwarders not verified yet that the
    // gate opened (the PIN or free hours): this way back works.
    std::vector<size_t> seen_working;
};
// `text` read against what PlayGuard made (a forwarder's name; another .nro
// by its file name), each try with a time added to `tally` too.
Log fold_log(const std::string& text, const std::vector<forwarders::Made>& made, gate_tally::Tally& tally);

// When a pause ends, from its time as ui::time_text writes it
// ("2026-10-21 07:00:00"): the time alone on `today` ("07:00"), else the
// day too ("2026-10-21 07:00").
std::string until_text(const std::string& full, const std::string& today);

// A week's refusals by day, the oldest first: "Mon 2, Wed 1" (`day_names`
// Sunday first, 7 of them; "?" for a day that is no date).
std::string refused_by_day(const gate_tally::Week& w, const std::string& today, const std::vector<std::string>& day_names);

}   // namespace forwarder_rules_logic
