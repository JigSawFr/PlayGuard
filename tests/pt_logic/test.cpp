// Host tests for source/action/pt_logic.cpp: when a play-timer write needs the
// temporary unlock, how much was played today, extra time and putting it back,
// what pt_flow's pickers start on, an unlock left on by an earlier run, and
// what stops play first today (Cadran's Today).
#include "check.h"
#include <cstdio>
#include <cstring>

#include "action/pt_logic.hpp"

static const uint64_t MIN_NS = 60000000000ULL;

// Timer on, every field read, 2 h every day, 45 min left, not unlocked.
static PtState counting()
{
    PtState pt;
    std::memset(&pt, 0, sizeof(pt));
    pt.session_valid = pt.fw_supported = pt.valid = true;
    pt.enabled_valid = pt.restricted_valid = pt.temporary_unlocked_valid = pt.remaining_valid = true;
    pt.enabled = true;
    for (auto& d : pt.day_min) d = 120;
    pt.remaining_ns = 45 * MIN_NS;
    return pt;
}

static void test_gate()
{
    PtState pt = counting();
    CHECK(pt_logic::state_known(pt));
    CHECK(pt_logic::needs_unlock(pt));
    pt.temporary_unlocked = true;
    CHECK(!pt_logic::needs_unlock(pt));

    // Suspended by the timer (limit reached) still needs the unlock, even
    // when 1453 reads false.
    pt = counting();
    pt.enabled = false;
    pt.restricted = true;
    CHECK(pt_logic::needs_unlock(pt));
    pt.restricted = false;
    CHECK(!pt_logic::needs_unlock(pt));

    // Any gating field unread: the state is unknown, nothing is written.
    bool PtState::*const fields[] = { &PtState::valid, &PtState::enabled_valid, &PtState::restricted_valid,
                                        &PtState::temporary_unlocked_valid };
    for (auto f : fields) {
        pt = counting();
        pt.*f = false;
        CHECK(!pt_logic::state_known(pt));
    }
}

static void test_played()
{
    PtState pt = counting();
    CHECK(pt_logic::played_today_min(pt, 3) == 75);   // 2 h limit, 45 min left
    pt.remaining_ns = 0;                                // nothing counted yet today
    CHECK(pt_logic::played_today_min(pt, 3) == -1);
    pt.restricted = true;                               // suspended: the whole limit
    CHECK(pt_logic::played_today_min(pt, 3) == 120);
    pt = counting();
    pt.remaining_ns = 200 * MIN_NS;                     // more left than the limit (extra time)
    CHECK(pt_logic::played_today_min(pt, 3) == 0);
    pt = counting();
    pt.day_min[3] = PT_DAY_NOLIMIT;
    CHECK(pt_logic::played_today_min(pt, 3) == -1);
    pt = counting();
    pt.enabled = false;
    CHECK(pt_logic::played_today_min(pt, 3) == -1);
    CHECK(pt_logic::played_today_min(counting(), 7) == -1);
    CHECK(pt_logic::played_today_min(counting(), -1) == -1);

    // Past the limit, a warning only: the time left reads below 0, by the
    // time over it (as on 22.5.0, a 0-minute day played 2 min).
    pt = counting();
    CHECK(pt_logic::over_limit_min(pt, 3) == -1);
    pt.alarm_only = true;
    pt.remaining_ns = -(int64_t)(2 * MIN_NS);
    CHECK(pt_logic::over_limit_min(pt, 3) == 2 && pt_logic::played_today_min(pt, 3) == 122);
    pt.day_min[3] = 0;
    CHECK(pt_logic::over_limit_min(pt, 3) == 2 && pt_logic::played_today_min(pt, 3) == 2);
    pt.remaining_ns = -(int64_t)(MIN_NS / 2 - 1);   // under half a minute over: 0, still over
    CHECK(pt_logic::over_limit_min(pt, 3) == 0);
    // Past the limit while unlocked, suspending (the 22.0.0 report): 1455
    // reads true, the time over it still counts.
    PtState unlocked = pt;
    unlocked.alarm_only = false;
    unlocked.restricted = true;
    unlocked.remaining_ns = -224 * (int64_t)1000000000;
    CHECK(pt_logic::over_limit_min(unlocked, 3) == 4 && pt_logic::played_today_min(unlocked, 3) == 4);
    pt.remaining_ns = INT64_MIN;                    // whatever it reads, at most a day
    CHECK(pt_logic::over_limit_min(pt, 3) == 1440);
    pt.remaining_ns = -(int64_t)MIN_NS;
    pt.day_min[3] = PT_DAY_NOLIMIT;
    CHECK(pt_logic::over_limit_min(pt, 3) == -1);
    pt.day_min[3] = 0;
    pt.remaining_valid = false;
    CHECK(pt_logic::over_limit_min(pt, 3) == -1);
    pt.remaining_valid = true;
    pt.enabled = false;
    CHECK(pt_logic::over_limit_min(pt, 3) == -1);
}

static void test_suspend_warning()
{
    const PtState pt = counting();   // 75 min played on Wednesday
    uint16_t days[7];
    for (auto& d : days) d = 60;
    CHECK(pt_logic::suspend_warning_min(pt, 3, days) == 75);
    days[3] = 75;
    CHECK(pt_logic::suspend_warning_min(pt, 3, days) == -1);   // not below
    days[3] = PT_DAY_NOLIMIT;
    CHECK(pt_logic::suspend_warning_min(pt, 3, days) == -1);
    days[3] = 0;
    CHECK(pt_logic::suspend_warning_min(pt, 3, days) == 75);
    CHECK(pt_logic::suspend_warning_min(pt, 3, nullptr) == -1);
    // A warning only at the limit: nothing is suspended.
    PtState alarm = pt;
    alarm.alarm_only = true;
    CHECK(pt_logic::suspend_warning_min(alarm, 3, days) == -1);
}

static void test_extra()
{
    PtState pt = counting();
    CHECK(pt_logic::can_add_extra_time(pt, 3, false));
    CHECK(!pt_logic::can_add_extra_time(pt, 3, true));    // read-only
    pt.day_min[3] = 1440;
    CHECK(!pt_logic::can_add_extra_time(pt, 3, false));   // already 24 h
    pt.day_min[3] = PT_DAY_NOLIMIT;
    CHECK(!pt_logic::can_add_extra_time(pt, 3, false));
    pt = counting();
    pt.enabled = false;
    CHECK(!pt_logic::can_add_extra_time(pt, 3, false));
    pt = counting();
    pt.fw_supported = false;
    CHECK(!pt_logic::can_add_extra_time(pt, 3, false));

    pt_logic::ExtraPlan p = pt_logic::plan_extra(120, 30, false, 0);
    CHECK(p.value == 150 && p.original == 120);
    p = pt_logic::plan_extra(150, 30, true, 120);          // second time today
    CHECK(p.value == 180 && p.original == 120);
    p = pt_logic::plan_extra(1430, 60, false, 0);          // capped at 24 h
    CHECK(p.value == 1440 && p.original == 1430);
}

static void test_stop()
{
    PtState pt = counting();
    CHECK(pt_logic::can_stop_today(pt, 3, false));
    CHECK(!pt_logic::can_stop_today(pt, 3, true));     // read-only
    pt.day_min[3] = 0;
    CHECK(!pt_logic::can_stop_today(pt, 3, false));    // already no play today
    pt.day_min[3] = PT_DAY_NOLIMIT;
    CHECK(pt_logic::can_stop_today(pt, 3, false));     // no limit today: sets one
    pt.enabled = false;
    CHECK(pt_logic::can_stop_today(pt, 3, false));     // timer off: still possible
    pt.valid = false;
    CHECK(!pt_logic::can_stop_today(pt, 3, false));
    CHECK(!pt_logic::can_stop_today(counting(), 7, false));

    pt_logic::ExtraPlan p = pt_logic::plan_stop(120, false, 0);
    CHECK(p.value == 0 && p.original == 120);
    p = pt_logic::plan_stop(150, true, 120);            // extra time added earlier today
    CHECK(p.value == 0 && p.original == 120);
    p = pt_logic::plan_stop(PT_DAY_NOLIMIT, false, 0);  // put "no limit" back tomorrow
    CHECK(p.value == 0 && p.original == PT_DAY_NOLIMIT);

    // The record restores like extra time: 0 still there the next day -> offer.
    pt_logic::ExtraRecord rec;
    rec.weekday = 3;
    rec.date = "2026-10-07";
    rec.base = 120;
    rec.value = 0;
    pt = counting();
    pt.day_min[3] = 0;
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, false) == pt_logic::Restore::Offer);

    // "In 5 minutes": the time played (75 min) plus 5, below the 2 h limit.
    pt = counting();
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == 80);
    pt.remaining_ns = 4 * MIN_NS;                        // 116 played: 121 would not stop sooner
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == -1);
    pt.remaining_ns = 6 * MIN_NS;
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == 119);
    pt.remaining_ns = 0;                                 // nothing counted yet: now only
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == -1);
    pt = counting();
    pt.restricted = true;                                // already reached
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == -1);
    pt = counting();
    pt.day_min[3] = PT_DAY_NOLIMIT;
    CHECK(pt_logic::stop_soon_limit(pt, 3, 5) == -1);
    CHECK(pt_logic::stop_soon_limit(counting(), 3, 0) == -1);
    // Put back the next day like "no more play": the record holds 80.
    rec.value = 80;
    pt = counting();
    pt.day_min[3] = 80;
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, false) == pt_logic::Restore::Offer);
}

// Extra time planned for a later day: put back once that day is over.
static void test_bonus()
{
    using pt_logic::Restore;
    pt_logic::BonusRecord rec;
    PtState pt = counting();
    CHECK(pt_logic::bonus_action(rec, "2026-10-08", pt, false) == Restore::None);    // nothing planned
    rec.weekday = 6;
    rec.date = "2026-10-10";   // a Saturday
    rec.base = 180;
    rec.value = 240;
    pt.day_min[6] = 240;
    CHECK(pt_logic::bonus_action(rec, "2026-10-08", pt, false) == Restore::None);    // before the day
    CHECK(pt_logic::bonus_action(rec, "2026-10-10", pt, false) == Restore::None);    // the day itself
    CHECK(pt_logic::bonus_action(rec, "2026-10-11", pt, false) == Restore::Offer);   // over
    CHECK(pt_logic::bonus_action(rec, "2026-10-17", pt, false) == Restore::Offer);   // a week on, still put back
    CHECK(pt_logic::bonus_action(rec, "2026-10-11", pt, true) == Restore::Later);    // read-only
    pt.day_min[6] = 200;
    CHECK(pt_logic::bonus_action(rec, "2026-10-11", pt, false) == Restore::Forget);  // changed since
    rec.date = "soon";
    CHECK(pt_logic::bonus_action(rec, "2026-10-11", pt, false) == Restore::None);

    // The days offered: tomorrow and the five after, with their weekdays.
    const auto days = pt_logic::bonus_days("2026-10-08", 4);   // a Thursday
    CHECK(days.size() == 6 && days[0].date == "2026-10-09" && days[0].weekday == 5);
    CHECK(days[2].date == "2026-10-11" && days[2].weekday == 0 && days[5].date == "2026-10-14" && days[5].weekday == 3);
    CHECK(pt_logic::bonus_days("today", 4).empty());

    // Only on a day with a limit below 24 h, the state read and writable.
    pt = counting();
    pt.day_min[6] = 180;
    CHECK(pt_logic::can_plan_bonus(pt, 6, false) && !pt_logic::can_plan_bonus(pt, 6, true));
    pt.day_min[6] = PT_DAY_NOLIMIT;
    CHECK(!pt_logic::can_plan_bonus(pt, 6, false));
    pt.day_min[6] = 1440;
    CHECK(!pt_logic::can_plan_bonus(pt, 6, false) && !pt_logic::can_plan_bonus(pt, 7, false));
}

static void test_restore()
{
    pt_logic::ExtraRecord rec;
    rec.weekday = 3;
    rec.date = "2026-10-07";
    rec.base = 120;
    rec.value = 150;
    PtState pt = counting();
    pt.day_min[3] = 150;
    using pt_logic::Restore;
    CHECK(pt_logic::restore_action(rec, "2026-10-07", pt, false) == Restore::None);    // same day
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, false) == Restore::Offer);
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, true) == Restore::Later);    // read-only
    PtState unread = pt;
    unread.valid = false;
    CHECK(pt_logic::restore_action(rec, "2026-10-08", unread, false) == Restore::Later);
    pt.day_min[3] = 90;                                                                   // changed since
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, false) == Restore::Forget);
    rec.weekday = -1;
    CHECK(pt_logic::restore_action(rec, "2026-10-08", pt, false) == Restore::None);
}

// Where today's limit stands, for Today's ring and headline and the child
// screen's ring.
static void test_ring_state()
{
    using R = pt_logic::RingState;
    PtState pt = counting();
    R r = pt_logic::ring_state(pt, 3, false);   // 2 h limit, 45 min left
    CHECK(r.kind == R::COUNTING && r.limit == 120 && r.left == 45 && r.played == 75 && r.over == -1);
    CHECK(r.fraction > 0.62f && r.fraction < 0.63f && !r.unlocked);
    pt.remaining_ns = 45 * MIN_NS + MIN_NS - 1;     // the minutes left rounded down
    CHECK(pt_logic::ring_state(pt, 3, false).left == 45);
    pt.remaining_ns = 200 * MIN_NS;                 // more left than the limit (extra time)
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::COUNTING && r.left == 200 && r.played == 0 && r.fraction == 0.0f);

    // Nothing counted yet today: the time left reads 0 (or was not read).
    pt = counting();
    pt.remaining_ns = 0;
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::IDLE && r.fraction == 0.0f && r.left == -1 && r.played == -1);
    pt = counting();
    pt.remaining_valid = false;
    CHECK(pt_logic::ring_state(pt, 3, false).kind == R::IDLE);

    // Suspended at the limit: the whole limit played, nothing over it.
    pt = counting();
    pt.restricted = true;
    pt.remaining_ns = 0;
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::TIME_UP && r.fraction == 1.0f && r.played == 120 && r.over == -1 && r.left == -1);
    pt.restricted_valid = false;                    // not read: not known to be suspended
    CHECK(pt_logic::ring_state(pt, 3, false).kind == R::IDLE);

    // Past the limit with the console only warning (alarm_only): time up all
    // the same, not a whole limit left (the child screen once said so).
    pt = counting();
    pt.alarm_only = true;
    pt.remaining_ns = -(int64_t)(10 * MIN_NS);
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::TIME_UP && r.fraction == 1.0f && r.played == 130 && r.over == 10 && r.left == -1);
    pt.restricted = true;                           // past it while suspending (unlocked earlier)
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::TIME_UP && r.over == 10 && r.played == 130);

    // Unlocked: the limit on hold, whatever the timer says (it may read off).
    pt = counting();
    r = pt_logic::ring_state(pt, 3, true);
    CHECK(r.kind == R::ON_HOLD && r.unlocked && r.limit == 120 && r.fraction < 0.0f && r.left == -1);
    pt.enabled = false;
    CHECK(pt_logic::ring_state(pt, 3, true).kind == R::ON_HOLD);
    pt = counting();
    pt.alarm_only = true;
    pt.remaining_ns = -(int64_t)(10 * MIN_NS);      // past the limit, unlocked: still on hold
    CHECK(pt_logic::ring_state(pt, 3, true).kind == R::ON_HOLD);

    // No play today: in words, on hold while unlocked.
    pt = counting();
    pt.day_min[3] = 0;
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::NO_PLAY && !r.unlocked && r.fraction == 1.0f);
    r = pt_logic::ring_state(pt, 3, true);
    CHECK(r.kind == R::NO_PLAY && r.unlocked && r.fraction < 0.0f);
    pt.enabled = false;                             // the timer off: still the limit's 0
    CHECK(pt_logic::ring_state(pt, 3, false).kind == R::NO_PLAY);

    // No limit today, or the timer off: free; unlocked changes nothing then.
    pt = counting();
    pt.day_min[3] = PT_DAY_NOLIMIT;
    r = pt_logic::ring_state(pt, 3, false);
    CHECK(r.kind == R::FREE && r.fraction < 0.0f && r.left == -1);
    r = pt_logic::ring_state(pt, 3, true);
    CHECK(r.kind == R::FREE && !r.unlocked);
    pt = counting();
    pt.enabled = false;
    CHECK(pt_logic::ring_state(pt, 3, false).kind == R::FREE);

    // Not read (or too old a firmware), or no weekday.
    bool PtState::*const fields[] = { &PtState::fw_supported, &PtState::valid, &PtState::enabled_valid };
    for (auto f : fields) {
        pt = counting();
        pt.*f = false;
        r = pt_logic::ring_state(pt, 3, false);
        CHECK(r.kind == R::UNKNOWN && r.fraction < 0.0f);
    }
    CHECK(pt_logic::ring_state(counting(), 7, false).kind == R::UNKNOWN);
    CHECK(pt_logic::ring_state(counting(), -1, false).kind == R::UNKNOWN);
}

static void test_restore_way()
{
    using pt_logic::RestoreWay;
    pt_logic::ExtraRecord rec;
    rec.weekday = 3;
    rec.date = "2026-10-07";
    rec.base = 120;
    rec.value = 150;
    PtState pt = counting();   // Wednesday at 150 (the extra time), 105 min played
    pt.day_min[3] = 150;

    // Extra time lowers back: by itself, whatever the preference.
    CHECK(pt_logic::restore_tightens(rec));
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::ByItself);
    CHECK(pt_logic::restore_way(rec, pt, 4, true) == RestoreWay::ByItself);
    // A week later, on that weekday, below the time played: the game in
    // progress would be suspended, the next day instead.
    pt.remaining_ns = 10 * MIN_NS;                       // 140 played of 150
    CHECK(pt_logic::restore_way(rec, pt, 3, false) == RestoreWay::Wait);
    CHECK(pt_logic::restore_way(rec, pt, 3, true) == RestoreWay::Wait);
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::ByItself);   // another day: nothing played on it
    pt.alarm_only = true;                                // only a warning: nothing is suspended
    CHECK(pt_logic::restore_way(rec, pt, 3, false) == RestoreWay::ByItself);
    pt = counting();
    pt.day_min[3] = 150;                                 // 105 played, under 120: no suspend
    CHECK(pt_logic::restore_way(rec, pt, 3, false) == RestoreWay::ByItself);

    // No more play today raises back: asked, or as any change with the preference.
    rec.value = 0;
    pt.day_min[3] = 0;
    CHECK(!pt_logic::restore_tightens(rec));
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::Ask);
    CHECK(pt_logic::restore_way(rec, pt, 4, true) == RestoreWay::Direct);
    // "In 5 minutes" (80 for that day) the same.
    rec.value = 80;
    CHECK(pt_logic::restore_way(rec, pt, 4, true) == RestoreWay::Direct);
    // "No limit" back is a removal: always asked, even with the preference.
    rec.base = PT_DAY_NOLIMIT;
    rec.value = 0;
    CHECK(!pt_logic::restore_tightens(rec));
    CHECK(pt_logic::restore_way(rec, pt, 4, true) == RestoreWay::Ask);
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::Ask);
    // No more play, then extra time on the same day: what comes back is the
    // usual limit, below the extra one: by itself.
    rec.base = 60;
    rec.value = 90;
    CHECK(pt_logic::restore_tightens(rec));
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::ByItself);
    // ... or above a smaller amount of extra time: asked.
    rec.value = 30;
    CHECK(pt_logic::restore_way(rec, pt, 4, false) == RestoreWay::Ask);
}

static PtBedtime bed(bool on, uint8_t h, uint8_t m, uint8_t eh, uint8_t em)
{
    PtBedtime b;
    b.on = on; b.hour = h; b.minute = m; b.end_hour = eh; b.end_minute = em;
    return b;
}

static void test_bedtime()
{
    // The block says 21:00 on Wednesday (3), off on Tuesday; the console reports 21:00.
    PtState pt = counting();
    for (auto& b : pt.bed) b = bed(false, 0, 0, 0, 0);
    pt.bed[3] = bed(true, 21, 0, 7, 0);
    pt.bedtime_valid = pt.bedtime_enabled = true;
    pt.bedtime_hour = 21;
    CHECK(pt_logic::bedtime_layout_ok(pt, 3));
    CHECK(pt_logic::bedtime_layout_ok(pt, 4));    // after midnight: the evening before
    CHECK(!pt_logic::bedtime_layout_ok(pt, 5));
    CHECK(!pt_logic::bedtime_layout_ok(pt, 7));
    pt.bedtime_minute = 30;                        // reported 21:30: not what the block holds
    CHECK(!pt_logic::bedtime_layout_ok(pt, 3));
    pt.bedtime_enabled = false;                    // off everywhere it is checked: the time does not matter
    CHECK(pt_logic::bedtime_layout_ok(pt, 1));
    pt.bedtime_valid = false;
    CHECK(!pt_logic::bedtime_layout_ok(pt, 1));
    pt.bedtime_valid = true;
    pt.valid = false;
    CHECK(!pt_logic::bedtime_layout_ok(pt, 1));
    pt.valid = true;

    PtBedtime u;
    CHECK(!pt_logic::bedtime_uniform(pt, &u));
    PtBedtime out[7];
    pt_logic::bedtime_every_day(pt, true, 20, 45, out);
    for (int n = 0; n < 7; n++) CHECK(out[n].on && out[n].hour == 20 && out[n].minute == 45);
    CHECK(out[3].end_hour == 7 && out[0].end_hour == 6 && out[0].end_minute == 0);   // 00:00 out of range: 06:00
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    CHECK(!pt_logic::bedtime_uniform(pt, &u));    // Wednesday still ends at 07:00
    CHECK(pt_logic::bedtime_end_every_day(pt, 8, 15, out));
    for (int n = 0; n < 7; n++) CHECK(out[n].end_hour == 8 && out[n].end_minute == 15 && out[n].hour == 20);
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    CHECK(pt_logic::bedtime_uniform(pt, &u) && u.on && u.end_hour == 8);

    pt_logic::bedtime_every_day(pt, false, 22, 0, out);
    for (int n = 0; n < 7; n++) CHECK(!out[n].on && out[n].hour == 0 && out[n].end_hour == 8);
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    CHECK(pt_logic::bedtime_uniform(pt, &u) && !u.on);
    CHECK(!pt_logic::bedtime_end_every_day(pt, 7, 0, out));   // no bedtime on

    // One set of days: Monday to Friday at 21:00, the weekend kept off.
    pt_logic::bedtime_days(pt, pt_logic::WEEKDAYS, true, 21, 0, out);
    for (int n = 1; n <= 5; n++) CHECK(out[n].on && out[n].hour == 21 && out[n].end_hour == 8);
    CHECK(!out[0].on && !out[6].on && out[6].end_hour == 8);
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    CHECK(pt_logic::bedtime_shared(pt, pt_logic::WEEKDAYS, &u) && u.on && u.hour == 21);
    CHECK(pt_logic::bedtime_shared(pt, pt_logic::WEEKEND, &u) && !u.on);
    CHECK(!pt_logic::bedtime_shared(pt, pt_logic::ALL_DAYS, &u) && !pt_logic::bedtime_uniform(pt, &u));
    CHECK(!pt_logic::bedtime_shared(pt, 0, &u));
    // Then Friday alone later: 22:30.
    pt_logic::bedtime_days(pt, 1u << 5, true, 22, 30, out);
    CHECK(out[5].hour == 22 && out[5].minute == 30 && out[4].hour == 21 && !out[6].on);
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    const std::vector<int> times = pt_logic::bedtime_times();
    CHECK(times[(size_t)pt_logic::bedtime_selected_days(pt, 1u << 5, times)] == 22 * 60 + 30);
    CHECK(times[(size_t)pt_logic::bedtime_selected_days(pt, pt_logic::WEEKDAYS, times)] == 21 * 60);   // they differ
    CHECK(pt_logic::bedtime_selected_days(pt, pt_logic::WEEKEND, times) == 0);                          // off

    // The runs, Monday first: Monday to Thursday at 21:00, Friday 22:30, the
    // weekend off (Saturday and Sunday one run, Sunday last).
    auto runs = pt_logic::bedtime_runs(pt);
    CHECK(runs.size() == 3);
    CHECK(runs[0].first == 1 && runs[0].last == 4 && runs[0].bed.hour == 21);
    CHECK(runs[1].first == 5 && runs[1].last == 5 && runs[1].bed.minute == 30);
    CHECK(runs[2].first == 6 && runs[2].last == 0 && !runs[2].bed.on);
    // The same alarm, another allowed-again time: still one run.
    pt_logic::bedtime_every_day(pt, true, 21, 0, out);
    for (int n = 0; n < 7; n++) pt.bed[n] = out[n];
    pt.bed[2].end_hour = 7;
    runs = pt_logic::bedtime_runs(pt);
    CHECK(runs.size() == 1 && runs[0].first == 1 && runs[0].last == 0);
    pt.valid = false;
    CHECK(pt_logic::bedtime_runs(pt).empty());
}

// What the pickers start on (pt_flow's quick values, the per-day picker, the bedtimes).
static void test_pickers()
{
    const auto& values = pt_logic::quick_values();
    CHECK(values.size() == 8 && values.front() == 30 && values.back() == 0);

    PtState pt = counting();                       // 2 h every day: in the list
    pt_logic::UniformPick u = pt_logic::uniform_pick(pt);
    CHECK(u.uniform && values[(size_t)u.selected] == 120 && u.seed == 120);
    for (auto& d : pt.day_min) d = 100;            // the same, but not a quick value: "Custom…"
    u = pt_logic::uniform_pick(pt);
    CHECK(u.uniform && u.selected == (int)values.size() && u.seed == 100);
    pt.day_min[6] = 0;                             // days differ: "Custom…", the pad starts on Sunday's
    u = pt_logic::uniform_pick(pt);
    CHECK(!u.uniform && u.selected == (int)values.size() && u.seed == 100);
    for (auto& d : pt.day_min) d = PT_DAY_NOLIMIT; // no limit: the pad starts on 1 h
    u = pt_logic::uniform_pick(pt);
    CHECK(u.uniform && u.selected == (int)values.size() && u.seed == 60);
    for (auto& d : pt.day_min) d = 0;              // 0 is a quick value (no play)
    u = pt_logic::uniform_pick(pt);
    CHECK(values[(size_t)u.selected] == 0 && u.seed == 0);
    pt.valid = false;                              // unread: nothing pretends to be current
    u = pt_logic::uniform_pick(pt);
    CHECK(!u.uniform && u.selected == (int)values.size() && u.seed == 60);

    pt_logic::LimitPick l = pt_logic::limit_pick(45);
    CHECK(values[(size_t)l.selected] == 45);
    CHECK(l.custom == (int)values.size() && l.no_limit == l.custom + 1);
    CHECK(pt_logic::limit_pick(50).selected == l.custom);
    CHECK(pt_logic::limit_pick(PT_DAY_NOLIMIT).selected == l.no_limit);
    CHECK(values[(size_t)pt_logic::limit_pick(0).selected] == 0);
    CHECK(pt_logic::limit_seed(50) == 50 && pt_logic::limit_seed(PT_DAY_NOLIMIT) == 60);

    const std::vector<int> times = pt_logic::bedtime_times();
    CHECK(times.size() == 33 && times[0] == -1 && times[1] == 16 * 60 && times.back() == 23 * 60 + 45);
    pt = counting();
    for (auto& b : pt.bed) b = bed(true, 21, 30, 7, 0);
    CHECK(times[(size_t)pt_logic::bedtime_selected(pt, times)] == 21 * 60 + 30);
    for (auto& b : pt.bed) b = bed(true, 15, 0, 7, 0);   // not in the list (set elsewhere): off
    CHECK(pt_logic::bedtime_selected(pt, times) == 0);
    for (auto& b : pt.bed) b = bed(false, 0, 0, 0, 0);
    CHECK(pt_logic::bedtime_selected(pt, times) == 0);
    pt.bed[5] = bed(true, 22, 0, 6, 0);                  // days differ: 21:00
    CHECK(times[(size_t)pt_logic::bedtime_selected(pt, times)] == 21 * 60);
    pt.valid = false;                                    // unread counts as differing
    CHECK(times[(size_t)pt_logic::bedtime_selected(pt, times)] == 21 * 60);

    const std::vector<int> ends = pt_logic::bedtime_end_times();
    CHECK(ends.size() == 17 && ends.front() == 5 * 60 && ends.back() == 9 * 60);
    pt = counting();
    for (auto& b : pt.bed) b = bed(true, 21, 0, 7, 15);
    CHECK(ends[(size_t)pt_logic::bedtime_end_selected(pt, ends)] == 7 * 60 + 15);
    for (auto& b : pt.bed) b = bed(true, 21, 0, 10, 0);  // out of the list: 06:00
    CHECK(ends[(size_t)pt_logic::bedtime_end_selected(pt, ends)] == 6 * 60);
    pt.bed[2] = bed(true, 21, 0, 8, 0);                  // days differ: 06:00
    CHECK(ends[(size_t)pt_logic::bedtime_end_selected(pt, ends)] == 6 * 60);
}

// relock_pending at a start: an unlock left on by an earlier run (a change
// cut short, or PIN › Unlock temporarily and PlayGuard closed from the
// HOME menu) is locked again; this run's own unlock is not.
static void test_pending_relock()
{
    using P = pt_logic::PendingRelock;
    CHECK(pt_logic::pending_relock_needs_state(true, false, false));
    CHECK(pt_logic::pending_relock(true, false, false, true, true) == P::Lock);
    CHECK(pt_logic::pending_relock(true, false, false, true, false) == P::Forget);   // locked meanwhile
    CHECK(pt_logic::pending_relock(true, false, false, false, true) == P::Keep);     // state unknown
    // No record, this run's own (the parent's unlock still in use), or
    // read-only (the lock would be refused): nothing read, nothing done.
    CHECK(!pt_logic::pending_relock_needs_state(false, false, false));
    CHECK(!pt_logic::pending_relock_needs_state(true, true, false));
    CHECK(!pt_logic::pending_relock_needs_state(true, false, true));
    CHECK(pt_logic::pending_relock(false, false, false, true, true) == P::None);
    CHECK(pt_logic::pending_relock(true, true, false, true, true) == P::None);
    CHECK(pt_logic::pending_relock(true, false, true, true, true) == P::None);
}

// Today's "+30 min" and the alarm notice.
static void test_extra_today()
{
    pt_logic::ExtraRecord rec;
    rec.weekday = 3;
    rec.date    = "2026-10-07";
    rec.base    = 120;
    rec.value   = 150;
    PtState pt = counting();
    pt.day_min[3] = 150;
    CHECK(pt_logic::recorded_today(rec, 3, "2026-10-07"));
    CHECK(!pt_logic::recorded_today(rec, 4, "2026-10-07"));
    CHECK(!pt_logic::recorded_today(rec, 3, "2026-10-14"));   // same weekday, a week later
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 3, pt) == 30);
    CHECK(pt_logic::extra_today_min(rec, "2026-10-08", 3, pt) == 0);   // another day
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 4, pt) == 0);
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 9, pt) == 0);
    pt.day_min[3] = 180;                                      // changed since
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 3, pt) == 0);
    pt.day_min[3] = 0;                                        // "no more play" lowered it: no "+"
    rec.value = 0;
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 3, pt) == 0);
    rec.value = 150;
    pt.day_min[3] = 150;
    pt.valid = false;
    CHECK(pt_logic::extra_today_min(rec, "2026-10-07", 3, pt) == 0);

    pt = counting();
    pt.alarm_disabled_valid = pt.alarm_disabled = true;
    CHECK(pt_logic::alarm_off(pt));
    pt.enabled = false;                                       // timer off: nothing to warn about
    CHECK(!pt_logic::alarm_off(pt));
    pt.enabled = true;
    pt.alarm_disabled_valid = false;
    CHECK(!pt_logic::alarm_off(pt));
    pt.alarm_disabled_valid = true;
    pt.fw_supported = false;
    CHECK(!pt_logic::alarm_off(pt));
}

static void test_next_stop()
{
    using pt_logic::NextStop;
    PtBedtime off = {};
    PtBedtime bed = {};
    bed.on = true;
    bed.hour = 21;
    bed.end_hour = 6;
    // 16:00, 40 min left: the time up first, at 16:40.
    NextStop n = pt_logic::next_stop(16 * 60, 40, bed);
    CHECK(n.kind == NextStop::TIME_UP && n.at == 16 * 60 + 40 && n.in == 40);
    // 20:30, 40 min left: the bedtime comes first.
    n = pt_logic::next_stop(20 * 60 + 30, 40, bed);
    CHECK(n.kind == NextStop::BEDTIME && n.at == 21 * 60 && n.in == 30);
    // Nothing counting down: the bedtime, or nothing without one.
    n = pt_logic::next_stop(10 * 60, -1, bed);
    CHECK(n.kind == NextStop::BEDTIME && n.in == 11 * 60);
    CHECK(pt_logic::next_stop(10 * 60, -1, off).kind == NextStop::NONE);
    // Past the bedtime: play again at 06:00.
    n = pt_logic::next_stop(22 * 60, 40, bed);
    CHECK(n.kind == NextStop::BEDTIME_NOW && n.at == 6 * 60);
    // Before the morning's end: the night before's, not told.
    CHECK(pt_logic::next_stop(5 * 60, 40, bed).kind == NextStop::NONE);
    // A time up after midnight, without a bedtime: the limit counts again by then.
    CHECK(pt_logic::next_stop(23 * 60 + 30, 40, off).kind == NextStop::NONE);
    n = pt_logic::next_stop(23 * 60, 40, off);
    CHECK(n.kind == NextStop::TIME_UP && n.at == 23 * 60 + 40);
    // A bedtime that does not run from an evening to a morning: not placed.
    PtBedtime odd = bed;
    odd.hour = 5;
    odd.end_hour = 6;
    n = pt_logic::next_stop(10 * 60, 40, odd);
    CHECK(n.kind == NextStop::TIME_UP);
}

// The child screen's ring: to the bedtime when it comes first, or in a free
// day's last hour; the time left otherwise.
static void test_counts_to_bed()
{
    using pt_logic::NextStop;
    PtBedtime bed = {};
    bed.on = true;
    bed.hour = 21;
    bed.end_hour = 6;
    // 20:30, 40 min of play left: the bedtime in 30 min comes first.
    CHECK(pt_logic::counts_to_bed(pt_logic::next_stop(20 * 60 + 30, 40, bed), 40, false));
    // 16:00, 40 min left: the time up first.
    CHECK(!pt_logic::counts_to_bed(pt_logic::next_stop(16 * 60, 40, bed), 40, false));
    // No limit today: only the bedtime's last hour.
    CHECK(pt_logic::counts_to_bed(pt_logic::next_stop(20 * 60 + 15, -1, bed), -1, true));
    CHECK(!pt_logic::counts_to_bed(pt_logic::next_stop(18 * 60, -1, bed), -1, true));
    // Nothing counting down and no free day (the time is up, unlocked): never.
    CHECK(!pt_logic::counts_to_bed(pt_logic::next_stop(20 * 60 + 30, -1, bed), -1, false));
    // Past the bedtime: nothing to count down to.
    CHECK(!pt_logic::counts_to_bed(pt_logic::next_stop(22 * 60, 40, bed), 40, false));
}

// An arrow held: 15 min a press, whole hours once it repeats for long enough;
// a pause or the other way starts again at 15 min.
static void test_held_step()
{
    pt_logic::Held held;
    int64_t t = 0;
    int m = pt_logic::held_step(held, 30, 15, t);
    CHECK(m == 45);
    // Held: the first repeat after 250 ms starts the count, then every 100 ms.
    t += 250000;
    m = pt_logic::held_step(held, m, 15, t);
    CHECK(m == 60);
    for (int i = 0; i < 4; i++) m = pt_logic::held_step(held, m, 15, t += 100000);
    CHECK(m == 120);   // 15 min a press so far
    m = pt_logic::held_step(held, m, 15, t += 100000);
    CHECK(m == 180);   // then the next whole hour
    m = pt_logic::held_step(held, 190, 15, t += 100000);
    CHECK(m == 240);
    // Down, held, from 3 h 10: 2 h 55 … then whole hours down.
    pt_logic::Held down;
    t = 0;
    m = pt_logic::held_step(down, 190, -15, t);
    CHECK(m == 175);
    t += 250000;
    for (int i = 0; i < 5; i++) m = pt_logic::held_step(down, m, -15, t += 100000);
    CHECK(m == 100);
    m = pt_logic::held_step(down, m, -15, t += 100000);
    CHECK(m == 60);
    // The other way, or a pause: 15 min again.
    CHECK(pt_logic::held_step(down, 60, 15, t += 100000) == 75);
    CHECK(pt_logic::held_step(down, 75, 15, t += 1000000) == 90);
    // L/R (5 min) never jump; the limits hold.
    pt_logic::Held five;
    t = 0;
    for (int i = 0; i < 10; i++) m = pt_logic::held_step(five, 100, 5, t += 100000);
    CHECK(m == 105);
    CHECK(pt_logic::held_step(five, 1435, 15, 0) == 1440);
    CHECK(pt_logic::held_step(five, 10, -15, 10000000) == 0);
}

int main()
{
    test_gate();
    test_played();
    test_suspend_warning();
    test_extra();
    test_stop();
    test_restore();
    test_bonus();
    test_ring_state();
    test_restore_way();
    test_bedtime();
    test_pickers();
    test_extra_today();
    test_pending_relock();
    test_next_stop();
    test_counts_to_bed();
    test_held_step();
    return CHECK_DONE("pt_logic gate, played time, suspend warning, extra time, no-more-play, planned bonus, today's ring, bedtime, picker, pending-relock, next-stop, child-ring and held-arrow assertions passed");
}
