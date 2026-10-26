// ui — time limits, play time, level and age names as text: PlayGuard's
// formatting helpers that only translate (ui.hpp includes this header),
// beside Aurore's (borealis/cadran/ui/format_text.hpp: durations, days,
// short answers). Plain C++ over Cadran's ui/tr.hpp, so a host suite can run
// them against a catalog of its own (tests/format_text).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include <borealis/cadran/ui/format_text.hpp>

namespace ui
{

std::string fmt_limit(uint16_t minutes);        // a limit: "No limit", "0 min (no play)", "45 min", "2 h 30"
// Seven limits in a few words: "2 h every day", "1 h to 3 h", "1 h to no limit", "No limit".
std::string days_summary(const uint16_t days[7]);
std::string fmt_played(uint16_t minutes);       // time played: "0 min", "45 min", "2 h 30"
std::string fmt_time_left(uint64_t ns);         // remaining time, rounded to minutes, 24 h at most
// A daily average and the days it spans: "53 min over 45 days", "… over 18 months", "… over 6 years".
std::string fmt_per_day_over(uint64_t per_day_s, int days);
std::string age_text(int age);                 // a restriction age: "None" for 0, else "12+"
std::string level_name(uint32_t level);         // localised restriction level, "?" beyond 4

}   // namespace ui
