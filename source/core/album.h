// album — the game each screenshot of the SD card's album was taken in, and
// that game's name, for the icon of a forwarder (Console › Forwarders). The
// album's file names carry the game in a form only the system reads: caps:a
// says it plainly (CapsAlbumFileId's application_id). album.c asks caps:a and
// ns on the console; the desktop build's sim_forwarder.cpp stands in.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once
#include "nx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// A screenshot by its file name's first 16 digits (YYYYMMDDHHMMSS, then the
// number of that second), and its game; app_id 0 when the system took it
// outside a game (the HOME menu, an applet).
typedef struct {
    char stamp[17];
    u64  app_id;
} AlbumShot;

// The SD card's screenshots (no videos), at most `max`, in no set order.
// 0 when caps:a cannot be asked: the screenshots then go unnamed.
size_t album_shots_of_games(AlbumShot *out, size_t max);

// The name of `app_id` in the console's language, as the HOME menu shows it;
// false, `out` empty, when ns knows none (a game deleted since).
bool album_game_name(u64 app_id, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif
