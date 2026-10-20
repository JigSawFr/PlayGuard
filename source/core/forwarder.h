// PlayGuard — NRO forwarders: a homebrew on the HOME menu, with its icon and
// name, started as a game (Console › Forwarders). A forwarder is an
// application PlayGuard builds (forwarder_pack.h) and installs on the SD card
// as sphaira does: its contents written to the SD card's content storage
// (ncm), their entry in its database, then the HOME menu's entry (ns). Its
// program is the loader in romfs:/forwarder/ (the forwarder/ folder), which
// starts the .nro its RomFS names. The console starts it only with fs and
// loader signature patches (sys-patch): util/patches.hpp says whether they
// are there.
//
// Installing and removing are changes: core_change_allowed() first (the
// read-only mode, the PIN asked before a change). Listing only reads.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once
#include "nx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FORWARDER_MAX 64

// One forwarder on the HOME menu: an entry whose program ID starts with
// FWD_ID_PREFIX (forwarder_pack.h), whoever made it.
typedef struct {
    u64  id;
    char name[0x200];     // UTF-8, the console's language; empty when unreadable
    char author[0x100];
    char version[0x10];   // the .nro's version, as its NACP gives it
} ForwarderRecord;

// The forwarders on the HOME menu, at most `max`.
Result forwarder_list(ForwarderRecord *out, u32 max, u32 *count);

// What a forwarder is made of. `nacp` (0x4000 bytes, may be NULL) and `icon`
// (a 256 x 256 JPEG) are the .nro's own, read by util/nro.hpp.
typedef struct {
    const char *nro_path;   // "sdmc:/switch/x/x.nro"
    const char *argv;       // its argv, its path first
    const char *gate_path;  // NULL, or PlayGuard's .nro, asking for the PIN first (FwdInput)
    const char *gate_argv;
    const char *name;       // the HOME menu's name
    const char *author;
    const void *nacp;
    const void *icon;
    size_t      icon_size;
} ForwarderSpec;

// Builds the forwarder and installs it on the SD card, replacing the one
// with the same ID (the same .nro and argv: forwarder_id) when there is one,
// whoever made it. *id: its program ID, set even when the install fails.
// NXM_RC_INVALID_ARGUMENT: a path, name or author too long, no icon, or the
// loader not in romfs:/forwarder/.
Result forwarder_install(const ForwarderSpec *spec, u64 *id);

// Deletes the forwarder `id` (its contents and its HOME menu entry), as
// System Settings' Data Management does. Nothing else of the .nro is touched.
Result forwarder_remove(u64 id);

// Whether `id` is a forwarder's (FWD_ID_PREFIX), which PlayGuard may
// replace or delete.
bool forwarder_is_one(u64 id);

#ifdef __cplusplus
}
#endif
