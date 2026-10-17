// PlayGuard — installed games check, console side (see gamecheck.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gamecheck.h"

#include <stdlib.h>
#include <string.h>

#include "nacp.h"
#include "write_guard.h"

#include <time.h>

#define RECORD_CHUNK  64
#define META_CHUNK    16
#define CONTENT_CHUNK 16
#define RECORD_METAS  64      // contents in one HOME menu entry, at most
#define VERIFY_BUFFER 0x100000   // what qlaunch gives the integrity check

// ns ApplicationRecord's LastEvent for an entry with its contents present
// (libnx-ext: NsExtApplicationEvent_Present), as Goldleaf and sphaira push it.
#define RECORD_EVENT_PRESENT 3

// The storages a game can be installed on, each opened once per scan.
typedef struct {
    NcmStorageId id;
    bool tried, ok;
    NcmContentMetaDatabase db;
    NcmContentStorage cs;
} Storage;

static Storage *storage_for(Storage *list, size_t n, NcmStorageId id)
{
    for (size_t i = 0; i < n; i++) {
        Storage *s = &list[i];
        if (s->id != id) continue;
        if (!s->tried) {
            s->tried = true;
            if (R_SUCCEEDED(ncmOpenContentMetaDatabase(&s->db, id))) {
                if (R_SUCCEEDED(ncmOpenContentStorage(&s->cs, id))) s->ok = true;
                else ncmContentMetaDatabaseClose(&s->db);
            }
        }
        return s->ok ? s : NULL;
    }
    return NULL;
}

static void storages_close(Storage *list, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (!list[i].ok) continue;
        ncmContentStorageClose(&list[i].cs);
        ncmContentMetaDatabaseClose(&list[i].db);
    }
}

// The tickets on the console (es: the common and the personalized ones),
// listed once per scan. Their rights IDs, in no particular order.
typedef struct {
    FsRightsId *ids;
    u32 count;
} Tickets;

static Result list_tickets(Service *es, u32 count_cmd, u32 list_cmd, Tickets *t)
{
    u32 count = 0;
    Result rc = serviceDispatchOut(es, count_cmd, count);
    if (R_FAILED(rc) || !count) return rc;
    FsRightsId *grown = (FsRightsId *)realloc(t->ids, (t->count + count) * sizeof(FsRightsId));
    if (!grown) return MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
    t->ids = grown;
    u32 written = 0;
    rc = serviceDispatchOut(es, list_cmd, written,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { t->ids + t->count, count * sizeof(FsRightsId) } },
    );
    if (R_SUCCEEDED(rc)) t->count += written < count ? written : count;
    return rc;
}

// es commands (switchbrew ES services; libnx-ext es.c): CountCommonTicket 9,
// CountPersonalizedTicket 10, ListCommonTicket 11, ListPersonalizedTicket 12.
static Result tickets_read(Tickets *t)
{
    memset(t, 0, sizeof(*t));
    Service es;
    Result rc = smGetService(&es, "es");
    if (R_FAILED(rc)) return rc;
    rc = list_tickets(&es, 9, 11, t);
    if (R_SUCCEEDED(rc)) rc = list_tickets(&es, 10, 12, t);
    serviceClose(&es);
    return rc;
}

static bool tickets_has(const Tickets *t, const FsRightsId *id)
{
    for (u32 i = 0; i < t->count; i++)
        if (memcmp(&t->ids[i], id, sizeof(*id)) == 0) return true;
    return false;
}

static bool rights_id_set(const FsRightsId *id)
{
    for (size_t i = 0; i < sizeof(id->c); i++)
        if (id->c[i]) return true;
    return false;
}

// Where the last gamecheck_check() spent its time (gamecheck_last_timing).
static GameCheckTiming s_timing;

static u64 now_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (u64)t.tv_sec * 1000000000ULL + (u64)t.tv_nsec;
}

void gamecheck_last_timing(GameCheckTiming *out)
{
    *out = s_timing;
}

// What one scan keeps open.
typedef struct {
    Storage storages[2];
    size_t storage_count;   // 0 without ncm
    const Tickets *tickets;   // NULL when they could not be listed
    bool avm;
    u32 title_index;          // the console language's title entry (nacp_console_language)
} Scan;

// One content (the game or its update) on its storage: known there, its
// files present, the system version and key generation it asks for, its
// ticket.
static void check_meta(GameFacts *g, const NsApplicationContentMetaStatus *m, Scan *sc)
{
    Storage *s = storage_for(sc->storages, sc->storage_count, (NcmStorageId)m->storageID);
    if (!s) {
        g->storage_unavailable = true;
        return;
    }
    NcmContentMetaKey key;
    memset(&key, 0, sizeof(key));
    key.id           = m->application_id;   // the content's own ID (an update's ends in 800)
    key.version      = m->version;
    key.type         = m->meta_type;
    key.install_type = NcmContentInstallType_Full;
    bool has = false;
    if (R_FAILED(ncmContentMetaDatabaseHas(&s->db, &has, &key)) || !has) {
        g->meta_missing = true;
        return;
    }
    u32 required = 0;
    if (R_SUCCEEDED(ncmContentMetaDatabaseGetRequiredSystemVersion(&s->db, &required, &key))) {
        u32 *field = m->meta_type == NcmContentMetaType_Patch ? &g->patch_required_hos : &g->base_required_hos;
        const u32 hos = gamecheck_sysver_to_hos(required);
        if (hos > *field) *field = hos;
    }
    NcmContentInfo infos[CONTENT_CHUNK];
    for (s32 start = 0;;) {
        s32 got = 0;
        if (R_FAILED(ncmContentMetaDatabaseListContentInfo(&s->db, &got, infos, CONTENT_CHUNK, &key, start)) || got <= 0) break;
        for (s32 i = 0; i < got; i++) {
            bool present = false;
            if (R_FAILED(ncmContentStorageHas(&s->cs, &present, &infos[i].content_id)) || !present) {
                g->files_missing = true;
                continue;
            }
            // The key the program is encrypted with (the other files of a
            // game share it): a title key needs a ticket. Reading it opens
            // the file, so only the program's is read.
            if (infos[i].content_type != NcmContentType_Program) continue;
            NcmRightsId rights;
            memset(&rights, 0, sizeof(rights));
            if (R_FAILED(ncmContentStorageGetRightsIdFromContentId(&s->cs, &rights, &infos[i].content_id, FsContentAttributes_All)))
                continue;
            if (rights.key_generation > g->key_generation) g->key_generation = rights.key_generation;
            if (!rights_id_set(&rights.rights_id)) continue;
            g->needs_ticket = true;
            if (sc->tickets && !tickets_has(sc->tickets, &rights.rights_id)) g->ticket_missing = true;
        }
        start += got;
        if (got < CONTENT_CHUNK) break;
    }
}

// A game's name and icon: ns' cache first, what the HOME menu itself draws
// (a few ms), then the storage only when the cache has none (a quarter of a
// second a game: most of a whole check when every game went that way). The
// name as the Activity tab reads it (nacp.h: compressed titles of 21.0.0+
// included).
static void check_game(GameFacts *g, NsApplicationControlData *cd, Scan *sc)
{
    u64 t = now_ns();
    u64 size = 0;
    if (cd && nacp_read_control(g->app_id, cd, true, &size) &&
        nacp_name(&cd->nacp, sizeof(cd->nacp), sc->title_index, g->name, sizeof(g->name))) {
        g->control_ok = true;
        nacp_copy_utf8(g->display_version, sizeof(g->display_version), cd->nacp.display_version);
    }
    u64 t2 = now_ns();
    s_timing.control_ns += t2 - t;
    t = t2;

    NsApplicationContentMetaStatus metas[META_CHUNK];
    bool all_card = true;
    for (s32 index = 0;;) {
        s32 got = 0;
        if (R_FAILED(nsListApplicationContentMetaStatus(g->app_id, index, metas, META_CHUNK, &got)) || got <= 0) break;
        for (s32 i = 0; i < got; i++) {
            const NsApplicationContentMetaStatus *m = &metas[i];
            g->meta_count++;
            if (g->content_count < GAMECHECK_MAX_CONTENTS) {
                GameContent *c = &g->contents[g->content_count++];
                c->id      = m->application_id;
                c->version = m->version;
                c->type    = m->meta_type;
                c->storage = m->storageID;
            }
            if (m->storageID != NcmStorageId_GameCard) all_card = false;
            const bool base  = m->meta_type == NcmContentMetaType_Application;
            const bool patch = m->meta_type == NcmContentMetaType_Patch;
            if (base) {
                g->has_base = true;
                g->base_version = m->version;
            }
            if (patch) {
                g->has_patch = true;
                g->patch_version = m->version;
            }
            // A card may be out of its slot: only the console's memory and
            // the SD card are checked. Add-ons do not stop the game from
            // starting.
            if ((base || patch) && sc->storage_count &&
                (m->storageID == NcmStorageId_BuiltInUser || m->storageID == NcmStorageId_SdCard))
                check_meta(g, m, sc);
        }
        index += got;
        if (got < META_CHUNK) break;
    }
    g->card_only = g->meta_count > 0 && all_card;
    t2 = now_ns();
    s_timing.contents_ns += t2 - t;
    t = t2;

    g->launch_rc = nsCheckApplicationLaunchVersion(g->app_id);
    // The version the console wants before it starts the game (6.0.0+):
    // only asked when it refuses to, the one case it explains.
    if (sc->avm && R_FAILED(g->launch_rc) && R_SUCCEEDED(avmGetLaunchRequiredVersion(g->app_id, &g->launch_required)))
        g->launch_checked = true;
    s_timing.state_ns += now_ns() - t;
}

// How the HOME menu sees the games (3.0.0+), VIEW_CHUNK at a time: one
// request for many, read before they are checked.
#define VIEW_CHUNK 32

static void read_views(const GameRecord *records, u32 count, GameFacts *out)
{
    if (!hosversionAtLeast(3, 0, 0)) return;
    const u64 t = now_ns();
    NsApplicationView *views = (NsApplicationView *)malloc(VIEW_CHUNK * sizeof(NsApplicationView));
    u64 ids[VIEW_CHUNK];
    if (!views) return;
    for (u32 start = 0; start < count; start += VIEW_CHUNK) {
        const u32 n = count - start < VIEW_CHUNK ? count - start : VIEW_CHUNK;
        for (u32 i = 0; i < n; i++) ids[i] = records[start + i].app_id;
        memset(views, 0, n * sizeof(NsApplicationView));
        if (R_FAILED(nsGetApplicationView(views, ids, (s32)n))) continue;
        for (u32 i = 0; i < n; i++) {
            if (views[i].application_id != ids[i]) continue;
            out[start + i].view_ok    = true;
            out[start + i].view_flags = views[i].flags;
        }
    }
    free(views);
    s_timing.state_ns += now_ns() - t;
}

Result gamecheck_records(GameRecord *out, u32 max, u32 *count, bool *truncated)
{
    *count = 0;
    *truncated = false;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    NsApplicationRecord records[RECORD_CHUNK];
    for (s32 offset = 0;;) {
        s32 got = 0;
        const Result list_rc = nsListApplicationRecord(records, RECORD_CHUNK, offset, &got);
        if (R_FAILED(list_rc)) {
            if (offset == 0) rc = list_rc;
            break;
        }
        if (got <= 0) break;
        for (s32 i = 0; i < got; i++) {
            if (*count == max) {
                *truncated = true;
                break;
            }
            GameRecord *r = &out[(*count)++];
            r->app_id  = records[i].application_id;
            r->updated = records[i].last_updated;
            r->event   = records[i].last_event;
        }
        offset += got;
        if (got < RECORD_CHUNK || *truncated) break;
    }
    nsExit();
    return rc;
}

u32 gamecheck_check(const GameRecord *records, u32 count, GameFacts *out,
                    bool (*on_game)(void *ctx, const GameFacts *game, u32 done), void *ctx)
{
    memset(out, 0, (size_t)count * sizeof(*out));
    memset(&s_timing, 0, sizeof(s_timing));
    if (!count || R_FAILED(nsInitialize())) return 0;
    const u64 started = now_ns();

    Scan sc;
    memset(&sc, 0, sizeof(sc));
    Tickets tickets;
    if (R_SUCCEEDED(tickets_read(&tickets))) sc.tickets = &tickets;
    sc.avm = hosversionAtLeast(6, 0, 0) && R_SUCCEEDED(avmInitialize());
    // Without ncm, only what ns says: the files are not checked.
    const bool ncm = R_SUCCEEDED(ncmInitialize());
    sc.storages[0].id = NcmStorageId_BuiltInUser;
    sc.storages[1].id = NcmStorageId_SdCard;
    sc.storage_count = ncm ? 2 : 0;
    nacp_console_language(&sc.title_index);
    NsApplicationControlData *cd = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    read_views(records, count, out);
    s_timing.setup_ns = now_ns() - started;
    u32 done = 0;
    while (done < count) {
        GameFacts *g = &out[done];
        g->app_id         = records[done].app_id;
        g->record_updated = records[done].updated;
        g->record_event   = records[done].event;
        check_game(g, cd, &sc);
        done++;
        if (on_game && !on_game(ctx, g, done)) break;
    }
    free(cd);
    if (ncm) {
        storages_close(sc.storages, sc.storage_count);
        ncmExit();
    }
    if (sc.avm) avmExit();
    free(tickets.ids);
    nsExit();
    s_timing.games = done;
    s_timing.total_ns = now_ns() - started;
    return done;
}

// --------------------------------------------------------------- changes

// ns IApplicationManagerInterface commands libnx has no wrapper for
// (switchbrew NS services; libnx-ext ns-ext.c): ListApplicationRecordContentMeta
// 17, DeleteApplicationRecord 27, PushApplicationRecord 16. A content of an
// entry: its key and its storage (0x18 bytes).
typedef struct {
    NcmContentMetaKey key;
    u8 storage_id;
    u8 pad[7];
} RecordMeta;

static Result app_manager(Service *out, Service **use)
{
    if (hosversionAtLeast(3, 0, 0)) {
        *use = out;
        return nsGetApplicationManagerInterface(out);
    }
    *use = nsGetServiceSession_ApplicationManagerInterface();
    return 0;
}

static void app_manager_close(Service *srv)
{
    if (hosversionAtLeast(3, 0, 0)) serviceClose(srv);
}

static Result record_list(Service *am, u64 app_id, RecordMeta *out, s32 max, s32 *count)
{
    const struct {
        u64 offset;
        u64 app_id;
    } in = { 0, app_id };
    *count = 0;
    return serviceDispatchInOut(am, 17, in, *count,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out, (size_t)max * sizeof(RecordMeta) } },
    );
}

static Result record_delete(Service *am, u64 app_id)
{
    return serviceDispatchIn(am, 27, app_id);
}

static Result record_push(Service *am, u64 app_id, const RecordMeta *metas, s32 count)
{
    const struct {
        u8  last_event;
        u8  pad[7];
        u64 app_id;
    } in = { RECORD_EVENT_PRESENT, { 0 }, app_id };
    return serviceDispatchIn(am, 16, in,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { metas, (size_t)count * sizeof(RecordMeta) } },
    );
}

Result gamecheck_remove(u64 app_id)
{
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    rc = nsDeleteApplicationCompletely(app_id);
    // An entry with nothing installed behind it (the dotted frame) may not
    // go that way: then the entry alone, as Goldleaf does. Never while
    // something is installed, which would stay behind with no entry.
    if (R_FAILED(rc)) {
        NsApplicationContentMetaStatus status;
        s32 installed = 0;
        Service srv, *am = NULL;
        if (R_SUCCEEDED(nsListApplicationContentMetaStatus(app_id, 0, &status, 1, &installed)) && installed == 0 &&
            R_SUCCEEDED(app_manager(&srv, &am))) {
            if (R_SUCCEEDED(record_delete(am, app_id))) rc = 0;
            app_manager_close(&srv);
        }
    }
    nsExit();
    return rc;
}

// Deletes a content's files and its key from its storage.
static Result ncm_delete(const RecordMeta *m)
{
    NcmContentMetaDatabase db;
    NcmContentStorage cs;
    Result rc = ncmOpenContentMetaDatabase(&db, (NcmStorageId)m->storage_id);
    if (R_FAILED(rc)) return rc;
    rc = ncmOpenContentStorage(&cs, (NcmStorageId)m->storage_id);
    if (R_FAILED(rc)) {
        ncmContentMetaDatabaseClose(&db);
        return rc;
    }
    NcmContentInfo infos[CONTENT_CHUNK];
    for (s32 start = 0; R_SUCCEEDED(rc);) {
        s32 got = 0;
        if (R_FAILED(ncmContentMetaDatabaseListContentInfo(&db, &got, infos, CONTENT_CHUNK, &m->key, start)) || got <= 0) break;
        for (s32 i = 0; i < got && R_SUCCEEDED(rc); i++) {
            bool present = false;
            if (R_SUCCEEDED(ncmContentStorageHas(&cs, &present, &infos[i].content_id)) && present)
                rc = ncmContentStorageDelete(&cs, &infos[i].content_id);
        }
        start += got;
        if (got < CONTENT_CHUNK) break;
    }
    if (R_SUCCEEDED(rc)) rc = ncmContentMetaDatabaseRemove(&db, &m->key);
    if (R_SUCCEEDED(rc)) rc = ncmContentMetaDatabaseCommit(&db);
    ncmContentStorageClose(&cs);
    ncmContentMetaDatabaseClose(&db);
    return rc;
}

Result gamecheck_remove_content(u64 app_id, const GameContent *content)
{
    if (!content || (content->type != GC_META_PATCH && content->type != GC_META_ADDON) ||
        (content->storage != GC_STORAGE_NAND && content->storage != GC_STORAGE_SD))
        return NXM_RC_INVALID_ARGUMENT;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    rc = ncmInitialize();
    if (R_FAILED(rc)) {
        nsExit();
        return rc;
    }
    Service srv, *am = NULL;
    rc = app_manager(&srv, &am);
    if (R_FAILED(rc)) {
        ncmExit();
        nsExit();
        return rc;
    }

    // The entry as it is: the content must be in it.
    RecordMeta *metas = (RecordMeta *)calloc(RECORD_METAS, sizeof(RecordMeta));
    s32 count = 0, found = -1;
    if (!metas) rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
    if (R_SUCCEEDED(rc)) rc = record_list(am, app_id, metas, RECORD_METAS, &count);
    if (count > RECORD_METAS) count = RECORD_METAS;
    for (s32 i = 0; R_SUCCEEDED(rc) && i < count; i++)
        if (metas[i].key.id == content->id && metas[i].key.version == content->version &&
            metas[i].key.type == content->type && metas[i].storage_id == content->storage)
            found = i;
    if (R_SUCCEEDED(rc) && found < 0) rc = NXM_RC_STATE_UNKNOWN;

    // Its files and its key, then the entry without it.
    if (R_SUCCEEDED(rc)) rc = ncm_delete(&metas[found]);
    if (R_SUCCEEDED(rc)) {
        metas[found] = metas[count - 1];
        count--;
        rc = record_delete(am, app_id);
        if (R_SUCCEEDED(rc) && count > 0) rc = record_push(am, app_id, metas, count);
    }
    // Without its update, the game is launched at the version left (the
    // console would ask for the removed update otherwise). A failure here
    // shows on the next scan, with the offer to reset it.
    if (R_SUCCEEDED(rc) && content->type == GC_META_PATCH && hosversionAtLeast(6, 0, 0)) {
        u32 left = 0;
        for (s32 i = 0; i < count; i++)
            if ((metas[i].key.type == NcmContentMetaType_Application || metas[i].key.type == NcmContentMetaType_Patch) &&
                metas[i].key.version > left)
                left = metas[i].key.version;
        if (R_SUCCEEDED(avmInitialize())) {
            avmPushLaunchVersion(app_id, left);
            avmExit();
        }
    }
    free(metas);
    app_manager_close(&srv);
    ncmExit();
    nsExit();
    return rc;
}

Result gamecheck_reset_launch_version(u64 app_id, u32 version)
{
    if (!hosversionAtLeast(6, 0, 0)) return NXM_RC_FW_UNSUPPORTED;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    Result rc = avmInitialize();
    if (R_FAILED(rc)) return rc;
    rc = avmPushLaunchVersion(app_id, version);
    u32 now = 0;
    if (R_SUCCEEDED(rc) && R_SUCCEEDED(avmGetLaunchRequiredVersion(app_id, &now)) && now > version) rc = NXM_RC_NOT_APPLIED;
    avmExit();
    return rc;
}

Result gamecheck_verify(u64 app_id, bool (*progress)(void *ctx, u64 current, u64 total), void *ctx, bool *corrupt,
                        Result *found)
{
    *corrupt = false;
    *found = 0;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    void *buffer = aligned_alloc(0x1000, VERIFY_BUFFER);
    if (!buffer) {
        nsExit();
        return MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
    }
    NsProgressAsyncResult async;
    rc = nsRequestVerifyApplication(&async, app_id, 0x7, buffer, VERIFY_BUFFER);   // 0x7: what qlaunch passes
    if (R_SUCCEEDED(rc)) {
        // Until it is done, the progress four times a second.
        bool cancelled = false;
        while (nsProgressAsyncResultWait(&async, 250000000ULL) == KERNELRESULT(TimedOut)) {
            NsSystemUpdateProgress p;
            memset(&p, 0, sizeof(p));
            if (R_FAILED(nsProgressAsyncResultGetProgress(&async, &p, sizeof(p)))) memset(&p, 0, sizeof(p));
            if (progress && !progress(ctx, p.current_size > 0 ? (u64)p.current_size : 0, p.total_size > 0 ? (u64)p.total_size : 0)) {
                cancelled = true;
                nsProgressAsyncResultCancel(&async);
                break;
            }
        }
        if (cancelled) {
            rc = NXM_RC_CANCELLED;
        } else {
            // What was found comes as the check's result itself (2016-0601,
            // seen on a console) or as its detail (an fs result).
            rc = nsProgressAsyncResultGet(&async);
            const Result detail = nsProgressAsyncResultGetDetailResult(&async);
            if (gamecheck_is_corruption(rc) || (R_SUCCEEDED(rc) && R_FAILED(detail))) {
                *corrupt = true;
                *found = R_FAILED(detail) ? detail : rc;
                rc = 0;
            }
        }
        nsProgressAsyncResultClose(&async);
    }
    free(buffer);
    nsExit();
    return rc;
}
