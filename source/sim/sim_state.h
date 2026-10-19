// sim_state — the simulated console the desktop and web builds run against
// (CONTRIBUTING.md lists the PLAYGUARD_SIM_* variables that shape it), shared
// by the files that stand in for the service layer:
//   sim_pctl.c       core/pctl_ops.h and core/sysinfo.h
//   sim_time.c       core/time_ops.h
//   sim_playstats.c  core/playstats.h
//   sim_gamecheck.c  core/gamecheck.h
//   sim_games.c      the made-up games the last two share
// Never compiled for the Switch.
//
// The state is read and changed from the UI thread and from workers, so it
// is behind a lock, held only to copy or change it: never across a sleep, a
// file or core_change_allowed() (which may wait for the PIN screen). In a
// browser a worker's file access is a call to the page's thread, answered
// once that thread is free again: a lock held across one, which the page's
// thread then waits for, would stop both for good (source/util/paths.cpp).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "../core/pctl_ops.h"
#include "../core/pure.h"

#ifdef __cplusplus
extern "C" {
#endif

// The console's settings, as the services return them. Fixed-width fields
// only: PLAYGUARD_SIM_KEEP keeps this layout in a file the desktop and the
// web builds read alike.
typedef struct {
    u32  hos;
    u32  safety_level;
    u32  pin_length;
    u32  rating_org;
    bool restriction_enabled, temp_unlocked, pairing_active, stereo_restricted, alarm_disabled, paused;
    bool limit_reached, autosync_off, network_set;   // network_set: PlayGuard set the network clock
    PctlCustomSettings custom;
    u16  block[PT_U16_COUNT];   // PlayTimerSettings, as 145601 returns it
    s64  clock_offset;          // the console clock against sim_base_now()
    s64  network_offset;        // the network clock against the console clock, until set
} SimConsole;

// What Console › Check the installed games changed (sim_gamecheck.c), kept
// with the console as the console keeps it. A list that is full takes no more.
typedef struct {
    u64 removed[32];        // whole games and single contents, by ID
    u64 launch_reset[16];
    u64 bumps[32];          // each change to a game bumps its entry
    u32 removed_count, launch_reset_count, bump_count, reserved;
} SimFixes;

typedef struct {
    SimConsole console;
    SimFixes   fixes;
} SimState;

// The state as it is now (a copy: read it without the lock).
SimState sim_state(void);
// The state to change, the lock held: sim_end() after, without a call to
// anything else in between. sim_end() saves it (PLAYGUARD_SIM_KEEP).
SimState *sim_begin(void);
void sim_end(void);

// Sets the console up from the variables (or the kept file) on first use;
// sim_state() and sim_begin() call it, which is all most callers need.
void sim_init(void);

// True when PLAYGUARD_SIM_FAIL lists `what` (comma separated).
bool sim_fails(const char *what);
// A service error the UI does not translate (pctl module 142), as the console
// would return for a refused command.
#define SIM_FAIL_RC ((Result)(142 | (100 << 9)))

// PLAYGUARD_SIM_SKIP: seconds the console's time is moved on (0 without it).
u64 sim_skip_s(void);
// "Now" before PLAYGUARD_SIM_SKIP: frozen by PLAYGUARD_SIM_NOW, else the host's.
u64 sim_start_now(void);
// "Now" before any network-clock change: the two above.
u64 sim_base_now(void);

#ifdef __cplusplus
}
#endif
