// Simulated core/pctl_ops.h and core/sysinfo.h (sim_state.h): the console's
// parental controls and play timer, kept as the real 0x44 block (core/pure.c
// encodes and decodes it, as on the console); the read-only switch is
// core/write_guard.c, checked before the state's lock is taken.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../core/pctl_ops.h"
#include "../core/sim_env.h"
#include "../core/sysinfo.h"
#include "../core/time_ops.h"
#include "../core/write_guard.h"
#include "sim_games.h"
#include "sim_state.h"

// The PIN of the simulated console: an easy one.
#define SIM_PIN "1234"

// ---------------------------------------------------------------- sysinfo
void sysinfo_get(SysInfo *out)
{
    const SimConsole c = sim_state().console;
    memset(out, 0, sizeof(*out));
    out->hos_version = c.hos;
    out->is_atmosphere = true;
    out->ams_valid = true;
    out->ams_major = 1; out->ams_minor = 12; out->ams_micro = 0;
    out->emummc_valid = true;
    out->emummc = sim_env_flag("PLAYGUARD_SIM_EMUMMC");
    out->blank_valid = true;
    out->blank = sim_env_flag("PLAYGUARD_SIM_BLANK");
    out->applet_mode = sim_env_flag("PLAYGUARD_SIM_APPLET");
    out->serial_valid = true;
    snprintf(out->serial, sizeof(out->serial), "%s", out->blank ? SYSINFO_BLANK_SERIAL : "XAW10000000001");
}
void sysinfo_version_string(u32 v, char *buf, size_t size)
{
    if (!v) { snprintf(buf, size, "?"); return; }
    snprintf(buf, size, "%u.%u.%u", (unsigned)HOSVER_MAJOR(v), (unsigned)HOSVER_MINOR(v), (unsigned)HOSVER_MICRO(v));
}

// ---------------------------------------------------------------- the changes
// Each change: the read-only switch and the change check first, without the
// lock (the check may wait for the PIN screen); then, under the lock, the
// console's own refusals and the change itself.

// Read-only, then the change check (write_guard.h), as pctl_ops.c does.
#define RO_GUARD() do { Result g_ = core_change_allowed(); if (R_FAILED(g_)) return g_; } while (0)

// The same gate as pctl_ops.c: no play-timer write while it counts down.
static bool timer_enabled(const SimConsole *c)
{
    return !c->temp_unlocked && c->block[0] != 0;   // the header is non-zero while any day has a limit (pure.h)
}
static bool pt_gated(const SimConsole *c)
{
    return (timer_enabled(c) || c->limit_reached) && !c->temp_unlocked;
}

// Refuses like the console when PLAYGUARD_SIM_FAIL lists `what`.
static Result sim_refuses(const char *what)
{
    return sim_fails(what) ? SIM_FAIL_RC : 0;
}

// ---------------------------------------------------------------- pctl
Result pctl_ops_init(void)   { sim_init(); return sim_env_flag("PLAYGUARD_SIM_NO_CFW") ? (Result)0x0C15 : 0; }
void   pctl_ops_exit(void)   {}
Result pctl_ops_reinit(void) { return pctl_ops_init(); }

void pctl_status_fetch(PctlStatus *o)
{
    memset(o, 0, sizeof(*o));
    if (R_FAILED(o->session_rc = pctl_ops_init())) return;
    const SimConsole c = sim_state().console;
    o->safety_level_ok = true;        o->safety_level = c.safety_level;
    o->pin_length_ok = true;          o->pin_length = c.pin_length;
    o->restriction_enabled_ok = true; o->restriction_enabled = c.restriction_enabled;
    o->temp_unlocked_ok = true;       o->temp_unlocked = c.temp_unlocked;
    o->pairing_active_ok = true;      o->pairing_active = c.pairing_active;
    o->settings_ok = true;            o->settings = c.custom;
    o->rating_org_ok = true;          o->rating_org = c.rating_org;
    o->stereo_vision_ok = true;       o->stereo_vision_restricted = c.stereo_restricted;
    o->free_comm_count_ok = true;     o->free_comm_count = 3;
    o->last_updated_ok = true;        o->last_updated = sim_base_now() - 3600 * 26;
}

void pctl_overview_fetch(PctlStatus *s, PtState *pt)
{
    pctl_status_fetch(s);
    pctl_play_timer_query(pt);
}

Result pctl_set_pin(void)
{
    RO_GUARD();
    if (sim_fails("pin_entry")) return NXM_RC_CANCELLED;   // as pctl_ops.c says B on the PIN screen
    sim_begin()->console.pin_length = 4;
    sim_end();
    return 0;
}
Result pctl_unlock_restriction_temporarily(void)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = sim_fails("unlock")      ? SIM_FAIL_RC
                      : !c->pin_length         ? 0x1A08E
                      : sim_fails("unverified") ? NXM_RC_UNLOCK_NOT_EFFECTIVE
                                               : 0;
    if (R_SUCCEEDED(rc)) c->temp_unlocked = true;
    sim_end();
    return rc;
}
Result pctl_get_pin(char *out, size_t out_size)
{
    if (out && out_size) memset(out, 0, out_size);
    Result g = core_reveal_allowed();
    if (R_FAILED(g)) return g;
    if (sim_fails("pin")) return SIM_FAIL_RC;
    if (!out || out_size < sizeof(SIM_PIN)) return NXM_RC_INVALID_ARGUMENT;
    if (!sim_state().console.pin_length) return NXM_RC_STATE_UNKNOWN;
    snprintf(out, out_size, "%s", SIM_PIN);
    return 0;
}
// PLAYGUARD_SIM_FAIL=pin fails the read.
Result pctl_pin_weak(bool *weak)
{
    if (!weak) return NXM_RC_INVALID_ARGUMENT;
    *weak = false;
    if (sim_fails("pin")) return SIM_FAIL_RC;
    if (!sim_state().console.pin_length) return NXM_RC_NO_PIN;
    *weak = pin_is_weak(SIM_PIN, strlen(SIM_PIN));
    return 0;
}
// HMAC-SHA256 with util/sha256 (sim_platform.cpp): C++, as the rest of it.
void sim_hmac_sha256(void *dst, const void *key, size_t key_size, const void *msg, size_t size);
// The recovery report's proof, keyed with the simulated PIN as pctl_pin_weak
// reads it (PLAYGUARD_SIM_FAIL=pin fails the read).
Result pctl_pin_hmac(const void *msg, size_t size, u8 *mac)
{
    if (!mac) return NXM_RC_INVALID_ARGUMENT;
    memset(mac, 0, 32);
    if (!msg) return NXM_RC_INVALID_ARGUMENT;
    if (sim_fails("pin")) return SIM_FAIL_RC;
    if (!sim_state().console.pin_length) return NXM_RC_NO_PIN;
    sim_hmac_sha256(mac, SIM_PIN, strlen(SIM_PIN), msg, size);
    return 0;
}
// Locking again never asks for the PIN (write_guard.h): read-only only.
Result pctl_lock_state(u32 *pin_length, bool *unlocked)
{
    Result rc = pctl_ops_init();
    if (R_FAILED(rc)) return rc;
    const SimConsole c = sim_state().console;
    if (pin_length) *pin_length = c.pin_length;
    if (unlocked) *unlocked = c.temp_unlocked;
    return 0;
}
Result pctl_relock(void)
{
    if (core_read_only()) return NXM_RC_READ_ONLY;
    const Result rc = sim_refuses("relock");
    if (R_FAILED(rc)) return rc;
    sim_begin()->console.temp_unlocked = false;
    sim_end();
    return 0;
}
// PLAYGUARD_SIM_FAIL=pin_entry: the PIN screen is cancelled.
Result pctl_ask_pin(void)
{
    if (!sim_state().console.pin_length) return NXM_RC_NO_PIN;
    return sim_fails("pin_entry") ? NXM_RC_CANCELLED : 0;
}
Result pctl_delete_parental_controls(void)
{
    RO_GUARD();
    const Result rc = sim_refuses("write");
    if (R_FAILED(rc)) return rc;
    SimConsole *c = &sim_begin()->console;
    c->pin_length = 0;
    c->restriction_enabled = false;
    c->safety_level = 0;
    c->temp_unlocked = false;
    memset(c->block, 0, sizeof(c->block));
    sim_end();
    return 0;
}
Result pctl_delete_pairing(void)
{
    RO_GUARD();
    const Result rc = sim_refuses("write");
    if (R_FAILED(rc)) return rc;
    sim_begin()->console.pairing_active = false;
    sim_end();
    return 0;
}
Result pctl_set_safety_level(u32 l)
{
    RO_GUARD();
    const Result rc = sim_fails("write") ? SIM_FAIL_RC : l > PctlSafetyLevel_Teen ? NXM_RC_INVALID_ARGUMENT : 0;
    if (R_FAILED(rc)) return rc;
    sim_begin()->console.safety_level = l;
    sim_end();
    return 0;
}
Result pctl_set_custom_settings(const PctlCustomSettings *settings)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = sim_fails("write") ? SIM_FAIL_RC : c->safety_level != PctlSafetyLevel_Custom ? NXM_RC_NOT_CUSTOM : 0;
    if (R_SUCCEEDED(rc)) c->custom = *settings;
    sim_end();
    return rc;
}
Result pctl_set_stereo_vision_restricted(bool r)
{
    RO_GUARD();
    const Result rc = sim_refuses("write");
    if (R_FAILED(rc)) return rc;
    sim_begin()->console.stereo_restricted = r;
    sim_end();
    return 0;
}
Result pctl_set_rating_org(u32 org)
{
    RO_GUARD();
    const Result rc = sim_fails("write") ? SIM_FAIL_RC : org >= RATING_ORG_COUNT ? NXM_RC_INVALID_ARGUMENT : 0;
    if (R_FAILED(rc)) return rc;
    sim_begin()->console.rating_org = org;
    sim_end();
    return 0;
}
Result pctl_get_level_settings(u32 level, PctlCustomSettings *out)
{
    // Made-up presets in the spirit of the console's.
    static const PctlCustomSettings presets[5] = { { 0, false, false }, { 0, false, false },
                                                   { 6, true, true }, { 12, true, true }, { 16, false, false } };
    if (!out || level > PctlSafetyLevel_Teen) return NXM_RC_INVALID_ARGUMENT;
    *out = level == PctlSafetyLevel_Custom ? sim_state().console.custom : presets[level];
    return 0;
}

// The settings beside the block: the gate, then PLAYGUARD_SIM_FAIL=write.
static Result sim_pt_setting_refused(const SimConsole *c)
{
    return pt_gated(c) ? NXM_RC_WRITE_GATED : sim_refuses("write");
}
Result pctl_play_timer_set_alarm_disabled(bool d)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = sim_pt_setting_refused(c);
    if (R_SUCCEEDED(rc)) c->alarm_disabled = d;
    sim_end();
    return rc;
}
Result pctl_play_timer_start(void)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = sim_pt_setting_refused(c);
    if (R_SUCCEEDED(rc)) c->paused = false;
    sim_end();
    return rc;
}
Result pctl_play_timer_stop(void)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = sim_pt_setting_refused(c);
    if (R_SUCCEEDED(rc)) c->paused = true;
    sim_end();
    return rc;
}

// ---------------------------------------------------------------- the play timer

// The time left goes down as on a console that counts PlayGuard's own time,
// but only by 1 to 20 s around the minute it starts on, then back up: the
// screens show the same minutes at every run (visual check) and the timer
// health check (action/timer_health_logic) sees it count.
static u64 counting_ns(void)
{
    if (sim_env_flag("PLAYGUARD_SIM_NOT_COUNTING")) return 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)(ts.tv_sec % 20) * 1000000000ULL;
}

static int sim_weekday(void)
{
    LocalTime today;
    return time_local_now(NULL, &today) ? today.wday : 0;
}

void pctl_play_timer_query(PtState *o)
{
    memset(o, 0, sizeof(*o));
    for (int i = 0; i < 7; i++) o->day_min[i] = PT_DAY_NOLIMIT;
    const SimConsole c = sim_state().console;
    o->fw_supported = c.hos >= PCTL_FW_MIN_PLAYTIMER;
    if (!o->fw_supported) return;
    if (R_FAILED(o->session_rc = pctl_ops_init())) return;
    o->session_valid = true;
    if (sim_fails("timer")) {
        o->config_rc = SIM_FAIL_RC;
    } else if (!pt_plausible(c.block)) {   // as pctl_ops.c (only pt_encode writes it here)
        o->config_rc = NXM_RC_PT_NOT_UNDERSTOOD;
        memcpy(o->block, c.block, sizeof(o->block));
    } else {
        o->valid = true;
        pt_decode(c.block, o->day_min);
        pt_bedtime_decode(c.block, o->bed);
        o->alarm_only = !pt_suspends(c.block);
        memcpy(o->block, c.block, sizeof(o->block));
    }
    o->enabled_valid = true;  o->enabled = timer_enabled(&c);
    o->temporary_unlocked_valid = true; o->temporary_unlocked = c.temp_unlocked;
    // Today's limit against what the made-up log says was played today.
    const int wd = sim_weekday();
    u16 days[7];
    pt_decode(c.block, days);
    const u16 limit = days[wd];
    const s64 played = (s64)sim_played_today_s();
    const bool reached = o->enabled && (c.limit_reached || (limit != PT_DAY_NOLIMIT && (s64)limit * 60 <= played));
    o->remaining_valid = true;
    o->remaining_ns = o->enabled && !reached && limit != PT_DAY_NOLIMIT && !sim_env_flag("PLAYGUARD_SIM_IDLE")
                          ? ((s64)limit * 60 - played) * 1000000000LL + 20000000000LL - (s64)counting_ns() : 0;
    // The last minutes of the day's time, whatever the limit (the child screen's end).
    const long long left_min = sim_env_int("PLAYGUARD_SIM_LEFT_MIN", -1, 0, 24 * 60);
    if (left_min >= 0 && o->remaining_ns > 0) o->remaining_ns = (s64)left_min * 60 * 1000000000LL + 20000000000LL;
    // A warning only: the console does not restrict, and 1454 goes below 0
    // by the time played over the limit (as observed; 2 min when the limit is
    // only said to be reached).
    const bool warn_only = !pt_suspends(c.block);
    if (reached && warn_only) {
        const s64 over = limit != PT_DAY_NOLIMIT && played > (s64)limit * 60 ? played - (s64)limit * 60 : 2 * 60;
        o->remaining_ns = -over * 1000000000LL;
    }
    o->restricted_valid = true; o->restricted = reached && !warn_only;
    o->alarm_disabled_valid = true; o->alarm_disabled = c.alarm_disabled;
    // 1954..1959 answer today's bedtime from the block, as the console is
    // taken to (pure.h).
    PtBedtime bed[7];
    pt_bedtime_decode(c.block, bed);
    o->bedtime_valid = true; o->bedtime_enabled = bed[wd].on;
    o->bedtime_hour = bed[wd].hour; o->bedtime_minute = bed[wd].minute;
    o->bedtime_reset_valid = true;
    o->bedtime_reset_hour = bed[wd].on ? bed[wd].end_hour : 6;
    o->bedtime_reset_minute = bed[wd].on ? bed[wd].end_minute : 0;
}

// The recorder's reading, built on the query above. 1459 as seen on hardware
// (docs/parental-controls.md): 2 at 0x00 with a limit today, the remaining
// time at 0x10; 1952 the made-up time played today.
void pctl_play_timer_sample(PtSample *o)
{
    memset(o, 0, sizeof(*o));
    PtState pt;
    pctl_play_timer_query(&pt);
    if (!pt.fw_supported) {
        o->session_rc = o->unlocked_rc = o->enabled_rc = o->restricted_rc = o->alarm_off_rc =
            o->remaining_rc = o->block_rc = o->bedtime_rc = o->display_rc = o->spent_rc = o->extra_rc =
                NXM_RC_FW_UNSUPPORTED;
        return;
    }
    o->block_rc = pt.config_rc;
    memcpy(o->block, pt.block, sizeof(o->block));
    o->unlocked = pt.temporary_unlocked;
    o->enabled = pt.enabled;
    o->restricted = pt.restricted;
    o->alarm_off = pt.alarm_disabled;
    o->remaining_ns = pt.remaining_ns;
    o->bedtime_on = pt.bedtime_enabled;
    o->bedtime_hour = pt.bedtime_hour;
    o->bedtime_minute = pt.bedtime_minute;
    const int wd = sim_weekday();
    if (pt.valid && pt.enabled && pt.day_min[wd] != PT_DAY_NOLIMIT) {
        o->display[0] = 2;
        memcpy(o->display + 0x10, &o->remaining_ns, sizeof(o->remaining_ns));
    }
    // 1454 + 1952 = today's limit, as observed (docs/parental-controls.md).
    o->spent_ns = pt.remaining_ns ? (u64)((s64)pt.day_min[wd] * 60 * 1000000000LL - pt.remaining_ns)
                                  : sim_played_today_s() * 1000000000ULL;
}

// What a block write (195101) meets once its arguments are checked: the gate,
// no PIN (pctl skips the write), PLAYGUARD_SIM_FAIL=write.
static Result sim_pt_gate(const SimConsole *c)
{
    if (pt_gated(c)) return NXM_RC_WRITE_GATED;
    if (!c->pin_length) return NXM_RC_NO_PIN;
    return sim_refuses("write");
}

Result pctl_play_timer_set_days(const u16 d[7])
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    const Result rc = c->hos < PCTL_FW_MIN_PLAYTIMER ? NXM_RC_FW_UNSUPPORTED
                      : !pt_days_ok(d)               ? NXM_RC_INVALID_ARGUMENT
                                                     : sim_pt_gate(c);
    if (R_SUCCEEDED(rc)) {
        pt_encode(c->block, d);   // read-modify-write, as pctl_ops.c does
        c->limit_reached = false;
    }
    sim_end();
    return rc;
}

static bool bedtime_ok(const PtBedtime bed[7], int today)
{
    if (today < 0 || today > 6) return false;
    for (int i = 0; i < 7; i++)
        if (!pt_bedtime_ok(&bed[i])) return false;
    return true;
}

// PLAYGUARD_SIM_FAIL=bedtime: the console does not take the bedtime (the
// layout guess is wrong), so the block stays as it was (NXM_RC_NOT_APPLIED).
Result pctl_play_timer_set_bedtime(const PtBedtime bed[7], int today)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    Result rc = c->hos < PCTL_FW_MIN_PLAYTIMER ? NXM_RC_FW_UNSUPPORTED
                : !bedtime_ok(bed, today)      ? NXM_RC_INVALID_ARGUMENT
                                               : sim_pt_gate(c);
    if (R_SUCCEEDED(rc)) rc = sim_fails("timer") ? NXM_RC_STATE_UNKNOWN : sim_fails("bedtime") ? NXM_RC_NOT_APPLIED : 0;
    if (R_SUCCEEDED(rc)) pt_bedtime_encode(c->block, bed);
    sim_end();
    return rc;
}

Result pctl_play_timer_set_alarm_only(bool alarm_only)
{
    RO_GUARD();
    SimConsole *c = &sim_begin()->console;
    Result rc = c->hos < PCTL_FW_MIN_PLAYTIMER ? NXM_RC_FW_UNSUPPORTED : sim_pt_gate(c);
    if (R_SUCCEEDED(rc) && sim_fails("timer")) rc = NXM_RC_STATE_UNKNOWN;
    if (R_SUCCEEDED(rc) && !pt_set_suspends(c->block, !alarm_only)) rc = NXM_RC_INVALID_ARGUMENT;
    sim_end();
    return rc;
}

void pctl_dump(char *buf, size_t n)
{
    const SimConsole c = sim_state().console;
    int used = snprintf(buf, n, "=== pctl state (simulated desktop backend) ===\n"
                        "safety_level=%u pin=%s enabled=%d temp_unlocked=%d\n145601 block:",
                        (unsigned)c.safety_level, c.pin_length ? "set" : "none", (int)c.restriction_enabled, (int)c.temp_unlocked);
    for (int i = 0; i < PT_U16_COUNT && used > 0 && (size_t)used < n; i++)
        used += snprintf(buf + used, n - (size_t)used, " %04X", c.block[i]);
    if (used > 0 && (size_t)used < n) snprintf(buf + used, n - (size_t)used, "\n");
}
