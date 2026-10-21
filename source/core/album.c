// PlayGuard — the album's screenshots and their games, the console side (see
// album.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "album.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nacp.h"

// What is read of the album at most: far past the screenshots an SD card
// holds (0x20 bytes each).
#define ALBUM_MAX 20000

size_t album_shots_of_games(AlbumShot *out, size_t max)
{
    if (!out || !max || R_FAILED(capsaInitialize())) return 0;
    size_t n = 0;
    u64 count = 0;
    if (R_SUCCEEDED(capsaGetAlbumFileCount(CapsAlbumStorage_Sd, &count)) && count > 0) {
        if (count > ALBUM_MAX) count = ALBUM_MAX;
        CapsAlbumEntry *entries = (CapsAlbumEntry *)calloc((size_t)count, sizeof(CapsAlbumEntry));
        u64 got = 0;
        if (entries && R_SUCCEEDED(capsaGetAlbumFileList(CapsAlbumStorage_Sd, &got, entries, count))) {
            if (got > count) got = count;
            for (u64 i = 0; i < got && n < max; i++) {
                const CapsAlbumFileId *f = &entries[i].file_id;
                if (f->content != CapsAlbumFileContents_ScreenShot) continue;
                const CapsAlbumFileDateTime *t = &f->datetime;
                AlbumShot *s = &out[n++];
                snprintf(s->stamp, sizeof(s->stamp), "%04u%02u%02u%02u%02u%02u%02u", (unsigned)t->year % 10000,
                         (unsigned)t->month % 100, (unsigned)t->day % 100, (unsigned)t->hour % 100,
                         (unsigned)t->minute % 100, (unsigned)t->second % 100, (unsigned)t->id % 100);
                s->app_id = f->application_id;
            }
        }
        free(entries);
    }
    capsaExit();
    return n;
}

bool album_game_name(u64 app_id, char *out, size_t out_size)
{
    if (!out || !out_size) return false;
    out[0] = '\0';
    if (!app_id || R_FAILED(nsInitialize())) return false;
    u32 title_index = 0;
    nacp_console_language(&title_index);
    NsApplicationControlData *cd = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    u64 size = 0;
    const bool ok = cd && nacp_read_control(app_id, cd, false, &size) &&
                    nacp_name(&cd->nacp, sizeof(cd->nacp), title_index, out, out_size);
    free(cd);
    nsExit();
    return ok;
}
