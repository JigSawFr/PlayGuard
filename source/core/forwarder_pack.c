// PlayGuard — an NRO forwarder's contents, built in memory (see forwarder_pack.h).
// The layouts follow sphaira's owo.cpp (ITotalJustice, GPLv3), itself after
// hacbrewpack, and switchbrew's NCA, PFS0, RomFS, CNMT, NACP and NPDM pages.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "forwarder_pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NCA3_MAGIC            0x3341434Eu   // "NCA3"
#define PFS0_MAGIC            0x30534650u   // "PFS0"
#define IVFC_MAGIC            0x43465649u   // "IVFC"
#define NCA_SDK_VERSION       0x000C1100u   // what sphaira writes
#define MEDIA_UNIT            0x200
#define PFS0_EXEFS_BLOCK      0x10000
#define PFS0_META_BLOCK       0x1000
#define IVFC_BLOCK            0x4000
#define IVFC_BLOCK_LOG2       14
#define IVFC_LEVELS           6
#define ROMFS_FILE_DATA       0x200
#define ROMFS_EMPTY           0xFFFFFFFFu
#define META_TYPE_APPLICATION 0x80

// NCA header offsets (switchbrew NCA).
#define NCA_MAGIC        0x200
#define NCA_CONTENT_TYPE 0x205
#define NCA_SIZE         0x208
#define NCA_PROGRAM_ID   0x210
#define NCA_SDK          0x21C
#define NCA_FS_TABLE     0x240
#define NCA_FS_HASHES    0x280
#define NCA_FS_HEADERS   0x400
#define NCA_FS_HEADER    0x200

// NCA content types.
#define NCA_PROGRAM 0
#define NCA_META    1
#define NCA_CONTROL 2

// ----------------------------------------------------------------- buffers

void fwd_buf_free(FwdBuf *b)
{
    free(b->data);
    b->data = NULL;
    b->size = b->cap = 0;
}

static bool buf_reserve(FwdBuf *b, size_t size)
{
    if (size <= b->cap) return true;
    size_t cap = b->cap ? b->cap : 0x1000;
    while (cap < size) cap *= 2;
    uint8_t *grown = (uint8_t *)realloc(b->data, cap);
    if (!grown) return false;
    memset(grown + b->cap, 0, cap - b->cap);
    b->data = grown;
    b->cap  = cap;
    return true;
}

// Writes at `off`, growing the buffer (and its size) as needed; the bytes
// skipped stay zero.
static bool buf_put(FwdBuf *b, size_t off, const void *data, size_t size)
{
    if (!buf_reserve(b, off + size)) return false;
    if (size) memcpy(b->data + off, data, size);
    if (off + size > b->size) b->size = off + size;
    return true;
}

static bool buf_append(FwdBuf *b, const void *data, size_t size)
{
    return buf_put(b, b->size, data, size);
}

// Zeroes up to the next multiple of `align`.
static bool buf_align(FwdBuf *b, size_t align)
{
    const size_t to = (b->size + align - 1) / align * align;
    if (!buf_reserve(b, to)) return false;
    b->size = to;
    return true;
}

static void put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

static void put_u32(uint8_t *p, uint32_t v)
{
    for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i));
}

static void put_u64(uint8_t *p, uint64_t v)
{
    for (int i = 0; i < 8; i++) p[i] = (uint8_t)(v >> (8 * i));
}

static uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint64_t align_up(uint64_t v, uint64_t align)
{
    return (v + align - 1) / align * align;
}

// ----------------------------------------------------------------- the ID

uint64_t forwarder_id(const char *nro_path, const char *argv)
{
    // sphaira hashes the path followed by the argv (which starts with the
    // path again), and keeps the first eight bytes, little-endian. A path
    // with a space, it has quoted (nro_add_arg_file): quoted here too.
    const bool quote = strchr(nro_path, ' ') != NULL;
    const size_t a = strlen(nro_path), b = strlen(argv);
    char *both = (char *)malloc(a + b + 3);
    if (!both) return 0;
    size_t n = 0;
    if (quote) both[n++] = '"';
    memcpy(both + n, nro_path, a);
    n += a;
    if (quote) both[n++] = '"';
    memcpy(both + n, argv, b + 1);
    uint8_t hash[32];
    forwarder_sha256(both, n + b, hash);
    free(both);
    uint64_t low = 0;
    for (int i = 7; i >= 0; i--) low = low << 8 | hash[i];
    return (uint64_t)FWD_ID_PREFIX << 56 | (low & 0x00FFFFFFFFFFF000ull);
}

// ----------------------------------------------------------------- NPDM

// Replaces the first kernel capability of `kind` (its low `kind` bits set,
// the next one clear) in [off, off + size) with `value`.
static bool patch_capability(uint8_t *npdm, size_t npdm_size, uint32_t off, uint32_t size, uint32_t kind, uint32_t value)
{
    const uint32_t pattern = (1u << kind) - 1, mask = (1u << kind) | pattern;
    if (off > npdm_size || size > npdm_size - off) return false;
    for (uint32_t i = 0; i + 4 <= size; i += 4) {
        if ((get_u32(npdm + off + i) & mask) != pattern) continue;
        put_u32(npdm + off + i, value | pattern);
        return true;
    }
    return false;
}

bool fwd_patch_npdm(uint8_t *npdm, size_t size, uint64_t id, bool force_debug)
{
    // META: ACI0 at 0x70, ACID at 0x78 (offset, size). ACI0: its program ID
    // at 0x10, kernel capabilities at 0x30. ACID: the program ID range at
    // 0x210, kernel capabilities at 0x230 (0x228 holds the services').
    if (size < 0x80 || get_u32(npdm) != 0x4154454Du) return false;   // "META"
    const uint32_t aci0 = get_u32(npdm + 0x70), acid = get_u32(npdm + 0x78);
    if (aci0 > size || size - aci0 < 0x40 || acid > size || size - acid < 0x240) return false;
    if (get_u32(npdm + aci0) != 0x30494341u || get_u32(npdm + acid + 0x200) != 0x44494341u) return false;   // "ACI0", "ACID"
    put_u64(npdm + aci0 + 0x10, id);
    put_u64(npdm + acid + 0x210, id);
    put_u64(npdm + acid + 0x218, id);
    if (force_debug) {
        // The debug flags (kind 16): Atmosphère 1.8.0 moved ForceDebug to
        // bit 19 for 19.0.0's layout, which sphaira sets in its place. Both
        // lists have them (npdmtool copies the ACI0's into the ACID), and
        // both take it: the loader refuses an ACI0 asking for a flag its
        // ACID does not grant ("The software could not be started").
        const uint32_t debug = 1u << 19;
        if (!patch_capability(npdm, size, aci0 + get_u32(npdm + aci0 + 0x30), get_u32(npdm + aci0 + 0x34), 16, debug) ||
            !patch_capability(npdm, size, acid + get_u32(npdm + acid + 0x230), get_u32(npdm + acid + 0x234), 16, debug))
            return false;
    }
    return true;
}

// ----------------------------------------------------------------- NACP

// Offsets in a control.nacp (libnx NacpStruct).
#define NACP_LANGS            16
#define NACP_LANG_SIZE        0x300
#define NACP_AUTHOR           0x200
#define NACP_TITLES_FORMAT    0x3215

static void put_text(uint8_t *dst, size_t size, const char *text)
{
    memset(dst, 0, size);
    const size_t n = strlen(text);
    memcpy(dst, text, n < size - 1 ? n : size - 1);
}

void fwd_patch_nacp(uint8_t *nacp, const char *name, const char *author, uint64_t id)
{
    // A compressed title block (21.0.0+) is not one names can be written
    // into: back to the plain entries, which every firmware reads.
    if ((name && *name) || (author && *author) || nacp[NACP_TITLES_FORMAT]) {
        if (nacp[NACP_TITLES_FORMAT]) {
            memset(nacp, 0, NACP_LANGS * NACP_LANG_SIZE);
            nacp[NACP_TITLES_FORMAT] = 0;
        }
        for (int l = 0; l < NACP_LANGS; l++) {
            uint8_t *entry = nacp + l * NACP_LANG_SIZE;
            if (name && *name) put_text(entry, NACP_AUTHOR, name);
            if (author && *author) put_text(entry + NACP_AUTHOR, NACP_LANG_SIZE - NACP_AUTHOR, author);
        }
    }
    // supported_language_flag: American English alone. The HOME menu reads
    // the icon and the name of the console's language when the NACP says it
    // supports it, and there is only icon_AmericanEnglish.dat: a .nro whose
    // NACP lists French would show no icon on a French console. As NTON
    // fixed it; with no language listed, the console takes this one anyway.
    put_u32(nacp + 0x302C, 1);
    nacp[0x3025] = 0;      // startup_user_account: no account picked at start
    nacp[0x3026] = 0;      // user_account_switch_lock
    nacp[0x3027] = 1;      // add_on_content_registration_type: on demand
    nacp[0x3034] = 0;      // screenshot: allowed
    nacp[0x3035] = 2;      // video_capture: automatic
    nacp[0x3036] = 0;      // data_loss_confirmation: none, it keeps no save data
    nacp[0x3037] = 0;      // play_log_policy: logged, as a game
    put_u64(nacp + 0x3038, id);                 // presence_group_id
    put_u64(nacp + 0x3070, id ^ 0x1000);        // add_on_content_base_id
    put_u64(nacp + 0x3078, id);                 // save_data_owner_id
    for (int i = 0; i < 4; i++) put_u64(nacp + 0x3080 + 8 * i, 0);   // save data sizes
    memset(nacp + 0x30A8, 0, 8);
    memcpy(nacp + 0x30A8, "PlayGrd", 7);        // application_error_code_category (a char[8])
    for (int i = 0; i < 8; i++) put_u64(nacp + 0x30B0 + 8 * i, id);  // local_communication_id
    nacp[0x30F0] = 2;      // logo_type: Nintendo
    nacp[0x30F1] = 0;      // logo_handling: automatic
    put_u64(nacp + 0x30F8, id);                 // pseudo_device_id_seed
    for (int i = 0; i < 4; i++) put_u64(nacp + 0x3148 + 8 * i, 0);   // their maximums
    nacp[0x3210] = 0;      // play_log_query_capability
    nacp[0x3213] = 0;      // required_network_service_license_on_launch
}

// ----------------------------------------------------------------- PFS0

bool fwd_pfs0(const FwdFile *files, uint32_t count, FwdBuf *out)
{
    size_t names = 0;
    for (uint32_t i = 0; i < count; i++) names += strlen(files[i].name) + 1;
    const size_t table = align_up(names, 0x20);
    uint8_t header[0x10] = { 0 };
    put_u32(header, PFS0_MAGIC);
    put_u32(header + 4, count);
    put_u32(header + 8, (uint32_t)table);
    if (!buf_append(out, header, sizeof(header))) return false;
    uint64_t data = 0;
    uint32_t name = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t entry[0x18] = { 0 };
        put_u64(entry, data);
        put_u64(entry + 8, files[i].size);
        put_u32(entry + 0x10, name);
        if (!buf_append(out, entry, sizeof(entry))) return false;
        data += files[i].size;
        name += (uint32_t)strlen(files[i].name) + 1;
    }
    const size_t strings = out->size;
    for (uint32_t i = 0; i < count; i++)
        if (!buf_append(out, files[i].name, strlen(files[i].name) + 1)) return false;
    if (!buf_put(out, strings + table, NULL, 0)) return false;   // the table's padding
    out->size = strings + table;
    for (uint32_t i = 0; i < count; i++)
        if (!buf_append(out, files[i].data, files[i].size)) return false;
    return true;
}

// ----------------------------------------------------------------- RomFS

static uint32_t romfs_hash(uint32_t parent, const char *name, size_t len)
{
    uint32_t h = parent ^ 123456789u;
    for (size_t i = 0; i < len; i++) {
        h = (h >> 5) | (h << 27);
        h ^= (uint8_t)name[i];
    }
    return h;
}

static uint32_t romfs_buckets(uint32_t entries)
{
    if (entries < 3) return 3;
    if (entries < 19) return entries | 1;
    uint32_t n = entries;
    while (n % 2 == 0 || n % 3 == 0 || n % 5 == 0 || n % 7 == 0 || n % 11 == 0 || n % 13 == 0 || n % 17 == 0) n++;
    return n;
}

bool fwd_romfs(const FwdFile *files, uint32_t count, FwdBuf *out, uint64_t *data_size)
{
    // The root folder only: one folder entry, `count` file entries, in the
    // order given, linked as siblings.
    const uint32_t dir_table = 0x18;
    uint32_t file_table = 0;
    for (uint32_t i = 0; i < count; i++) file_table += 0x20 + (uint32_t)align_up(strlen(files[i].name), 4);
    const uint32_t dir_buckets = romfs_buckets(1), file_buckets = romfs_buckets(count);

    uint64_t data = 0;
    uint64_t *data_off = (uint64_t *)calloc(count ? count : 1, sizeof(uint64_t));
    uint32_t *buckets = (uint32_t *)malloc((size_t)file_buckets * 4);
    uint8_t *ftable = (uint8_t *)calloc(file_table ? file_table : 1, 1);
    if (!data_off || !buckets || !ftable) {
        free(data_off);
        free(buckets);
        free(ftable);
        return false;
    }
    for (uint32_t i = 0; i < count; i++) {
        data = align_up(data, 0x10);
        data_off[i] = data;
        data += files[i].size;
    }
    for (uint32_t i = 0; i < file_buckets; i++) buckets[i] = ROMFS_EMPTY;

    uint32_t entry = 0;
    for (uint32_t i = 0; i < count; i++) {
        const size_t len = strlen(files[i].name);
        const uint32_t size = 0x20 + (uint32_t)align_up(len, 4);
        uint8_t *e = ftable + entry;
        put_u32(e, 0);   // parent: the root
        put_u32(e + 4, i + 1 < count ? entry + size : ROMFS_EMPTY);
        put_u64(e + 8, data_off[i]);
        put_u64(e + 0x10, files[i].size);
        const uint32_t b = romfs_hash(0, files[i].name, len) % file_buckets;
        put_u32(e + 0x18, buckets[b]);
        buckets[b] = entry;
        put_u32(e + 0x1C, (uint32_t)len);
        memcpy(e + 0x20, files[i].name, len);
        entry += size;
    }

    uint8_t root[0x18];
    put_u32(root, 0);
    put_u32(root + 4, ROMFS_EMPTY);
    put_u32(root + 8, ROMFS_EMPTY);
    put_u32(root + 0xC, count ? 0 : ROMFS_EMPTY);
    put_u32(root + 0x10, ROMFS_EMPTY);
    put_u32(root + 0x14, 0);
    uint32_t dbuckets[3];
    for (uint32_t i = 0; i < dir_buckets; i++) dbuckets[i] = ROMFS_EMPTY;
    dbuckets[romfs_hash(0, "", 0) % dir_buckets] = 0;

    const uint64_t dir_hash_off = align_up(ROMFS_FILE_DATA + data, 4);
    const uint64_t dir_off = dir_hash_off + 4ull * dir_buckets;
    const uint64_t file_hash_off = dir_off + dir_table;
    const uint64_t file_off = file_hash_off + 4ull * file_buckets;
    uint8_t header[0x50];
    put_u64(header, 0x50);
    put_u64(header + 0x08, dir_hash_off);
    put_u64(header + 0x10, 4ull * dir_buckets);
    put_u64(header + 0x18, dir_off);
    put_u64(header + 0x20, dir_table);
    put_u64(header + 0x28, file_hash_off);
    put_u64(header + 0x30, 4ull * file_buckets);
    put_u64(header + 0x38, file_off);
    put_u64(header + 0x40, file_table);
    put_u64(header + 0x48, ROMFS_FILE_DATA);

    const size_t base = out->size;
    bool ok = buf_put(out, base, header, sizeof(header));
    for (uint32_t i = 0; ok && i < count; i++)
        ok = buf_put(out, base + ROMFS_FILE_DATA + data_off[i], files[i].data, files[i].size);
    uint8_t word[4];
    for (uint32_t i = 0; ok && i < dir_buckets; i++) {
        put_u32(word, dbuckets[i]);
        ok = buf_put(out, base + dir_hash_off + 4 * i, word, 4);
    }
    ok = ok && buf_put(out, base + dir_off, root, sizeof(root));
    for (uint32_t i = 0; ok && i < file_buckets; i++) {
        put_u32(word, buckets[i]);
        ok = buf_put(out, base + file_hash_off + 4 * i, word, 4);
    }
    ok = ok && buf_put(out, base + file_off, ftable, file_table);
    free(data_off);
    free(buckets);
    free(ftable);
    if (!ok) return false;
    *data_size = out->size - base;
    return buf_align(out, IVFC_BLOCK);
}

// ----------------------------------------------------------------- NCA

// One NCA being built: its header apart, its sections after 0xC00.
typedef struct {
    uint8_t header[FWD_NCA_HEADER];
    FwdBuf  body;   // the whole NCA, the header's room included
} Nca;

static bool nca_begin(Nca *n)
{
    memset(n, 0, sizeof(*n));
    return buf_put(&n->body, 0, n->header, FWD_NCA_HEADER);
}

static uint8_t *fs_header(Nca *n, int index)
{
    return n->header + NCA_FS_HEADERS + index * NCA_FS_HEADER;
}

// Closes section `index`, which started at `start`: the body aligned to a
// media unit, its entry in the section table, its header's hash.
static bool nca_section_end(Nca *n, int index, size_t start)
{
    if (!buf_align(&n->body, MEDIA_UNIT)) return false;
    uint8_t *e = n->header + NCA_FS_TABLE + index * 0x10;
    put_u32(e, (uint32_t)(start / MEDIA_UNIT));
    put_u32(e + 4, (uint32_t)(n->body.size / MEDIA_UNIT));
    e[8] = 1;   // as hacbrewpack and sphaira write it
    forwarder_sha256(fs_header(n, index), NCA_FS_HEADER, n->header + NCA_FS_HASHES + index * 0x20);
    return true;
}

// A PFS0 section: its hash table (one SHA-256 per block), then the PFS0.
static bool nca_pfs0(Nca *n, int index, const FwdFile *files, uint32_t count, uint32_t block)
{
    FwdBuf pfs0 = { 0 };
    if (!fwd_pfs0(files, count, &pfs0)) {
        fwd_buf_free(&pfs0);
        return false;
    }
    const size_t blocks = (pfs0.size + block - 1) / block;
    uint8_t *table = (uint8_t *)malloc(blocks * 32 + 1);
    if (!table) {
        fwd_buf_free(&pfs0);
        return false;
    }
    for (size_t i = 0; i < blocks; i++) {
        const size_t off = i * block, left = pfs0.size - off;
        forwarder_sha256(pfs0.data + off, left < block ? left : block, table + i * 32);
    }
    const size_t start = n->body.size, table_size = blocks * 32;
    const size_t pfs0_off = align_up(table_size, MEDIA_UNIT);
    bool ok = buf_append(&n->body, table, table_size) && buf_put(&n->body, start + pfs0_off, pfs0.data, pfs0.size);

    uint8_t *h = fs_header(n, index);
    put_u16(h, 2);    // version
    h[2] = 1;         // PFS0
    h[3] = 2;         // HierarchicalSha256
    h[4] = 1;         // not encrypted
    forwarder_sha256(table, table_size, h + 0x08);   // master hash
    put_u32(h + 0x28, block);
    put_u32(h + 0x2C, 2);                            // layers
    put_u64(h + 0x30, 0);                            // the hash table
    put_u64(h + 0x38, table_size);
    put_u64(h + 0x40, pfs0_off);                     // the PFS0
    put_u64(h + 0x48, pfs0.size);
    free(table);
    fwd_buf_free(&pfs0);
    return ok && nca_section_end(n, index, start);
}

// A RomFS section: its six IVFC levels, the RomFS last, each hashing the
// next by 0x4000-byte blocks.
static bool nca_romfs(Nca *n, int index, const FwdFile *files, uint32_t count)
{
    FwdBuf level[IVFC_LEVELS] = { { 0 } };
    uint64_t sizes[IVFC_LEVELS];
    bool ok = fwd_romfs(files, count, &level[IVFC_LEVELS - 1], &sizes[IVFC_LEVELS - 1]);
    for (int l = IVFC_LEVELS - 2; ok && l >= 0; l--) {
        const FwdBuf *src = &level[l + 1];
        uint8_t hash[32];
        for (size_t off = 0; ok && off < src->size; off += IVFC_BLOCK) {
            const size_t left = src->size - off;
            forwarder_sha256(src->data + off, left < IVFC_BLOCK ? left : IVFC_BLOCK, hash);
            ok = buf_append(&level[l], hash, sizeof(hash));
        }
        ok = ok && buf_align(&level[l], IVFC_BLOCK);
        sizes[l] = level[l].size;
    }
    const size_t start = n->body.size;
    uint8_t *h = fs_header(n, index);
    put_u16(h, 2);    // version
    h[2] = 0;         // RomFS
    h[3] = 3;         // HierarchicalIntegrity
    h[4] = 1;         // not encrypted
    put_u32(h + 0x08, IVFC_MAGIC);
    put_u32(h + 0x0C, 0x20000);
    put_u32(h + 0x10, 32);                 // master hash size
    put_u32(h + 0x14, IVFC_LEVELS + 1);    // max layers
    uint64_t logical = 0;
    for (int l = 0; ok && l < IVFC_LEVELS; l++) {
        uint8_t *info = h + 0x18 + l * 0x18;
        put_u64(info, logical);
        put_u64(info + 8, sizes[l]);
        put_u32(info + 0x10, IVFC_BLOCK_LOG2);
        logical += sizes[l];
        ok = buf_append(&n->body, level[l].data, level[l].size);
    }
    if (ok) forwarder_sha256(level[0].data, level[0].size, h + 0xC8);   // master hash
    for (int l = 0; l < IVFC_LEVELS; l++) fwd_buf_free(&level[l]);
    return ok && nca_section_end(n, index, start);
}

// The header's fields, sealed by the caller, then the NCA hashed.
static bool nca_finish(Nca *n, const FwdInput *in, uint64_t id, uint8_t nca_type, uint8_t ncm_type, FwdContent *out)
{
    put_u32(n->header + NCA_MAGIC, NCA3_MAGIC);
    n->header[NCA_CONTENT_TYPE] = nca_type;   // distribution 0 (system), key generation 0, no key area
    put_u64(n->header + NCA_SIZE, n->body.size);
    put_u64(n->header + NCA_PROGRAM_ID, id);
    put_u32(n->header + NCA_SDK, NCA_SDK_VERSION);
    if (in->seal) in->seal(in->seal_ctx, n->header);
    memcpy(n->body.data, n->header, FWD_NCA_HEADER);
    out->nca  = n->body;
    out->type = ncm_type;
    forwarder_sha256(out->nca.data, out->nca.size, out->hash);
    n->body = (FwdBuf){ 0 };
    return true;
}

// A content's entry, as both the CNMT and the database list it.
static void content_info(uint8_t *p, const FwdContent *c)
{
    memcpy(p, c->hash, 16);   // content ID
    put_u32(p + 0x10, (uint32_t)c->nca.size);
    p[0x14] = (uint8_t)((uint64_t)c->nca.size >> 32);
    p[0x15] = 0;              // attributes
    p[0x16] = c->type;
    p[0x17] = 0;              // ID offset
}

// ----------------------------------------------------------------- the package

void forwarder_package_free(FwdPackage *p)
{
    for (int i = 0; i < 3; i++) fwd_buf_free(&p->contents[i].nca);
}

static bool fits(const char *s, size_t max)
{
    return s && strlen(s) <= max;
}

FwdError forwarder_pack(const FwdInput *in, FwdPackage *out)
{
    memset(out, 0, sizeof(*out));
    if (!fits(in->nro_path, FWD_PATH_MAX) || !*in->nro_path || !fits(in->argv, FWD_ARGV_MAX) ||
        !fits(in->name ? in->name : "", FWD_NAME_MAX) || !fits(in->author ? in->author : "", FWD_AUTHOR_MAX) ||
        !in->icon || !in->icon_size || !in->exefs_main || !in->main_size || !in->exefs_npdm || !in->npdm_size)
        return FWD_BAD_INPUT;
    const bool gate = in->gate_path != NULL;
    if (gate && (!*in->gate_path || !fits(in->gate_path, FWD_PATH_MAX) || !fits(in->gate_argv, FWD_ARGV_MAX)))
        return FWD_BAD_INPUT;
    const uint64_t id = forwarder_id(in->nro_path, in->argv);
    if (!id) return FWD_NO_MEMORY;
    out->id = id;
    Nca n;

    // The program: the loader, and the .nro's path and argv in its RomFS.
    uint8_t *npdm = (uint8_t *)malloc(in->npdm_size);
    if (!npdm) return FWD_NO_MEMORY;
    memcpy(npdm, in->exefs_npdm, in->npdm_size);
    if (!fwd_patch_npdm(npdm, in->npdm_size, id, in->force_debug)) {
        free(npdm);
        return FWD_BAD_NPDM;
    }
    const FwdFile exefs[] = { { "main", in->exefs_main, in->main_size }, { "main.npdm", npdm, in->npdm_size } };
    // With a gate, the gate first, and the .nro as where the loader returns
    // (sphaira's hbl: returnNroPath, returnArgv).
    const char *first = gate ? in->gate_path : in->nro_path, *first_argv = gate ? in->gate_argv : in->argv;
    const FwdFile args[] = { { "nextArgv", first_argv, strlen(first_argv) },
                             { "nextNroPath", first, strlen(first) },
                             { "returnArgv", in->argv, strlen(in->argv) },
                             { "returnNroPath", in->nro_path, strlen(in->nro_path) },
                             { "nextNroSha256", in->gate_sha256, 32 } };
    const uint32_t nargs = !gate ? 2 : in->gate_sha256 ? 5 : 4;
    bool ok = nca_begin(&n) && nca_pfs0(&n, 0, exefs, 2, PFS0_EXEFS_BLOCK) && nca_romfs(&n, 1, args, nargs) &&
              nca_finish(&n, in, id, NCA_PROGRAM, FWD_CONTENT_PROGRAM, &out->contents[0]);
    free(npdm);
    fwd_buf_free(&n.body);
    if (!ok) return FWD_NO_MEMORY;

    // The control data: the NACP for the HOME menu, and the icon.
    uint8_t *nacp = (uint8_t *)calloc(1, FWD_NACP_SIZE);
    if (!nacp) return FWD_NO_MEMORY;
    if (in->nacp) memcpy(nacp, in->nacp, FWD_NACP_SIZE);
    fwd_patch_nacp(nacp, in->name, in->author, id);
    const FwdFile control[] = { { "control.nacp", nacp, FWD_NACP_SIZE },
                                { "icon_AmericanEnglish.dat", in->icon, in->icon_size } };
    ok = nca_begin(&n) && nca_romfs(&n, 0, control, 2) &&
         nca_finish(&n, in, id, NCA_CONTROL, FWD_CONTENT_CONTROL, &out->contents[1]);
    free(nacp);
    fwd_buf_free(&n.body);
    if (!ok) return FWD_NO_MEMORY;

    // The content meta: the CNMT (its header, the application's, the two
    // contents and their hashes, an empty digest) in a PFS0.
    uint8_t cnmt[0x20 + 0x10 + 2 * 0x38 + 0x20] = { 0 };
    put_u64(cnmt, id);
    cnmt[0x0C] = META_TYPE_APPLICATION;
    put_u16(cnmt + 0x0E, 0x10);   // extended header size
    put_u16(cnmt + 0x10, 2);      // contents
    put_u64(cnmt + 0x20, id | 0x800);   // patch ID
    for (int i = 0; i < 2; i++) {
        uint8_t *p = cnmt + 0x30 + i * 0x38;
        memcpy(p, out->contents[i].hash, 32);
        content_info(p + 0x20, &out->contents[i]);
    }
    char cnmt_name[40];
    snprintf(cnmt_name, sizeof(cnmt_name), "Application_%016llx.cnmt", (unsigned long long)id);
    const FwdFile meta[] = { { cnmt_name, cnmt, sizeof(cnmt) } };
    ok = nca_begin(&n) && nca_pfs0(&n, 0, meta, 1, PFS0_META_BLOCK) &&
         nca_finish(&n, in, id, NCA_META, FWD_CONTENT_META, &out->contents[2]);
    fwd_buf_free(&n.body);
    if (!ok) return FWD_NO_MEMORY;

    // The database's key (NcmContentMetaKey: version 0, an application, a
    // full install) and value (its header with the three contents, the
    // application's header, the contents, the meta first).
    put_u64(out->meta_key, id);
    out->meta_key[0x0C] = META_TYPE_APPLICATION;
    uint8_t *v = out->meta_value;
    put_u16(v, 0x10);
    put_u16(v + 2, 3);
    put_u64(v + 8, id | 0x800);
    content_info(v + 0x18, &out->contents[2]);
    content_info(v + 0x30, &out->contents[0]);
    content_info(v + 0x48, &out->contents[1]);
    out->meta_value_size = 0x60;
    return FWD_OK;
}
