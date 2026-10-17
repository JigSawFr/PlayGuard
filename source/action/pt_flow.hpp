// pt_flow — the "write-before-temporary-unlock" gate used by every play-timer
// write. If the timer is counting down, writing a new limit destabilises
// Atmosphère, so: ask the user → UnlockRestrictionTemporarily (1201) →
// verify IsRestrictionTemporaryUnlocked (1006) → let the caller write → lock
// again (automatically, or offered). The service layer re-checks the same state
// right before the write.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "util/pctl_ops_c.hpp"

namespace pt_flow
{

// `body`, then, while the console is linked to the phone app, the note that
// its next synchronisation overwrites what is written here: every write
// confirmation ends with it. `st`: a status already read, else it is read now.
std::string with_link_note(const std::string& body, const PctlStatus* st = nullptr);

// One dialog for the whole change. `body` asks the question ("Set 2 h for every
// day?"); when the timer is counting down the same dialog explains the
// temporary unlock and its button unlocks first. When `new_days` would put
// today's limit below the time already played, the dialog says the game in
// progress will be suspended, and while the phone app is linked, that it can
// overwrite the change (with_link_note). With nothing to say and no unlock needed,
// `write` runs at once. `write(did_unlock)` only runs when it is safe to
// write; otherwise the user has already been told why.
// `danger`: the confirm button is drawn in the "bad" colour (removing or
// overwriting limits).
void confirm_write(const std::string& body, const std::string& confirm_label,
                   std::function<void(bool did_unlock)> write, const uint16_t* new_days = nullptr,
                   bool danger = false);

// Writes the seven limits (pctl_play_timer_set_days) and records the change
// in the history with `source` ("uniform", "profile" …) and `detail` (a
// profile's name). Every limit write of the app goes through here.
Result write_days(const uint16_t days[7], const std::string& source, const std::string& detail = "");
// Removes the play-time limit (pctl_play_timer_clear), recorded the same way.
Result clear_days(const std::string& source);

// After a write made through confirm_write: toast the result, then lock again
// if the write needed an unlock (at once when the "lock again automatically"
// preference is on, else by offering it), then `refresh`.
void finish_write(Result rc, bool did_unlock, const std::string& ok_text,
                  const std::string& error_prefix, std::function<void()> refresh);

// "Same limit every day" picker (quick values + Custom…), then confirm_write.
void choose_uniform_limit(const PtState& pt, std::function<void()> refresh);

// One day's limit picker: the quick values, "Enter a duration…" (the number
// pad) and "No limit", with `current` pre-selected. `on_value` gets the
// minutes (PT_DAY_NOLIMIT for no limit).
void pick_limit(const std::string& title, uint16_t current, std::function<void(uint16_t)> on_value);

// "Extra time today": raises today's weekday limit by 15 / 30 / 60 min and
// remembers the previous value (config extra_*), so that it can be put back.
void add_extra_time(const PtState& pt, std::function<void()> refresh);

// "No more play today": today's weekday limit to 0 for today only (through
// confirm_write: the suspend warning says the game in progress stops at the
// lock). Recorded like extra time, so the usual limit comes back the next day.
void stop_today(const PtState& pt, std::function<void()> refresh);

// A limit changed for one earlier day still holds -> put the usual one back.
// Extra time: by itself, with no question and no PIN (it only lowers the
// limit, as locking again does), through the temporary unlock while the timer
// counts down, locked again at once; it waits while it cannot be written
// (read-only, firmware not supported, state unknown, the console lock) and,
// on today's weekday, while it would suspend the game in progress (then the
// next day). No more play today: put back as any change (the PIN) when the
// "by itself" preference is on, else asked (keep it, or put it back); "no
// limit" to put back is always asked. Called at start-up, when the date
// changes while the app is open and when read-only mode ends; `asked`: from
// the Overview or the bell, where what stops it is said. `refresh` runs after
// a change.
void offer_extra_time_restore(std::function<void()> refresh = nullptr, bool asked = false);
// The last restore waited for a state it could not read: try again (the main
// screen, every 30 s).
void retry_extra_time_restore();

// That offer is pending (what offer_extra_time_restore would ask now), and
// the line the Overview shows for it ("Put Saturday's limit back to 2 h").
bool restore_pending(const PtState& pt);
std::string restore_label();

// "+30 min" when extra time was added today and is still there, else "".
std::string extra_today_text(const PtState& pt);

// The timer is on and its "time's up" alarm is off: the console says nothing
// when time is up. The Overview and First steps point it out.
bool alarm_off(const PtState& pt);
// Turns that alarm back on after a confirmation (the PIN asked before a
// change, read-only mode: as for any change), recorded with `source`.
// Not an advanced action: it puts back what the console does by default.
void turn_alarm_on(const std::string& source, std::function<void()> refresh);
// Turns the alarm on or off after a confirmation, through confirm_write (the
// service layer refuses it while the timer counts down and nothing unlocked).
void set_alarm(bool on, const std::string& source, std::function<void()> refresh);
// Writes the alarm flag and records it in the history (callers already went
// through confirm_write).
Result write_alarm_disabled(bool disabled, const std::string& source);
// Advanced: pauses (StopPlayTimer) or resumes (StartPlayTimer) the countdown,
// through confirm_write.
void set_countdown(bool running, std::function<void()> refresh);

// What the console does when the time is up: suspend the software, or a
// warning only (pctl_play_timer_set_alarm_only). time_up_text: the current
// choice, "" when the block is not read. choose_time_up: the two choices,
// then confirm_write and write_alarm_only; refused, with the reason, while no
// limit and no bedtime is set.
std::string time_up_text(const PtState& pt);
void choose_time_up(const PtState& pt, std::function<void()> refresh);
// Writes the choice and records it in the history ("time_up") with `source`.
Result write_alarm_only(bool alarm_only, const std::string& source);

// Bedtime, for every day. choose_bedtime: the alarm (16:00 to 23:45, or
// off); choose_bedtime_end: when play is allowed again (05:00 to 09:00), on
// the days whose alarm is on. Both refuse while the block's bedtime is not
// what the console reports (pt_logic::bedtime_layout_ok), then go through
// confirm_write and write_bedtime.
void choose_bedtime(const PtState& pt, std::function<void()> refresh);
void choose_bedtime_end(const PtState& pt, std::function<void()> refresh);
// Writes the bedtimes (pctl_play_timer_set_bedtime, checked by the console's
// answer) and records the change in the history with `source`.
Result write_bedtime(const PtBedtime bed[7], const std::string& source);
// "21:00, allowed again at 06:00", "Off", or "Varies by day".
std::string bedtime_text(const PtState& pt);

// At start-up, before the first screen (and again on the main screen): an
// earlier run unlocked (for a change, or Security › Unlock temporarily) and
// stopped before locking again (config relock_pending) -> lock again now,
// with a toast. The record stays until the console reads back as locked.
// Also called when the app leaves read-only mode, which keeps the record;
// this run's own unlock is left alone (pt_logic::pending_relock).
void relock_if_interrupted();

// Security › Unlock temporarily: unlocks, recorded as above, so that closing
// PlayGuard from the HOME menu (which asks nothing) locks again at the next
// start.
Result unlock_temporarily();
// Quitting while unlocked, "Quit, still unlocked": a choice, so the record goes.
void keep_unlocked();
// Quitting while unlocked, "Turn back on and quit": the lock waits for the end of
// main(), once the screen is gone (lock_at_exit_if_asked). Locked while the
// app still ran, the console would at once stop the game PlayGuard runs over
// (the play timer's "limit reached", the PIN) with PlayGuard behind it, and
// close it only after that. Recorded first: a lock that fails then, with no
// screen left to say so, is made at the next start.
void lock_at_exit();
// In main(), after the main loop: the lock asked for above, then the end of
// the game PlayGuard runs over (back to the HOME menu). On failure the record
// stays and hbloader's next homebrew starts as usual.
void lock_at_exit_if_asked();

}   // namespace pt_flow
