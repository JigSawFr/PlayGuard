// PlayGuard — the service-free part of the play data layer (see playstats_logic.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "playstats_logic.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool playstats_convert(const PdmPlayEvent *p, PlayLogEvent *e)
{
    e->app_id    = 0;
    e->ts_user   = p->timestamp_user;
    e->ts_steady = p->timestamp_steady;
    e->uid[0] = e->uid[1] = 0;

    if (p->play_event_type == PdmPlayEventType_Account) {
        // 0: the account is opened (picked in the game), 1: closed. The u32
        // halves are stored swapped in each u64, as for the ProgramId below.
        const u8 type = p->event_data.account.type;
        if (type > 1) return false;
        const u32 *w = p->event_data.account.uid;
        e->uid[0] = ((u64)w[0] << 32) | w[1];
        e->uid[1] = ((u64)w[2] << 32) | w[3];
        e->kind   = type == 0 ? PlayLogEv_AccountOpen : PlayLogEv_AccountClose;
        return true;
    }
    if (p->play_event_type == PdmPlayEventType_PowerStateChange) {   // sleep, wake, shutdown
        e->kind = PlayLogEv_Away;
        return true;
    }
    if (p->play_event_type != PdmPlayEventType_Applet) return false;

    const u8 type = p->event_data.applet.event_type;
    if (p->event_data.applet.applet_id != AppletId_application) {
        // The HOME menu taking the focus: no game has it any more (this is
        // what ends a session whose game crashed without "out of focus").
        if (p->event_data.applet.applet_id == AppletId_SystemAppletMenu && type == PdmAppletEventType_InFocus) {
            e->kind = PlayLogEv_Away;
            return true;
        }
        return false;
    }
    // Same filter as the system's own play statistics.
    if (p->event_data.applet.log_policy != PdmPlayLogPolicy_All) return false;
    // The two halves of the ProgramId are stored swapped.
    e->app_id = ((u64)p->event_data.applet.program_id[0] << 32) | p->event_data.applet.program_id[1];
    switch (type) {
        case PdmAppletEventType_InFocus:
            e->kind = PlayLogEv_Focus;
            return true;
        case PdmAppletEventType_OutOfFocus:
        case PdmAppletEventType_OutOfFocus4:
            e->kind = PlayLogEv_Unfocus;   // the HOME menu in front: it waits in the background
            return true;
        case PdmAppletEventType_Exit:
        case PdmAppletEventType_Exit5:
        case PdmAppletEventType_Exit6:
            e->kind = PlayLogEv_Exit;
            return true;
        case PdmAppletEventType_Launch:   // a new start: whatever was in focus is over,
            e->app_id = 0;                // and the accounts the previous game had open
            e->kind   = PlayLogEv_Launch;
            return true;
        default:
            return false;
    }
}

// A whole chunk, not the first old entry: a clock set back makes the user
// times go back and forth. Only the last week matters; the log holds years.
s32 playstats_first_recent_index(PlayEventQuery query, s32 start, s32 total, u64 since, PdmPlayEvent *chunk)
{
    s32 hi = start + total;   // one past the newest entry
    while (hi > start) {
        const s32 lo = hi - PLAYSTATS_EVENT_CHUNK > start ? hi - PLAYSTATS_EVENT_CHUNK : start;
        s32 got = 0;
        if (R_FAILED(query(lo, chunk, hi - lo, &got)) || got <= 0) return start;   // all of it, then
        bool recent = false;
        for (s32 i = 0; i < got && !recent; i++) recent = chunk[i].timestamp_user >= since;
        if (!recent) return hi;
        hi = lo;
    }
    return start;
}

Result playstats_collect_events(PlayEventQuery query, s32 start, s32 total, u64 since, size_t max,
                                PlayLogEvent **events, size_t *count, bool *capped)
{
    *events = NULL;
    *count  = 0;
    *capped = false;
    if (total <= 0 || max == 0) return 0;
    PdmPlayEvent *chunk = (PdmPlayEvent *)malloc(sizeof(PdmPlayEvent) * PLAYSTATS_EVENT_CHUNK);
    if (!chunk) return MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
    s32 index = playstats_first_recent_index(query, start, total, since, chunk);
    // The clock set back months: every entry written since looks recent.
    // Only the newest, so the read stays bounded (and today stays right).
    if ((u64)(start + total - index) > max) {
        index   = (s32)((u64)start + (u64)total - max);
        *capped = true;
    }
    s32 remaining = start + total - index;
    size_t cap = 0;
    Result rc = 0;
    while (remaining > 0 && R_SUCCEEDED(rc)) {
        s32 got = 0;
        rc = query(index, chunk, remaining < PLAYSTATS_EVENT_CHUNK ? remaining : PLAYSTATS_EVENT_CHUNK, &got);
        if (R_FAILED(rc) || got <= 0) break;
        index += got;
        remaining -= got;
        for (s32 i = 0; i < got; i++) {
            PlayLogEvent e;
            if (!playstats_convert(&chunk[i], &e) || e.ts_user < since) continue;
            if (*count == max) {   // more than asked for after all (the log grew meanwhile)
                *capped   = true;
                remaining = 0;
                break;
            }
            if (*count == cap) {
                size_t grown = cap ? cap * 2 : 1024;
                if (grown > max) grown = max;
                PlayLogEvent *bigger = (PlayLogEvent *)realloc(*events, grown * sizeof(PlayLogEvent));
                if (!bigger) {
                    rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
                    break;
                }
                *events = bigger;
                cap = grown;
            }
            (*events)[(*count)++] = e;
        }
    }
    free(chunk);
    return rc;
}

// The whole log, a chunk at a time: only the IDs are kept, never the events.
size_t playstats_logged_games(PlayEventQuery query, s32 start, s32 total, u64 *ids, size_t max, Result *rc)
{
    *rc = 0;
    size_t n = 0;
    if (total <= 0 || max == 0) return 0;
    PdmPlayEvent *chunk = (PdmPlayEvent *)malloc(sizeof(PdmPlayEvent) * PLAYSTATS_EVENT_CHUNK);
    if (!chunk) {
        *rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
        return 0;
    }
    s32 index = start, remaining = total;
    while (remaining > 0 && n < max) {
        s32 got = 0;
        *rc = query(index, chunk, remaining < PLAYSTATS_EVENT_CHUNK ? remaining : PLAYSTATS_EVENT_CHUNK, &got);
        if (R_FAILED(*rc) || got <= 0) break;
        index += got;
        remaining -= got;
        for (s32 i = 0; i < got && n < max; i++) {
            PlayLogEvent e;
            if (!playstats_convert(&chunk[i], &e) || e.kind != PlayLogEv_Focus || !e.app_id) continue;
            bool seen = false;
            for (size_t k = 0; k < n && !seen; k++) seen = ids[k] == e.app_id;
            if (!seen) ids[n++] = e.app_id;
        }
    }
    free(chunk);
    return n;
}

void playstats_add_played(PlayStats *out, const u64 *ids, size_t count, PlayTotalsQuery query, void *ctx)
{
    for (size_t i = 0; i < count; i++) {
        bool seen = false;
        for (u32 k = 0; k < out->count && !seen; k++) seen = out->games[k].app_id == ids[i];
        if (seen) continue;
        GameStat g;
        memset(&g, 0, sizeof(g));
        g.app_id = ids[i];
        if (!query(ctx, ids[i], &g)) continue;
        // Full: one more played game is enough to say the list is cut.
        if (out->count == PLAYSTATS_MAX) {
            out->truncated = true;
            return;
        }
        out->games[out->count++] = g;
    }
}

void playnames_use_language(PlayNames *m, u8 lang)
{
    if (m->lang == lang) return;
    m->lang  = lang;
    m->count = 0;
}

const char *playnames_find(const PlayNames *m, u64 id)
{
    for (u32 i = 0; i < m->count; i++)
        if (m->e[i].id == id) return m->e[i].name;
    return NULL;
}

void playnames_add(PlayNames *m, u64 id, const char *name)
{
    if (!name[0] || playnames_find(m, id) || m->count == PLAYSTATS_MAX) return;
    m->e[m->count].id = id;
    nacp_copy_utf8(m->e[m->count].name, sizeof(m->e[m->count].name), name);
    m->count++;
}

void playnames_seed(PlayNames *m, const PlayStats *known)
{
    if (!known || !known->names_lang) return;
    if (m->lang == 0 && m->count == 0) m->lang = known->names_lang;   // the first read of the run
    if (m->lang != known->names_lang) return;   // this run's names, in the language of now
    for (u32 i = 0; i < known->count && i < PLAYSTATS_MAX; i++) {
        const GameStat *g = &known->games[i];
        if (nacp_utf8_valid(g->name, strnlen(g->name, sizeof(g->name)))) playnames_add(m, g->app_id, g->name);
    }
}

// ---------------------------------------------------------------- icon store

static bool icon_path(const IconStore *s, u64 id, const char *ext, char *path, size_t size)
{
    const int n = snprintf(path, size, "%s/%016llX%s", s->dir, (unsigned long long)id, ext);
    return n > 0 && (size_t)n < size;
}

// "<16 hex digits><ext>", as icon_path() names them.
static bool is_icon_file(const char *name, const char *ext)
{
    return strlen(name) == 16 + 4 && strcmp(name + 16, ext) == 0;
}

// Adds up the icons of the folder (with `remove_all`, deletes them); a .tmp
// left by a crash goes either way. Returns how many files were deleted.
static u32 scan_once(IconStore *s, bool remove_all)
{
    s->bytes = 0;
    s->files = 0;
    DIR *d = opendir(s->dir);
    if (!d) return 0;
    char path[sizeof(s->dir) + 32];
    u32 removed = 0;
    for (struct dirent *e; (e = readdir(d));) {
        const bool icon = is_icon_file(e->d_name, ".jpg");
        if (!icon && !is_icon_file(e->d_name, ".tmp")) continue;
        if (snprintf(path, sizeof(path), "%s/%s", s->dir, e->d_name) >= (int)sizeof(path)) continue;
        struct stat st;
        if (remove_all || !icon) {
            if (remove(path) == 0) removed++;
        } else if (stat(path, &st) == 0) {
            s->bytes += (u64)st.st_size;
            s->files++;
        }
    }
    closedir(d);
    return removed;
}

static void scan(IconStore *s, bool remove_all)
{
    // Again while it deletes: a listing may skip entries deleted under it.
    for (int pass = 0; pass < 4 && scan_once(s, remove_all) > 0; pass++) {}
}

// mkdir -p, for a path with no "." or ".." parts.
static bool make_dirs(char *path)
{
    for (char *p = path + 1;; p++) {
        if (*p != '/' && *p != '\0') continue;
        const char c = *p;
        *p = '\0';
        const bool ok = mkdir(path, 0777) == 0 || errno == EEXIST;
        *p = c;
        if (!ok) return false;
        if (c == '\0') return true;
    }
}

// A new file for writing, readable by all but writable by its owner only
// (fopen "wb" would ask for 0666).
static FILE *create_file(const char *path)
{
    const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return NULL;
    FILE *f = fdopen(fd, "wb");
    if (!f) close(fd);
    return f;
}

bool icon_store_open(IconStore *s, const char *dir, u8 lang, u64 max_bytes, u32 max_files)
{
    memset(s, 0, sizeof(*s));
    if (!dir || !dir[0] || strlen(dir) >= sizeof(s->dir)) return false;
    strcpy(s->dir, dir);
    if (!make_dirs(s->dir)) return false;
    s->lang      = lang;
    s->max_bytes = max_bytes;
    s->max_files = max_files;
    char path[sizeof(s->dir) + 8];
    snprintf(path, sizeof(path), "%s/lang", s->dir);
    int stored = -1;
    FILE *f = fopen(path, "rb");
    if (f) {
        stored = fgetc(f);
        fclose(f);
    }
    if (stored == (int)lang) scan(s, false);
    // Another language, or full: start again with the games played now.
    if (stored != (int)lang || s->bytes >= max_bytes || s->files >= max_files) {
        scan(s, true);
        s->bytes = 0;
        s->files = 0;
        f = create_file(path);
        if (!f) return false;
        const bool written = fputc(lang, f) != EOF;
        if (fclose(f) != 0 || !written) return false;
    }
    s->ready = true;
    return true;
}

static bool looks_like_jpeg(const unsigned char *p, size_t size)
{
    return size >= 4 && size <= PLAYSTATS_ICON_MAX && p[0] == 0xFF && p[1] == 0xD8;
}

unsigned char *icon_store_get(const IconStore *s, u64 id, size_t *size)
{
    *size = 0;
    char path[sizeof(s->dir) + 32];
    if (!s->ready || !icon_path(s, id, ".jpg", path, sizeof(path))) return NULL;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char *buf = NULL;
    long len = -1;
    if (fseek(f, 0, SEEK_END) == 0 && (len = ftell(f)) > 0 && len <= PLAYSTATS_ICON_MAX && fseek(f, 0, SEEK_SET) == 0)
        buf = (unsigned char *)malloc((size_t)len);
    if (buf && fread(buf, 1, (size_t)len, f) == (size_t)len && looks_like_jpeg(buf, (size_t)len)) {
        *size = (size_t)len;
    } else {
        free(buf);
        buf = NULL;
    }
    fclose(f);
    return buf;
}

void icon_store_put(IconStore *s, u64 id, const unsigned char *jpeg, size_t size)
{
    char tmp[sizeof(s->dir) + 32], path[sizeof(s->dir) + 32];
    if (!s->ready || !looks_like_jpeg(jpeg, size) || s->files >= s->max_files || s->bytes + size > s->max_bytes ||
        !icon_path(s, id, ".tmp", tmp, sizeof(tmp)) || !icon_path(s, id, ".jpg", path, sizeof(path)))
        return;
    // Written aside, then renamed: a file cut by a crash is never read as an icon.
    FILE *f = create_file(tmp);
    if (!f) return;
    const bool written = fwrite(jpeg, 1, size, f) == size;
    if (fclose(f) != 0 || !written) {
        remove(tmp);
        return;
    }
    remove(path);   // the console's rename does not replace a file
    if (rename(tmp, path) != 0) {
        remove(tmp);
        return;
    }
    s->bytes += size;
    s->files++;
}
