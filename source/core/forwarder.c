// PlayGuard — NRO forwarders, console side (see forwarder.h). The install
// follows sphaira's owo.cpp (ITotalJustice, GPLv3): the header key from spl,
// the contents through placeholders, the database entry, the HOME menu's.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "forwarder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forwarder_pack.h"
#include "nacp.h"
#include "sysinfo.h"
#include "write_guard.h"

#define RECORD_CHUNK 64
#define LOADER_MAIN  "romfs:/forwarder/main"
#define LOADER_NPDM  "romfs:/forwarder/main.npdm"

// ns ApplicationRecord's LastEvent for an entry with its contents present,
// as gamecheck.c pushes it.
#define RECORD_EVENT_PRESENT 3

// The NCA header key's sources (switchbrew Cryptosystem: header_kek_source,
// header_key_source): spl seals them with the console's keys and hands the
// key back, which is how sphaira gets it (no prod.keys).
static const u8 HEADER_KEK_SOURCE[0x10] = {
    0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A,
};
static const u8 HEADER_KEY_SOURCE[0x20] = {
    0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0,
    0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2,
};

void forwarder_sha256(const void *data, size_t size, uint8_t out[32])
{
    sha256CalculateHash(out, data, size);
}

bool forwarder_is_one(u64 id)
{
    return (id >> 56) == FWD_ID_PREFIX;
}

Result forwarder_list(ForwarderRecord *out, u32 max, u32 *count)
{
    *count = 0;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    u32 title_index = 0;
    nacp_console_language(&title_index);
    NsApplicationControlData *cd = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    NsApplicationRecord records[RECORD_CHUNK];
    for (s32 offset = 0; *count < max;) {
        s32 got = 0;
        const Result list_rc = nsListApplicationRecord(records, RECORD_CHUNK, offset, &got);
        if (R_FAILED(list_rc)) {
            if (offset == 0) rc = list_rc;
            break;
        }
        if (got <= 0) break;
        for (s32 i = 0; i < got && *count < max; i++) {
            if (!forwarder_is_one(records[i].application_id)) continue;
            ForwarderRecord *f = &out[(*count)++];
            memset(f, 0, sizeof(*f));
            f->id = records[i].application_id;
            u64 size = 0;
            if (cd && nacp_read_control(f->id, cd, false, &size)) {
                nacp_name(&cd->nacp, sizeof(cd->nacp), title_index, f->name, sizeof(f->name));
                const NacpLanguageEntry *e = &cd->nacp.lang[title_index < 16 ? title_index : 0];
                nacp_copy_utf8(f->author, sizeof(f->author), e->author[0] ? e->author : cd->nacp.lang[0].author);
                nacp_copy_utf8(f->version, sizeof(f->version), cd->nacp.display_version);
            }
        }
        offset += got;
        if (got < RECORD_CHUNK) break;
    }
    free(cd);
    nsExit();
    return rc;
}

// A whole file into a new buffer.
static void *read_file(const char *path, size_t *size)
{
    *size = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    void *data = NULL;
    if (fseek(f, 0, SEEK_END) == 0) {
        const long n = ftell(f);
        if (n > 0 && fseek(f, 0, SEEK_SET) == 0 && (data = malloc((size_t)n)) != NULL) {
            if (fread(data, 1, (size_t)n, f) == (size_t)n) {
                *size = (size_t)n;
            } else {
                free(data);
                data = NULL;
            }
        }
    }
    fclose(f);
    return data;
}

static Result header_key(u8 key[0x20])
{
    Result rc = splCryptoInitialize();
    if (R_FAILED(rc)) return rc;
    u8 kek[0x10];
    rc = splCryptoGenerateAesKek(HEADER_KEK_SOURCE, 0, 0, kek);
    if (R_SUCCEEDED(rc)) rc = splCryptoGenerateAesKey(kek, HEADER_KEY_SOURCE, key);
    if (R_SUCCEEDED(rc)) rc = splCryptoGenerateAesKey(kek, HEADER_KEY_SOURCE + 0x10, key + 0x10);
    splCryptoExit();
    return rc;
}

// An NCA header, encrypted with the header key: AES-XTS over its six
// sectors, Nintendo's tweak.
static void seal_header(void *ctx, uint8_t *header)
{
    const u8 *key = (const u8 *)ctx;
    Aes128XtsContext xts;
    aes128XtsContextCreate(&xts, key, key + 0x10, true);
    for (u64 sector = 0; sector < FWD_NCA_HEADER / 0x200; sector++) {
        aes128XtsContextResetSector(&xts, sector, true);
        aes128XtsEncrypt(&xts, header + sector * 0x200, header + sector * 0x200, 0x200);
    }
}

// Atmosphère 1.8.0+ (sysinfo: Exosphère's version).
static bool ams_180(void)
{
    SysInfo si;
    sysinfo_get(&si);
    return si.ams_valid && MAKEHOSVERSION(si.ams_major, si.ams_minor, si.ams_micro) >= MAKEHOSVERSION(1, 8, 0);
}

// ns IApplicationManagerInterface commands libnx has no wrapper for
// (switchbrew NS services): PushApplicationRecord 16 (as gamecheck.c),
// InvalidateApplicationControlCache 404, which sphaira sends so the HOME
// menu reads the new name and icon.
typedef struct {
    NcmContentMetaKey key;
    u8 storage_id;
    u8 pad[7];
} RecordMeta;

static Result push_record(u64 id, const RecordMeta *meta)
{
    Service srv, *am = &srv;
    Result rc;
    if (hosversionAtLeast(3, 0, 0)) {
        rc = nsGetApplicationManagerInterface(&srv);
        if (R_FAILED(rc)) return rc;
    } else {
        am = nsGetServiceSession_ApplicationManagerInterface();
    }
    const struct {
        u8  last_event;
        u8  pad[7];
        u64 app_id;
    } in = { RECORD_EVENT_PRESENT, { 0 }, id };
    rc = serviceDispatchIn(am, 16, in,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { meta, sizeof(*meta) } },
    );
    if (R_SUCCEEDED(rc)) serviceDispatchIn(am, 404, id);
    if (hosversionAtLeast(3, 0, 0)) serviceClose(&srv);
    return rc;
}

// The contents into the SD card's storage, then their database entry.
static Result write_contents(const FwdPackage *p)
{
    NcmContentStorage cs;
    Result rc = ncmOpenContentStorage(&cs, NcmStorageId_SdCard);
    if (R_FAILED(rc)) return rc;
    for (int i = 0; i < 3 && R_SUCCEEDED(rc); i++) {
        const FwdContent *c = &p->contents[i];
        NcmContentId content_id;
        NcmPlaceHolderId placeholder;
        memcpy(&content_id, c->hash, sizeof(content_id));
        rc = ncmContentStorageGeneratePlaceHolderId(&cs, &placeholder);
        if (R_FAILED(rc)) break;
        ncmContentStorageDeletePlaceHolder(&cs, &placeholder);
        rc = ncmContentStorageCreatePlaceHolder(&cs, &content_id, &placeholder, (s64)c->nca.size);
        if (R_SUCCEEDED(rc)) rc = ncmContentStorageWritePlaceHolder(&cs, &placeholder, 0, c->nca.data, c->nca.size);
        if (R_SUCCEEDED(rc)) {
            ncmContentStorageDelete(&cs, &content_id);   // the same content left by an earlier install
            rc = ncmContentStorageRegister(&cs, &content_id, &placeholder);
        }
        if (R_FAILED(rc)) ncmContentStorageDeletePlaceHolder(&cs, &placeholder);
    }
    ncmContentStorageClose(&cs);
    if (R_FAILED(rc)) return rc;

    NcmContentMetaDatabase db;
    rc = ncmOpenContentMetaDatabase(&db, NcmStorageId_SdCard);
    if (R_FAILED(rc)) return rc;
    NcmContentMetaKey key;
    memcpy(&key, p->meta_key, sizeof(key));
    rc = ncmContentMetaDatabaseSet(&db, &key, p->meta_value, p->meta_value_size);
    if (R_SUCCEEDED(rc)) rc = ncmContentMetaDatabaseCommit(&db);
    ncmContentMetaDatabaseClose(&db);
    return rc;
}

Result forwarder_install(const ForwarderSpec *spec, u64 *id)
{
    *id = 0;
    if (!spec->nro_path || !spec->argv || !spec->icon || !spec->icon_size) return NXM_RC_INVALID_ARGUMENT;
    *id = forwarder_id(spec->nro_path, spec->argv);
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;

    size_t main_size = 0, npdm_size = 0;
    void *main = read_file(LOADER_MAIN, &main_size);
    void *npdm = read_file(LOADER_NPDM, &npdm_size);
    u8 key[0x20];
    Result rc = main && npdm ? header_key(key) : NXM_RC_INVALID_ARGUMENT;
    FwdPackage p;
    memset(&p, 0, sizeof(p));
    if (R_SUCCEEDED(rc)) {
        const FwdInput in = {
            .nro_path = spec->nro_path, .argv = spec->argv, .gate_path = spec->gate_path, .gate_argv = spec->gate_argv,
            .name = spec->name, .author = spec->author,
            .nacp = spec->nacp, .icon = spec->icon, .icon_size = spec->icon_size,
            .exefs_main = main, .main_size = main_size, .exefs_npdm = npdm, .npdm_size = npdm_size,
            .force_debug = ams_180(), .seal = seal_header, .seal_ctx = key,
        };
        const FwdError e = forwarder_pack(&in, &p);
        if (e == FWD_NO_MEMORY) rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
        else if (e != FWD_OK) rc = NXM_RC_INVALID_ARGUMENT;
    }
    memset(key, 0, sizeof(key));
    free(main);
    free(npdm);

    if (R_SUCCEEDED(rc)) rc = nsInitialize();
    if (R_SUCCEEDED(rc)) {
        // A forwarder of the same .nro (made here or by another tool) goes
        // first, as sphaira replaces one: nothing of it stays behind. None
        // there is the usual case, not an error.
        nsDeleteApplicationCompletely(*id);
        rc = ncmInitialize();
        if (R_SUCCEEDED(rc)) {
            rc = write_contents(&p);
            ncmExit();
        }
        if (R_SUCCEEDED(rc)) {
            RecordMeta meta;
            memset(&meta, 0, sizeof(meta));
            memcpy(&meta.key, p.meta_key, sizeof(meta.key));
            meta.storage_id = NcmStorageId_SdCard;
            rc = push_record(*id, &meta);
            // Its contents without an entry would be lost space: removed.
            if (R_FAILED(rc)) nsDeleteApplicationCompletely(*id);
        }
        nsExit();
    }
    forwarder_package_free(&p);
    return rc;
}

Result forwarder_remove(u64 id)
{
    if (!forwarder_is_one(id)) return NXM_RC_INVALID_ARGUMENT;
    const Result gate = core_change_allowed();
    if (R_FAILED(gate)) return gate;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) return rc;
    rc = nsDeleteApplicationCompletely(id);
    nsExit();
    return rc;
}
