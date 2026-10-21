// gate_tally — the PIN gate's tries counted by day and by forwarder, kept
// four weeks (nand_dir()/gate_tally.json): the week's tries in History and
// Console › Forwarders, and the bell when the refusals climb. Filled from
// the gate's log as PlayGuard reads it (forwarder_flow::take_log). Plain
// C++ (no libnx, no UI) for the host tests (tests/gate_tally).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "core/gate.h"

namespace gate_tally
{

constexpr int KEEP_DAYS = 28;

struct Count
{
    int opened = 0;    // the PIN entered, or in its free hours
    int refused = 0;   // the PIN screen left with B
    int blocked = 0;   // not opened, the PIN not asked: paused, closed, the day's time used up, an error
    bool operator==(const Count& o) const { return opened == o.opened && refused == o.refused && blocked == o.blocked; }
};

// Day ("2026-10-20") -> forwarder (its .nro, "sdmc:/…") -> its count.
using Tally = std::map<std::string, std::map<std::string, Count>>;

// The file's text, and back; a damaged one reads as nothing.
Tally       parse(const std::string& text);
std::string text(const Tally& t);

// One try of `nro` on `day` (a log line's first ten characters).
void add(Tally& t, const std::string& day, const std::string& nro, GateResult r);
// The days before `today` - KEEP_DAYS + 1 dropped, and any that is no date.
void trim(Tally& t, const std::string& today);

// A forwarder's last seven days, `today` the last: each day's count, Monday
// or not (days[0] six days ago).
struct Week
{
    std::string          nro;
    std::array<Count, 7> days{};
    Count                total;
};
// Those tried in the seven days, the most refused first, then by .nro.
std::vector<Week> week(const Tally& t, const std::string& today);

// The refusals in the seven days to `today`, and in the seven before.
void refusals(const Tally& t, const std::string& today, int* this_week, int* week_before);
// Many more than usual: five at least this week, and more than twice the
// week before's.
bool rising(int this_week, int week_before);

}   // namespace gate_tally
