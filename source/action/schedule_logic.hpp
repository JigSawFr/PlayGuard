// schedule_logic — a profile scheduled for a period (Play timer › Profiles ›
// Scheduled profile, config schedule_*): offered when the period begins,
// the limits from before offered back once it is over. Plain C++ so the host
// tests can run it (tests/schedule_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace schedule_logic
{

struct Schedule
{
    std::string file, name;          // the profile (util/profiles: its file stem, its name)
    std::string from, to;            // "YYYY-MM-DD", both included; from empty: none
    bool        applied = false;     // put in force for the period
    std::array<uint16_t, 7> prev{};  // the limits it replaced (Sun..Sat), when applied
};

enum class Step
{
    None,     // nothing scheduled, before the period, or applied and not over
    Apply,    // within the period, not in force yet: offer it
    Revert,   // applied, the period over: offer the limits from before back
    Drop,     // never applied and the period over: forget it
};
Step step(const Schedule& s, const std::string& today);

// `count` dates from `first` on ("YYYY-MM-DD"), for the period's pickers;
// empty when `first` is not a date.
std::vector<std::string> dates_from(const std::string& first, int count);

// A period the pickers can give: real dates, `from` <= `to`.
bool valid_period(const std::string& from, const std::string& to);

}   // namespace schedule_logic
