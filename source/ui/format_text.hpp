// ui — time limits, play time, day, level and age names as text: the
// formatting helpers that only translate (ui.hpp includes this header). Plain
// C++ over ui/tr.hpp, so a host suite can run them against a catalog of its
// own (tests/format_text).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

namespace ui
{

std::string fmt_minutes(uint16_t minutes);      // a limit: "No limit", "0 min (no play)", "45 min", "2 h 30"
// Seven limits in a few words: "2 h every day", "1 h to 3 h", "1 h to no limit", "No limit".
std::string days_summary(const uint16_t days[7]);
std::string fmt_played(uint16_t minutes);       // time played: "0 min", "45 min", "2 h 30"
std::string fmt_duration_ns(uint64_t ns);       // remaining time, rounded to minutes
std::string fmt_play_time(uint64_t seconds);    // "0 min", "< 1 min", "45 min", "152 h 30" (no 24 h cap)
std::string day_name(int day);                  // 0 = Sunday, as a title ("Monday")
std::string day_name_in_text(int day);          // inside a sentence (fr: "lundi")
std::string bool_text(bool ok, bool value, const std::string& yes, const std::string& no);
std::string yes_no(bool value);                // "Yes" / "No"
std::string age_text(int age);                 // a restriction age: "None" for 0, else "12+"
// The value of a line that opens a screen of its own (or another tab), with
// a chevron after it: "12 changes  ›", or "›" alone. A line that asks first,
// in a dialog, ends its title with "…" instead; one that acts at once, neither.
std::string opens(const std::string& value = "");
std::string level_name(uint32_t level);         // localised restriction level, "?" beyond 4

}   // namespace ui
