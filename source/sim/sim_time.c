// Simulated core/time_ops.h (sim_state.h): the console clock, the network
// clock and the time zone, the host's standing in for the console's (TZ=
// changes it).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../core/sim_env.h"
#include "../core/time_ops.h"
#include "../core/write_guard.h"
#include "sim_state.h"

void time_clock_snapshot(TimeSnapshot *o)
{
    const SimConsole c = sim_state().console;
    memset(o, 0, sizeof(*o));
    const u64 now = sim_base_now();
    o->automatic = !c.autosync_off;
    o->network_time = now + c.clock_offset + c.network_offset;
    // With "Synchronise" on, the console clock follows the network clock.
    o->user_time = o->automatic ? o->network_time : now + c.clock_offset;
    o->local_time = now;
    o->accuracy = c.network_set || sim_env_flag("PLAYGUARD_SIM_ACCURATE");
    snprintf(o->location, sizeof(o->location), "Europe/Paris");
}

Result time_network_accuracy(bool *accurate)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    *accurate = s.accuracy;
    return s.accuracy_rc;
}

// Both writes, once allowed: the network clock becomes `utc`, the console
// clock with it while "Synchronise" is on.
static void set_network(u64 utc, TimeApply *o)
{
    o->target = utc;
    if (sim_fails("clock")) {
        o->write_attempted = true;
        o->write_rc = SIM_FAIL_RC;
        return;
    }
    const u64 now = sim_base_now();
    const s64 network = (s64)utc - (s64)now;
    SimState *s = sim_begin();
    SimConsole *c = &s->console;
    const u64 user_before = now + c->clock_offset + (c->autosync_off ? 0 : c->network_offset);
    if (!c->autosync_off) c->clock_offset = network;
    c->network_offset = network - c->clock_offset;
    c->network_set = true;
    // The user clock moved: the time spent starts over, and the clock set
    // back counts nothing until it passes the time it was set back from
    // (docs/parental-controls.md, Time spent and time left). The console
    // that lives on only (PLAYGUARD_SIM_KEEP): without it, the made-up log
    // says the time spent.
    const u64 user_after = now + c->clock_offset + (c->autosync_off ? 0 : c->network_offset);
    if (sim_keeping() && user_after != user_before) {
        s->spent.spent_s = 0;
        s->spent.hold_until = user_after < user_before ? user_before : 0;
    }
    sim_end();
    o->write_attempted = o->verify_attempted = o->verified = true;
    o->readback = utc;
}
void time_clock_apply(u64 utc, TimeApply *o)
{
    memset(o, 0, sizeof(*o));
    time_clock_snapshot(&o->before);
    o->target = utc;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) {
        o->open_rc = gate;
    } else if (!o->before.automatic) {
        o->refused_automatic = true;   // as time_ops.c: the user clock would not follow
    } else {
        set_network(utc, o);
    }
    time_clock_snapshot(&o->after);
}
void time_clock_set_to_user(TimeApply *o)
{
    memset(o, 0, sizeof(*o));
    time_clock_snapshot(&o->before);
    o->target = o->before.user_time;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) o->open_rc = gate;
    else set_network(o->before.user_time, o);
    time_clock_snapshot(&o->after);
}
void time_clock_dump(char *buf, size_t n) { snprintf(buf, n, "=== System clocks (simulated) ===\n"); }

// The steady clock: the console's time before any clock change, so the user
// clock moves against it only when the simulated network clock is set.
Result time_steady_now(u64 *seconds, u8 source_id[16])
{
    if (seconds) *seconds = sim_base_now();
    if (source_id) memset(source_id, 0x53, 16);
    return 0;
}

static bool host_to_local(void *ctx, u64 posix, LocalTime *out)
{
    (void)ctx;
    time_t t = (time_t)posix;
    struct tm tmv;
    if (!localtime_r(&t, &tmv)) return false;
    out->year = (u16)(tmv.tm_year + 1900); out->month = (u8)(tmv.tm_mon + 1); out->day = (u8)tmv.tm_mday;
    out->hour = (u8)tmv.tm_hour; out->minute = (u8)tmv.tm_min; out->second = (u8)tmv.tm_sec;
    out->wday = (u8)tmv.tm_wday;
    return true;
}
static int host_to_posix(void *ctx, const LocalTime *w, u64 out[2])
{
    (void)ctx;
    int n = 0;
    for (int dst = 0; dst <= 1; dst++) {   // both readings of a repeated hour
        struct tm tmv;
        memset(&tmv, 0, sizeof(tmv));
        tmv.tm_year = w->year - 1900; tmv.tm_mon = w->month - 1; tmv.tm_mday = w->day;
        tmv.tm_hour = w->hour; tmv.tm_min = w->minute; tmv.tm_sec = w->second;
        tmv.tm_isdst = dst;
        const time_t t = mktime(&tmv);
        LocalTime back;
        if (t == (time_t)-1 || !host_to_local(NULL, (u64)t, &back)) continue;
        if (back.day != w->day || back.hour != w->hour || back.minute != w->minute) continue;   // in a gap
        if (n == 0 || out[0] != (u64)t) out[n++] = (u64)t;
    }
    if (n == 2 && out[1] < out[0]) { u64 x = out[0]; out[0] = out[1]; out[1] = x; }
    return n;
}
const TimeRule *time_console_rule(void)
{
    static const TimeRule rule = { host_to_local, host_to_posix, NULL };
    return &rule;
}
bool time_local_now(u64 *posix, LocalTime *local)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    const u64 now = s.user_time;   // the user clock, as the snapshot says
    if (posix) *posix = now;
    return local ? host_to_local(NULL, now, local) : true;
}
void time_format_local(u64 posix, char *buf, size_t size)
{
    time_t t = (time_t)posix; struct tm tmv;
    if (!localtime_r(&t, &tmv) || !strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tmv)) time_format_utc(posix, buf, size);
}
