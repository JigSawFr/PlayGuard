// sync_system — the system document (docs/sync-protocol.md, `system`): what
// the console is and how it is doing, beside the parental controls the state
// document carries: versions, model, name, region, which system runs, the
// serial number (only when sync.conf's publish_serial says so), the space
// left, power, network and when it started. Built by PlayGuard and by the
// agent from the same readers (core/console_info.h); unknown values are null.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../core/console_info.h"
#include "../core/sysinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t            ts;              // POSIX seconds; 0 unknown (then no boot_ts)
    const SysInfo      *sys;
    const ConsoleLive  *live;
    const ConsoleSpace *sd, *nand;       // NULL: not read yet
    const char         *hekate;          // "6.5.4"; NULL or "" unknown
    const char         *app_version;     // PlayGuard's; NULL unknown
    const char         *agent_version;   // NULL: no agent
    const char         *serial;          // NULL: not published (or not known)
} SyncSystem;

size_t sync_system_build(const SyncSystem *s, char *out, size_t cap);

#ifdef __cplusplus
}
#endif
