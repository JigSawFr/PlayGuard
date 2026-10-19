// Simulated core/gamecheck.h, Console › Check the installed games: the
// made-up games (sim_games.h) that are installed, fine, then one of each
// problem, unless PLAYGUARD_SIM_GAMES_OK. What the actions change (sim_state.h's
// SimFixes) stays until the app quits, or from one run to the next with
// PLAYGUARD_SIM_KEEP: a removed game or content is gone, a launch version
// reset sticks.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../core/gamecheck.h"
#include "../core/sim_env.h"
#include "../core/write_guard.h"
#include "sim_games.h"
#include "sim_state.h"

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

static bool sim_in(const u64 *list, u32 n, u64 id)
{
    for (u32 i = 0; i < n; i++)
        if (list[i] == id) return true;
    return false;
}

static void sim_add_content(const SimFixes *fixes, GameFacts *g, u64 id, u32 version, u8 type, u8 storage)
{
    if (sim_in(fixes->removed, fixes->removed_count, id)) return;
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
// returns them one at a time). Too large for a thread's stack.
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

// A game that is fine: its update installed, launchable.
static void sim_fine(const SimFixes *fixes, GameFacts *g)
{
    snprintf(g->display_version, sizeof(g->display_version), "1.3.0");
    g->record_event = 3;
    sim_add_content(fixes, g, g->app_id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
    sim_add_content(fixes, g, g->app_id | 0x800, 0x30000, GC_META_PATCH, GC_STORAGE_SD);
    g->base_required_hos = MAKEHOSVERSION(18, 0, 0);
    g->key_generation = 0x12;
    g->view_ok = true;
    g->view_flags = GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH;
    g->launch_checked = true;
    g->launch_required = 0x30000;
}

// A game with `issue`, as the console's records and views show it.
static void sim_broken(const SimFixes *fixes, GameFacts *g, GameIssue issue)
{
    const u64 id = g->app_id;
    if (g->control_ok) snprintf(g->display_version, sizeof(g->display_version), "1.0.0");
    g->record_event = issue == GC_ARCHIVED ? 11 : 3;
    g->view_ok = true;
    g->view_flags = GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH;
    switch (issue) {
        case GC_ARCHIVED:
            g->view_flags = 0;
            break;
        case GC_NO_BASE:
            sim_add_content(fixes, g, id | 0x800, 0x10000, GC_META_PATCH, GC_STORAGE_SD);
            break;
        case GC_FIRMWARE_TOO_OLD:   // the update needs it, not the game
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            sim_add_content(fixes, g, id | 0x800, 0x50000, GC_META_PATCH, GC_STORAGE_SD);
            sim_add_content(fixes, g, (id & ~0xFFFULL) + 0x1001, 0x10000, GC_META_ADDON, GC_STORAGE_SD);
            g->base_required_hos = MAKEHOSVERSION(19, 0, 0);
            if (g->has_patch) {
                g->patch_required_hos = MAKEHOSVERSION(24, 1, 0);
                snprintf(g->display_version, sizeof(g->display_version), "2.1.0");   // the update's
            }
            break;
        case GC_LAUNCH_VERSION:   // its update was removed outside the console
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            g->launch_checked = true;
            g->launch_required = sim_in(fixes->launch_reset, fixes->launch_reset_count, id) ? 0 : 0x40000;
            g->launch_rc = g->launch_required ? (Result)0x1F0C : 0;   // refused until then
            break;
        case GC_TICKET_MISSING:
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            g->needs_ticket = g->ticket_missing = true;
            break;
        case GC_NEEDS_VERIFY:
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_NAND);
            g->view_flags |= GC_VIEW_NEEDS_VERIFY;
            break;
        case GC_KEY_TOO_NEW:
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            g->key_generation = 0x18;
            break;
        case GC_FILES_MISSING:
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            g->files_missing = true;
            break;
        default:   // GC_NO_CONTROL
            sim_add_content(fixes, g, id, 0, GC_META_APPLICATION, GC_STORAGE_SD);
            break;
    }
}

// NULL when out of memory.
static SimGames *sim_games(void)
{
    SimGames *out = (SimGames *)calloc(1, sizeof(SimGames));
    if (!out) return NULL;
    const SimFixes fixes = sim_state().fixes;
    const bool all_ok = sim_env_flag("PLAYGUARD_SIM_GAMES_OK");
    for (size_t i = 0; i < SIM_GAME_COUNT && out->count < GAMECHECK_MAX; i++) {
        const SimGame *s = &SIM_GAMES[i];
        if (!s->installed || (s->issue != GC_OK && all_ok)) continue;
        // A broken game removed is gone; a fine one keeps its entry (its
        // contents go one by one).
        if (s->issue != GC_OK && sim_in(fixes.removed, fixes.removed_count, s->id)) continue;
        GameFacts *g = &out->games[out->count++];
        g->app_id = s->id;
        g->control_ok = s->issue != GC_NO_CONTROL;   // the name and icon unreadable
        if (g->control_ok) snprintf(g->name, sizeof(g->name), "%s", s->name);
        sim_rate(g, s->age);
        if (s->card) {   // a game card left out of its slot: not a problem
            g->record_event = 5;
            sim_add_content(&fixes, g, g->app_id, 0, GC_META_APPLICATION, GC_STORAGE_GAMECARD);
            g->card_only = true;
        } else if (s->issue == GC_OK) {
            sim_fine(&fixes, g);
        } else {
            sim_broken(&fixes, g, s->issue);
        }
    }
    return out;
}

// What the changes below meet first: the read-only switch and the change
// check (write_guard.h), then PLAYGUARD_SIM_FAIL.
static Result sim_change_allowed(const char *fails)
{
    const Result rc = core_change_allowed();
    if (R_FAILED(rc)) return rc;
    return sim_fails(fails) ? SIM_FAIL_RC : 0;
}

static void sim_remember(u64 *list, u32 *n, u32 cap, u64 id)
{
    if (*n < cap) list[(*n)++] = id;
}

// Each change to a game (a removal, a launch version reset) bumps its entry,
// as the console's LastUpdated does.
static void sim_bump(SimFixes *f, u64 id)
{
    sim_remember(f->bumps, &f->bump_count, COUNT_OF(f->bumps), id);
}

Result gamecheck_remove(u64 app_id)
{
    const Result rc = sim_change_allowed("remove");
    if (R_FAILED(rc)) return rc;
    SimFixes *f = &sim_begin()->fixes;
    sim_remember(f->removed, &f->removed_count, COUNT_OF(f->removed), app_id);
    sim_end();
    return 0;
}

Result gamecheck_remove_content(u64 app_id, const GameContent *content)
{
    if (!content || (content->type != GC_META_PATCH && content->type != GC_META_ADDON) ||
        (content->storage != GC_STORAGE_NAND && content->storage != GC_STORAGE_SD))
        return NXM_RC_INVALID_ARGUMENT;
    const Result rc = sim_change_allowed("remove");
    if (R_FAILED(rc)) return rc;
    SimFixes *f = &sim_begin()->fixes;
    sim_remember(f->removed, &f->removed_count, COUNT_OF(f->removed), content->id);
    sim_bump(f, app_id);
    sim_end();
    return 0;
}

Result gamecheck_reset_launch_version(u64 app_id, u32 version)
{
    (void)version;
    const Result rc = sim_change_allowed("launch");
    if (R_FAILED(rc)) return rc;
    SimFixes *f = &sim_begin()->fixes;
    sim_remember(f->launch_reset, &f->launch_reset_count, COUNT_OF(f->launch_reset), app_id);
    sim_bump(f, app_id);
    sim_end();
    return 0;
}

static void sim_sleep_ms(long ms)
{
    if (ms <= 0) return;
    const struct timespec pause = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&pause, NULL);
}

// PLAYGUARD_SIM_CHECK_MS (default 150): how long the console takes over one game.
static long sim_check_ms(void)
{
    return (long)sim_env_int("PLAYGUARD_SIM_CHECK_MS", 150, 0, 60000);
}

// Four seconds or so of progress over 3.8 GB, whatever PLAYGUARD_SIM_CHECK_MS
// (tools/desktop_smoke.py takes its screens on the way); the game whose data
// the console found corrupt has corrupt data.
Result gamecheck_verify(u64 app_id, bool (*progress)(void *ctx, u64 current, u64 total), void *ctx, bool *corrupt,
                        Result *found)
{
    *corrupt = false;
    *found = 0;
    if (sim_fails("verify")) return SIM_FAIL_RC;
    const u64 total = 3891ULL << 20;
    for (u64 step = 0; step <= 8; step++) {
        if (progress && !progress(ctx, total * step / 8, step ? total : 0)) return NXM_RC_CANCELLED;
        sim_sleep_ms(500);
    }
    const SimGame *g = sim_game(app_id);
    *corrupt = g && g->issue == GC_NEEDS_VERIFY;
    if (*corrupt) *found = 16 | (601 << 9);   // 2016-0601, as a console answers
    return 0;
}

static u64 sim_updated(const SimFixes *f, u64 id)
{
    u64 n = 1791400000ULL + (id & 0xFFFFF);
    for (u32 i = 0; i < f->bump_count; i++)
        if (f->bumps[i] == id) n++;
    return n;
}

Result gamecheck_records(GameRecord *out, u32 max, u32 *count, bool *truncated)
{
    *count = 0;
    *truncated = false;
    SimGames *all = sim_games();
    if (!all) return NXM_RC_STATE_UNKNOWN;
    const SimFixes fixes = sim_state().fixes;
    for (u32 i = 0; i < all->count; i++) {
        if (*count == max) {
            *truncated = true;
            break;
        }
        GameRecord *r = &out[(*count)++];
        r->app_id  = all->games[i].app_id;
        r->updated = sim_updated(&fixes, r->app_id);
        r->event   = all->games[i].record_event;
    }
    free(all);
    return 0;
}

static GameCheckTiming s_sim_timing;

void gamecheck_last_timing(GameCheckTiming *out)
{
    *out = s_sim_timing;
}

u32 gamecheck_check(const GameRecord *records, u32 count, GameFacts *out,
                    bool (*on_game)(void *ctx, const GameFacts *game, u32 done), void *ctx)
{
    memset(out, 0, (size_t)count * sizeof(*out));
    memset(&s_sim_timing, 0, sizeof s_sim_timing);
    SimGames *all = sim_games();
    if (!all) return 0;
    const long ms = sim_check_ms();
    u32 done = 0;
    while (done < count) {
        GameFacts *g = &out[done];
        g->app_id = records[done].app_id;
        for (u32 i = 0; i < all->count; i++)
            if (all->games[i].app_id == g->app_id) *g = all->games[i];
        g->record_updated = records[done].updated;
        g->record_event   = records[done].event;
        sim_sleep_ms(ms);
        done++;
        s_sim_timing.games = done;
        s_sim_timing.total_ns += (u64)ms * 1000000ULL;
        s_sim_timing.state_ns = s_sim_timing.total_ns;
        if (on_game && !on_game(ctx, g, done)) break;
    }
    free(all);
    return done;
}
