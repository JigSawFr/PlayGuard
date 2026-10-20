// sim_games.h: the made-up games. Five played and installed, fine; a game
// card; a game deleted since; then one of each problem the HOME menu shows
// (an icon that keeps loading, "!", a dotted frame, …), Space Farm Tales the
// first of them: played long ago, its name and icon unreadable since.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include "sim_games.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../core/sim_env.h"
#include "sim_icons.h"
#include "sim_state.h"

#define PLAYED(...) true, { __VA_ARGS__ }
#define NOT_PLAYED  false, { 0 }

const SimGame SIM_GAMES[] = {
    //  id                      name                     age icon  total_min launches today week days_ago prev_week nobody
    { 0x0100A1B2C3D40000ULL, "Star Kart Racers",          3, 0, PLAYED(3650, 300, 45, 310, 0, 190, 50), true, false, GC_OK },
    { 0x0100A1B2C3D41000ULL, "Island Builders",           7, 1, PLAYED(2980, 160, 20, 140, 0, 160, 0),  true, false, GC_OK },
    { 0x0100A1B2C3D42000ULL, "Pixel Quest Deluxe",       12, 2, PLAYED(1210,  45,  0,  95, 2,  95, 0),  true, false, GC_OK },
    { 0x0100A1B2C3D43000ULL, "Dragon Valley Legends",    16, 3, PLAYED(6100, 120,  0,   0, 12,  0, 0),  true, false, GC_OK },
    { 0x0100A1B2C3D44000ULL, "Puzzle Garden",             3, 4, PLAYED( 380,  52,  0,  30, 5,  35, 0),  true, false, GC_OK },
    { 0x0100A1B2C3D4A000ULL, "Sky Kingdom",               7, 0, NOT_PLAYED,                             true, true,  GC_OK },
    { 0x0100A1B2C3D45000ULL, "",                         -1, -1, PLAYED(   0,   0, 15,  15, 0,   0, 0), false, false, GC_OK },
    // Then little played and left aside for months.
    { 0x0100A1B2C3D46000ULL, "Space Farm Tales",         -1, -1, PLAYED(  95,   4,  0,   0, 75,  0, 0), true, false, GC_NO_CONTROL },
    { 0x0100A1B2C3D47000ULL, "Garden Defenders",          7, 2, NOT_PLAYED, true, false, GC_NO_BASE },
    { 0x0100A1B2C3D48000ULL, "Hedgehog Rush",             7, 3, NOT_PLAYED, true, false, GC_ARCHIVED },
    { 0x0100A1B2C3D49000ULL, "Robot Factory",            12, 4, NOT_PLAYED, true, false, GC_FIRMWARE_TOO_OLD },
    { 0x0100A1B2C3D4B000ULL, "Ocean Explorer",            3, 1, NOT_PLAYED, true, false, GC_FILES_MISSING },
    { 0x0100A1B2C3D4C000ULL, "Castle Siege",             16, 2, NOT_PLAYED, true, false, GC_TICKET_MISSING },
    { 0x0100A1B2C3D4D000ULL, "Moon Racer",                3, 3, NOT_PLAYED, true, false, GC_LAUNCH_VERSION },
    // The longest name, for the screens that cut it; its data corrupt.
    { 0x0100A1B2C3D4E000ULL, "Forest Tales: Legends of the Ancient Grove for Nintendo Switch™",
                                                         12, 4, NOT_PLAYED, true, false, GC_NEEDS_VERIFY },
    { 0x0100A1B2C3D4F000ULL, "Star Pilots",              18, 0, NOT_PLAYED, true, false, GC_KEY_TOO_NEW },
};
const size_t SIM_GAME_COUNT = sizeof(SIM_GAMES) / sizeof(SIM_GAMES[0]);

const SimGame *sim_game(u64 id)
{
    for (size_t i = 0; i < SIM_GAME_COUNT; i++)
        if (SIM_GAMES[i].id == id) return &SIM_GAMES[i];
    return NULL;
}

u32 sim_played_today_min(void)
{
    u32 total = 0;
    for (size_t i = 0; i < SIM_GAME_COUNT; i++) total += SIM_GAMES[i].play.today_min;
    return total;
}

u64 sim_played_today_s(void)
{
    const u64 played = (u64)sim_played_today_min() * 60;
    const u64 skip = sim_skip_s();
    if (!skip || sim_env_flag("PLAYGUARD_SIM_IDLE")) return played;
    const time_t from = (time_t)sim_start_now(), to = (time_t)(sim_start_now() + skip);
    struct tm a, b;
    if (!localtime_r(&from, &a) || !localtime_r(&to, &b)) return played + skip;
    if (a.tm_year == b.tm_year && a.tm_yday == b.tm_yday) return played + skip;
    return (u64)(b.tm_hour * 3600 + b.tm_min * 60 + b.tm_sec);
}

unsigned char *sim_game_icon(u64 id, size_t *size)
{
    *size = 0;
    // A forwarder's: the icon it was installed with in this run, else one of
    // the made-up ones (sim_forwarder.cpp).
    if ((id >> 56) == 0x05) {
        unsigned char *own = sim_forwarder_icon(id, size);
        if (own) return own;
        const size_t k = (size_t)(id >> 12) % (sizeof(SIM_ICONS) / sizeof(SIM_ICONS[0]));
        unsigned char *jpeg = (unsigned char *)malloc(SIM_ICON_SIZES[k]);
        if (!jpeg) return NULL;
        memcpy(jpeg, SIM_ICONS[k], SIM_ICON_SIZES[k]);
        *size = SIM_ICON_SIZES[k];
        return jpeg;
    }
    const SimGame *g = sim_game(id);
    if (!g || g->icon < 0) return NULL;
    unsigned char *jpeg = (unsigned char *)malloc(SIM_ICON_SIZES[g->icon]);
    if (!jpeg) return NULL;
    memcpy(jpeg, SIM_ICONS[g->icon], SIM_ICON_SIZES[g->icon]);
    *size = SIM_ICON_SIZES[g->icon];
    return jpeg;
}
