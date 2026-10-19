// sim_games — the simulated console's games, one table for the Activity tab
// (sim_playstats.c) and Console › Check the installed games
// (sim_gamecheck.c): a game played and installed is the same entry in both.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stddef.h>

#include "../core/gamecheck.h"

#ifdef __cplusplus
extern "C" {
#endif

// What the activity log says of a game (all 0: never played).
typedef struct {
    u32 total_min, launches, today_min, week_min, days_ago, prev_week_min;
    u32 nobody_min;   // of total_min, played with no account chosen (a party mode that never asks)
} SimPlay;

typedef struct {
    u64         id;
    const char *name;   // "" for a game deleted since (no name, no totals)
    int         age;    // its rating, the same from every organisation; -1 none
    int         icon;   // in sim_icons.h, -1 none
    bool        played; // in the activity log
    SimPlay     play;
    // Installed: GC_OK for a game that is fine, else the one problem the
    // HOME menu would show (sim_gamecheck.c). Not installed: `installed` false.
    bool        installed;
    bool        card;      // on a game card left out of its slot (not a problem)
    GameIssue   issue;
} SimGame;

// In the order the Activity tab and the installed games list them.
extern const SimGame SIM_GAMES[];
extern const size_t SIM_GAME_COUNT;

// The game with this ID, NULL when the console has none.
const SimGame *sim_game(u64 id);

// Minutes the made-up activity log says were played today, all games together.
u32 sim_played_today_min(void);
// Seconds played today: those minutes, and PLAYGUARD_SIM_SKIP's played all
// along unless no game runs (PLAYGUARD_SIM_IDLE); past midnight, since
// midnight. The play timer counts them, and the Activity tab's first game.
u64 sim_played_today_s(void);

// The game's icon (a copy to free), NULL when it has none.
unsigned char *sim_game_icon(u64 id, size_t *size);

#ifdef __cplusplus
}
#endif
