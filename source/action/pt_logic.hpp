// pt_logic — the decisions behind pt_flow, without any UI: is the state known,
// does a write need the temporary unlock, how much was played, what extra time
// gives and when it is put back. Plain C++ over the service-layer structs so
// the host tests can run it (tests/pt_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "util/pctl_ops_c.hpp"

namespace pt_logic
{

// Every field a play-timer write is gated on was read.
bool state_known(const PtState& pt);

// The timer counts down (or has suspended the game) and is not unlocked: a
// write has to unlock first. Only meaningful when state_known().
bool needs_unlock(const PtState& pt);

// Minutes already played today (`weekday` 0 = Sunday), or -1 when the system
// does not say (timer off, no limit today, or no game counted yet).
int played_today_min(const PtState& pt, int weekday);

// The minutes already played when `new_days` puts today's limit below them
// (the game in progress is then suspended at the lock), else -1.
int suspend_warning_min(const PtState& pt, int weekday, const uint16_t new_days[7]);

// "Extra time today…" can be offered: writable, a limit applies today and it
// is below 24 h.
bool can_add_extra_time(const PtState& pt, int weekday, bool read_only);

// Extra time on a day: the limit to write and the one to put back later. When
// extra time was already added today (`again`), what gets put back stays the
// limit from before the first addition (`recorded_base`).
struct ExtraPlan
{
    uint16_t value;      // new limit, at most 24 h
    uint16_t original;   // what "put back" restores
};
ExtraPlan plan_extra(uint16_t base, uint16_t extra, bool again, uint16_t recorded_base);

// "No more play today" can be offered: writable, the play timer can be read
// and today's limit is not already 0 (with no limit today it sets one).
bool can_stop_today(const PtState& pt, int weekday, bool read_only);

// No more play today: 0 for today, and what to put back the next day (the
// limit from before any extra time added earlier today, as for extra time).
ExtraPlan plan_stop(uint16_t base, bool again, uint16_t recorded_base);

// "No more play today", in `minutes` instead of now: today's limit becomes
// the time played plus `minutes`, so the child can save first. -1 when that
// would not be below today's limit, or the time played is not known (no
// limit today, nothing counted yet).
int stop_soon_limit(const PtState& pt, int weekday, int minutes);

// What config remembers about extra time (config extra_*).
struct ExtraRecord
{
    int         weekday = -1;   // -1: nothing recorded
    std::string date;           // "YYYY-MM-DD" it was added on
    uint16_t    base  = 0;      // limit before
    uint16_t    value = 0;      // limit written
};

enum class Restore
{
    None,    // nothing recorded, or still the day it was added
    Later,   // cannot tell now (read failed, read-only): ask at the next start
    Forget,  // the limit changed since: nothing to put back, drop the record
    Offer,   // the extra time is still there: offer to put `base` back
};
Restore restore_action(const ExtraRecord& rec, const std::string& today, const PtState& pt, bool read_only);

// Bedtime. Its place in the block is inferred (core/pure.h): it can be
// changed only while what the block holds for today, or for the day before
// (after midnight), is what the console reports (1954/1956/1957).
bool bedtime_layout_ok(const PtState& pt, int weekday);

// The seven bedtimes when they are all the same.
bool bedtime_uniform(const PtState& pt, PtBedtime* out);

// Every day's alarm at hour:minute (`on`), or off. A day keeps its
// allowed-again time when it is in range (05:00 to 09:00), else gets 06:00,
// the companion app's default.
void bedtime_every_day(const PtState& pt, bool on, uint8_t hour, uint8_t minute, PtBedtime out[7]);

// Play allowed again at hour:minute on every day whose bedtime is on. False
// (out untouched) when none is.
bool bedtime_end_every_day(const PtState& pt, uint8_t hour, uint8_t minute, PtBedtime out[7]);

// What pt_flow's pickers start on.

// Limits offered by the pickers, in minutes (0 = no play that day).
const std::vector<uint16_t>& quick_values();

// "Same limit every day": the row selected first (one of quick_values(), or
// the "Custom…" row after them when the days differ or the value is not in
// the list), and the number pad's first value.
struct UniformPick
{
    bool     uniform;    // the seven limits were read and are the same
    int      selected;
    uint16_t seed;
};
UniformPick uniform_pick(const PtState& pt);

// One day's picker: quick_values(), then "Enter a duration…" (`custom`),
// then "No limit" (`no_limit`), with `current` selected.
struct LimitPick
{
    int selected, custom, no_limit;
};
LimitPick limit_pick(uint16_t current);
// The number pad's first value for that day.
uint16_t limit_seed(uint16_t current);

// The bedtime picker's times in minutes after midnight: -1 (off), then 16:00
// to 23:45 every quarter of an hour; and the row selected first (the bedtime
// when every day has the same, 21:00 when the days differ, else off).
std::vector<int> bedtime_times();
int bedtime_selected(const PtState& pt, const std::vector<int>& times);
// The same for the allowed-again time: 05:00 to 09:00, 06:00 unless every day
// has the same one in the list.
std::vector<int> bedtime_end_times();
int bedtime_end_selected(const PtState& pt, const std::vector<int>& times);

// Extra time (or no more play) was recorded today, on `weekday`: a second
// addition puts back what the first one recorded.
bool recorded_today(const ExtraRecord& rec, int weekday, const std::string& today);

// The minutes of extra time added today that `weekday`'s limit still holds
// (the Overview's "+30 min"), else 0.
int extra_today_min(const ExtraRecord& rec, const std::string& today, int weekday, const PtState& pt);

// The timer is on and its "time's up" alarm is off.
bool alarm_off(const PtState& pt);

}   // namespace pt_logic
