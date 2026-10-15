// Host tests for source/core/gamecheck.c and gamecheck_rules.c, against a
// scripted console (ns, ncm, es, avm): what the scan reads of each game and
// the problem it becomes (in order), tickets, key generations, the launch
// version, the HOME menu's view flags, every session closed, paging, ncm or
// es missing, a storage that cannot be opened; the actions offered; the
// removals (the whole game, an update or an add-on with the HOME menu entry
// rebuilt), the launch version reset and the integrity check, each behind
// the read-only and PIN gates; the system-version and key-generation tables.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gamecheck.h"
#include "write_guard.h"

// ------------------------------------------------------------ the console

#define MAX_GAMES   700
#define MAX_METAS   24
#define MAX_TICKETS 16

typedef struct {
    u8  type;
    u8  storage;
    u32 version;
    u64 id;
    bool registered;          // known to the storage's meta database
    bool installed;           // listed by ns (removed: no longer)
    u32 required_sysver;      // RequiredSystemVersion
    int files;                // contents listed for it
    int files_present;        // how many of them the storage has
    int files_deleted;
    u8  key_generation;
    bool titlekey;            // encrypted with a title key: rights id set
} FakeMeta;

typedef struct {
    u64 id;
    const char *name;         // NULL: no control data
    u8 record_event;
    bool record;              // the HOME menu entry exists
    int meta_count;
    FakeMeta metas[MAX_METAS];
    Result launch_rc;
    u32 view_flags;
    bool launch_known;        // avm knows a launch required version
    u32 launch_required;
} FakeGame;

typedef struct { u64 id; u32 version; u8 type; u8 install_type; u8 pad[2]; u8 storage; u8 pad2[7]; } RecordMetaWire;

static struct {
    FakeGame games[MAX_GAMES];
    int count;
    u32 hos;
    Result ns_init_rc, list_rc, ncm_init_rc, es_rc, avm_init_rc, view_rc, delete_rc, verify_rc;
    bool sd_unopenable, avm_ignores_push, delete_leaves_record;
    FsRightsId tickets[MAX_TICKETS];
    int ticket_count;
    int ns_open, ncm_open, db_open, cs_open, avm_open, es_open, am_open, async_open;
    int ns_inits, commits, pushes, record_deletes, avm_pushes;
    u64 deleted;
    RecordMetaWire pushed[MAX_METAS];
    int pushed_count;
    int verify_waits, progress_calls;
    bool verify_corrupt;
} C;

static void reset(void)
{
    memset(&C, 0, sizeof(C));
    C.hos = MAKEHOSVERSION(22, 1, 0);
}

bool hosversionAtLeast(u8 major, u8 minor, u8 micro)
{
    return C.hos >= MAKEHOSVERSION(major, minor, micro);
}

static FakeGame *add_game(u64 id, const char *name)
{
    FakeGame *g = &C.games[C.count++];
    memset(g, 0, sizeof(*g));
    g->id = id;
    g->name = name;
    g->record = true;
    g->record_event = 3;
    return g;
}

static FakeMeta *add_meta(FakeGame *g, u8 type, u8 storage, u32 version)
{
    FakeMeta *m = &g->metas[g->meta_count++];
    memset(m, 0, sizeof(*m));
    m->type = type;
    m->storage = storage;
    m->version = version;
    m->id = type == NcmContentMetaType_Patch ? g->id | 0x800 : type == NcmContentMetaType_AddOnContent ? (g->id & ~0xFFFULL) + 0x1000 + (u64)g->meta_count : g->id;
    m->registered = m->installed = true;
    m->files = m->files_present = 2;
    m->key_generation = 0x10;
    return m;
}

static FakeGame *find(u64 id)
{
    for (int i = 0; i < C.count; i++)
        if (C.games[i].id == id) return &C.games[i];
    return NULL;
}

static FakeMeta *meta_of(NcmStorageId storage, const NcmContentMetaKey *key)
{
    for (int i = 0; i < C.count; i++)
        for (int j = 0; j < C.games[i].meta_count; j++) {
            FakeMeta *m = &C.games[i].metas[j];
            if (m->storage == storage && m->id == key->id && m->version == key->version && m->type == key->type &&
                key->install_type == NcmContentInstallType_Full)
                return m;
        }
    return NULL;
}

// A content's rights id: the meta's id, then its key generation (as a title
// key ticket's rights id ends with it).
static FsRightsId rights_of(const FakeMeta *m)
{
    FsRightsId r;
    memset(&r, 0, sizeof(r));
    memcpy(r.c, &m->id, sizeof(m->id));
    r.c[15] = m->key_generation;
    return r;
}

static void add_ticket(const FakeMeta *m)
{
    C.tickets[C.ticket_count++] = rights_of(m);
}

// Content IDs: bytes 0..7 the meta's address in the console, byte 8 the file.
static NcmContentId content_id_of(FakeMeta *m, int file)
{
    NcmContentId id;
    memset(&id, 0, sizeof(id));
    memcpy(id.c, &m, sizeof(m));
    id.c[8] = (u8)file;
    return id;
}

static FakeMeta *meta_of_content(const NcmContentId *id, int *file)
{
    FakeMeta *m;
    memcpy(&m, id->c, sizeof(m));
    *file = id->c[8];
    return m;
}

// ------------------------------------------------------------ ns

Result nsInitialize(void)
{
    C.ns_inits++;
    if (C.ns_init_rc) return C.ns_init_rc;
    C.ns_open++;
    return 0;
}

void nsExit(void)
{
    assert(C.ns_open > 0);
    assert(C.am_open == 0 && C.async_open == 0);   // what hangs off ns first
    C.ns_open--;
}

Result nsListApplicationRecord(NsApplicationRecord *records, s32 count, s32 entry_offset, s32 *out)
{
    assert(C.ns_open == 1);
    if (C.list_rc) return C.list_rc;
    s32 n = 0;
    for (s32 i = entry_offset; i < C.count && n < count; i++, n++) {
        memset(&records[n], 0, sizeof(records[n]));
        records[n].application_id = C.games[i].id;
        records[n].last_event = C.games[i].record_event;
    }
    *out = n;
    return 0;
}

Result nsGetApplicationControlData(NsApplicationControlSource source, u64 id, NsApplicationControlData *buffer, size_t size,
                                   u64 *actual)
{
    assert(C.ns_open == 1 && source == NsApplicationControlSource_Storage && size == sizeof(*buffer));
    const FakeGame *g = find(id);
    if (!g || !g->name) return 0x1234;
    memset(&buffer->nacp, 0, sizeof(buffer->nacp));
    snprintf(buffer->nacp.lang[0].name, sizeof(buffer->nacp.lang[0].name), "%s", g->name);
    *actual = sizeof(buffer->nacp) + 0x1000;
    return 0;
}

Result nacpGetLanguageEntry(NacpStruct *nacp, NacpLanguageEntry **entry)
{
    *entry = &nacp->lang[0];
    return 0;
}

Result nsListApplicationContentMetaStatus(u64 id, s32 index, NsApplicationContentMetaStatus *list, s32 count, s32 *out)
{
    assert(C.ns_open == 1);
    const FakeGame *g = find(id);
    if (!g) return 0x2345;
    s32 n = 0, seen = 0;
    for (int i = 0; i < g->meta_count && n < count; i++) {
        const FakeMeta *m = &g->metas[i];
        if (!m->installed) continue;
        if (seen++ < index) continue;
        memset(&list[n], 0, sizeof(list[n]));
        list[n].meta_type      = m->type;
        list[n].storageID      = m->storage;
        list[n].version        = m->version;
        list[n].application_id = m->id;
        n++;
    }
    *out = n;
    return 0;
}

Result nsCheckApplicationLaunchVersion(u64 id)
{
    assert(C.ns_open == 1);
    const FakeGame *g = find(id);
    return g ? g->launch_rc : 0x3456;
}

Result nsDeleteApplicationCompletely(u64 id)
{
    assert(C.ns_open == 1);
    if (C.delete_rc) return C.delete_rc;
    C.deleted = id;
    FakeGame *g = find(id);
    if (g) {
        for (int i = 0; i < g->meta_count; i++) g->metas[i].installed = false;
        if (!C.delete_leaves_record) g->record = false;
    }
    return 0;
}

Result nsGetApplicationView(NsApplicationView *views, const u64 *ids, s32 count)
{
    assert(C.ns_open == 1 && count == 1);
    if (C.view_rc) return C.view_rc;
    const FakeGame *g = find(ids[0]);
    memset(views, 0, sizeof(*views));
    views->application_id = ids[0];
    views->flags = g ? g->view_flags : 0;
    return 0;
}

#define SRV_ES 1
#define SRV_AM 2

Result nsGetApplicationManagerInterface(Service *out)
{
    assert(C.ns_open == 1);
    out->id = SRV_AM;
    out->open = true;
    C.am_open++;
    return 0;
}

Service *nsGetServiceSession_ApplicationManagerInterface(void)
{
    assert(!"pre-3.0.0 only");
    return NULL;
}

Result smGetService(Service *out, const char *name)
{
    assert(strcmp(name, "es") == 0);
    if (C.es_rc) return C.es_rc;
    out->id = SRV_ES;
    out->open = true;
    C.es_open++;
    return 0;
}

void serviceClose(Service *srv)
{
    assert(srv->open);
    srv->open = false;
    if (srv->id == SRV_ES) C.es_open--;
    if (srv->id == SRV_AM) C.am_open--;
}

// es: 9 / 10 count the common / personalized tickets, 11 / 12 list them (the
// fake keeps every ticket common). ns application manager: 17 lists an
// entry's contents, 27 deletes the entry, 16 pushes it.
Result mock_dispatch(Service *srv, u32 command, void *out, size_t out_size, const void *in, size_t in_size,
                     SfDispatchParams p)
{
    assert(srv && srv->open);
    if (srv->id == SRV_ES) {
        assert(C.es_open == 1);
        if (command == 9 || command == 10) {
            assert(out_size == sizeof(u32));
            *(u32 *)out = command == 9 ? (u32)C.ticket_count : 0;
            return 0;
        }
        if (command == 11 || command == 12) {
            assert(out_size == sizeof(u32) && (p.buffer_attrs[0] & SfBufferAttr_Out));
            const u32 room = (u32)(p.buffers[0].size / sizeof(FsRightsId));
            const u32 n = command == 11 ? ((u32)C.ticket_count < room ? (u32)C.ticket_count : room) : 0;
            memcpy((void *)p.buffers[0].ptr, C.tickets, n * sizeof(FsRightsId));
            *(u32 *)out = n;
            return 0;
        }
        assert(!"unexpected es command");
    }
    assert(srv->id == SRV_AM && C.am_open == 1);
    if (command == 17) {
        assert(in_size == 16 && out_size == sizeof(s32) && (p.buffer_attrs[0] & SfBufferAttr_Out));
        u64 args[2];
        memcpy(args, in, sizeof(args));
        const FakeGame *g = find(args[1]);
        assert(args[0] == 0 && g && g->record);
        RecordMetaWire *w = (RecordMetaWire *)p.buffers[0].ptr;
        const s32 room = (s32)(p.buffers[0].size / sizeof(RecordMetaWire));
        s32 n = 0;
        for (int i = 0; i < g->meta_count && n < room; i++) {
            const FakeMeta *m = &g->metas[i];
            if (!m->installed) continue;
            memset(&w[n], 0, sizeof(w[n]));
            w[n].id = m->id;
            w[n].version = m->version;
            w[n].type = m->type;
            w[n].storage = m->storage;
            n++;
        }
        *(s32 *)out = n;
        return 0;
    }
    if (command == 27) {
        assert(in_size == sizeof(u64));
        u64 id;
        memcpy(&id, in, sizeof(id));
        FakeGame *g = find(id);
        if (!g || !g->record) return 0x4A10;
        g->record = false;
        C.record_deletes++;
        return 0;
    }
    if (command == 16) {
        assert(in_size == 16 && (p.buffer_attrs[0] & SfBufferAttr_In));
        const u8 *bytes = (const u8 *)in;
        assert(bytes[0] == 3);   // Present
        u64 id;
        memcpy(&id, bytes + 8, sizeof(id));
        FakeGame *g = find(id);
        assert(g && !g->record);   // deleted first
        g->record = true;
        C.pushes++;
        C.pushed_count = (int)(p.buffers[0].size / sizeof(RecordMetaWire));
        memcpy(C.pushed, p.buffers[0].ptr, p.buffers[0].size);
        return 0;
    }
    assert(!"unexpected application manager command");
    return 1;
}

// ------------------------------------------------------------ ncm

Result ncmInitialize(void)
{
    if (C.ncm_init_rc) return C.ncm_init_rc;
    C.ncm_open++;
    return 0;
}

void ncmExit(void)
{
    assert(C.ncm_open > 0 && C.db_open == 0 && C.cs_open == 0);   // the storages first
    C.ncm_open--;
}

static Result open_storage(NcmStorageId id)
{
    assert(C.ncm_open == 1);
    assert(id == NcmStorageId_BuiltInUser || id == NcmStorageId_SdCard);   // never the game card
    return id == NcmStorageId_SdCard && C.sd_unopenable ? 0x4567 : 0;
}

Result ncmOpenContentMetaDatabase(NcmContentMetaDatabase *out, NcmStorageId id)
{
    const Result rc = open_storage(id);
    if (rc) return rc;
    out->storage = id;
    out->open = true;
    C.db_open++;
    return 0;
}

Result ncmOpenContentStorage(NcmContentStorage *out, NcmStorageId id)
{
    const Result rc = open_storage(id);
    if (rc) return rc;
    out->storage = id;
    out->open = true;
    C.cs_open++;
    return 0;
}

void ncmContentMetaDatabaseClose(NcmContentMetaDatabase *db)
{
    assert(db->open);
    db->open = false;
    C.db_open--;
}

void ncmContentStorageClose(NcmContentStorage *cs)
{
    assert(cs->open);
    cs->open = false;
    C.cs_open--;
}

Result ncmContentMetaDatabaseHas(NcmContentMetaDatabase *db, bool *out, const NcmContentMetaKey *key)
{
    assert(db->open);
    const FakeMeta *m = meta_of(db->storage, key);
    *out = m && m->registered;
    return 0;
}

Result ncmContentMetaDatabaseGetRequiredSystemVersion(NcmContentMetaDatabase *db, u32 *out, const NcmContentMetaKey *key)
{
    assert(db->open);
    const FakeMeta *m = meta_of(db->storage, key);
    if (!m || !m->registered) return 0x5678;
    *out = m->required_sysver;
    return 0;
}

Result ncmContentMetaDatabaseListContentInfo(NcmContentMetaDatabase *db, s32 *written, NcmContentInfo *info, s32 count,
                                             const NcmContentMetaKey *key, s32 start)
{
    assert(db->open);
    FakeMeta *m = meta_of(db->storage, key);
    if (!m || !m->registered) return 0x5678;
    s32 n = 0;
    for (s32 i = start; i < m->files && n < count; i++, n++) {
        memset(&info[n], 0, sizeof(info[n]));
        info[n].content_id = content_id_of(m, (int)i);
    }
    *written = n;
    return 0;
}

Result ncmContentStorageHas(NcmContentStorage *cs, bool *out, const NcmContentId *id)
{
    assert(cs->open);
    int file;
    const FakeMeta *m = meta_of_content(id, &file);
    *out = file < m->files_present && file >= m->files_deleted;
    return 0;
}

Result ncmContentStorageGetRightsIdFromContentId(NcmContentStorage *cs, NcmRightsId *out, const NcmContentId *id,
                                                 FsContentAttributes attr)
{
    assert(cs->open && attr == FsContentAttributes_All);
    int file;
    const FakeMeta *m = meta_of_content(id, &file);
    memset(out, 0, sizeof(*out));
    out->key_generation = m->key_generation;
    if (m->titlekey) out->rights_id = rights_of(m);
    return 0;
}

Result ncmContentStorageDelete(NcmContentStorage *cs, const NcmContentId *id)
{
    assert(cs->open);
    int file;
    FakeMeta *m = meta_of_content(id, &file);
    assert(file == m->files_deleted);   // each file once, in order
    m->files_deleted++;
    return 0;
}

Result ncmContentMetaDatabaseRemove(NcmContentMetaDatabase *db, const NcmContentMetaKey *key)
{
    assert(db->open);
    FakeMeta *m = meta_of(db->storage, key);
    assert(m && m->files_deleted == m->files_present);   // the files first
    m->registered = false;
    m->installed = false;
    return 0;
}

Result ncmContentMetaDatabaseCommit(NcmContentMetaDatabase *db)
{
    assert(db->open);
    C.commits++;
    return 0;
}

// ------------------------------------------------------------ avm

Result avmInitialize(void)
{
    assert(hosversionAtLeast(6, 0, 0));
    if (C.avm_init_rc) return C.avm_init_rc;
    C.avm_open++;
    return 0;
}

void avmExit(void)
{
    assert(C.avm_open > 0);
    C.avm_open--;
}

Result avmGetLaunchRequiredVersion(u64 id, u32 *version)
{
    assert(C.avm_open == 1);
    const FakeGame *g = find(id);
    if (!g || !g->launch_known) return 0x6789;
    *version = g->launch_required;
    return 0;
}

Result avmPushLaunchVersion(u64 id, u32 version)
{
    assert(C.avm_open == 1);
    C.avm_pushes++;
    FakeGame *g = find(id);
    if (g && !C.avm_ignores_push) {
        g->launch_known = true;
        g->launch_required = version;
    }
    return 0;
}

// ------------------------------------------------------------ the integrity check

Result nsRequestVerifyApplication(NsProgressAsyncResult *a, u64 id, u32 unk, void *buffer, size_t size)
{
    assert(C.ns_open == 1 && unk == 0x7 && buffer && size == 0x100000 && ((uintptr_t)buffer & 0xFFF) == 0);
    assert(find(id));
    if (C.verify_rc) return C.verify_rc;
    a->open = true;
    C.async_open++;
    return 0;
}

Result nsProgressAsyncResultWait(NsProgressAsyncResult *a, u64 timeout)
{
    assert(a->open && timeout > 0);
    return ++C.verify_waits < 4 ? KERNELRESULT(TimedOut) : 0;
}

Result nsProgressAsyncResultGetProgress(NsProgressAsyncResult *a, void *buffer, size_t size)
{
    assert(a->open && size == sizeof(NsSystemUpdateProgress));
    NsSystemUpdateProgress *p = (NsSystemUpdateProgress *)buffer;
    p->current_size = C.verify_waits * 100;
    p->total_size = C.verify_waits == 1 ? 0 : 400;
    return 0;
}

Result nsProgressAsyncResultGet(NsProgressAsyncResult *a)
{
    assert(a->open);
    return 0;
}

Result nsProgressAsyncResultGetDetailResult(NsProgressAsyncResult *a)
{
    assert(a->open);
    return C.verify_corrupt ? 0x2E02 : 0;
}

void nsProgressAsyncResultClose(NsProgressAsyncResult *a)
{
    assert(a->open);
    a->open = false;
    C.async_open--;
}

// ------------------------------------------------------------ helpers

#define HOS(a, b, c) MAKEHOSVERSION(a, b, c)
#define SYSVER(a, b, c) (((u32)(a) << 26) | ((u32)(b) << 20) | ((u32)(c) << 16))
static const u32 CURRENT = HOS(22, 1, 0);

static GameCheck R;

static void all_closed(void)
{
    assert(C.ns_open == 0 && C.ncm_open == 0 && C.db_open == 0 && C.cs_open == 0);
    assert(C.avm_open == 0 && C.es_open == 0 && C.am_open == 0 && C.async_open == 0);
}

static const GameFacts *facts(u64 id)
{
    for (u32 i = 0; i < R.count; i++)
        if (R.games[i].app_id == id) return &R.games[i];
    assert(!"game not scanned");
    return NULL;
}

static GameIssue issue_of(u64 id)
{
    return gamecheck_classify(facts(id), C.hos);
}

static void scan(void)
{
    gamecheck_scan(&R);
    all_closed();
}

// ------------------------------------------------------------ tests

static void test_tables(void)
{
    assert(gamecheck_sysver_to_hos(0) == 0);
    assert(gamecheck_sysver_to_hos(SYSVER(1, 0, 0)) == HOS(1, 0, 0));
    assert(gamecheck_sysver_to_hos(SYSVER(20, 0, 0)) == HOS(20, 0, 0));
    assert(gamecheck_sysver_to_hos(SYSVER(21, 2, 1)) == HOS(21, 2, 1));
    assert(gamecheck_sysver_to_hos(SYSVER(19, 0, 1) | 0x1A) == HOS(19, 0, 1));   // the relstep ignored
    assert(gamecheck_sysver_to_hos(0x50000000u) == HOS(20, 0, 0));

    assert(gamecheck_max_key_generation(HOS(9, 0, 0)) == 0x0A);
    assert(gamecheck_max_key_generation(HOS(12, 0, 3)) == 0x0B);   // 12.1.0 brought 0x0C
    assert(gamecheck_max_key_generation(HOS(21, 2, 0)) == 0x15);
    assert(gamecheck_max_key_generation(HOS(22, 1, 0)) == 0x16);
    assert(gamecheck_max_key_generation(HOS(23, 0, 1)) == 0x17);
    assert(gamecheck_max_key_generation(HOS(24, 0, 0)) == 0);   // newer than the table: not judged
    assert(gamecheck_max_key_generation(0) == 0);
    assert(gamecheck_key_generation_firmware(0x0A) == HOS(9, 0, 0));
    assert(gamecheck_key_generation_firmware(0x17) == HOS(23, 0, 0));
    assert(gamecheck_key_generation_firmware(0x30) == 0);
}

static void test_rules(void)
{
    // The order: the first that applies explains the others.
    GameFacts g;
    memset(&g, 0, sizeof(g));
    assert(gamecheck_classify(&g, CURRENT) == GC_ARCHIVED);
    g.meta_count = 1;
    g.files_missing = true;
    g.control_ok = false;
    g.view_ok = true;
    g.view_flags = GC_VIEW_NEEDS_VERIFY;
    g.key_generation = 0x17;
    g.patch_required_hos = HOS(30, 0, 0);
    g.ticket_missing = true;
    g.launch_checked = true;
    g.launch_required = 0x20000;
    g.launch_rc = 1;
    assert(gamecheck_classify(&g, CURRENT) == GC_NO_BASE);
    g.has_base = true;
    assert(gamecheck_classify(&g, CURRENT) == GC_FILES_MISSING);
    g.files_missing = false;
    assert(gamecheck_classify(&g, CURRENT) == GC_NEEDS_VERIFY);
    g.view_flags = GC_VIEW_CAN_LAUNCH;
    assert(gamecheck_classify(&g, CURRENT) == GC_KEY_TOO_NEW);
    assert(gamecheck_classify(&g, HOS(23, 0, 0)) == GC_FIRMWARE_TOO_OLD);   // that firmware has the key
    g.key_generation = 0x16;
    assert(gamecheck_classify(&g, CURRENT) == GC_FIRMWARE_TOO_OLD);
    assert(gamecheck_classify(&g, 0) == GC_TICKET_MISSING);   // the console's version unknown: not judged
    g.patch_required_hos = 0;
    g.base_required_hos = HOS(22, 1, 0);   // exactly the console's: fine
    assert(gamecheck_classify(&g, CURRENT) == GC_TICKET_MISSING);
    g.ticket_missing = false;
    assert(gamecheck_classify(&g, CURRENT) == GC_LAUNCH_VERSION);
    g.launch_rc = 0;   // wanted, but the console starts it anyway: no problem from it
    g.control_ok = true;
    assert(gamecheck_classify(&g, CURRENT) == GC_OK);
    g.control_ok = false;
    g.launch_rc = 1;
    g.has_patch = true;
    g.patch_version = 0x20000;
    assert(gamecheck_installed_version(&g) == 0x20000);
    assert(gamecheck_classify(&g, CURRENT) == GC_LAUNCH_REFUSED);
    g.launch_rc = 0;
    assert(gamecheck_classify(&g, CURRENT) == GC_NO_CONTROL);
    g.control_ok = true;
    assert(gamecheck_classify(&g, CURRENT) == GC_OK);
    g.storage_unavailable = true;
    assert(gamecheck_classify(&g, CURRENT) == GC_STORAGE_UNAVAILABLE);
    g.storage_unavailable = false;
    g.card_only = true;
    g.files_missing = true;
    assert(gamecheck_classify(&g, CURRENT) == GC_OK);
}

static void test_actions(void)
{
    GameFacts g;
    memset(&g, 0, sizeof(g));
    g.meta_count = g.content_count = 3;
    g.has_base = g.has_patch = true;
    g.contents[0] = (GameContent){ 0x0100000000010000ULL, 0, GC_META_APPLICATION, GC_STORAGE_SD };
    g.contents[1] = (GameContent){ 0x0100000000010800ULL, 0x10000, GC_META_PATCH, GC_STORAGE_SD };
    g.contents[2] = (GameContent){ 0x0100000000011001ULL, 0, GC_META_ADDON, GC_STORAGE_NAND };
    g.patch_version = 0x10000;
    assert(gamecheck_actions(&g, GC_OK) == 0);
    assert(gamecheck_actions(&g, GC_STORAGE_UNAVAILABLE) == 0);
    assert(gamecheck_actions(&g, GC_ISSUE_COUNT) == 0);
    assert(gamecheck_actions(&g, GC_FIRMWARE_TOO_OLD) == (GA_REMOVE | GA_REMOVE_UPDATE | GA_REMOVE_ADDON | GA_VERIFY));
    g.launch_checked = true;
    g.launch_required = 0x30000;
    assert(!(gamecheck_actions(&g, GC_NO_CONTROL) & GA_RESET_LAUNCH));   // the console starts it anyway
    g.launch_rc = 0xC0DE;
    assert(gamecheck_actions(&g, GC_LAUNCH_VERSION) & GA_RESET_LAUNCH);
    g.launch_required = 0x10000;
    assert(!(gamecheck_actions(&g, GC_LAUNCH_REFUSED) & GA_RESET_LAUNCH));
    g.launch_rc = 0;
    // On a game card: neither the update nor the add-on.
    g.contents[1].storage = GC_STORAGE_GAMECARD;
    g.contents[2].storage = GC_STORAGE_GAMECARD;
    assert(gamecheck_actions(&g, GC_NO_CONTROL) == (GA_REMOVE | GA_VERIFY));
    // Without the game: removing all of it is the cleaning, nothing to check.
    g.has_base = false;
    g.contents[1].storage = GC_STORAGE_SD;
    assert(gamecheck_actions(&g, GC_NO_BASE) == GA_REMOVE);
    assert(gamecheck_actions(&g, GC_ARCHIVED) == GA_REMOVE);
}

static void test_every_problem(void)
{
    reset();
    FakeGame *g;
    FakeMeta *m;
    g = add_game(0x0100000000010000ULL, "Fine");   // game + update on the SD card, add-on, tickets
    m = add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    m->required_sysver = SYSVER(12, 0, 0);
    m->titlekey = true;
    add_ticket(m);
    m = add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x30000);
    m->required_sysver = SYSVER(20, 0, 0);
    m->key_generation = 0x14;
    add_meta(g, NcmContentMetaType_AddOnContent, NcmStorageId_SdCard, 0)->registered = false;   // add-ons not checked
    g->view_flags = GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH;
    g->launch_known = true;
    g->launch_required = 0x40000;   // known of, the console starts it anyway

    g = add_game(0x0100000000020000ULL, "Archived");
    g->record_event = 11;
    g = add_game(0x0100000000030000ULL, "Update only");
    add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000);
    g = add_game(0x0100000000040000ULL, "Missing file");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_BuiltInUser, 0);
    add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000)->files_present = 1;
    g = add_game(0x0100000000050000ULL, "Unknown to ncm");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0)->registered = false;
    g = add_game(0x0100000000060000ULL, "Too new");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0)->required_sysver = SYSVER(20, 0, 0);
    add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x20000)->required_sysver = SYSVER(23, 0, 0);
    g = add_game(0x0100000000070000ULL, "Refused");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g->launch_rc = 0xABCD;
    g = add_game(0x0100000000080000ULL, NULL);   // the icon that keeps loading
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g = add_game(0x0100000000090000ULL, "Card");   // a card out of its slot
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_GameCard, 0)->registered = false;
    g->record_event = 5;
    g = add_game(0x01000000000A0000ULL, "Card with an update");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_GameCard, 0)->registered = false;
    add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000);
    g = add_game(0x01000000000B0000ULL, "No ticket");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0)->titlekey = true;
    g = add_game(0x01000000000C0000ULL, "New keys");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0)->key_generation = 0x17;
    g = add_game(0x01000000000D0000ULL, "Update removed");   // the console still wants v2
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g->launch_known = true;
    g->launch_required = 0x20000;
    g->launch_rc = 0xC0DE;
    g = add_game(0x01000000000E0000ULL, "Corrupt");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g->view_flags = GC_VIEW_NEEDS_VERIFY;

    scan();
    assert(R_SUCCEEDED(R.rc) && R_SUCCEEDED(R.tickets_rc) && R.count == (u32)C.count && !R.truncated);

    const GameFacts *f = facts(0x0100000000010000ULL);
    assert(f->control_ok && strcmp(f->name, "Fine") == 0 && f->record_event == 3);
    assert(f->meta_count == 3 && f->content_count == 3 && f->has_base && f->has_patch && f->patch_version == 0x30000);
    assert(f->contents[1].type == GC_META_PATCH && f->contents[1].id == 0x0100000000010800ULL &&
           f->contents[1].version == 0x30000 && f->contents[1].storage == GC_STORAGE_SD);
    assert(!f->card_only && !f->meta_missing && !f->files_missing && !f->storage_unavailable);
    assert(f->base_required_hos == HOS(12, 0, 0) && f->patch_required_hos == HOS(20, 0, 0));
    assert(f->key_generation == 0x14 && f->needs_ticket && !f->ticket_missing);
    assert(f->view_ok && f->view_flags == (GC_VIEW_HAS_MAIN_CONTENTS | GC_VIEW_CAN_LAUNCH));
    assert(f->launch_checked && f->launch_required == 0x40000);
    assert(issue_of(0x0100000000010000ULL) == GC_OK);

    assert(facts(0x0100000000020000ULL)->meta_count == 0 && facts(0x0100000000020000ULL)->record_event == 11);
    assert(issue_of(0x0100000000020000ULL) == GC_ARCHIVED);
    assert(issue_of(0x0100000000030000ULL) == GC_NO_BASE);
    assert(facts(0x0100000000040000ULL)->files_missing && issue_of(0x0100000000040000ULL) == GC_FILES_MISSING);
    assert(facts(0x0100000000050000ULL)->meta_missing && issue_of(0x0100000000050000ULL) == GC_FILES_MISSING);
    f = facts(0x0100000000060000ULL);
    assert(f->base_required_hos == HOS(20, 0, 0) && f->patch_required_hos == HOS(23, 0, 0));
    assert(issue_of(0x0100000000060000ULL) == GC_FIRMWARE_TOO_OLD);
    assert(facts(0x0100000000070000ULL)->launch_rc == 0xABCD && issue_of(0x0100000000070000ULL) == GC_LAUNCH_REFUSED);
    f = facts(0x0100000000080000ULL);
    assert(!f->control_ok && f->name[0] == '\0' && issue_of(0x0100000000080000ULL) == GC_NO_CONTROL);
    assert(facts(0x0100000000090000ULL)->card_only && issue_of(0x0100000000090000ULL) == GC_OK);
    f = facts(0x01000000000A0000ULL);
    assert(!f->card_only && f->has_base && issue_of(0x01000000000A0000ULL) == GC_OK);   // the card is not read
    f = facts(0x01000000000B0000ULL);
    assert(f->needs_ticket && f->ticket_missing && issue_of(0x01000000000B0000ULL) == GC_TICKET_MISSING);
    f = facts(0x01000000000C0000ULL);
    assert(f->key_generation == 0x17 && issue_of(0x01000000000C0000ULL) == GC_KEY_TOO_NEW);
    f = facts(0x01000000000D0000ULL);
    assert(issue_of(0x01000000000D0000ULL) == GC_LAUNCH_VERSION);
    assert(gamecheck_actions(f, GC_LAUNCH_VERSION) & GA_RESET_LAUNCH);
    assert(issue_of(0x01000000000E0000ULL) == GC_NEEDS_VERIFY);
}

static void test_sd_card_unreadable(void)
{
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "On the SD card");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g = add_game(0x0100000000020000ULL, "In the console");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_BuiltInUser, 0);
    C.sd_unopenable = true;
    scan();
    assert(facts(0x0100000000010000ULL)->storage_unavailable);
    assert(issue_of(0x0100000000010000ULL) == GC_STORAGE_UNAVAILABLE);
    assert(gamecheck_actions(facts(0x0100000000010000ULL), GC_STORAGE_UNAVAILABLE) == 0);
    assert(issue_of(0x0100000000020000ULL) == GC_OK);
}

static void test_without_services(void)
{
    // No ncm: only what ns says, nothing reported missing for want of it.
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "Fine");
    FakeMeta *m = add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    m->files_present = 0;
    m->titlekey = true;
    m->key_generation = 0x30;
    g = add_game(0x0100000000020000ULL, "Update only");
    add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000);
    C.ncm_init_rc = 0x9999;
    scan();
    const GameFacts *f = facts(0x0100000000010000ULL);
    assert(!f->storage_unavailable && !f->files_missing && !f->meta_missing && !f->needs_ticket && !f->key_generation);
    assert(issue_of(0x0100000000010000ULL) == GC_OK);
    assert(issue_of(0x0100000000020000ULL) == GC_NO_BASE);

    // No es: tickets not judged. No avm, no view: nothing from them.
    reset();
    g = add_game(0x0100000000010000ULL, "Fine");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0)->titlekey = true;
    g->view_flags = GC_VIEW_NEEDS_VERIFY;
    g->launch_known = true;
    g->launch_required = 0x50000;
    g->launch_rc = 0xC0DE;
    C.es_rc = 0x1E5;
    C.avm_init_rc = 0x2A2;
    C.view_rc = 0x3B3;
    scan();
    f = facts(0x0100000000010000ULL);
    assert(R.tickets_rc == 0x1E5 && f->needs_ticket && !f->ticket_missing);
    assert(!f->view_ok && !f->launch_checked);
    assert(issue_of(0x0100000000010000ULL) == GC_LAUNCH_REFUSED);   // not explained without avm

    // Before 6.0.0 there is no avm: never opened.
    reset();
    C.hos = HOS(5, 1, 0);
    g = add_game(0x0100000000010000ULL, "Old");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g->launch_known = true;
    g->launch_required = 0x50000;
    scan();
    assert(!facts(0x0100000000010000ULL)->launch_checked);
}

static void test_list_failures(void)
{
    reset();
    C.ns_init_rc = 0x1111;
    gamecheck_scan(&R);
    assert(R.rc == 0x1111 && R.count == 0 && C.ns_open == 0);

    reset();
    add_game(0x0100000000010000ULL, "A");
    C.list_rc = 0x2222;
    scan();
    assert(R.rc == 0x2222 && R.count == 0);
}

static void test_paging(void)
{
    // More games than one chunk of records, more than the list holds, and
    // more contents than one chunk (and than the list of contents kept).
    reset();
    for (int i = 0; i < GAMECHECK_MAX + 10; i++) {
        FakeGame *g = add_game(0x0100000000000000ULL + ((u64)(i + 1) << 16), "Game");
        add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    }
    FakeGame *many = &C.games[0];
    for (int i = 0; i < 20; i++) add_meta(many, NcmContentMetaType_AddOnContent, NcmStorageId_SdCard, 0);
    scan();
    assert(R_SUCCEEDED(R.rc) && R.count == GAMECHECK_MAX && R.truncated);
    assert(R.games[GAMECHECK_MAX - 1].app_id == C.games[GAMECHECK_MAX - 1].id);
    assert(facts(many->id)->meta_count == 21 && facts(many->id)->content_count == GAMECHECK_MAX_CONTENTS);
    for (u32 i = 0; i < R.count; i++) assert(gamecheck_classify(&R.games[i], CURRENT) == GC_OK);

    reset();
    for (int i = 0; i < 64; i++) add_game(0x0100000000000000ULL + ((u64)(i + 1) << 16), "Game");   // exactly one chunk
    scan();
    assert(R.count == 64 && !R.truncated);
}

static bool s_confirm;
static bool confirm(void) { return s_confirm; }

static void test_remove(void)
{
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "A");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    assert(gamecheck_remove(0x0100000000010000ULL) == 0 && C.deleted == 0x0100000000010000ULL);
    assert(C.record_deletes == 0);   // nothing left to clean
    all_closed();

    // The deletion fails with nothing installed (the dotted frame): the
    // entry alone goes.
    reset();
    add_game(0x0100000000010000ULL, "Archived");
    C.delete_rc = 0x7777;
    assert(gamecheck_remove(0x0100000000010000ULL) == 0 && C.record_deletes == 1 && !find(0x0100000000010000ULL)->record);
    all_closed();

    // … but never while something is installed.
    reset();
    g = add_game(0x0100000000010000ULL, "A");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    C.delete_rc = 0x7777;
    assert(gamecheck_remove(0x0100000000010000ULL) == 0x7777 && C.record_deletes == 0 && g->record);
    all_closed();

    // Read-only, the PIN refused: no session opened.
    reset();
    core_set_read_only(true);
    assert(gamecheck_remove(0x0100000000010000ULL) == NXM_RC_READ_ONLY && C.ns_inits == 0 && C.deleted == 0);
    core_set_read_only(false);
    core_set_change_check(confirm);
    s_confirm = false;
    assert(gamecheck_remove(0x0100000000010000ULL) == NXM_RC_NOT_CONFIRMED && C.ns_inits == 0 && C.deleted == 0);
    s_confirm = true;
    add_game(0x0100000000010000ULL, "A");
    assert(gamecheck_remove(0x0100000000010000ULL) == 0 && C.deleted == 0x0100000000010000ULL);
    core_set_change_check(NULL);

    reset();
    C.ns_init_rc = 0x1111;
    assert(gamecheck_remove(0x0100000000010000ULL) == 0x1111 && C.deleted == 0);
}

static GameContent content_of(const FakeMeta *m)
{
    GameContent c = { m->id, m->version, m->type, m->storage };
    return c;
}

static void test_remove_content(void)
{
    // The update: its files and key go, the entry is rebuilt with the rest,
    // the launch version goes back to the game's.
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "A");
    FakeMeta *base = add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0x10000 - 0x10000);
    FakeMeta *patch = add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x40000);
    FakeMeta *addon = add_meta(g, NcmContentMetaType_AddOnContent, NcmStorageId_BuiltInUser, 0x10000);
    g->launch_known = true;
    g->launch_required = 0x40000;
    GameContent c = content_of(patch);
    assert(gamecheck_remove_content(g->id, &c) == 0);
    all_closed();
    assert(patch->files_deleted == 2 && !patch->registered && !patch->installed && C.commits == 1);
    assert(base->installed && base->files_deleted == 0 && addon->installed);
    assert(C.record_deletes == 1 && C.pushes == 1 && g->record && C.pushed_count == 2);
    for (int i = 0; i < C.pushed_count; i++) assert(C.pushed[i].id != patch->id);
    assert(C.avm_pushes == 1 && g->launch_required == 0);
    // The game is fine again.
    scan();
    assert(issue_of(g->id) == GC_OK && !facts(g->id)->has_patch);

    // An add-on: no launch version change.
    c = content_of(addon);
    assert(gamecheck_remove_content(g->id, &c) == 0);
    all_closed();
    assert(!addon->installed && C.avm_pushes == 1 && C.pushed_count == 1 && C.pushed[0].id == base->id);

    // The last content of an entry: deleted, nothing pushed back.
    reset();
    g = add_game(0x0100000000020000ULL, "Update only");
    patch = add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000);
    c = content_of(patch);
    assert(gamecheck_remove_content(g->id, &c) == 0);
    assert(C.record_deletes == 1 && C.pushes == 0 && !g->record);
    all_closed();

    // Not in the entry: nothing deleted.
    reset();
    g = add_game(0x0100000000010000ULL, "A");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    patch = add_meta(g, NcmContentMetaType_Patch, NcmStorageId_SdCard, 0x10000);
    c = content_of(patch);
    c.version = 0x20000;
    assert(gamecheck_remove_content(g->id, &c) == NXM_RC_STATE_UNKNOWN);
    assert(patch->files_deleted == 0 && patch->installed && C.record_deletes == 0);
    all_closed();

    // Never the game itself, nothing on a game card, nothing with no content.
    const int inits = C.ns_inits;
    c = content_of(&g->metas[0]);
    assert(gamecheck_remove_content(g->id, &c) == NXM_RC_INVALID_ARGUMENT);
    c = content_of(patch);
    c.storage = GC_STORAGE_GAMECARD;
    assert(gamecheck_remove_content(g->id, &c) == NXM_RC_INVALID_ARGUMENT);
    assert(gamecheck_remove_content(g->id, NULL) == NXM_RC_INVALID_ARGUMENT);
    assert(C.ns_inits == inits);

    // The gates, before any session.
    c = content_of(patch);
    core_set_read_only(true);
    assert(gamecheck_remove_content(g->id, &c) == NXM_RC_READ_ONLY && C.ns_inits == inits);
    core_set_read_only(false);
    core_set_change_check(confirm);
    s_confirm = false;
    assert(gamecheck_remove_content(g->id, &c) == NXM_RC_NOT_CONFIRMED && C.ns_inits == inits);
    core_set_change_check(NULL);
    assert(patch->installed);
}

static void test_reset_launch_version(void)
{
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "A");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    g->launch_known = true;
    g->launch_required = 0x30000;
    assert(gamecheck_reset_launch_version(g->id, 0) == 0 && g->launch_required == 0);
    all_closed();

    // Pushed, the console still wants more: said so.
    g->launch_required = 0x30000;
    C.avm_ignores_push = true;
    assert(gamecheck_reset_launch_version(g->id, 0) == NXM_RC_NOT_APPLIED);
    all_closed();

    C.avm_ignores_push = false;
    core_set_read_only(true);
    assert(gamecheck_reset_launch_version(g->id, 0) == NXM_RC_READ_ONLY && C.avm_pushes == 2);
    core_set_read_only(false);

    C.hos = HOS(5, 1, 0);
    assert(gamecheck_reset_launch_version(g->id, 0) == NXM_RC_FW_UNSUPPORTED);
}

static int s_progress_calls;
static u64 s_last_current, s_last_total;
static void on_progress(void *ctx, u64 current, u64 total)
{
    assert(ctx == &s_progress_calls);
    s_progress_calls++;
    s_last_current = current;
    s_last_total = total;
}

static void test_verify(void)
{
    reset();
    FakeGame *g = add_game(0x0100000000010000ULL, "A");
    add_meta(g, NcmContentMetaType_Application, NcmStorageId_SdCard, 0);
    bool corrupt = true;
    s_progress_calls = 0;
    assert(gamecheck_verify(g->id, on_progress, &s_progress_calls, &corrupt) == 0 && !corrupt);
    assert(s_progress_calls == 3 && s_last_current == 300 && s_last_total == 400);
    all_closed();

    reset();
    g = add_game(0x0100000000010000ULL, "A");
    C.verify_corrupt = true;
    assert(gamecheck_verify(g->id, NULL, NULL, &corrupt) == 0 && corrupt);
    all_closed();

    // Only reads: also in read-only mode.
    reset();
    g = add_game(0x0100000000010000ULL, "A");
    core_set_read_only(true);
    assert(gamecheck_verify(g->id, NULL, NULL, &corrupt) == 0 && !corrupt);
    core_set_read_only(false);

    reset();
    g = add_game(0x0100000000010000ULL, "A");
    C.verify_rc = 0x5A5A;
    assert(gamecheck_verify(g->id, NULL, NULL, &corrupt) == 0x5A5A && !corrupt);
    all_closed();
}

int main(void)
{
    test_tables();
    test_rules();
    test_actions();
    test_every_problem();
    test_sd_card_unreadable();
    test_without_services();
    test_list_failures();
    test_paging();
    test_remove();
    test_remove_content();
    test_reset_launch_version();
    test_verify();
    puts("gamecheck assertions passed");
    return 0;
}
