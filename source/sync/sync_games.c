// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sync_games.h"

#include <stdio.h>

#include "sync_json.h"
#include "sync_state.h"   // SYNC_SCHEMA

// What one game takes at most: its id, a long name, the numbers.
#define GAME_ROOM 640

size_t sync_games_build(const SyncGames *g, char *out, size_t cap, size_t *kept)
{
    SyncJson j;
    sync_json_init(&j, out, cap);
    sync_json_obj(&j);
    sync_json_kint(&j, "schema", SYNC_SCHEMA);
    sync_json_kint_or_null(&j, "ts", g->ts != 0, (int64_t)g->ts);
    sync_json_kint_or_null(&j, "installed", g->installed >= 0, g->installed);
    sync_json_kint_or_null(&j, "accounts", g->accounts >= 0, g->accounts);
    sync_json_kint_or_null(&j, "total_min", g->total_min >= 0, g->total_min);
    sync_json_kint_or_null(&j, "homebrews", g->homebrews >= 0, g->homebrews);
    sync_json_kint_or_null(&j, "forwarders", g->forwarders >= 0, g->forwarders);
    sync_json_kint_or_null(&j, "homebrew_min", g->homebrew_min >= 0, g->homebrew_min);
    size_t n = 0;
    sync_json_key(&j, "games");
    sync_json_obj(&j);
    for (; n < g->n_games; n++) {
        if (j.len + GAME_ROOM + 16 > cap) break;   // the rest does not fit: the least played
        const SyncGame *e = &g->games[n];
        char id[17];
        snprintf(id, sizeof(id), "%016llX", (unsigned long long)e->app_id);
        sync_json_key(&j, id);
        sync_json_obj(&j);
        sync_json_kstr_or_null(&j, "name", e->name);
        sync_json_kstr_or_null(&j, "version", e->version);
        sync_json_kbool(&j, "installed", e->installed);
        sync_json_kint_or_null(&j, "total_min", e->total_min >= 0, e->total_min);
        sync_json_kint_or_null(&j, "launches", e->launches >= 0, e->launches);
        sync_json_kint_or_null(&j, "first_played", e->first_played != 0, (int64_t)e->first_played);
        sync_json_kint_or_null(&j, "last_played", e->last_played != 0, (int64_t)e->last_played);
        sync_json_obj_end(&j);
    }
    sync_json_obj_end(&j);
    sync_json_kbool(&j, "complete", n == g->n_games);
    sync_json_obj_end(&j);
    if (kept) *kept = n;
    return sync_json_end(&j);
}
