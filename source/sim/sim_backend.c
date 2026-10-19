// Simulated service layer for the desktop (PLATFORM_DESKTOP) build.
//
// Implements pctl_ops.h, time_ops.h, sysinfo.h and playstats.h with in-memory
// state so the whole borealis UI can be built, navigated and screenshotted on a
// PC. Never compiled for the Switch. Environment knobs:
//   PLAYGUARD_SIM_FW=20.5.0     pretend to run on another firmware
//   PLAYGUARD_SIM_NO_CFW=1      make pctl_ops_init fail (init error screen)
//   PLAYGUARD_SIM_TIMER_OFF=1   start with no play timer configured
//   PLAYGUARD_SIM_UNLOCKED=1    start with parental controls temporarily unlocked
//   PLAYGUARD_SIM_UNPAIRED=1    start with no companion app linked
//   PLAYGUARD_SIM_ACCURATE=1    report the network clock as accurate
//   PLAYGUARD_SIM_EMUMMC=1      running on emuMMC (default: sysMMC)
//   PLAYGUARD_SIM_BLANK=1       PRODINFO blanked (serial XAW00000000000)
//   PLAYGUARD_SIM_NO_PDM=1      the play-data service (Activity tab) fails
//   PLAYGUARD_SIM_GAMES=200     that many more games played this week (a large library)
//   PLAYGUARD_SIM_NO_PLAY=1     nothing played for weeks (the all-time totals stay)
//   PLAYGUARD_SIM_NOT_SET_UP=1  parental controls never set up (no PIN, no restriction)
//   PLAYGUARD_SIM_APPLET=1      started from the album (applet mode)
//   PLAYGUARD_SIM_RESTRICTED=1  today's limit is reached (the console says "Time's up")
//   PLAYGUARD_SIM_NOT_COUNTING=1 the time left does not go down (a clock set back):
//                               Today says so after 90 s on screen
//   PLAYGUARD_SIM_AUTOSYNC_OFF=1 "Synchronise clock via Internet" is off
//   PLAYGUARD_SIM_NETWORK_OFFSET=1411  the network clock is that many seconds
//                               ahead of the console clock until it is set
//                               (the console clock follows it while
//                               "Synchronise" is on)
//   PLAYGUARD_SIM_NOW=1791471600  the console's time, frozen (POSIX seconds):
//                               the same screens at every run (visual check)
//   PLAYGUARD_SIM_IDLE=1        no game running: the timer reports no time left
//                               yet (Today falls back on the activity log)
//   PLAYGUARD_SIM_SKIP=900      the console's time that many seconds later, a
//                               game played all along (unless IDLE): the time
//                               left goes down by as much, to the limit and
//                               past it, and the activity log's first game
//                               has it; read at each call, so the web page
//                               moves it while PlayGuard runs
//   PLAYGUARD_SIM_ACCOUNTS=2    that many user accounts (0 to 8), sharing the
//                               play time 2:1 between each and the next
//   PLAYGUARD_SIM_KEEP=1        the simulated console's settings (PIN, limits,
//                               bedtime, clocks, …) and the installed games'
//                               fixes kept in
//                               playguard_data/sim_console.bin from one run to
//                               the next: the variables above that set its
//                               starting state count for the first run only
//   PLAYGUARD_SIM_FAIL=a,b,...  make these fail: unlock (1201), unverified
//                               (1201 fine, 1006 still false), write (every
//                               setting write), relock (1007), timer (145601
//                               read), clock (network clock write), pin (1208),
//                               pin_entry (the PIN screen is cancelled, when
//                               asking for the PIN as when setting one),
//                               bedtime (the console does not take the bedtime),
//                               remove (Console › Check the installed games: a removal),
//                               launch (its launch version reset), verify
//                               (its integrity check)
//   PLAYGUARD_SIM_GAMES_OK=1    every installed game fine (Console › Check the installed games)
//   PLAYGUARD_SIM_CHECK_MS=150  how long checking one installed game takes
// The play-timer limits are kept as the real 0x44 block (core/pure.c encodes
// and decodes it, as on the console); the read-only switch is core/write_guard.c.
// Game patches are read from ./playguard_data/sd/ (the simulated SD card root).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../core/calendar.h"
#include "../core/gamecheck.h"
#include "../core/pctl_ops.h"
#include "../core/playstats.h"
#include "../core/sim_env.h"
#include "../core/sysinfo.h"
#include "../core/pure.h"
#include "../core/time_ops.h"
#include "../core/write_guard.h"
#include "sim_icons.h"

typedef struct {
    bool init;
    u32  hos;
    u32  safety_level;
    u32  pin_length;
    bool restriction_enabled, temp_unlocked, pairing_active, stereo_restricted, alarm_disabled, paused;
    bool limit_reached, autosync_off;
    u32  rating_org;
    PctlCustomSettings custom;
    u16  block[PT_U16_COUNT];   // PlayTimerSettings, as 145601 returns it
    s64  clock_offset;     // the console clock against base_now()
    s64  network_offset;   // the network clock against the console clock, until set
    bool network_set;      // PlayGuard set the network clock
} SimConsole;
static SimConsole S;

// What Console › Check the installed games changed (gamecheck below): kept
// with S by PLAYGUARD_SIM_KEEP, as the console keeps it.
typedef struct {
    u64    removed[32];        // whole games and single contents, by ID
    size_t removed_count;
    u64    launch_reset[16];
    size_t launch_reset_count;
    u64    bumps[32];          // each change to a game bumps its entry
    size_t bump_count;
} SimFixes;
static SimFixes G;

// PLAYGUARD_SIM_KEEP: S and G as the last run left them, in the simulated
// data folder (the web page keeps that folder in the browser). A file from
// another build, whose S or G has another size, is not read.
#define SIM_KEEP_FILE "./playguard_data/sim_console.bin"
static const char SIM_KEEP_MAGIC[8] = "PGSIM02";
static pthread_mutex_t s_keep_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
    SimConsole console;
    SimFixes   fixes;
} SimKept;
static SimKept s_kept;   // what the file holds

static bool sim_keeping(void)
{
    return sim_env("PLAYGUARD_SIM_KEEP") != NULL;
}

static void sim_keep_load(void)
{
    FILE *f = fopen(SIM_KEEP_FILE, "rb");
    if (!f) return;
    char magic[sizeof(SIM_KEEP_MAGIC)];
    u32 size = 0;
    SimKept kept;
    if (fread(magic, 1, sizeof(magic), f) == sizeof(magic) && memcmp(magic, SIM_KEEP_MAGIC, sizeof(magic)) == 0 &&
        fread(&size, sizeof(size), 1, f) == 1 && size == sizeof(kept) && fread(&kept, 1, sizeof(kept), f) == sizeof(kept)) {
        memcpy(&S, &kept.console, sizeof(S));
        memcpy(&G, &kept.fixes, sizeof(G));
        memcpy(&s_kept, &kept, sizeof(kept));
        S.init = true;
    }
    fclose(f);
}

// Writes S and G when they changed since the file was written: called where
// the UI reads the console's state, which it does after each change.
static void sim_keep_save(void)
{
    if (!sim_keeping()) return;
    pthread_mutex_lock(&s_keep_lock);
    SimKept now;
    memset(&now, 0, sizeof(now));   // the padding too: compared byte for byte
    memcpy(&now.console, &S, sizeof(S));
    memcpy(&now.fixes, &G, sizeof(G));
    if (memcmp(&now, &s_kept, sizeof(now)) != 0) {
        // Not world-writable (fopen creates with 0666, less the umask).
        const int fd = open(SIM_KEEP_FILE ".tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        FILE *f = fd >= 0 ? fdopen(fd, "wb") : NULL;
        if (fd >= 0 && !f) close(fd);
        const u32 size = sizeof(now);
        bool ok = f && fwrite(SIM_KEEP_MAGIC, 1, sizeof(SIM_KEEP_MAGIC), f) == sizeof(SIM_KEEP_MAGIC) &&
                  fwrite(&size, sizeof(size), 1, f) == 1 && fwrite(&now, 1, sizeof(now), f) == sizeof(now);
        if (f) ok = fclose(f) == 0 && ok;
        if (ok && rename(SIM_KEEP_FILE ".tmp", SIM_KEEP_FILE) == 0) memcpy(&s_kept, &now, sizeof(now));
        else remove(SIM_KEEP_FILE ".tmp");
    }
    pthread_mutex_unlock(&s_keep_lock);
}

// Minutes played today in the made-up activity log below (45 + 20 + 15): the
// timer's time left is the day's limit minus this (and PLAYGUARD_SIM_SKIP).
#define SIM_PLAYED_TODAY_MIN 80

// PLAYGUARD_SIM_SKIP: seconds the console's time is moved on (0 without it).
static u64 sim_skip(void)
{
    const char *skip = sim_env("PLAYGUARD_SIM_SKIP");
    if (!skip || !*skip) return 0;
    char *end = NULL;
    const long long v = strtoll(skip, &end, 10);
    return end && !*end && v > 0 ? (u64)v : 0;
}

// "Now" before PLAYGUARD_SIM_SKIP: frozen by PLAYGUARD_SIM_NOW, else the host's.
static u64 start_now(void)
{
    const char *frozen = sim_env("PLAYGUARD_SIM_NOW");
    if (frozen && *frozen) {
        char *end = NULL;
        const unsigned long long v = strtoull(frozen, &end, 10);
        if (end && !*end && v) return (u64)v;
    }
    return (u64)time(NULL);
}

// "Now" before any network-clock change.
static u64 base_now(void)
{
    return start_now() + sim_skip();
}

// A service error the UI does not translate (pctl module 142), as the console
// would return for a refused command.
#define SIM_FAIL_RC ((Result)(142 | (100 << 9)))

// True when PLAYGUARD_SIM_FAIL lists `what` (comma separated).
static bool fails(const char *what)
{
    const char *list = sim_env("PLAYGUARD_SIM_FAIL");
    const size_t n = strlen(what);
    for (const char *p = list; p && *p;) {
        const char *end = strchr(p, ',');
        const size_t len = end ? (size_t)(end - p) : strlen(p);
        if (len == n && strncmp(p, what, n) == 0) return true;
        p = end ? end + 1 : NULL;
    }
    return false;
}

static void sim_init(void)
{
    if (S.init) return;
    S.init = true;
    S.hos = PCTL_FW_TESTED_MAX;   // the newest verified one: no firmware gate
    const char *fw = sim_env("PLAYGUARD_SIM_FW");
    unsigned a, b, c;
    if (fw && sscanf(fw, "%u.%u.%u", &a, &b, &c) == 3) S.hos = MAKEHOSVERSION(a, b, c);
    const bool blank = sim_env("PLAYGUARD_SIM_NOT_SET_UP") != NULL;
    S.safety_level = blank ? PctlSafetyLevel_None : PctlSafetyLevel_Child;
    S.pin_length = blank ? 0 : 4;
    S.restriction_enabled = !blank;
    S.pairing_active = !blank && sim_env("PLAYGUARD_SIM_UNPAIRED") == NULL;
    S.temp_unlocked = sim_env("PLAYGUARD_SIM_UNLOCKED") != NULL;
    S.custom.rating_age = 12;
    S.rating_org = 6;   // PEGI
    S.custom.sns_post_restriction = true;
    S.limit_reached = sim_env("PLAYGUARD_SIM_RESTRICTED") != NULL;
    S.autosync_off = sim_env("PLAYGUARD_SIM_AUTOSYNC_OFF") != NULL;
    S.alarm_disabled = sim_env("PLAYGUARD_SIM_ALARM_OFF") != NULL;
    const char *net = sim_env("PLAYGUARD_SIM_NETWORK_OFFSET");
    if (net) S.network_offset = strtoll(net, NULL, 10);
    bool off = blank || sim_env("PLAYGUARD_SIM_TIMER_OFF") != NULL;
    u16 days[7];
    for (int i = 0; i < 7; i++) days[i] = off ? PT_DAY_NOLIMIT : ((i == 0 || i == 6) ? 180 : 120);
    pt_encode(S.block, days);
    if (!off) {   // a bedtime at 21:00, play allowed again at 06:00
        PtBedtime bed[7];
        for (int i = 0; i < 7; i++) bed[i] = (PtBedtime){ true, 21, 0, 6, 0 };
        pt_bedtime_encode(S.block, bed);
    }
    if (sim_env("PLAYGUARD_SIM_ALARM_ONLY")) pt_set_suspends(S.block, false);
    if (sim_keeping()) sim_keep_load();
}

// The time left goes down as on a console that counts PlayGuard's own time,
// but only by 1 to 20 s around the minute it starts on, then back up: the
// screens show the same minutes at every run (visual check) and the timer
// health check (action/timer_health_logic) sees it count.
static u64 counting_ns(void)
{
    if (sim_env("PLAYGUARD_SIM_NOT_COUNTING")) return 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)(ts.tv_sec % 20) * 1000000000ULL;
}

// Seconds played today: the made-up log's, and PLAYGUARD_SIM_SKIP's played
// all along unless no game runs; past midnight, since midnight.
static u64 played_today_s(void)
{
    const u64 played = (u64)SIM_PLAYED_TODAY_MIN * 60;
    const u64 skip = sim_skip();
    if (!skip || sim_env("PLAYGUARD_SIM_IDLE")) return played;
    const time_t from = (time_t)start_now(), to = (time_t)(start_now() + skip);
    struct tm a, b;
    if (!localtime_r(&from, &a) || !localtime_r(&to, &b)) return played + skip;
    if (a.tm_year == b.tm_year && a.tm_yday == b.tm_yday) return played + skip;
    return (u64)(b.tm_hour * 3600 + b.tm_min * 60 + b.tm_sec);
}

static int sim_weekday(void)
{
    LocalTime today;
    return time_local_now(NULL, &today) ? today.wday : 0;
}

// The header is non-zero while any day has a limit (pure.h).
static bool timer_enabled(void)
{
    return !S.temp_unlocked && S.block[0] != 0;
}

// ---------------------------------------------------------------- sysinfo
void sysinfo_get(SysInfo *out)
{
    sim_init();
    memset(out, 0, sizeof(*out));
    out->hos_version = S.hos;
    out->is_atmosphere = true;
    out->ams_valid = true;
    out->ams_major = 1; out->ams_minor = 12; out->ams_micro = 0;
    out->emummc_valid = true;
    out->emummc = sim_env("PLAYGUARD_SIM_EMUMMC") != NULL;
    out->blank_valid = true;
    out->blank = sim_env("PLAYGUARD_SIM_BLANK") != NULL;
    out->applet_mode = sim_env("PLAYGUARD_SIM_APPLET") != NULL;
    out->serial_valid = true;
    snprintf(out->serial, sizeof(out->serial), "%s", out->blank ? SYSINFO_BLANK_SERIAL : "XAW10000000001");
}
void sysinfo_version_string(u32 v, char *buf, size_t size)
{
    if (!v) { snprintf(buf, size, "?"); return; }
    snprintf(buf, size, "%u.%u.%u", (unsigned)HOSVER_MAJOR(v), (unsigned)HOSVER_MINOR(v), (unsigned)HOSVER_MICRO(v));
}

// ---------------------------------------------------------------- pctl
Result pctl_ops_init(void)   { sim_init(); return sim_env("PLAYGUARD_SIM_NO_CFW") ? (Result)0x0C15 : 0; }
void   pctl_ops_exit(void)   {}
Result pctl_ops_reinit(void) { return pctl_ops_init(); }

void pctl_status_fetch(PctlStatus *o)
{
    memset(o, 0, sizeof(*o));
    if (R_FAILED(o->session_rc = pctl_ops_init())) return;
    o->safety_level_ok = true;        o->safety_level = S.safety_level;
    o->pin_length_ok = true;          o->pin_length = S.pin_length;
    o->restriction_enabled_ok = true; o->restriction_enabled = S.restriction_enabled;
    o->temp_unlocked_ok = true;       o->temp_unlocked = S.temp_unlocked;
    o->pairing_active_ok = true;      o->pairing_active = S.pairing_active;
    o->settings_ok = true;            o->settings = S.custom;
    o->rating_org_ok = true;          o->rating_org = S.rating_org;
    o->stereo_vision_ok = true;       o->stereo_vision_restricted = S.stereo_restricted;
    o->free_comm_count_ok = true;     o->free_comm_count = 3;
    o->last_updated_ok = true;        o->last_updated = base_now() - 3600 * 26;
    sim_keep_save();
}

// Read-only, then the change check (write_guard.h), as pctl_ops.c does.
#define RO_GUARD() do { Result g_ = core_change_allowed(); if (R_FAILED(g_)) return g_; } while (0)
// Refuses like the console when PLAYGUARD_SIM_FAIL lists `what`.
#define FAIL_IF(what) do { if (fails(what)) return SIM_FAIL_RC; } while (0)

Result pctl_set_pin(void)
{
    RO_GUARD();
    if (fails("pin_entry")) return NXM_RC_CANCELLED;   // as pctl_ops.c says B on the PIN screen
    S.pin_length = 4;
    return 0;
}
Result pctl_unlock_restriction_temporarily(void)
{
    RO_GUARD();
    FAIL_IF("unlock");
    if (!S.pin_length) return 0x1A08E;
    if (fails("unverified")) return NXM_RC_UNLOCK_NOT_EFFECTIVE;
    S.temp_unlocked = true;
    return 0;
}
Result pctl_get_pin(char *out, size_t out_size)
{
    if (out && out_size) memset(out, 0, out_size);
    Result g = core_reveal_allowed();
    if (R_FAILED(g)) return g;
    FAIL_IF("pin");
    if (!out || out_size < 5) return NXM_RC_INVALID_ARGUMENT;
    if (!S.pin_length) return NXM_RC_STATE_UNKNOWN;
    snprintf(out, out_size, "1234");
    return 0;
}
// The simulated PIN is 1234: an easy one (PLAYGUARD_SIM_FAIL=pin fails the read).
Result pctl_pin_weak(bool *weak)
{
    if (!weak) return NXM_RC_INVALID_ARGUMENT;
    *weak = false;
    sim_init();
    FAIL_IF("pin");
    if (!S.pin_length) return NXM_RC_NO_PIN;
    *weak = pin_is_weak("1234", 4);
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
    sim_init();
    FAIL_IF("pin");
    if (!S.pin_length) return NXM_RC_NO_PIN;
    sim_hmac_sha256(mac, "1234", 4, msg, size);
    return 0;
}
// Locking again never asks for the PIN (write_guard.h): read-only only.
Result pctl_lock_state(u32 *pin_length, bool *unlocked)
{
    Result rc = pctl_ops_init();
    if (R_FAILED(rc)) return rc;
    if (pin_length) *pin_length = S.pin_length;
    if (unlocked) *unlocked = S.temp_unlocked;
    return 0;
}
Result pctl_relock(void)                          { if (core_read_only()) return NXM_RC_READ_ONLY; FAIL_IF("relock"); S.temp_unlocked = false; return 0; }
// PLAYGUARD_SIM_FAIL=pin_entry: the PIN screen is cancelled.
Result pctl_ask_pin(void)
{
    sim_init();
    if (!S.pin_length) return NXM_RC_NO_PIN;
    return fails("pin_entry") ? NXM_RC_CANCELLED : 0;
}
Result pctl_delete_parental_controls(void)
{
    RO_GUARD();
    FAIL_IF("write");
    S.pin_length = 0; S.restriction_enabled = false; S.safety_level = 0; S.temp_unlocked = false;
    memset(S.block, 0, sizeof(S.block));
    return 0;
}
Result pctl_delete_pairing(void)                  { RO_GUARD(); FAIL_IF("write"); S.pairing_active = false; return 0; }
Result pctl_set_safety_level(u32 l)               { RO_GUARD(); FAIL_IF("write"); if (l > PctlSafetyLevel_Teen) return NXM_RC_INVALID_ARGUMENT; S.safety_level = l; return 0; }
Result pctl_set_custom_settings(const PctlCustomSettings *c)
{
    RO_GUARD();
    FAIL_IF("write");
    if (S.safety_level != PctlSafetyLevel_Custom) return NXM_RC_NOT_CUSTOM;
    S.custom = *c;
    return 0;
}
Result pctl_set_stereo_vision_restricted(bool r)  { RO_GUARD(); FAIL_IF("write"); S.stereo_restricted = r; return 0; }
Result pctl_set_rating_org(u32 org)               { RO_GUARD(); FAIL_IF("write"); if (org >= RATING_ORG_COUNT) return NXM_RC_INVALID_ARGUMENT; S.rating_org = org; return 0; }
Result pctl_get_level_settings(u32 level, PctlCustomSettings *out)
{
    // Made-up presets in the spirit of the console's.
    static const PctlCustomSettings presets[5] = { { 0, false, false }, { 0, false, false },
                                                   { 6, true, true }, { 12, true, true }, { 16, false, false } };
    if (!out || level > PctlSafetyLevel_Teen) return NXM_RC_INVALID_ARGUMENT;
    *out = level == PctlSafetyLevel_Custom ? S.custom : presets[level];
    return 0;
}
// The same gate as pctl_ops.c: no play-timer write while it counts down.
static bool pt_gated(void) { return (timer_enabled() || S.limit_reached) && !S.temp_unlocked; }
#define PT_GATE() do { if (pt_gated()) return NXM_RC_WRITE_GATED; } while (0)
Result pctl_play_timer_set_alarm_disabled(bool d) { RO_GUARD(); PT_GATE(); FAIL_IF("write"); S.alarm_disabled = d; return 0; }
Result pctl_play_timer_start(void)                { RO_GUARD(); PT_GATE(); FAIL_IF("write"); S.paused = false; return 0; }
Result pctl_play_timer_stop(void)                 { RO_GUARD(); PT_GATE(); FAIL_IF("write"); S.paused = true; return 0; }

void pctl_play_timer_query(PtState *o)
{
    sim_init();
    memset(o, 0, sizeof(*o));
    for (int i = 0; i < 7; i++) o->day_min[i] = PT_DAY_NOLIMIT;
    o->fw_supported = S.hos >= PCTL_FW_MIN_PLAYTIMER;
    if (!o->fw_supported) return;
    if (R_FAILED(o->session_rc = pctl_ops_init())) return;
    o->session_valid = true;
    if (fails("timer")) {
        o->config_rc = SIM_FAIL_RC;
    } else if (!pt_plausible(S.block)) {   // as pctl_ops.c (only pt_encode writes it here)
        o->config_rc = NXM_RC_PT_NOT_UNDERSTOOD;
        memcpy(o->block, S.block, sizeof(o->block));
    } else {
        o->valid = true;
        pt_decode(S.block, o->day_min);
        pt_bedtime_decode(S.block, o->bed);
        o->alarm_only = !pt_suspends(S.block);
        memcpy(o->block, S.block, sizeof(o->block));
    }
    o->enabled_valid = true;  o->enabled = timer_enabled();
    o->temporary_unlocked_valid = true; o->temporary_unlocked = S.temp_unlocked;
    // Today's limit against what the made-up log says was played today.
    const int wd = sim_weekday();
    u16 days[7];
    pt_decode(S.block, days);
    const u16 limit = days[wd];
    const s64 played = (s64)played_today_s();
    const bool reached = o->enabled && (S.limit_reached || (limit != PT_DAY_NOLIMIT && (s64)limit * 60 <= played));
    o->remaining_valid = true;
    o->remaining_ns = o->enabled && !reached && limit != PT_DAY_NOLIMIT && !sim_env("PLAYGUARD_SIM_IDLE")
                          ? ((s64)limit * 60 - played) * 1000000000LL + 20000000000LL - (s64)counting_ns() : 0;
    // The last minutes of the day's time, whatever the limit (the child screen's end).
    const char *left_min = sim_env("PLAYGUARD_SIM_LEFT_MIN");
    if (left_min && o->remaining_ns > 0) o->remaining_ns = (s64)atoi(left_min) * 60 * 1000000000LL + 20000000000LL;
    // A warning only: the console does not restrict, and 1454 goes below 0
    // by the time played over the limit (as observed; 2 min when the limit is
    // only said to be reached).
    const bool warn_only = !pt_suspends(S.block);
    if (reached && warn_only) {
        const s64 over = limit != PT_DAY_NOLIMIT && played > (s64)limit * 60 ? played - (s64)limit * 60 : 2 * 60;
        o->remaining_ns = -over * 1000000000LL;
    }
    o->restricted_valid = true; o->restricted = reached && !warn_only;
    o->alarm_disabled_valid = true; o->alarm_disabled = S.alarm_disabled;
    // 1954..1959 answer today's bedtime from the block, as the console is
    // taken to (pure.h).
    PtBedtime bed[7];
    pt_bedtime_decode(S.block, bed);
    o->bedtime_valid = true; o->bedtime_enabled = bed[wd].on;
    o->bedtime_hour = bed[wd].hour; o->bedtime_minute = bed[wd].minute;
    o->bedtime_reset_valid = true;
    o->bedtime_reset_hour = bed[wd].on ? bed[wd].end_hour : 6;
    o->bedtime_reset_minute = bed[wd].on ? bed[wd].end_minute : 0;
    sim_keep_save();
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
    if (pt.fw_supported && pt.valid && pt.enabled && pt.day_min[wd] != PT_DAY_NOLIMIT) {
        o->display[0] = 2;
        memcpy(o->display + 0x10, &o->remaining_ns, sizeof(o->remaining_ns));
    }
    // 1454 + 1952 = today's limit, as observed (docs/parental-controls.md).
    o->spent_ns = pt.remaining_ns ? (u64)((s64)pt.day_min[wd] * 60 * 1000000000LL - pt.remaining_ns)
                                  : played_today_s() * 1000000000ULL;
}

// What a block write (195101) meets once its arguments are checked: the gate,
// no PIN (pctl skips the write), PLAYGUARD_SIM_FAIL=write.
static Result sim_pt_gate(void)
{
    if (pt_gated()) return NXM_RC_WRITE_GATED;
    if (!S.pin_length) return NXM_RC_NO_PIN;
    return fails("write") ? SIM_FAIL_RC : 0;
}

Result pctl_play_timer_set_days(const u16 d[7])
{
    RO_GUARD();
    sim_init();
    if (S.hos < PCTL_FW_MIN_PLAYTIMER) return NXM_RC_FW_UNSUPPORTED;
    if (!pt_days_ok(d)) return NXM_RC_INVALID_ARGUMENT;
    Result rc = sim_pt_gate();
    if (R_FAILED(rc)) return rc;
    pt_encode(S.block, d);   // read-modify-write, as pctl_ops.c does
    S.limit_reached = false;
    return 0;
}
// PLAYGUARD_SIM_FAIL=bedtime: the console does not take the bedtime (the
// layout guess is wrong), so the block stays as it was (NXM_RC_NOT_APPLIED).
Result pctl_play_timer_set_bedtime(const PtBedtime bed[7], int today)
{
    RO_GUARD();
    sim_init();
    if (S.hos < PCTL_FW_MIN_PLAYTIMER) return NXM_RC_FW_UNSUPPORTED;
    if (today < 0 || today > 6) return NXM_RC_INVALID_ARGUMENT;
    for (int i = 0; i < 7; i++) if (!pt_bedtime_ok(&bed[i])) return NXM_RC_INVALID_ARGUMENT;
    Result rc = sim_pt_gate();
    if (R_FAILED(rc)) return rc;
    if (fails("timer")) return NXM_RC_STATE_UNKNOWN;
    if (fails("bedtime")) return NXM_RC_NOT_APPLIED;
    pt_bedtime_encode(S.block, bed);
    return 0;
}

Result pctl_play_timer_set_alarm_only(bool alarm_only)
{
    RO_GUARD();
    sim_init();
    if (S.hos < PCTL_FW_MIN_PLAYTIMER) return NXM_RC_FW_UNSUPPORTED;
    Result rc = sim_pt_gate();
    if (R_FAILED(rc)) return rc;
    if (fails("timer")) return NXM_RC_STATE_UNKNOWN;
    return pt_set_suspends(S.block, !alarm_only) ? 0 : NXM_RC_INVALID_ARGUMENT;
}

void pctl_dump(char *buf, size_t n)
{
    int used = snprintf(buf, n, "=== pctl state (simulated desktop backend) ===\n"
                        "safety_level=%u pin=%s enabled=%d temp_unlocked=%d\n145601 block:",
                        (unsigned)S.safety_level, S.pin_length ? "set" : "none", (int)S.restriction_enabled, (int)S.temp_unlocked);
    for (int i = 0; i < PT_U16_COUNT && used > 0 && (size_t)used < n; i++)
        used += snprintf(buf + used, n - (size_t)used, " %04X", S.block[i]);
    if (used > 0 && (size_t)used < n) snprintf(buf + used, n - (size_t)used, "\n");
}

// ---------------------------------------------------------------- time
void pctl_overview_fetch(PctlStatus *s, PtState *pt)
{
    pctl_status_fetch(s);
    pctl_play_timer_query(pt);
}

Result time_network_accuracy(bool *accurate)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    *accurate = s.accuracy;
    return s.accuracy_rc;
}

void time_clock_snapshot(TimeSnapshot *o)
{
    sim_init();
    sim_keep_save();
    memset(o, 0, sizeof(*o));
    u64 now = base_now();
    o->automatic = !S.autosync_off;
    o->network_time = now + S.clock_offset + S.network_offset;
    // With "Synchronise" on, the console clock follows the network clock.
    o->user_time = o->automatic ? o->network_time : now + S.clock_offset;
    o->local_time = now;
    o->accuracy = S.network_set || sim_env("PLAYGUARD_SIM_ACCURATE");
    snprintf(o->location, sizeof(o->location), "Europe/Paris");
}
// Both writes, once allowed: the network clock becomes `utc`, the console
// clock with it while "Synchronise" is on.
static void set_network(u64 utc, TimeApply *o)
{
    o->target = utc;
    if (fails("clock")) {
        o->write_attempted = true;
        o->write_rc = SIM_FAIL_RC;
        return;
    }
    const s64 network = (s64)utc - (s64)base_now();
    if (!S.autosync_off) S.clock_offset = network;
    S.network_offset = network - S.clock_offset;
    S.network_set = true;
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
    if (seconds) *seconds = base_now();
    if (source_id) memset(source_id, 0x53, 16);
    return 0;
}

// The host's time zone stands in for the console's (TZ= changes it).
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

// ---------------------------------------------------------------- playstats
// Made-up accounts (PLAYGUARD_SIM_ACCOUNTS, 2 by default: Alice and Léo),
// each playing twice what the next one does: Alice 2/3 of everything and
// Léo 1/3 (but the time on no account, nobody_s()).
static const char *const SIM_ACCOUNT_NAMES[] = { "Alice", "Léo", "Maya", "Noah", "Inès", "Hugo", "Zoé", "Sam" };
#define SIM_ACCOUNTS_MAX (sizeof(SIM_ACCOUNT_NAMES) / sizeof(SIM_ACCOUNT_NAMES[0]))

static size_t sim_accounts(void)
{
    const char *n = sim_env("PLAYGUARD_SIM_ACCOUNTS");
    if (!n || !*n) return 2;
    const int v = atoi(n);
    return v < 0 ? 0 : (size_t)v > SIM_ACCOUNTS_MAX ? SIM_ACCOUNTS_MAX : (size_t)v;
}

size_t playstats_accounts(PlayAccount *out, size_t max, Result *rc)
{
    if (rc) *rc = 0;
    size_t n = 0;
    for (; n < max && n < sim_accounts(); n++) {
        memset(&out[n], 0, sizeof(out[n]));
        out[n].uid[0] = out[n].uid[1] = n + 1;
        snprintf(out[n].nickname, sizeof(out[n].nickname), "%s", SIM_ACCOUNT_NAMES[n]);
    }
    return n;
}

// The share of `v` the k-th of `n` accounts played (weights 2^(n-1-k)), the
// last one taking what the others leave so that the shares add up to `v`.
static u64 share_of(u64 v, size_t k, size_t n)
{
    const u64 total = (1ULL << n) - 1;
    if (k + 1 < n) return v * (1ULL << (n - 1 - k)) / total;
    u64 given = 0;
    for (size_t j = 0; j + 1 < n; j++) given += v * (1ULL << (n - 1 - j)) / total;
    return v - given;
}

// The share of `v` that `account` played (NULL: all of it).
static u64 share(u64 v, const PlayAccount *account)
{
    if (!account) return v;
    const size_t n = sim_accounts();
    const u64 k = account->uid[0] - 1;
    return k < n ? share_of(v, (size_t)k, n) : 0;
}

// All-time seconds of a game played with no account chosen (a party mode
// that never asks): in its total, in no account's.
static u64 nobody_s(u64 app_id)
{
    return app_id == 0x0100A1B2C3D40000ULL ? 50 * 60 : 0;
}

void playstats_fetch(PlayStats *out)
{
    playstats_fetch_for(out, NULL);
}

// When the made-up week was played: each day's games one after the other,
// half from 16:30 and half from 19:00 on school days, from 10:00 and 15:00 at
// the weekend, today's in one stretch ending before now; folded as the
// console's log is (util/playlog.h).
static void sim_hours(PlayStats *out)
{
    const TimeRule *rule = time_console_rule();
    u64 starts[7];
    for (int k = 0; k < 7; k++) starts[k] = local_midnight(rule, out->now, k);
    enum { GAMES = 16 };
    PlayLogEvent ev[7 * GAMES * 2];
    size_t n = 0;
    for (int k = 6; k >= 0; k--) {
        const bool weekend = out->day_wday[k] == 0 || out->day_wday[k] == 6;
        u64 today_s = 0;
        for (u32 i = 0; i < out->count && i < GAMES; i++) today_s += out->games[i].day_s[0];
        for (int block = 0; block < (k == 0 ? 1 : 2); block++) {
            u64 at;
            if (k == 0) {
                const u64 latest = out->now > today_s + 3600 ? out->now - today_s - 3600 : starts[0];
                at = starts[0] + 13 * 3600 + 1800;
                if (at > latest) at = latest > starts[0] ? latest : starts[0];
            } else {
                at = starts[k] + (u64)(weekend ? (block ? 15 * 60 : 10 * 60) : (block ? 19 * 60 : 16 * 60 + 30)) * 60;
            }
            for (u32 i = 0; i < out->count && i < GAMES; i++) {
                const u32 s = out->games[i].day_s[k];
                if (!s || (k != 0 && (int)(i % 2) != block)) continue;
                ev[n++] = (PlayLogEvent){ out->games[i].app_id, PlayLogEv_Focus, at, 0, { 0, 0 } };
                at += s;
                ev[n++] = (PlayLogEvent){ out->games[i].app_id, PlayLogEv_Unfocus, at, 0, { 0, 0 } };
                at += 120;   // two minutes to change games: the same session
            }
        }
    }
    playlog_fold_hours(ev, n, out->now, starts, &out->hours);
    out->hours_ok = true;
}

void playstats_fetch_for(PlayStats *out, const PlayAccount *account)
{
    memset(out, 0, sizeof(*out));
    time_local_now(&out->now, NULL);
    if (sim_env("PLAYGUARD_SIM_NO_PDM")) {
        out->stats_rc = out->events_rc = (Result)0x1A0C;
        return;
    }
    // Made-up games; one was deleted since (no name, no totals), the last one is
    // little played and left aside for months.
    static const struct {
        u64 id; const char *name; u32 total_min, launches, today_min, week_min, days_ago, prev_week_min;
    } games[] = {
        { 0x0100A1B2C3D40000ULL, "Star Kart Racers",      3650, 300, 45, 310, 0, 190 },
        { 0x0100A1B2C3D41000ULL, "Island Builders",       2980, 160, 20, 140, 0, 160 },
        { 0x0100A1B2C3D42000ULL, "Pixel Quest Deluxe",    1210,  45,  0,  95, 2,  95 },
        { 0x0100A1B2C3D43000ULL, "Dragon Valley Legends", 6100, 120,  0,   0, 12,  0 },
        { 0x0100A1B2C3D44000ULL, "Puzzle Garden",          380,  52,  0,  30, 5,  35 },
        { 0x0100A1B2C3D45000ULL, "",                          0,   0, 15,  15, 0,  0 },
        { 0x0100A1B2C3D46000ULL, "Space Farm Tales",        95,   4,  0,   0, 75,  0 },
    };
    const bool idle_weeks = sim_env("PLAYGUARD_SIM_NO_PLAY") != NULL;
    out->windows_ok = true;
    LocalTime today;
    const int wday = host_to_local(NULL, out->now, &today) ? today.wday : 0;
    for (int k = 0; k < 7; k++) out->day_wday[k] = (u8)((wday - k + 7) % 7);
    for (size_t i = 0; i < sizeof(games) / sizeof(games[0]); i++) {
        GameStat *g = &out->games[out->count++];
        g->app_id = games[i].id;
        snprintf(g->name, sizeof(g->name), "%s", games[i].name);
        g->totals_ok = games[i].total_min > 0;
        g->total_s = (u64)games[i].total_min * 60;
        g->launches = games[i].launches;
        const u32 days_ago = idle_weeks ? games[i].days_ago + 30 : games[i].days_ago;
        g->last_played = g->totals_ok ? out->now - (u64)days_ago * 86400 - 3600 : 0;
        g->first_played = g->totals_ok ? out->now - 400ULL * 86400 : 0;
        g->today_s = idle_weeks ? 0 : games[i].today_min * 60;
        g->week_s = idle_weeks ? 0 : games[i].week_min * 60;
        // PLAYGUARD_SIM_SKIP's time played today, on the first game (the
        // timer counts the same).
        if (i == 0 && !idle_weeks) {
            const u64 more = played_today_s() > SIM_PLAYED_TODAY_MIN * 60ULL ? played_today_s() - SIM_PLAYED_TODAY_MIN * 60ULL : 0;
            g->today_s += (u32)more;
            g->week_s += (u32)more;
            g->total_s += more;
        }
        // The rest of the week spread over the six days before, unevenly.
        static const u32 spread[6] = { 30, 0, 25, 15, 0, 30 };
        g->day_s[0] = g->today_s;
        const u32 rest = g->week_s - g->today_s;
        u32 given = 0;
        for (int k = 1; k < 7; k++) {
            g->day_s[k] = k < 6 ? rest * spread[k - 1] / 100 : rest - given;
            given += g->day_s[k];
        }
        // One account: its share of every figure; a game it never played goes.
        g->total_s  = account ? share(g->total_s - nobody_s(g->app_id), account) : g->total_s;
        g->launches = (u32)share(g->launches, account);
        g->today_s  = (u32)share(g->today_s, account);
        g->week_s   = (u32)share(g->week_s, account);
        g->prev_week_s = idle_weeks ? 0 : (u32)share((u64)games[i].prev_week_min * 60, account);
        // The month: the last 7 days, and the 23 before at the rate of the
        // week before them; 1 h 30 for a game last played 7 to 29 days ago.
        g->month_s = g->week_s + (u32)((u64)g->prev_week_s * 23 / 7);
        if (!g->week_s && !g->prev_week_s && days_ago >= 7 && days_ago < 30) g->month_s = (u32)share(90 * 60, account);
        for (int k = 0; k < 7; k++) g->day_s[k] = (u32)share(g->day_s[k], account);
        if (account && !g->week_s && !g->total_s) out->count--;
    }
    // The week before: 8 h in all (the last 7 days have 9 h 50; Star Kart
    // Racers 2 h more), none after weeks without play.
    out->prev_week_ok = true;
    out->prev_week_s = idle_weeks ? 0 : (u32)share(480ULL * 60, account);
    out->month_ok = true;
    // A large library: filler games, each played a little this week.
    const char *more = sim_env("PLAYGUARD_SIM_GAMES");
    const int n = more ? atoi(more) : 0;
    int i = 0;
    for (; i < n && out->count < PLAYSTATS_MAX; i++) {
        GameStat *g = &out->games[out->count++];
        g->app_id = 0x0100B00000000000ULL + ((u64)i << 12);
        snprintf(g->name, sizeof(g->name), "Game %03d", i + 1);
        g->totals_ok = true;
        g->week_s = g->month_s = g->day_s[1] = (u32)(60 * (n - i));
        g->total_s = g->week_s * 10;
        g->launches = 3;
        g->last_played = out->now - 86400;
        g->first_played = out->now - 30ULL * 86400;
    }
    out->truncated = i < n;   // more than the list holds, as on a console
    sim_hours(out);
}

// The made-up games have their names already; no SD card icons either.
void playstats_remember(const PlayStats *known) { (void)known; }
void playstats_set_icon_dir(const char *dir) { (void)dir; }
static atomic_bool s_cancel;
void playstats_cancel(void) { atomic_store(&s_cancel, true); }
bool playstats_cancelled(void) { return atomic_load(&s_cancel); }

void playstats_icons(PlayIcon *icons, size_t count)
{
    // The made-up games above, in order; the deleted one has no icon. The
    // installed games of Console › Check the installed games (…6000 and up)
    // reuse them, but for the one whose name and icon cannot be read.
    for (size_t i = 0; i < count; i++) {
        icons[i].jpeg = NULL;
        icons[i].size = 0;
        const u64 base = 0x0100A1B2C3D40000ULL;
        if (icons[i].app_id < base || (icons[i].app_id - base) % 0x1000 || (icons[i].app_id - base) / 0x1000 >= 16) continue;
        size_t k = (size_t)((icons[i].app_id - base) / 0x1000);
        if (k == 5 || k == 6) continue;
        k %= 5;
        icons[i].jpeg = (unsigned char *)malloc(SIM_ICON_SIZES[k]);
        if (!icons[i].jpeg) continue;
        memcpy(icons[i].jpeg, SIM_ICONS[k], SIM_ICON_SIZES[k]);
        icons[i].size = SIM_ICON_SIZES[k];
    }
}

size_t playstats_by_account(u64 app_id, AccountPlay *out, size_t max, Result *rc)
{
    if (rc) *rc = sim_env("PLAYGUARD_SIM_NO_PDM") ? (Result)0x1A0C : 0;
    const size_t n = sim_accounts();
    if (sim_env("PLAYGUARD_SIM_NO_PDM") || max < n) return 0;
    // The made-up accounts sharing the play time 2:1 (none for a deleted
    // game), less the time on no account.
    PlayStats all;
    playstats_fetch(&all);
    for (u32 i = 0; i < all.count; i++) {
        if (all.games[i].app_id != app_id || !all.games[i].totals_ok) continue;
        const u64 t = all.games[i].total_s - nobody_s(app_id);
        for (size_t k = 0; k < n; k++) {
            snprintf(out[k].nickname, sizeof(out[k].nickname), "%s", SIM_ACCOUNT_NAMES[k]);
            out[k].total_s = share_of(t, k, n);
            out[k].launches = (u32)share_of(all.games[i].launches, k, n);
        }
        return n;
    }
    return 0;
}

// ---------------------------------------------------------------- gamecheck
// The made-up games of the Activity tab, fine, then one of each problem the
// HOME menu shows (an icon that keeps loading, "!", a dotted frame, …),
// unless PLAYGUARD_SIM_GAMES_OK. What the actions change (G) stays until the
// app quits, or from one run to the next with PLAYGUARD_SIM_KEEP: a removed
// game or content is gone, a launch version reset sticks.

static bool sim_in(const u64 *list, size_t n, u64 id)
{
    for (size_t i = 0; i < n; i++)
        if (list[i] == id) return true;
    return false;
}

static void sim_add_content(GameFacts *g, u64 id, u32 version, u8 type, u8 storage)
{
    if (sim_in(G.removed, G.removed_count, id)) return;
    GameContent *c = &g->contents[g->content_count++];
    c->id = id;
    c->version = version;
    c->type = type;
    c->storage = storage;
    g->meta_count++;
    if (type == GC_META_APPLICATION) {
        g->has_base = true;
        g->base_version = version;
    }
    if (type == GC_META_PATCH) {
        g->has_patch = true;
        g->patch_version = version;
    }
}

// Every game the simulated console has, fully checked (gamecheck_check
// returns them one at a time).
typedef struct {
    u32       count;
    GameFacts games[GAMECHECK_MAX];
} SimGames;

// The same age from the 13 rating organisations, none from the others.
static void sim_rate(GameFacts *g, int age)
{
    memset(g->rating_age, 0xFF, sizeof(g->rating_age));
    for (int org = 0; org < RATING_ORG_COUNT; org++) g->rating_age[org] = (int8_t)age;
}

static void sim_games(SimGames *out)
{
    memset(out, 0, sizeof(*out));
    static const char *fine[] = { "Star Kart Racers", "Island Builders", "Pixel Quest Deluxe",
                                  "Dragon Valley Legends", "Puzzle Garden" };
    static const int fine_age[] = { 3, 7, 12, 16, 3 };
    for (size_t i = 0; i < sizeof(fine) / sizeof(fine[0]); i++) {
        GameFacts *g = &out->games[out->count++];
        g->app_id = 0x0100A1B2C3D40000ULL + i * 0x1000;
        snprintf(g->name, sizeof(g->name), "%s", fine[i]);
        snprintf(g->display_version, sizeof(g->display_version), "1.3.0");
        sim_rate(g, fine_age[i]);
        g->control_ok = true;
        g->record_event = 3;
        sim_add_content(g, g->app_id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
        sim_add_content(g, g->app_id | 0x800, 0x30000, GC_META_PATCH, GC_STORAGE_SD);
        g->base_required_hos = MAKEHOSVERSION(18, 0, 0);
        g->key_generation = 0x12;
        g->view_ok = true;
        g->view_flags = GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH;
        g->launch_checked = true;
        g->launch_required = 0x30000;
    }
    // A game card left out of its slot: not a problem.
    GameFacts *card = &out->games[out->count++];
    card->app_id = 0x0100A1B2C3D4A000ULL;
    snprintf(card->name, sizeof(card->name), "Sky Kingdom");
    sim_rate(card, 7);
    card->control_ok = true;
    card->record_event = 5;
    sim_add_content(card, card->app_id, 0, GC_META_APPLICATION, GC_STORAGE_GAMECARD);
    card->card_only = true;
    if (sim_env("PLAYGUARD_SIM_GAMES_OK")) return;

    static const struct { u64 id; const char *name; GameIssue issue; int age; } broken[] = {
        { 0x0100A1B2C3D46000ULL, "",                 GC_NO_CONTROL, -1 },
        { 0x0100A1B2C3D47000ULL, "Garden Defenders", GC_NO_BASE, 7 },
        { 0x0100A1B2C3D48000ULL, "Hedgehog Rush",    GC_ARCHIVED, 7 },
        { 0x0100A1B2C3D49000ULL, "Robot Factory",    GC_FIRMWARE_TOO_OLD, 12 },
        { 0x0100A1B2C3D4B000ULL, "Ocean Explorer",   GC_FILES_MISSING, 3 },
        { 0x0100A1B2C3D4C000ULL, "Castle Siege",     GC_TICKET_MISSING, 16 },
        { 0x0100A1B2C3D4D000ULL, "Moon Racer",       GC_LAUNCH_VERSION, 3 },
        { 0x0100A1B2C3D4E000ULL, "Forest Tales: Legends of the Ancient Grove for Nintendo Switch™",     GC_NEEDS_VERIFY, 12 },
        { 0x0100A1B2C3D4F000ULL, "Star Pilots",      GC_KEY_TOO_NEW, 18 },
    };
    for (size_t i = 0; i < sizeof(broken) / sizeof(broken[0]); i++) {
        const u64 id = broken[i].id;
        if (sim_in(G.removed, G.removed_count, id)) continue;
        GameFacts *g = &out->games[out->count++];
        g->app_id = id;
        snprintf(g->name, sizeof(g->name), "%s", broken[i].name);
        g->control_ok = broken[i].name[0] != '\0';
        if (g->control_ok) snprintf(g->display_version, sizeof(g->display_version), "1.0.0");
        sim_rate(g, broken[i].age);
        g->record_event = broken[i].issue == GC_ARCHIVED ? 11 : 3;
        g->view_ok = true;
        g->view_flags = GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH;
        switch (broken[i].issue) {
            case GC_ARCHIVED:
                g->view_flags = 0;
                break;
            case GC_NO_BASE:
                sim_add_content(g, id | 0x800, 0x10000, GC_META_PATCH, GC_STORAGE_SD);
                break;
            case GC_FIRMWARE_TOO_OLD:   // the update needs it, not the game
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                sim_add_content(g, id | 0x800, 0x50000, GC_META_PATCH, GC_STORAGE_SD);
                sim_add_content(g, (id & ~0xFFFULL) + 0x1001, 0x10000, GC_META_ADDON, GC_STORAGE_SD);
                g->base_required_hos = MAKEHOSVERSION(19, 0, 0);
                if (g->has_patch) {
                    g->patch_required_hos = MAKEHOSVERSION(24, 1, 0);
                    snprintf(g->display_version, sizeof(g->display_version), "2.1.0");   // the update's
                }
                break;
            case GC_LAUNCH_VERSION:   // its update was removed outside the console
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                g->launch_checked = true;
                g->launch_required = sim_in(G.launch_reset, G.launch_reset_count, id) ? 0 : 0x40000;
                g->launch_rc = g->launch_required ? (Result)0x1F0C : 0;   // refused until then
                break;
            case GC_TICKET_MISSING:
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                g->needs_ticket = g->ticket_missing = true;
                break;
            case GC_NEEDS_VERIFY:
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_NAND);
                g->view_flags |= GC_VIEW_NEEDS_VERIFY;
                break;
            case GC_KEY_TOO_NEW:
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                g->key_generation = 0x18;
                break;
            case GC_FILES_MISSING:
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                g->files_missing = true;
                break;
            default:   // GC_NO_CONTROL
                sim_add_content(g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
                break;
        }
    }
}

static void sim_remember(u64 *list, size_t *n, size_t cap, u64 id)
{
    if (*n < cap) list[(*n)++] = id;
}

Result gamecheck_remove(u64 app_id)
{
    RO_GUARD();
    FAIL_IF("remove");
    sim_remember(G.removed, &G.removed_count, sizeof(G.removed) / sizeof(G.removed[0]), app_id);
    sim_keep_save();
    return 0;
}

static void sim_bump(u64 id);

Result gamecheck_remove_content(u64 app_id, const GameContent *content)
{
    if (!content || (content->type != GC_META_PATCH && content->type != GC_META_ADDON) ||
        (content->storage != GC_STORAGE_NAND && content->storage != GC_STORAGE_SD))
        return NXM_RC_INVALID_ARGUMENT;
    RO_GUARD();
    FAIL_IF("remove");
    sim_remember(G.removed, &G.removed_count, sizeof(G.removed) / sizeof(G.removed[0]), content->id);
    sim_bump(app_id);
    sim_keep_save();
    return 0;
}

Result gamecheck_reset_launch_version(u64 app_id, u32 version)
{
    (void)version;
    RO_GUARD();
    FAIL_IF("launch");
    sim_remember(G.launch_reset, &G.launch_reset_count, sizeof(G.launch_reset) / sizeof(G.launch_reset[0]), app_id);
    sim_bump(app_id);
    sim_keep_save();
    return 0;
}

// Four seconds or so of progress over 3.8 GB; "Forest Tales" has corrupt data.
Result gamecheck_verify(u64 app_id, bool (*progress)(void *ctx, u64 current, u64 total), void *ctx, bool *corrupt,
                        Result *found)
{
    *corrupt = false;
    *found = 0;
    if (fails("verify")) return SIM_FAIL_RC;
    const u64 total = 3891ULL << 20;
    for (u64 step = 0; step <= 8; step++) {
        if (progress && !progress(ctx, total * step / 8, step ? total : 0)) return NXM_RC_CANCELLED;
        const struct timespec pause = { 0, 500000000L };
        nanosleep(&pause, NULL);
    }
    *corrupt = app_id == 0x0100A1B2C3D4E000ULL;
    if (*corrupt) *found = 16 | (601 << 9);   // 2016-0601, as a console answers
    return 0;
}

// Each change to a game (a removal, a launch version reset) bumps its entry,
// as the console's LastUpdated does.

static u64 sim_updated(u64 id)
{
    u64 n = 1791400000ULL + (id & 0xFFFFF);
    for (size_t i = 0; i < G.bump_count; i++)
        if (G.bumps[i] == id) n++;
    return n;
}

static void sim_bump(u64 id)
{
    if (G.bump_count < sizeof(G.bumps) / sizeof(G.bumps[0])) G.bumps[G.bump_count++] = id;
}

Result gamecheck_records(GameRecord *out, u32 max, u32 *count, bool *truncated)
{
    static SimGames all;
    sim_games(&all);
    *count = 0;
    *truncated = false;
    for (u32 i = 0; i < all.count; i++) {
        if (*count == max) {
            *truncated = true;
            break;
        }
        GameRecord *r = &out[(*count)++];
        r->app_id  = all.games[i].app_id;
        r->updated = sim_updated(r->app_id);
        r->event   = all.games[i].record_event;
    }
    return 0;
}

static GameCheckTiming s_sim_timing;

void gamecheck_last_timing(GameCheckTiming *out)
{
    *out = s_sim_timing;
}

// PLAYGUARD_SIM_CHECK_MS (default 150) per game, as a console takes.
u32 gamecheck_check(const GameRecord *records, u32 count, GameFacts *out,
                    bool (*on_game)(void *ctx, const GameFacts *game, u32 done), void *ctx)
{
    static SimGames all;
    sim_games(&all);
    const char *ms_env = sim_env("PLAYGUARD_SIM_CHECK_MS");
    const long ms = ms_env ? strtol(ms_env, NULL, 10) : 150;
    memset(out, 0, (size_t)count * sizeof(*out));
    memset(&s_sim_timing, 0, sizeof s_sim_timing);
    u32 done = 0;
    while (done < count) {
        GameFacts *g = &out[done];
        g->app_id = records[done].app_id;
        for (u32 i = 0; i < all.count; i++)
            if (all.games[i].app_id == g->app_id) *g = all.games[i];
        g->record_updated = records[done].updated;
        g->record_event   = records[done].event;
        if (ms > 0) {
            const struct timespec pause = { ms / 1000, (ms % 1000) * 1000000L };
            nanosleep(&pause, NULL);
        }
        done++;
        s_sim_timing.games = done;
        s_sim_timing.total_ns += ms > 0 ? (u64)ms * 1000000ULL : 0;
        s_sim_timing.state_ns = s_sim_timing.total_ns;
        if (on_game && !on_game(ctx, g, done)) break;
    }
    return done;
}
