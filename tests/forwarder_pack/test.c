// Host tests for source/core/forwarder_pack.c: a forwarder built and read
// back as the console reads it. The program ID as sphaira derives it; the
// PFS0 and RomFS layouts (the loader's own lookup of nextNroPath); every hash
// of every section (PFS0 blocks, the six IVFC levels, the section headers);
// the content IDs, taken after the header is sealed; the CNMT and the
// database entry naming the three contents; the NACP and NPDM patches; bad
// input refused.
#include "check.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forwarder_pack.h"

// ------------------------------------------------------------------ SHA-256 (FIPS 180-4)

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static uint32_t ror(uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

static void sha_block(uint32_t *h, const uint8_t *p)
{
    uint32_t w[64];
    for (int i = 0; i < 16; i++) w[i] = (uint32_t)p[4 * i] << 24 | p[4 * i + 1] << 16 | p[4 * i + 2] << 8 | p[4 * i + 3];
    for (int i = 16; i < 64; i++) {
        const uint32_t s0 = ror(w[i - 15], 7) ^ ror(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const uint32_t s1 = ror(w[i - 2], 17) ^ ror(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; i++) {
        const uint32_t t1 = hh + (ror(e, 6) ^ ror(e, 11) ^ ror(e, 25)) + ((e & f) ^ (~e & g)) + K[i] + w[i];
        const uint32_t t2 = (ror(a, 2) ^ ror(a, 13) ^ ror(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

void forwarder_sha256(const void *data, size_t size, uint8_t out[32])
{
    uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    const uint8_t *p = (const uint8_t *)data;
    size_t left = size;
    for (; left >= 64; left -= 64, p += 64) sha_block(h, p);
    uint8_t tail[128] = { 0 };
    memcpy(tail, p, left);
    tail[left] = 0x80;
    const size_t n = left < 56 ? 64 : 128;
    const uint64_t bits = (uint64_t)size * 8;
    for (int i = 0; i < 8; i++) tail[n - 1 - i] = (uint8_t)(bits >> (8 * i));
    sha_block(h, tail);
    if (n == 128) sha_block(h, tail + 64);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 4; j++) out[4 * i + j] = (uint8_t)(h[i] >> (24 - 8 * j));
}

static bool hash_is(const void *data, size_t size, const uint8_t *expected)
{
    uint8_t h[32];
    forwarder_sha256(data, size, h);
    return memcmp(h, expected, 32) == 0;
}

// ------------------------------------------------------------------ readers

static uint32_t u32_at(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint64_t u64_at(const uint8_t *p)
{
    return (uint64_t)u32_at(p) | (uint64_t)u32_at(p + 4) << 32;
}

// A file of a PFS0, by name; NULL when absent.
static const uint8_t *pfs0_file(const uint8_t *pfs0, size_t size, const char *name, uint64_t *file_size)
{
    if (size < 0x10 || u32_at(pfs0) != 0x30534650u) return NULL;
    const uint32_t count = u32_at(pfs0 + 4), strings = u32_at(pfs0 + 8);
    const uint8_t *names = pfs0 + 0x10 + count * 0x18;
    const uint8_t *data = names + strings;
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *e = pfs0 + 0x10 + i * 0x18;
        if (strcmp((const char *)names + u32_at(e + 0x10), name) != 0) continue;
        *file_size = u64_at(e + 8);
        if ((size_t)(data - pfs0) + u64_at(e) + *file_size > size) return NULL;
        return data + u64_at(e);
    }
    return NULL;
}

// A file of a RomFS's root folder, by name, the way the forwarder's loader
// finds nextNroPath: the root's first file, then its siblings.
static const uint8_t *romfs_file(const uint8_t *romfs, size_t size, const char *name, uint64_t *file_size)
{
    if (size < 0x50 || u64_at(romfs) != 0x50) return NULL;
    const uint64_t dir_off = u64_at(romfs + 0x18), file_off = u64_at(romfs + 0x38);
    const uint64_t file_table = u64_at(romfs + 0x40), data = u64_at(romfs + 0x48);
    uint32_t at = u32_at(romfs + dir_off + 0xC);
    const size_t len = strlen(name);
    while (at != 0xFFFFFFFFu && at < file_table) {
        const uint8_t *f = romfs + file_off + at;
        if (u32_at(f + 0x1C) == len && memcmp(f + 0x20, name, len) == 0) {
            *file_size = u64_at(f + 0x10);
            return romfs + data + u64_at(f + 8);
        }
        at = u32_at(f + 4);
    }
    return NULL;
}

// The same through the file hash table, as fs looks a path up.
static bool romfs_hashed(const uint8_t *romfs, const char *name)
{
    const uint64_t hash_off = u64_at(romfs + 0x28), buckets = u64_at(romfs + 0x30) / 4;
    const uint64_t file_off = u64_at(romfs + 0x38);
    uint32_t h = 0 ^ 123456789u;
    for (const char *c = name; *c; c++) {
        h = (h >> 5) | (h << 27);
        h ^= (uint8_t)*c;
    }
    for (uint32_t at = u32_at(romfs + hash_off + 4 * (h % buckets)); at != 0xFFFFFFFFu;) {
        const uint8_t *f = romfs + file_off + at;
        if (u32_at(f + 0x1C) == strlen(name) && memcmp(f + 0x20, name, strlen(name)) == 0) return true;
        at = u32_at(f + 0x18);
    }
    return false;
}

// Section `index` of a plain-header NCA: its bytes, its FS header.
typedef struct {
    const uint8_t *data;
    size_t size;
    const uint8_t *fs;
} Section;

static bool section(const FwdContent *c, int index, Section *s)
{
    const uint8_t *h = c->nca.data;
    const uint8_t *e = h + 0x240 + index * 0x10;
    const uint64_t start = (uint64_t)u32_at(e) * 0x200, end = (uint64_t)u32_at(e + 4) * 0x200;
    if (!start || end <= start || end > c->nca.size) return false;
    s->data = h + start;
    s->size = end - start;
    s->fs = h + 0x400 + index * 0x200;
    // The section header's hash, in the NCA header.
    return hash_is(s->fs, 0x200, h + 0x280 + index * 0x20) && e[8] == 1;
}

// A PFS0 section's PFS0, every hash checked.
static const uint8_t *pfs0_of(const Section *s, uint64_t *size)
{
    const uint8_t *fs = s->fs;
    if (fs[2] != 1 || fs[3] != 2 || fs[4] != 1 || u32_at(fs + 0x2C) != 2) return NULL;
    const uint32_t block = u32_at(fs + 0x28);
    const uint64_t table_off = u64_at(fs + 0x30), table = u64_at(fs + 0x38);
    const uint64_t off = u64_at(fs + 0x40);
    *size = u64_at(fs + 0x48);
    if (table_off + table > s->size || off + *size > s->size || off % 0x200) return NULL;
    if (!hash_is(s->data + table_off, table, fs + 8)) return NULL;
    for (uint64_t i = 0; i * block < *size; i++) {
        const uint64_t left = *size - i * block;
        if ((i + 1) * 32 > table || !hash_is(s->data + off + i * block, left < block ? left : block, s->data + table_off + i * 32))
            return NULL;
    }
    return s->data + off;
}

// A RomFS section's RomFS, every IVFC level checked against the one above.
static const uint8_t *romfs_of(const Section *s, uint64_t *size)
{
    const uint8_t *fs = s->fs;
    if (fs[2] != 0 || fs[3] != 3 || fs[4] != 1 || u32_at(fs + 8) != 0x43465649u || u32_at(fs + 0x14) != 7) return NULL;
    uint64_t off[6], len[6];
    for (int l = 0; l < 6; l++) {
        const uint8_t *info = fs + 0x18 + l * 0x18;
        off[l] = u64_at(info);
        len[l] = u64_at(info + 8);
        if (u32_at(info + 0x10) != 14 || off[l] + len[l] > s->size) return NULL;
        if (l && off[l] != off[l - 1] + len[l - 1]) return NULL;
    }
    if (!hash_is(s->data + off[0], len[0], fs + 0xC8)) return NULL;
    for (int l = 0; l < 5; l++) {
        for (uint64_t b = 0; b * 0x4000 < len[l + 1]; b++) {
            if ((b + 1) * 32 > len[l]) return NULL;
            const uint64_t left = len[l + 1] - b * 0x4000;
            // The last block of the RomFS is hashed with its padding, as
            // sphaira does: the level is padded to the block.
            const uint64_t n = l + 1 == 5 && left < 0x4000 ? 0x4000 : (left < 0x4000 ? left : 0x4000);
            if (!hash_is(s->data + off[l + 1] + b * 0x4000, n, s->data + off[l] + b * 32)) return NULL;
        }
    }
    *size = len[5];
    return s->data + off[5];
}

// ------------------------------------------------------------------ fixtures

#define NPDM_SIZE 0x348

// An NPDM laid out as npdmtool writes the loader's (forwarder/out/
// forwarder.npdm, its sizes made smaller): META, ACI0 at 0x80 with two kernel
// capabilities (the kernel flags, the debug flags), ACID at 0x100 with the
// kernel flags only.
static void make_npdm(uint8_t *n)
{
    memset(n, 0, NPDM_SIZE);
    memcpy(n, "META", 4);
    n[0x70] = 0x80;
    n[0x74] = 0x48;
    n[0x78] = 0x00;
    n[0x79] = 0x01;
    n[0x7C] = 0x48;
    n[0x7D] = 0x02;
    memcpy(n + 0x80, "ACI0", 4);
    n[0x80 + 0x30] = 0x40;
    n[0x80 + 0x34] = 8;
    const uint32_t kc[2] = { 0x020073B7u, 0x0004FFFFu };   // 3 cores; ForceDebugProd
    memcpy(n + 0xC0, kc, 8);
    memcpy(n + 0x100 + 0x200, "ACID", 4);
    n[0x100 + 0x228] = 0x40;
    n[0x100 + 0x229] = 0x02;
    n[0x100 + 0x22C] = 4;
    memcpy(n + 0x340, kc, 4);
}

static int s_seals;

// Stands in for the header key: flips every byte, so a header left plain or
// sealed twice shows.
static void seal(void *ctx, uint8_t *header)
{
    (void)ctx;
    s_seals++;
    for (int i = 0; i < FWD_NCA_HEADER; i++) header[i] ^= 0xFF;
}

static void unseal(FwdContent *c)
{
    for (int i = 0; i < FWD_NCA_HEADER; i++) c->nca.data[i] ^= 0xFF;
}

#define PATH "sdmc:/switch/playguard/playguard.nro"

static uint8_t s_main[0x18000];   // more than one 0x10000 hash block
static uint8_t s_icon[0x6000];

static FwdInput input(const uint8_t *npdm, const uint8_t *nacp)
{
    for (size_t i = 0; i < sizeof(s_main); i++) s_main[i] = (uint8_t)(i * 7);
    for (size_t i = 0; i < sizeof(s_icon); i++) s_icon[i] = (uint8_t)(i * 13 + 1);
    s_icon[0] = 0xFF;
    s_icon[1] = 0xD8;
    FwdInput in = {
        .nro_path = PATH, .argv = PATH, .name = "PlayGuard", .author = "JigSawFr", .nacp = nacp,
        .icon = s_icon, .icon_size = sizeof(s_icon), .exefs_main = s_main, .main_size = sizeof(s_main),
        .exefs_npdm = npdm, .npdm_size = NPDM_SIZE, .force_debug = false, .seal = seal, .seal_ctx = NULL,
    };
    return in;
}

// ------------------------------------------------------------------ tests

static void test_id(void)
{
    // sphaira's: the first 8 bytes of SHA-256(path + argv), little-endian,
    // its 12 low bits clear, 0x05 on top (python3 hashlib gives the same).
    CHECK(forwarder_id(PATH, PATH) == 0x050537E2542E2000ull);
    CHECK(forwarder_id("sdmc:/switch/Checkpoint/Checkpoint.nro", "sdmc:/switch/Checkpoint/Checkpoint.nro") ==
          0x052736A5B3AFA000ull);
    // A path with a space: quoted, as sphaira hashes it (python3 hashlib).
    CHECK(forwarder_id("sdmc:/switch/My App/app.nro", "\"sdmc:/switch/My App/app.nro\"") == 0x0535CFDDDB0FF000ull);
    const uint64_t other = forwarder_id(PATH, PATH " --child");
    CHECK(other != forwarder_id(PATH, PATH));
    CHECK(other >> 56 == FWD_ID_PREFIX && (other & 0xFFF) == 0);
}

static void test_pfs0(void)
{
    const FwdFile files[] = { { "main", "abc", 3 }, { "main.npdm", "defgh", 5 } };
    FwdBuf b = { 0 };
    if (!CHECK(fwd_pfs0(files, 2, &b))) return;
    CHECK(u32_at(b.data) == 0x30534650u && u32_at(b.data + 4) == 2);
    CHECK(u32_at(b.data + 8) % 0x20 == 0);   // the string table, padded
    uint64_t n = 0;
    const uint8_t *f = pfs0_file(b.data, b.size, "main.npdm", &n);
    CHECK(f && n == 5 && memcmp(f, "defgh", 5) == 0);
    f = pfs0_file(b.data, b.size, "main", &n);
    CHECK(f && n == 3 && memcmp(f, "abc", 3) == 0);
    CHECK(!pfs0_file(b.data, b.size, "other", &n));
    fwd_buf_free(&b);
}

static void test_romfs(void)
{
    char big[0x5000];
    memset(big, 'x', sizeof(big));
    const FwdFile files[] = { { "a", "1", 1 }, { "nextNroPath", PATH, strlen(PATH) }, { "big.bin", big, sizeof(big) } };
    FwdBuf b = { 0 };
    uint64_t size = 0;
    if (!CHECK(fwd_romfs(files, 3, &b, &size))) return;
    CHECK(b.size % 0x4000 == 0 && size <= b.size && size > sizeof(big));
    CHECK(u64_at(b.data + 0x48) == 0x200);   // file data after the header's 0x200
    uint64_t n = 0;
    const uint8_t *f = romfs_file(b.data, size, "nextNroPath", &n);
    CHECK(f && n == strlen(PATH) && memcmp(f, PATH, n) == 0);
    f = romfs_file(b.data, size, "big.bin", &n);
    CHECK(f && n == sizeof(big) && (f - b.data - 0x200) % 0x10 == 0 && memcmp(f, big, n) == 0);
    f = romfs_file(b.data, size, "a", &n);
    CHECK(f && n == 1 && *f == '1');
    CHECK(!romfs_file(b.data, size, "nextArgv", &n));
    CHECK(romfs_hashed(b.data, "a") && romfs_hashed(b.data, "nextNroPath") && romfs_hashed(b.data, "big.bin"));
    CHECK(!romfs_hashed(b.data, "missing"));
    // The root folder, alone in its table and its hash table.
    const uint8_t *root = b.data + u64_at(b.data + 0x18);
    CHECK(u64_at(b.data + 0x20) == 0x18 && u32_at(root + 4) == 0xFFFFFFFFu && u32_at(root + 8) == 0xFFFFFFFFu);
    CHECK(u32_at(root + 0x14) == 0);
    fwd_buf_free(&b);

    FwdBuf empty = { 0 };
    CHECK(fwd_romfs(NULL, 0, &empty, &size) && u32_at(empty.data + u64_at(empty.data + 0x18) + 0xC) == 0xFFFFFFFFu);
    fwd_buf_free(&empty);
}

static void test_npdm(void)
{
    uint8_t n[NPDM_SIZE];
    make_npdm(n);
    CHECK(fwd_patch_npdm(n, sizeof(n), 0x0500000000001000ull, false));
    CHECK(u64_at(n + 0x80 + 0x10) == 0x0500000000001000ull);
    CHECK(u64_at(n + 0x100 + 0x210) == 0x0500000000001000ull && u64_at(n + 0x100 + 0x218) == 0x0500000000001000ull);
    CHECK(u32_at(n + 0xC4) == 0x0004FFFFu);   // the debug flags kept
    CHECK(fwd_patch_npdm(n, sizeof(n), 0x0500000000002000ull, true));
    CHECK(u32_at(n + 0xC4) == 0x0008FFFFu);   // ForceDebug, as sphaira
    CHECK(u32_at(n + 0x340) == 0x020073B7u && u32_at(n + 0x344) == 0);   // the ACID's flags left alone
    CHECK(u32_at(n + 0xC0) == 0x020073B7u);   // the kernel flags untouched

    uint8_t bad[NPDM_SIZE];
    make_npdm(bad);
    memcpy(bad, "MATE", 4);
    CHECK(!fwd_patch_npdm(bad, sizeof(bad), 1, false));
    make_npdm(bad);
    bad[0x79] = 0x7F;   // ACID out of the file
    CHECK(!fwd_patch_npdm(bad, sizeof(bad), 1, false));
    make_npdm(bad);
    memset(bad + 0xC4, 0, 4);   // no debug flags to set
    CHECK(fwd_patch_npdm(bad, sizeof(bad), 1, false));
    CHECK(!fwd_patch_npdm(bad, sizeof(bad), 1, true));
    CHECK(!fwd_patch_npdm(bad, 0x40, 1, false));
}

static void test_nacp(void)
{
    uint8_t *nacp = (uint8_t *)calloc(1, FWD_NACP_SIZE);
    strcpy((char *)nacp, "Old name");
    strcpy((char *)nacp + 0x200, "Old author");
    strcpy((char *)nacp + 0x3060, "1.2.3");
    nacp[0x3025] = 1;   // asked for an account at start
    memset(nacp + 0x3080, 0xFF, 8);
    fwd_patch_nacp(nacp, "New", "", 0x0512345678ABC000ull);
    for (int l = 0; l < 16; l++) CHECK(strcmp((const char *)nacp + l * 0x300, "New") == 0);
    CHECK(strcmp((const char *)nacp + 0x200, "Old author") == 0);   // an empty author keeps the NACP's
    CHECK(strcmp((const char *)nacp + 0x300 + 0x200, "") == 0);
    CHECK(strcmp((const char *)nacp + 0x3060, "1.2.3") == 0);       // the version kept
    CHECK(nacp[0x3025] == 0 && u64_at(nacp + 0x3080) == 0);
    CHECK(u64_at(nacp + 0x3078) == 0x0512345678ABC000ull && u64_at(nacp + 0x3038) == 0x0512345678ABC000ull);
    CHECK(u64_at(nacp + 0x3070) == (0x0512345678ABC000ull ^ 0x1000));
    CHECK(u64_at(nacp + 0x30B0) == 0x0512345678ABC000ull && u64_at(nacp + 0x30E8) == 0x0512345678ABC000ull);
    CHECK(memcmp(nacp + 0x30A8, "PlayGrd", 8) == 0);
    CHECK(u32_at(nacp + 0x302C) == 1);   // American English alone: the one icon there is

    // A name too long for its field: cut, terminated.
    char longname[0x300];
    memset(longname, 'n', sizeof(longname) - 1);
    longname[sizeof(longname) - 1] = '\0';
    fwd_patch_nacp(nacp, longname, "A", 1);
    CHECK(strlen((const char *)nacp) == 0x1FF && strcmp((const char *)nacp + 0x200, "A") == 0);

    // The compressed titles of 21.0.0+: back to plain entries.
    memset(nacp, 0xAB, 0x3000);
    nacp[0x3215] = 1;
    fwd_patch_nacp(nacp, "Plain", "Me", 1);
    CHECK(nacp[0x3215] == 0 && strcmp((const char *)nacp + 15 * 0x300, "Plain") == 0);
    CHECK(nacp[0x300 + 0x2FF] == 0);
    free(nacp);
}

static void test_pack(void)
{
    uint8_t npdm[NPDM_SIZE];
    make_npdm(npdm);
    uint8_t *nacp = (uint8_t *)calloc(1, FWD_NACP_SIZE);
    strcpy((char *)nacp, "From the NRO");
    strcpy((char *)nacp + 0x3060, "1.5.0");
    FwdInput in = input(npdm, nacp);
    FwdPackage p;
    s_seals = 0;
    if (!CHECK(forwarder_pack(&in, &p) == FWD_OK)) {
        forwarder_package_free(&p);
        free(nacp);
        return;
    }
    const uint64_t id = forwarder_id(PATH, PATH);
    CHECK(p.id == id);
    CHECK(s_seals == 3);
    // Each content's ID: the hash of the NCA as written, header sealed.
    const uint8_t types[3] = { FWD_CONTENT_PROGRAM, FWD_CONTENT_CONTROL, FWD_CONTENT_META };
    const uint8_t nca_types[3] = { 0, 2, 1 };
    for (int i = 0; i < 3; i++) {
        FwdContent *c = &p.contents[i];
        CHECK(c->type == types[i]);
        CHECK(hash_is(c->nca.data, c->nca.size, c->hash));
        CHECK(c->nca.size % 0x200 == 0);
        CHECK(u32_at(c->nca.data + 0x200) != 0x3341434Eu);   // sealed
        unseal(c);
        const uint8_t *h = c->nca.data;
        CHECK(u32_at(h + 0x200) == 0x3341434Eu);
        CHECK(h[0x204] == 0 && h[0x205] == nca_types[i] && h[0x206] == 0 && h[0x220] == 0);
        CHECK(u64_at(h + 0x208) == c->nca.size && u64_at(h + 0x210) == id);
        CHECK(u32_at(h + 0x21C) == 0x000C1100u);
        for (int k = 0x230; k < 0x240; k++) CHECK(h[k] == 0);   // no rights ID
        for (int k = 0x300; k < 0x340; k++) CHECK(h[k] == 0);   // no key area
    }

    // The program: the loader in its PFS0 (its NPDM patched), the .nro in
    // its RomFS.
    Section s;
    uint64_t size = 0, n = 0;
    const uint8_t *f;
    if (CHECK(section(&p.contents[0], 0, &s))) {
        CHECK(s.data == p.contents[0].nca.data + 0xC00);
        const uint8_t *pfs0 = pfs0_of(&s, &size);
        if (CHECK(pfs0)) {
            f = pfs0_file(pfs0, size, "main", &n);
            CHECK(f && n == sizeof(s_main) && memcmp(f, s_main, n) == 0);
            f = pfs0_file(pfs0, size, "main.npdm", &n);
            CHECK(f && n == NPDM_SIZE && u64_at(f + 0x80 + 0x10) == id && u64_at(f + 0x100 + 0x210) == id);
        }
    }
    if (CHECK(section(&p.contents[0], 1, &s))) {
        const uint8_t *romfs = romfs_of(&s, &size);
        if (CHECK(romfs)) {
            f = romfs_file(romfs, size, "nextNroPath", &n);
            CHECK(f && n == strlen(PATH) && memcmp(f, PATH, n) == 0);
            f = romfs_file(romfs, size, "nextArgv", &n);
            CHECK(f && n == strlen(PATH) && memcmp(f, PATH, n) == 0);
        }
    }
    CHECK(u32_at(p.contents[0].nca.data + 0x240 + 2 * 0x10) == 0);   // no logo section

    // The control data.
    if (CHECK(section(&p.contents[1], 0, &s))) {
        const uint8_t *romfs = romfs_of(&s, &size);
        if (CHECK(romfs)) {
            f = romfs_file(romfs, size, "control.nacp", &n);
            CHECK(f && n == FWD_NACP_SIZE && strcmp((const char *)f, "PlayGuard") == 0 &&
                  strcmp((const char *)f + 0x200, "JigSawFr") == 0 && strcmp((const char *)f + 0x3060, "1.5.0") == 0 &&
                  u64_at(f + 0x3078) == id);
            f = romfs_file(romfs, size, "icon_AmericanEnglish.dat", &n);
            CHECK(f && n == sizeof(s_icon) && memcmp(f, s_icon, n) == 0);
        }
    }
    CHECK(strcmp((const char *)nacp, "From the NRO") == 0);   // the caller's NACP untouched

    // The content meta: the CNMT names the program and the control data.
    if (CHECK(section(&p.contents[2], 0, &s))) {
        const uint8_t *pfs0 = pfs0_of(&s, &size);
        char name[40];
        snprintf(name, sizeof(name), "Application_%016llx.cnmt", (unsigned long long)id);
        f = pfs0 ? pfs0_file(pfs0, size, name, &n) : NULL;
        if (CHECK(f && n == 0xC0)) {
            CHECK(u64_at(f) == id && u32_at(f + 8) == 0 && f[0xC] == 0x80);
            CHECK(f[0xE] == 0x10 && f[0x10] == 2 && f[0x12] == 0);
            CHECK(u64_at(f + 0x20) == (id | 0x800));
            for (int i = 0; i < 2; i++) {
                const uint8_t *c = f + 0x30 + i * 0x38;
                CHECK(memcmp(c, p.contents[i].hash, 32) == 0 && memcmp(c + 0x20, p.contents[i].hash, 16) == 0);
                CHECK(u32_at(c + 0x30) == p.contents[i].nca.size && c[0x36] == p.contents[i].type);
            }
        }
    }

    // The database: its key and value.
    CHECK(u64_at(p.meta_key) == id && u32_at(p.meta_key + 8) == 0 && p.meta_key[0xC] == 0x80 && p.meta_key[0xD] == 0);
    CHECK(p.meta_value_size == 0x60);
    const uint8_t *v = p.meta_value;
    CHECK(v[0] == 0x10 && v[2] == 3 && v[4] == 0 && u64_at(v + 8) == (id | 0x800));
    const int order[3] = { 2, 0, 1 };   // the meta first
    for (int i = 0; i < 3; i++) {
        const FwdContent *c = &p.contents[order[i]];
        const uint8_t *info = v + 0x18 + i * 0x18;
        CHECK(memcmp(info, c->hash, 16) == 0 && u32_at(info + 0x10) == c->nca.size && info[0x14] == 0);
        CHECK(info[0x16] == c->type);
    }
    forwarder_package_free(&p);

    // The same input, the same bytes: an edit with nothing changed rebuilds
    // the same contents.
    FwdPackage again, first;
    CHECK(forwarder_pack(&in, &first) == FWD_OK && forwarder_pack(&in, &again) == FWD_OK);
    for (int i = 0; i < 3; i++) CHECK(memcmp(first.contents[i].hash, again.contents[i].hash, 32) == 0);
    forwarder_package_free(&first);
    forwarder_package_free(&again);

    // No NACP: a blank one with the name.
    in.nacp = NULL;
    in.seal = NULL;
    if (CHECK(forwarder_pack(&in, &p) == FWD_OK) && CHECK(section(&p.contents[1], 0, &s))) {
        const uint8_t *romfs = romfs_of(&s, &size);
        f = romfs ? romfs_file(romfs, size, "control.nacp", &n) : NULL;
        CHECK(f && strcmp((const char *)f + 15 * 0x300, "PlayGuard") == 0 && f[0x3060] == 0);
    }
    forwarder_package_free(&p);
    free(nacp);
}

static void test_refused(void)
{
    uint8_t npdm[NPDM_SIZE];
    make_npdm(npdm);
    FwdPackage p;
    FwdInput in = input(npdm, NULL);
    char path[FWD_PATH_MAX + 2];
    memset(path, 'p', sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    in.nro_path = path;
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_INPUT);
    forwarder_package_free(&p);
    in = input(npdm, NULL);
    in.icon = NULL;
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_INPUT);
    forwarder_package_free(&p);
    in = input(npdm, NULL);
    in.exefs_main = NULL;
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_INPUT);
    forwarder_package_free(&p);
    in = input(npdm, NULL);
    in.nro_path = "";
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_INPUT);
    forwarder_package_free(&p);
    char name[FWD_NAME_MAX + 2];
    memset(name, 'n', sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    in = input(npdm, NULL);
    in.name = name;
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_INPUT);
    forwarder_package_free(&p);
    uint8_t bad[NPDM_SIZE];
    make_npdm(bad);
    memcpy(bad + 0x80, "ACIX", 4);
    in = input(bad, NULL);
    CHECK(forwarder_pack(&in, &p) == FWD_BAD_NPDM);
    forwarder_package_free(&p);
}

int main(void)
{
    test_id();
    test_pfs0();
    test_romfs();
    test_npdm();
    test_nacp();
    test_pack();
    test_refused();
    return CHECK_DONE("forwarder_pack tests passed");
}
