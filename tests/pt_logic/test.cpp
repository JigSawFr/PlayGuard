// Host tests for source/action/pt_logic.cpp: when a play-timer write needs the
// temporary unlock, how much was played today, extra time and putting it back,
// what pt_flow's pickers start on, and an unlock left on by an earlier run.
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
// cut short, or Security › Unlock temporarily and PlayGuard closed from the
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

// The Overview's "+30 min" and the alarm notice.
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

int main()
{
    test_gate();
    test_played();
    test_suspend_warning();
    test_extra();
    test_stop();
    test_restore();
    test_bedtime();
    test_pickers();
    test_extra_today();
    test_pending_relock();
    return CHECK_DONE("pt_logic gate, played time, suspend warning, extra time, no-more-play, bedtime, picker and pending-relock assertions passed");
}
