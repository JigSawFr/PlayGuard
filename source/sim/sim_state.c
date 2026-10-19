// sim_state.h: the simulated console's state, its lock, its first values
// (the PLAYGUARD_SIM_* variables) and PLAYGUARD_SIM_KEEP's file.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include "sim_state.h"

#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../core/sim_env.h"
#include "../core/sysinfo.h"

static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static SimState s_state;
static atomic_bool s_ready;
// Each sim_end() counts one; s_saved is the count PLAYGUARD_SIM_KEEP's file
// was last brought up to.
static atomic_uint s_changes, s_saved;

// ---------------------------------------------------------------- the variables

bool sim_fails(const char *what)
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

u64 sim_skip_s(void)
{
    return (u64)sim_env_int("PLAYGUARD_SIM_SKIP", 0, 0, LLONG_MAX);
}

u64 sim_start_now(void)
{
    const long long frozen = sim_env_int("PLAYGUARD_SIM_NOW", 0, 0, LLONG_MAX);
    return frozen ? (u64)frozen : (u64)time(NULL);
}

u64 sim_base_now(void)
{
    return sim_start_now() + sim_skip_s();
}

// The console a first run starts with: parental controls set up with a PIN,
// 2 h a day (3 h at the weekend), a bedtime at 21:00, unless the variables
// say otherwise.
static void sim_defaults(SimConsole *c)
{
    c->hos = PCTL_FW_TESTED_MAX;   // the newest verified one: no firmware gate
    const char *fw = sim_env("PLAYGUARD_SIM_FW");
    unsigned a, b, m;
    if (fw && sscanf(fw, "%u.%u.%u", &a, &b, &m) == 3) c->hos = MAKEHOSVERSION(a, b, m);
    const bool blank = sim_env_flag("PLAYGUARD_SIM_NOT_SET_UP");
    c->safety_level = blank ? PctlSafetyLevel_None : PctlSafetyLevel_Child;
    c->pin_length = blank ? 0 : 4;
    c->restriction_enabled = !blank;
    c->pairing_active = !blank && !sim_env_flag("PLAYGUARD_SIM_UNPAIRED");
    c->temp_unlocked = sim_env_flag("PLAYGUARD_SIM_UNLOCKED");
    c->custom.rating_age = 12;
    c->rating_org = 6;   // PEGI
    c->custom.sns_post_restriction = true;
    c->limit_reached = sim_env_flag("PLAYGUARD_SIM_RESTRICTED");
    c->autosync_off = sim_env_flag("PLAYGUARD_SIM_AUTOSYNC_OFF");
    c->alarm_disabled = sim_env_flag("PLAYGUARD_SIM_ALARM_OFF");
    // A century either way is far more than a clock is ever off by.
    const long long century = 100LL * 366 * 86400;
    c->network_offset = sim_env_int("PLAYGUARD_SIM_NETWORK_OFFSET", 0, -century, century);
    const bool off = blank || sim_env_flag("PLAYGUARD_SIM_TIMER_OFF");
    u16 days[7];
    for (int i = 0; i < 7; i++) days[i] = off ? PT_DAY_NOLIMIT : ((i == 0 || i == 6) ? 180 : 120);
    pt_encode(c->block, days);
    if (!off) {   // a bedtime at 21:00, play allowed again at 06:00
        PtBedtime bed[7];
        for (int i = 0; i < 7; i++) bed[i] = (PtBedtime){ true, 21, 0, 6, 0 };
        pt_bedtime_encode(c->block, bed);
    }
    if (sim_env_flag("PLAYGUARD_SIM_ALARM_ONLY")) pt_set_suspends(c->block, false);
}

// ---------------------------------------------------------------- PLAYGUARD_SIM_KEEP
// The state as the last run left it, in the simulated data folder (the web
// page keeps that folder in the browser). A file from a build whose state has
// another layout is not read: SIM_KEEP_MAGIC changes with it.

#define SIM_KEEP_FILE "./playguard_data/sim_console.bin"
static const char SIM_KEEP_MAGIC[8] = "PGSIM03";
// Change SIM_KEEP_MAGIC with any change to SimState (a field added, removed,
// moved or resized), then this size: the same size can hide another layout.
_Static_assert(sizeof(SimState) == 768, "SimState changed: change SIM_KEEP_MAGIC, then this size");

static bool sim_keeping(void)
{
    return sim_env_flag("PLAYGUARD_SIM_KEEP");
}

static bool sim_keep_read(SimState *out)
{
    FILE *f = fopen(SIM_KEEP_FILE, "rb");
    if (!f) return false;
    char magic[sizeof(SIM_KEEP_MAGIC)];
    u32 size = 0;
    const bool ok = fread(magic, 1, sizeof(magic), f) == sizeof(magic) &&
                    memcmp(magic, SIM_KEEP_MAGIC, sizeof(magic)) == 0 && fread(&size, sizeof(size), 1, f) == 1 &&
                    size == sizeof(*out) && fread(out, 1, sizeof(*out), f) == sizeof(*out);
    fclose(f);
    return ok;
}

static bool sim_keep_write(const SimState *state)
{
    // Not world-writable (fopen creates with 0666, less the umask).
    const int fd = open(SIM_KEEP_FILE ".tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE *f = fd >= 0 ? fdopen(fd, "wb") : NULL;
    if (fd >= 0 && !f) close(fd);
    const u32 size = sizeof(*state);
    bool ok = f && fwrite(SIM_KEEP_MAGIC, 1, sizeof(SIM_KEEP_MAGIC), f) == sizeof(SIM_KEEP_MAGIC) &&
              fwrite(&size, sizeof(size), 1, f) == 1 && fwrite(state, 1, sizeof(*state), f) == sizeof(*state);
    if (f) ok = fclose(f) == 0 && ok;
    if (ok && rename(SIM_KEEP_FILE ".tmp", SIM_KEEP_FILE) == 0) return true;
    remove(SIM_KEEP_FILE ".tmp");
    return false;
}

// Brings the file up to the state, outside the state's lock. One thread
// writes at a time, and none waits for it: a thread that finds another
// writing leaves its change to it, which looks again once done.
static void sim_keep_flush(void)
{
    static pthread_mutex_t writing = PTHREAD_MUTEX_INITIALIZER;
    static SimState written;   // what the file holds, under `writing`
    static bool written_ok;
    if (!sim_keeping()) return;
    while (atomic_load(&s_saved) != atomic_load(&s_changes)) {
        if (pthread_mutex_trylock(&writing) != 0) return;
        SimState now;
        pthread_mutex_lock(&s_lock);
        memcpy(&now, &s_state, sizeof(now));
        const unsigned changes = atomic_load(&s_changes);
        pthread_mutex_unlock(&s_lock);
        if (!(written_ok && memcmp(&now, &written, sizeof(now)) == 0) && sim_keep_write(&now)) {
            memcpy(&written, &now, sizeof(now));
            written_ok = true;
        }
        // Even when the write failed: the next change tries again.
        atomic_store(&s_saved, changes);
        pthread_mutex_unlock(&writing);
    }
}

// ---------------------------------------------------------------- the state

void sim_init(void)
{
    if (atomic_load(&s_ready)) return;
    // The file is read before the lock is taken: two first calls at once may
    // both read it, the first to get the lock sets the state up.
    SimState first;
    memset(&first, 0, sizeof(first));
    const bool kept = sim_keeping() && sim_keep_read(&first);
    if (!kept) {
        memset(&first, 0, sizeof(first));
        sim_defaults(&first.console);
    }
    pthread_mutex_lock(&s_lock);
    const bool set_up = !atomic_load(&s_ready);
    if (set_up) {
        memcpy(&s_state, &first, sizeof(first));
        if (!kept) atomic_fetch_add(&s_changes, 1);   // a first run's state goes to the file
        atomic_store(&s_ready, true);
    }
    pthread_mutex_unlock(&s_lock);
    if (set_up) sim_keep_flush();
}

SimState sim_state(void)
{
    sim_init();
    SimState out;
    pthread_mutex_lock(&s_lock);
    memcpy(&out, &s_state, sizeof(out));
    pthread_mutex_unlock(&s_lock);
    return out;
}

SimState *sim_begin(void)
{
    sim_init();
    pthread_mutex_lock(&s_lock);
    return &s_state;
}

void sim_end(void)
{
    atomic_fetch_add(&s_changes, 1);
    pthread_mutex_unlock(&s_lock);
    sim_keep_flush();
}
