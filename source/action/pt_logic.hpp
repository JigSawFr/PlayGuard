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
// does not say (timer off, no limit today, or no game counted yet). Past the
// limit (the time left below 0): the limit plus the time over it.
int played_today_min(const PtState& pt, int weekday);

// Minutes played past today's limit, else -1: the time left reads below 0
// once more was played than the limit, which the console lets happen when it
// only warns (alarm_only) or while parental controls are unlocked.
int over_limit_min(const PtState& pt, int weekday);

// Where today's limit stands, as Today's ring and headline and the child
// screen's ring say it (each in its own words). `unlocked`: parental controls
// known to be unlocked, which puts a limit on hold (the console reports the
// timer off while unlocked, though the limit still stands).
struct RingState
{
    enum Kind
    {
        UNKNOWN,    // the play timer not read (or a firmware too old)
        NO_PLAY,    // today's limit is 0 (on hold when `unlocked`)
        ON_HOLD,    // unlocked: the limit stands, nothing stops play
        FREE,       // no limit today, or the timer off
        TIME_UP,    // the limit reached: suspended, or played past it (a warning only)
        COUNTING,   // counting down: `left` minutes left
        IDLE,       // a limit, no game counted yet today
    } kind = UNKNOWN;
    bool     unlocked = false;   // `unlocked`, with a limit today
    uint16_t limit = 0;          // today's
    // The share of the limit played: 1 once it is up (or no play), 0 with
    // nothing counted yet, -1 when there is none to show (on hold, no limit).
    float fraction = -1.0f;
    int   left = -1;     // COUNTING: the minutes left, rounded down
    int   played = -1;   // COUNTING, TIME_UP: the minutes played (played_today_min)
    int   over = -1;     // TIME_UP: the minutes past the limit, -1 when suspended at it
};
RingState ring_state(const PtState& pt, int weekday, bool unlocked);

// The minutes already played when `new_days` puts today's limit below them
// (the game in progress is then suspended at the lock), else -1; -1 too when
// the console only warns at the limit (alarm_only).
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

// Extra time planned for a later day (config bonus_*): that weekday's limit
// is raised now, for `date` only, and put back once that day is over.
struct BonusRecord
{
    int         weekday = -1;   // -1: nothing planned
    std::string date;           // "YYYY-MM-DD" of the day it is for
    uint16_t    base  = 0;      // limit before
    uint16_t    value = 0;      // limit written
};
// What a check on `today` does with it, as restore_action: None while the day
// is not over (or nothing planned), Later when the state cannot be read,
// Forget when the limit changed since, Offer to put `base` back.
Restore bonus_action(const BonusRecord& rec, const std::string& today, const PtState& pt, bool read_only);
// The days a bonus can be planned for: tomorrow and the five after (the
// same weekday a week on would be today's), as dates and weekdays.
struct PlanDay
{
    std::string date;
    int         weekday;
};
std::vector<PlanDay> bonus_days(const std::string& today, int today_weekday);
// A bonus can be planned for `weekday`: the play timer read, a limit set that
// day, below 24 h.
bool can_plan_bonus(const PtState& pt, int weekday, bool read_only);

// How a record restore_action() offers to put back is put back. `weekday`:
// today's (0 = Sunday); `by_itself`: the preference to put back "no more
// play today" with no question (config extra_auto_restore).
enum class RestoreWay
{
    ByItself,   // extra time: putting the limit back only lowers it, so it is
                // done with no question and no PIN, as locking again is
    Wait,       // the same, but on today's weekday and below the time already
                // played: the game in progress would be suspended; the next day
    Direct,     // no more play, with the preference on: put back as any change (the PIN)
    Ask,        // no more play: asked first (the preference off, or "no limit" to put back)
};
RestoreWay restore_way(const ExtraRecord& rec, const PtState& pt, int weekday, bool by_itself);

// Putting `rec.base` back lowers the limit that holds `rec.value`: never to
// "no limit", which is a removal and is always asked.
bool restore_tightens(const ExtraRecord& rec);

// config relock_pending: PlayGuard unlocked parental controls (for a change,
// or PIN › Unlock temporarily) and may have stopped before locking them
// again. What a start (or leaving read-only) does with that record:
enum class PendingRelock
{
    None,     // no record, or this run's own (an unlock still in use), or read-only (kept for a start that can lock)
    Keep,     // the console's state could not be read: try again next time
    Forget,   // locked already (by the HOME menu, a restart): the record goes
    Lock,     // still unlocked: lock again now
};
// `this_run`: the record was made since PlayGuard started. `state_read` and
// `unlocked`: the console's temporary unlock (1006), which needs reading only
// when pending_relock_needs_state().
bool pending_relock_needs_state(bool pending, bool this_run, bool read_only);
PendingRelock pending_relock(bool pending, bool this_run, bool read_only, bool state_read, bool unlocked);

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
// The same for the days of `days` only (bit n: weekday n, 0 = Sunday); the
// other days keep theirs.
constexpr uint8_t ALL_DAYS = 0x7F, WEEKDAYS = 0x3E, WEEKEND = 0x41;
void bedtime_days(const PtState& pt, uint8_t days, bool on, uint8_t hour, uint8_t minute, PtBedtime out[7]);
// The bedtime the days of `days` share (false when they differ, or nothing was read).
bool bedtime_shared(const PtState& pt, uint8_t days, PtBedtime* out);
// The week's alarms as runs of days in a row, Monday first, with the same
// alarm (on, hour, minute; the allowed-again time aside): Monday to Friday at
// 21:30 and the weekend at 22:30 are two. Days as 0 = Sunday.
struct BedRun
{
    int       first, last;
    PtBedtime bed;
};
std::vector<BedRun> bedtime_runs(const PtState& pt);

// Play allowed again at hour:minute on every day whose bedtime is on. False
// (out untouched) when none is.
bool bedtime_end_every_day(const PtState& pt, uint8_t hour, uint8_t minute, PtBedtime out[7]);

// What pt_flow's pickers start on.

// Limits offered by the pickers, in minutes (0 = no play that day).
const std::vector<uint16_t>& quick_values();

// "Same limit every day": the row selected first (one of quick_values(), or
// the "Custom…" row after them when the days differ or the value is not in
// the list), and the duration picker's first value.
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
// The duration picker's first value for that day.
uint16_t limit_seed(uint16_t current);

// The bedtime picker's times in minutes after midnight: -1 (off), then 16:00
// to 23:45 every quarter of an hour; and the row selected first (the bedtime
// when every day has the same, 21:00 when the days differ, else off).
std::vector<int> bedtime_times();
int bedtime_selected(const PtState& pt, const std::vector<int>& times);
// The same for the days of `days`.
int bedtime_selected_days(const PtState& pt, uint8_t days, const std::vector<int>& times);
// The same for the allowed-again time: 05:00 to 09:00, 06:00 unless every day
// has the same one in the list.
std::vector<int> bedtime_end_times();
int bedtime_end_selected(const PtState& pt, const std::vector<int>& times);

// Extra time (or no more play) was recorded today, on `weekday`: a second
// addition puts back what the first one recorded.
bool recorded_today(const ExtraRecord& rec, int weekday, const std::string& today);

// The minutes of extra time added today that `weekday`'s limit still holds
// (Today's "+30 min"), else 0.
int extra_today_min(const ExtraRecord& rec, const std::string& today, int weekday, const PtState& pt);

// The timer is on and its "time's up" alarm is off.
bool alarm_off(const PtState& pt);

// What stops play first today, as Cadran's Today says it: at `now` (minutes
// after midnight), with `left` minutes of play left (-1: none counting down)
// and today's bedtime `bed` (an evening alarm, play allowed again the next
// morning). Before the morning's end, the night is the day before's: its
// bedtime is not this one's to tell.
struct NextStop
{
    enum Kind
    {
        NONE,
        TIME_UP,       // `at`, `in` minutes of play from now
        BEDTIME,       // at `at`, in `in` minutes
        BEDTIME_NOW,   // play allowed again at `at`
    } kind = NONE;
    int at = 0, in = 0;
};
NextStop next_stop(int now, int left, const PtBedtime& bed);

// The child screen counts down to the bedtime instead of the play time left:
// the bedtime comes first while play counts down (`left` >= 0), or, on a day
// without a limit (`free`), in its last hour.
bool counts_to_bed(const NextStop& n, int left, bool free);

// A duration changed with an arrow held (the minutes prompt, the week): 15 min
// a press, then whole hours once borealis repeats it for most of a second (a
// repeat every 100 ms, after 250 ms); `now_us`, the press's time. Kept between
// presses in `held`. Returns the new value, 0 to 24 h.
struct Held
{
    int     presses = 0;   // in a row, signed by their way
    int64_t last_us = 0;
};
int held_step(Held& held, int minutes, int delta, int64_t now_us);

}   // namespace pt_logic
