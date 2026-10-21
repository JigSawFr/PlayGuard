// forwarder_pack — the contents of an NRO forwarder, built in memory: what
// the HOME menu needs to show a homebrew as a game (an icon, a name) and
// start it as one. Three NCAs: the program (the forwarder's loader, the
// forwarder/ folder, which reads the .nro's path from its RomFS), the control
// data (control.nacp and the icon), and the content meta that lists them; the
// key and value the content meta database takes; the program ID. As sphaira
// (ITotalJustice, owo.cpp, GPLv3) builds them, after hacbrewpack: plain
// sections (no key area, no title key), a header PlayGuard cannot sign. The
// console starts them only with fs and loader signature patches (sys-patch).
//
// Plain C, no service: the host tests build a forwarder and read it back.
// The SHA-256 and the header's encryption are the caller's (forwarder.c on
// the console, with libnx and the console's header key; the tests their own).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FWD_NACP_SIZE      0x4000
#define FWD_NCA_HEADER     0xC00
#define FWD_PATH_MAX       0x300    // FS_MAX_PATH - 1: what the loader reads of a path
#define FWD_ARGV_MAX       0x7FF    // its argv buffer, less the terminator
#define FWD_NAME_MAX       0x1FF    // a NACP title entry's name, less the terminator
#define FWD_AUTHOR_MAX     0xFF     // its publisher
#define FWD_ID_PREFIX      0x05     // the program ID's top byte, as sphaira gives them

// NcmContentType of each content.
#define FWD_CONTENT_META    0
#define FWD_CONTENT_PROGRAM 1
#define FWD_CONTENT_CONTROL 3

// SHA-256 of `size` bytes, into `out` (32 bytes). Provided by the caller's
// side: forwarder.c on the console, the host test.
void forwarder_sha256(const void *data, size_t size, uint8_t out[32]);

// The program ID of the forwarder that starts `nro_path` with `argv`, as
// sphaira derives it (a SHA-256 of both): the same .nro gives the same ID,
// whichever tool made its forwarder, so a new one replaces the old. A path
// with a space is hashed quoted, as sphaira passes it.
uint64_t forwarder_id(const char *nro_path, const char *argv);

// A growing buffer.
typedef struct {
    uint8_t *data;
    size_t size, cap;
} FwdBuf;
void fwd_buf_free(FwdBuf *b);

typedef struct {
    FwdBuf  nca;        // the encrypted NCA
    uint8_t hash[32];   // its SHA-256; the content ID is its first 16 bytes
    uint8_t type;       // FWD_CONTENT_*
} FwdContent;

typedef struct {
    uint64_t   id;
    FwdContent contents[3];     // program, control, meta: written in that order
    uint8_t    meta_key[0x10];  // NcmContentMetaKey
    uint8_t    meta_value[0x60];// NcmContentMetaHeader, its application header, the three NcmContentInfo
    size_t     meta_value_size;
} FwdPackage;
void forwarder_package_free(FwdPackage *p);

typedef struct {
    const char *nro_path;   // as hbmenu passes it: "sdmc:/switch/x/x.nro"
    const char *argv;       // what the .nro gets as its argv, its path first
    // Started first instead, when not NULL: an .nro that starts the one above
    // itself (envSetNextLoad, with its path and argv) or ends the forwarder,
    // PlayGuard asking for the PIN (util/forwarder_gate.hpp). The loader then
    // goes back to the .nro above, not to it, once that one exits: the
    // forwarder ends. The program ID stays the .nro's.
    const char *gate_path;
    const char *gate_argv;
    // The gate's SHA-256 (32 bytes), or NULL: the loader starts the gate only
    // when its file still has it (nextNroSha256 in the RomFS, on the
    // console's storage), so one changed on the SD card is refused.
    const uint8_t *gate_sha256;
    const char *name;       // the HOME menu's name, in every language (empty: the NACP's)
    const char *author;     // its publisher (empty: the NACP's)
    const void *nacp;       // the .nro's control.nacp (FWD_NACP_SIZE bytes), or NULL for a blank one
    const void *icon;       // JPEG, 256 x 256
    size_t      icon_size;
    const void *exefs_main; // romfs:/forwarder/main
    size_t      main_size;
    const void *exefs_npdm; // romfs:/forwarder/main.npdm
    size_t      npdm_size;
    bool        force_debug;// Atmosphère 1.8.0+: its new ForceDebug bit, as sphaira sets it
    // Encrypts a 0xC00-byte NCA header in place (AES-XTS, the console's
    // header key), before the NCA is hashed. NULL leaves it plain (tests).
    void      (*seal)(void *ctx, uint8_t *header);
    void       *seal_ctx;
} FwdInput;

typedef enum {
    FWD_OK = 0,
    FWD_BAD_INPUT,      // a path, argv, name or author too long, no icon, no loader, a gate without its argv
    FWD_BAD_NPDM,       // the loader's NPDM is not one this code can patch
    FWD_NO_MEMORY,
} FwdError;

// Builds the three NCAs and the database entry into `out` (freed with
// forwarder_package_free, also after an error).
FwdError forwarder_pack(const FwdInput *in, FwdPackage *out);

// Pieces, for the tests.

// The NPDM with `id` as its program ID (ACI0, and ACID's range) and, with
// `force_debug`, the debug flags as sphaira sets them on Atmosphère 1.8.0+.
bool fwd_patch_npdm(uint8_t *npdm, size_t size, uint64_t id, bool force_debug);
// The NACP for the HOME menu: `name` and `author` in every language (when not
// empty), the IDs it names set to `id`, no save data, no account needed.
void fwd_patch_nacp(uint8_t *nacp, const char *name, const char *author, uint64_t id);

typedef struct {
    const char *name;   // in the root folder, without its "/"
    const void *data;
    size_t      size;
} FwdFile;
// A RomFS with `files` in its root folder; *data_size is its size before the
// padding to the hash block.
bool fwd_romfs(const FwdFile *files, uint32_t count, FwdBuf *out, uint64_t *data_size);
// A PFS0 (a partition file system) with `files`.
bool fwd_pfs0(const FwdFile *files, uint32_t count, FwdBuf *out);

#ifdef __cplusplus
}
#endif
