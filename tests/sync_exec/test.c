// Host tests for source/sync/sync_exec.c: remote orders carried out on the
// console through the real service layer (source/core/pctl_ops.c) and a
// stateful fake pctl service (tests/pctl_session/switch.h): the unlock ->
// write -> lock sequence when the timer counts down, every way it can fail,
// the record of an unlock not yet locked again, extra time and its restore,
// the console lock, bedtime, restrictions, profiles, read-only mode and
// remote_timer_writes. Commands 1043 (delete everything) and 1941 (unlink
// the companion app) are never sent.
#include "check.h"
#include <stdio.h>
#include <string.h>

#include "pctl_ops.h"
#include "sync_apply.h"
#include "sync_exec.h"
#include "write_guard.h"

enum { MOCK_ERROR = 0x701 };
static Service service;
static struct {
    unsigned refs;
    bool enabled, restricted, unlocked, unlock_effective;
    u32 safety_level;
    u8 age, sns, comm;
    bool vr;
    bool alarm_disabled;
    s64 remaining_ns;            // 1454: today's time left
    u16 block[34];
    int bed_day;                 // the day 1954/1956/1957 report
    u32 fail_command;            // that command returns MOCK_ERROR
    u32 cmds[64];                // writes, in order
    unsigned n_cmds;
} model;

static void log_cmd(u32 c)
{
    REQUIRE(model.n_cmds < 64);
    model.cmds[model.n_cmds++] = c;
}

// The limits Sun..Sat as the console shows them (0xFFFF none).
static void set_days(const u16 days[7])
{
    memset(model.block, 0, sizeof(model.block));
    pt_encode(model.block, days);
}

static void reset(void)
{
    CHECK(model.refs == 0);
    memset(&model, 0, sizeof(model));
    model.unlock_effective = true;
    model.safety_level = PctlSafetyLevel_Child;
    model.age = 12;
    model.sns = 1;
    model.comm = 1;
    const u16 days[7] = { 180, 120, 120, 120, 120, 120, 180 };
    set_days(days);
    model.enabled = true;
    model.bed_day = 1;
    model.remaining_ns = 3600000000000LL;   // an hour left
    core_set_read_only(false);
    core_set_change_check(NULL);
}

bool hosversionAtLeast(u8 major, u8 minor, u8 micro)
{
    return MAKEHOSVERSION(23, 0, 1) >= MAKEHOSVERSION(major, minor, micro);
}

Result pctlInitialize(void)
{
    CHECK(model.refs == 0);
    model.refs++;
    return 0;
}

void pctlExit(void)
{
    CHECK(model.refs == 1);
    model.refs--;
}

Service *pctlGetServiceSession_Service(void)
{
    CHECK(model.refs == 1);
    return &service;
}

Result pctlauthRegisterPasscode(void) { CHECK(!"no PIN applet from a remote order"); return MOCK_ERROR; }
/* pctl_pin_hmac's (the recovery report's proof): no remote order reads the PIN. */
void hmacSha256CalculateMac(void *dst, const void *key, size_t key_size, const void *src, size_t size)
{
    (void)dst; (void)key; (void)key_size; (void)src; (void)size;
    CHECK(!"no PIN read from a remote order");
}
Result pctlauthShowForConfiguration(void) { CHECK(!"no PIN applet from a remote order"); return MOCK_ERROR; }

static Result put(void *out, size_t out_size, const void *value, size_t size)
{
    REQUIRE(out_size == size);
    memcpy(out, value, size);
    return 0;
}

Result mock_dispatch(Service *srv, u32 command, void *out, size_t out_size, const void *in, size_t in_size,
                     SfDispatchParams params)
{
    CHECK(srv == &service && model.refs == 1);
    CHECK(command != 1043 && command != 1941);   // never from a remote order
    CHECK(command != 1456 && command != 1951);
    const bool is_write = command == 195101 || command == 1033 || command == 1036 || command == 1063 ||
                          command == 1953 || command == 1201 || command == 1007 || command == 1451 || command == 1452;
    if (is_write) log_cmd(command);
    if (command == model.fail_command) return MOCK_ERROR;
    switch (command) {
    case 195101:
        REQUIRE(in_size == 0x44);
        memcpy(model.block, in, in_size);
        model.enabled = model.block[0] != 0;
        return 0;
    case 1033: CHECK(in_size == 4); model.safety_level = *(const u32 *)in; return 0;
    case 1036: {
        REQUIRE(in_size == 3);
        const u8 *b = (const u8 *)in;
        model.age = b[0];
        model.sns = b[1];
        model.comm = b[2];
        return 0;
    }
    case 1063: CHECK(in_size == 1); model.vr = *(const u8 *)in; return 0;
    case 1953: CHECK(in_size == 1); model.alarm_disabled = *(const u8 *)in; return 0;
    case 1201: model.unlocked = model.unlock_effective; return 0;
    case 1007: model.unlocked = false; return 0;
    default: break;
    }
    if (out == NULL) return 0;
    memset(out, 0, out_size);
    u8 b = 0;
    u32 w = 0;
    u64 q = 0;
    PtBedtime bt[7];
    switch (command) {
    case 1453: b = model.enabled && !model.unlocked; return put(out, out_size, &b, 1);
    case 1455: b = model.restricted; return put(out, out_size, &b, 1);
    case 1006: b = model.unlocked; return put(out, out_size, &b, 1);
    case 1458: b = model.alarm_disabled; return put(out, out_size, &b, 1);
    case 1954: case 1956: case 1957:
        pt_bedtime_decode(model.block, bt);
        b = command == 1954 ? bt[model.bed_day].on : command == 1956 ? bt[model.bed_day].hour : bt[model.bed_day].minute;
        return put(out, out_size, &b, 1);
    case 1958: b = 6; return put(out, out_size, &b, 1);
    case 1959: b = 0; return put(out, out_size, &b, 1);
    case 1031: case 1403: b = 1; return put(out, out_size, &b, 1);
    case 1062: b = model.vr; return put(out, out_size, &b, 1);
    case 1032: w = model.safety_level; return put(out, out_size, &w, 4);
    case 1206: w = 6; return put(out, out_size, &w, 4);
    case 1208:
        memcpy((void *)params.buffers[0].ptr, "123456", 7);
        w = 6;
        return put(out, out_size, &w, 4);
    case 1037: w = 6; return put(out, out_size, &w, 4);
    case 1039: w = 0; return put(out, out_size, &w, 4);
    case 1406: q = 0; return put(out, out_size, &q, 8);
    case 1454: return put(out, out_size, &model.remaining_ns, 8);
    case 1035: { u8 raw[3] = { model.age, model.sns, model.comm }; return put(out, out_size, raw, 3); }
    case 145601: return put(out, out_size, model.block, sizeof(model.block));
    default: CHECK(!"unexpected command"); return 0;
    }
    (void)in;
}

// ---------------------------------------------------------------- helpers

static SyncRecords rec;
static const char *today = "2026-10-05";   // a Monday

static SyncExecCtx ctx_with(bool timer_writes)
{
    SyncExecCtx c = { timer_writes, 1, today, &rec, NULL, NULL, NULL };
    return c;
}

static SyncIntent order(const char *entity, const char *payload)
{
    SyncIntent in;
    CHECK(sync_apply_parse(entity, payload, strlen(payload), &in) == SyncReason_None);
    return in;
}

static void run(const char *entity, const char *payload, SyncOutcome *out)
{
    SyncExecCtx c = ctx_with(true);
    SyncIntent in = order(entity, payload);
    sync_exec(&in, &c, out);
    CHECK(model.refs == 0);   // every session released
}

static void days_now(u16 days[7])
{
    pt_decode(model.block, days);
}

static bool cmds_are(const u32 *expected, unsigned n)
{
    if (model.n_cmds != n) return false;
    for (unsigned i = 0; i < n; i++)
        if (model.cmds[i] != expected[i]) return false;
    return true;
}

// ---------------------------------------------------------------- tests

static void test_limit_with_unlock(void)
{
    reset();
    sync_records_clear(&rec);
    SyncOutcome out;
    run("limit_mon", "90", &out);
    CHECK(out.applied && out.changed && out.did_unlock && !out.relock_failed);
    const u32 expected[] = { 1201, 195101, 1007 };
    CHECK(cmds_are(expected, 3));
    u16 d[7];
    days_now(d);
    CHECK(d[1] == 90 && d[0] == 180 && d[2] == 120);
    CHECK(!model.unlocked && !rec.relock_pending);
    CHECK(out.change == SyncChange_Limits && out.n == 7 && out.before[1] == 120 && out.after[1] == 90);

    // The same value again: nothing written.
    model.n_cmds = 0;
    run("limit_mon", "90", &out);
    CHECK(out.applied && !out.changed && model.n_cmds == 0);

    // The timer off: no unlock needed.
    reset();
    const u16 none[7] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
    set_days(none);
    model.enabled = false;
    run("limit_uniform", "60", &out);
    CHECK(out.applied && !out.did_unlock);
    const u32 plain[] = { 195101 };
    CHECK(cmds_are(plain, 1));
    days_now(d);
    for (int i = 0; i < 7; i++) CHECK(d[i] == 60);

    // A week, and "no limit" as 1440.
    reset();
    run("limits_week", "60,1440,90,90,90,90,240", &out);
    CHECK(out.applied);
    days_now(d);
    CHECK(d[0] == 60 && d[1] == 0xFFFF && d[6] == 240);
    // Today's limit (Monday).
    reset();
    run("max_screentime_today", "45", &out);
    days_now(d);
    CHECK(out.applied && d[1] == 45 && d[2] == 120);
    // Remove the limit.
    reset();
    run("remove_limit", "PRESS", &out);
    days_now(d);
    CHECK(out.applied && d[0] == 0xFFFF && d[6] == 0xFFFF && !model.enabled);
}

static void test_refusals(void)
{
    SyncOutcome out;
    // remote_timer_writes off: nothing is sent at all.
    reset();
    sync_records_clear(&rec);
    SyncExecCtx off = ctx_with(false);
    SyncIntent in = order("limit_mon", "90");
    sync_exec(&in, &off, &out);
    CHECK(!out.applied && out.reason == SyncReason_TimerWritesDisabled && model.n_cmds == 0);
    // ... but restrictions and locking need no unlock.
    in = order("restriction_level", "teen");
    sync_exec(&in, &off, &out);
    CHECK(out.applied && model.safety_level == PctlSafetyLevel_Teen);
    in = order("lock_now", "PRESS");
    sync_exec(&in, &off, &out);
    CHECK(out.applied);

    // Read-only mode: refused before any session.
    reset();
    core_set_read_only(true);
    run("limit_mon", "90", &out);
    CHECK(!out.applied && out.reason == SyncReason_ReadOnly && model.n_cmds == 0);
    run("vr_restricted", "ON", &out);
    CHECK(!out.applied && out.reason == SyncReason_ReadOnly && model.n_cmds == 0);
    core_set_read_only(false);

    // Not an order for the console.
    in = order("sync_now", "PRESS");
    SyncExecCtx c = ctx_with(true);
    sync_exec(&in, &c, &out);
    CHECK(!out.applied && out.reason == SyncReason_UnknownEntity);
}

static void test_unlock_failures(void)
{
    SyncOutcome out;
    // 1201 succeeds but the console does not report the unlock: nothing
    // written, the record dropped once it reads back as locked.
    reset();
    sync_records_clear(&rec);
    model.unlock_effective = false;
    run("limit_mon", "90", &out);
    CHECK(!out.applied && out.reason == SyncReason_UnlockFailed && !rec.relock_pending);
    for (unsigned i = 0; i < model.n_cmds; i++) CHECK(model.cmds[i] != 195101);

    // The write fails: locked again anyway.
    reset();
    sync_records_clear(&rec);
    model.fail_command = 195101;
    run("limit_mon", "90", &out);
    CHECK(!out.applied && out.reason == SyncReason_PctlError && out.rc == MOCK_ERROR);
    const u32 relocked[] = { 1201, 195101, 1007 };
    CHECK(cmds_are(relocked, 3) && !model.unlocked && !rec.relock_pending);

    // Locking again fails: the record stays, so the next start locks.
    reset();
    sync_records_clear(&rec);
    model.fail_command = 1007;
    run("limit_mon", "90", &out);
    CHECK(out.applied && out.relock_failed && rec.relock_pending && model.unlocked);
    // A later "lock now" clears it.
    model.fail_command = 0;
    run("lock_now", "PRESS", &out);
    CHECK(out.applied && !rec.relock_pending && !model.unlocked);
}

static void test_extra_time(void)
{
    SyncOutcome out;
    reset();
    sync_records_clear(&rec);
    run("add_bonus_time_30", "PRESS", &out);
    u16 d[7];
    days_now(d);
    CHECK(out.applied && d[1] == 150 && !strcmp(out.source, "remote_extra"));
    CHECK(rec.extra_weekday == 1 && !strcmp(rec.extra_date, today) && rec.extra_base == 120 && rec.extra_value == 150);
    // Again the same day: the base stays the limit from before any extra time.
    run("add_bonus_time", "15", &out);
    days_now(d);
    CHECK(out.applied && d[1] == 165 && rec.extra_base == 120 && rec.extra_value == 165);
    // Capped at 24 h.
    for (int i = 0; i < 8; i++) run("add_bonus_time", "180", &out);   // 165 + 8 * 180 > 24 h
    days_now(d);
    CHECK(d[1] == 1440 && rec.extra_base == 120);
    run("add_bonus_time", "15", &out);
    CHECK(!out.applied && out.reason == SyncReason_NoLimitToday);

    // No limit today: nothing to extend.
    reset();
    sync_records_clear(&rec);
    const u16 days[7] = { 120, 0xFFFF, 120, 120, 120, 120, 120 };
    set_days(days);
    run("add_bonus_time_15", "PRESS", &out);
    CHECK(!out.applied && out.reason == SyncReason_NoLimitToday && model.n_cmds == 0);

    // No more play today, then put back the next day.
    reset();
    sync_records_clear(&rec);
    run("stop_today", "PRESS", &out);
    days_now(d);
    CHECK(out.applied && d[1] == 0 && rec.extra_base == 120 && rec.extra_value == 0);
    SyncExecCtx next = ctx_with(false);   // putting back needs no remote_timer_writes
    next.today = "2026-10-06";
    next.weekday = 2;
    // It raises the limit again: not by itself, PlayGuard asks (main's #182).
    model.n_cmds = 0;
    CHECK(!sync_exec_restore_extra(&next, &out));
    days_now(d);
    CHECK(d[1] == 0 && rec.extra_weekday == 1 && model.n_cmds == 0);

    // Extra time, put back the next day by itself: the limit comes down.
    reset();
    sync_records_clear(&rec);
    run("add_bonus_time_15", "PRESS", &out);
    days_now(d);
    CHECK(out.applied && d[1] == 135 && rec.extra_base == 120);
    CHECK(sync_exec_restore_extra(&next, &out));
    days_now(d);
    CHECK(out.applied && d[1] == 120 && rec.extra_weekday == -1);
    CHECK(!sync_exec_restore_extra(&next, &out));   // nothing left

    // A week later, the same weekday: not while it would stop the game (130
    // minutes played of 135), then once it would not (75 of 135).
    run("add_bonus_time_15", "PRESS", &out);
    SyncExecCtx week = ctx_with(false);
    week.today = "2026-10-12";
    week.weekday = 1;
    model.remaining_ns = 5 * 60000000000LL;
    model.n_cmds = 0;
    CHECK(!sync_exec_restore_extra(&week, &out));
    CHECK(rec.extra_weekday == 1 && model.n_cmds == 0);
    model.remaining_ns = 60 * 60000000000LL;
    CHECK(sync_exec_restore_extra(&week, &out));
    days_now(d);
    CHECK(out.applied && d[1] == 120 && rec.extra_weekday == -1);

    // A "no limit" base: never by itself.
    reset();
    sync_records_clear(&rec);
    rec.extra_weekday = 1;
    rec.extra_base = 0xFFFF;
    rec.extra_value = 135;
    strcpy(rec.extra_date, "2026-10-05");
    const u16 raised[7] = { 180, 135, 120, 120, 120, 120, 180 };
    set_days(raised);
    CHECK(!sync_exec_restore_extra(&next, &out));
    CHECK(rec.extra_weekday == 1 && model.n_cmds == 0);

    // The same day: nothing yet.
    reset();
    sync_records_clear(&rec);
    run("add_bonus_time_15", "PRESS", &out);
    SyncExecCtx same = ctx_with(true);
    CHECK(!sync_exec_restore_extra(&same, &out));
    // Changed since: the record goes, nothing is written.
    const u16 changed[7] = { 180, 200, 120, 120, 120, 120, 180 };
    set_days(changed);
    model.n_cmds = 0;
    CHECK(!sync_exec_restore_extra(&next, &out));
    CHECK(rec.extra_weekday == -1 && out.records_changed && model.n_cmds == 0);
}

static void test_console_lock(void)
{
    SyncOutcome out;
    reset();
    sync_records_clear(&rec);
    run("console_lock", "ON", &out);
    u16 d[7];
    days_now(d);
    CHECK(out.applied && out.console_lock_after == 1 && rec.console_lock && rec.console_lock_prev_ok);
    for (int i = 0; i < 7; i++) CHECK(d[i] == 0);
    CHECK(rec.console_lock_prev[0] == 180 && rec.console_lock_prev[1] == 120);
    // Extra time and no more play are refused while locked.
    run("add_bonus_time_30", "PRESS", &out);
    CHECK(!out.applied && out.reason == SyncReason_ConsoleLocked);
    run("stop_today", "PRESS", &out);
    CHECK(!out.applied && out.reason == SyncReason_ConsoleLocked);
    // Again: nothing.
    model.n_cmds = 0;
    run("console_lock", "ON", &out);
    CHECK(out.applied && !out.changed && model.n_cmds == 0);
    // Off: the limits come back.
    run("console_lock", "OFF", &out);
    days_now(d);
    CHECK(out.applied && out.console_lock_after == 0 && !rec.console_lock && d[0] == 180 && d[1] == 120);

    // A limit order while locked replaces the lock.
    run("console_lock", "ON", &out);
    run("limit_mon", "60", &out);
    CHECK(out.applied && out.console_lock_after == 0 && !rec.console_lock && !rec.console_lock_prev_ok);

    // Off with nothing saved: the limit is removed.
    reset();
    sync_records_clear(&rec);
    rec.console_lock = true;
    run("console_lock", "OFF", &out);
    days_now(d);
    CHECK(out.applied && d[0] == 0xFFFF && d[3] == 0xFFFF);
}

static void test_restrictions(void)
{
    SyncOutcome out;
    reset();
    sync_records_clear(&rec);
    run("restriction_level", "young_child", &out);
    CHECK(out.applied && model.safety_level == PctlSafetyLevel_YoungChild && out.before[0] == 3 && out.after[0] == 2);
    model.n_cmds = 0;
    run("restriction_level", "young_child", &out);
    CHECK(out.applied && !out.changed && model.n_cmds == 0);
    run("vr_restricted", "ON", &out);
    CHECK(out.applied && model.vr);
    // Posting / communication: only at the custom level.
    run("sns_post_restricted", "OFF", &out);
    CHECK(!out.applied && out.reason == SyncReason_NotCustom);
    run("restriction_level", "custom", &out);
    run("sns_post_restricted", "OFF", &out);
    CHECK(out.applied && model.sns == 0 && model.comm == 1 && model.age == 12);
    run("free_communication_restricted", "OFF", &out);
    CHECK(out.applied && model.comm == 0 && out.change == SyncChange_Custom && out.n == 3);
    // A failed write is reported with its result.
    model.fail_command = 1033;
    run("restriction_level", "teen", &out);
    CHECK(!out.applied && out.reason == SyncReason_PctlError && out.rc == MOCK_ERROR);
}

static void test_alarm_and_bedtime(void)
{
    SyncOutcome out;
    reset();
    sync_records_clear(&rec);
    run("play_timer_alarm", "OFF", &out);
    CHECK(out.applied && out.did_unlock && model.alarm_disabled && out.after[0] == 1);
    run("play_timer_alarm", "ON", &out);
    CHECK(out.applied && !model.alarm_disabled);

    // Bedtime at 21:00 every day; the console reports it.
    reset();
    sync_records_clear(&rec);
    run("bedtime_alarm", "21:00:00", &out);
    CHECK(out.applied && out.changed && out.did_unlock);
    PtBedtime bt[7];
    pt_bedtime_decode(model.block, bt);
    for (int i = 0; i < 7; i++) CHECK(bt[i].on && bt[i].hour == 21 && bt[i].minute == 0 && bt[i].end_hour == 6);
    u16 d[7];
    days_now(d);
    CHECK(d[0] == 180 && d[1] == 120);   // the limits stay
    // The end of the day.
    run("bedtime_end_time", "07:30", &out);
    pt_bedtime_decode(model.block, bt);
    CHECK(out.applied && bt[3].end_hour == 7 && bt[3].end_minute == 30 && bt[3].hour == 21);
    // Off, then the end time alone is refused.
    run("bedtime_enabled", "OFF", &out);
    pt_bedtime_decode(model.block, bt);
    CHECK(out.applied && !bt[0].on && !bt[6].on);
    run("bedtime_end_time", "07:00", &out);
    CHECK(!out.applied && out.reason == SyncReason_BedtimeOff);
    // On again: the default 21:00.
    run("bedtime_enabled", "ON", &out);
    pt_bedtime_decode(model.block, bt);
    CHECK(out.applied && bt[2].on && bt[2].hour == 21);

    // The block does not hold what the console reports (its place in the
    // block is inferred): refused. Sunday and Monday say 20:00 in the block,
    // the console answers 22:00 for Monday.
    reset();
    sync_records_clear(&rec);
    PtBedtime other[7];
    pt_bedtime_decode(model.block, other);
    for (int i = 0; i < 7; i++) {
        other[i].on = true;
        other[i].hour = i <= 1 ? 20 : 22;
        other[i].minute = 0;
        other[i].end_hour = 6;
    }
    pt_bedtime_encode(model.block, other);
    model.bed_day = 3;   // what 1954/1956/1957 report: Wednesday's 22:00
    run("bedtime_alarm", "21:00", &out);
    CHECK(!out.applied && out.reason == SyncReason_Unsupported);
}

static bool profile_days(void *ctx, const char *name, uint16_t days[7])
{
    (void)ctx;
    if (strcmp(name, "Holidays")) return false;
    for (int i = 0; i < 7; i++) days[i] = 240;
    return true;
}

static void test_profiles_and_unlock(void)
{
    SyncOutcome out;
    reset();
    sync_records_clear(&rec);
    SyncExecCtx c = ctx_with(true);
    c.profile_days = profile_days;
    SyncIntent in = order("profile", "Holidays");
    sync_exec(&in, &c, &out);
    u16 d[7];
    days_now(d);
    CHECK(out.applied && d[0] == 240 && d[6] == 240 && !strcmp(out.source, "remote_profile"));
    in = order("profile", "Nope");
    sync_exec(&in, &c, &out);
    CHECK(!out.applied && out.reason == SyncReason_NoSuchProfile);

    // Unlock on demand stays unlocked (no pending relock); lock_now locks.
    reset();
    sync_records_clear(&rec);
    run("unlocked", "ON", &out);
    CHECK(out.applied && model.unlocked && !rec.relock_pending && out.change == SyncChange_Unlock);
    run("unlocked", "OFF", &out);
    CHECK(out.applied && !model.unlocked && out.change == SyncChange_Relock);
}

// The host's save_records: what the records become, on the SD card before
// the console changes (logged as command 1 among the writes).
static struct {
    int calls, fail_at;
    SyncRecords last;
} saved;

static bool save_hook(void *ctx, const SyncRecords *r)
{
    (void)ctx;
    saved.calls++;
    log_cmd(1);
    saved.last = *r;
    return saved.calls != saved.fail_at;
}

static void run_saving(const char *entity, const char *payload, SyncOutcome *out)
{
    SyncExecCtx c = ctx_with(true);
    c.save_records = save_hook;
    SyncIntent in = order(entity, payload);
    sync_exec(&in, &c, out);
    CHECK(model.refs == 0);
}

static void start_saving(void)
{
    reset();
    sync_records_clear(&rec);
    memset(&saved, 0, sizeof(saved));
}

static void test_records_saved_first(void)
{
    SyncOutcome out;
    u16 d[7];
    // Extra time: the time to put back and the unlock's record are on the
    // card before the unlock and the write.
    start_saving();
    run_saving("add_bonus_time_30", "PRESS", &out);
    CHECK(out.applied && out.did_unlock && saved.calls == 1);
    const u32 first[] = { 1, 1201, 195101, 1007 };
    CHECK(cmds_are(first, 4));
    CHECK(saved.last.relock_pending && saved.last.extra_weekday == 1 && saved.last.extra_base == 120 &&
           saved.last.extra_value == 150 && !strcmp(saved.last.extra_date, today));
    CHECK(!rec.relock_pending && rec.extra_value == 150);

    // Not saved: refused, nothing sent to the console, the records as they were.
    start_saving();
    saved.fail_at = 1;
    run_saving("add_bonus_time_30", "PRESS", &out);
    CHECK(!out.applied && out.rc == NXM_RC_NOT_SAVED);
    const u32 none[] = { 1 };
    CHECK(cmds_are(none, 1));
    CHECK(rec.extra_weekday == -1 && !rec.relock_pending);
    days_now(d);
    CHECK(d[1] == 120);

    // The write fails: locked again, and the previous records saved again.
    start_saving();
    model.fail_command = 195101;
    run_saving("stop_today", "PRESS", &out);
    CHECK(!out.applied && saved.calls == 2);
    const u32 put_back[] = { 1, 1201, 195101, 1007, 1 };
    CHECK(cmds_are(put_back, 5));
    CHECK(rec.extra_weekday == -1 && !rec.relock_pending && saved.last.extra_weekday == -1 && !saved.last.relock_pending);

    // The console lock: the limits it replaces are saved before the zeros.
    start_saving();
    run_saving("console_lock", "ON", &out);
    CHECK(out.applied && saved.calls == 1 && saved.last.console_lock && saved.last.console_lock_prev_ok);
    CHECK(saved.last.console_lock_prev[0] == 180 && saved.last.console_lock_prev[1] == 120);
    CHECK(rec.console_lock && !rec.relock_pending);
    // A limit order replaces it: the lock's record goes before the write.
    saved.calls = 0;
    model.n_cmds = 0;
    run_saving("limit_mon", "90", &out);
    CHECK(out.applied && out.console_lock_after == 0 && saved.calls == 1 && !saved.last.console_lock);
    CHECK(!rec.console_lock && !rec.console_lock_prev_ok);

    // Nothing to put back (a restriction, the timer off): saved only for an unlock.
    start_saving();
    model.enabled = false;
    run_saving("vr_restricted", "OFF", &out);
    CHECK(saved.calls == 0);
}

int main(void)
{
    test_limit_with_unlock();
    test_refusals();
    test_unlock_failures();
    test_extra_time();
    test_console_lock();
    test_restrictions();
    test_alarm_and_bedtime();
    test_profiles_and_unlock();
    test_records_saved_first();
    return CHECK_DONE("sync_exec: all tests passed");
}
