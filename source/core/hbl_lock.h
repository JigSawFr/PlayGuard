// hbl_lock — the homebrew menu locked (Console › Forwarders): the album and
// a game started while R is held no longer open the homebrew menu, so a
// homebrew starts only from its forwarder on the HOME menu, which can ask
// for the PIN (util/forwarder_gate.hpp). Shared by the app and the rescue
// sysmodule (sysmodule/), which can lift it at boot.
//
// Atmosphère's loader decides it from atmosphere/config/override_config.ini
// (libstratosphere's cfg_override), read again at every program start:
// [hbl_config] program_id_0..7 (the album, 010000000000100D, by default) and
// override_any_app (true by default: R on any game). Locking adds a block of
// PlayGuard's at the end of the file, which the reader takes last:
//   program_id_0..7=0000000000000000, override_any_app=false
// and nothing else of the file changes. Unlocking takes the block away. One
// thing of Atmosphère's to know: a key it read stays as it was until the
// console restarts, also once its line is gone. So unlocking also writes
// back, in a second block, Atmosphère's own value for each key the lock
// set that the rest of the file does not set (program_id_0, the album;
// override_any_app=true): the menu opens again at once. That block changes
// nothing once the console has restarted; the sysmodule takes it away at
// boot (hbl_lock_apply, HblLock_DropDefaults).
//
// Each block is between a line starting "; >>> PlayGuard" and one starting
// "; <<< PlayGuard", comments for Atmosphère. A file without them is left as
// it is, whoever wrote it.
//
// Plain C, no libnx: host-tested (tests/hbl_lock).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// From the SD card root.
#define HBL_LOCK_CONFIG_PATH "atmosphere/config/override_config.ini"
// In the data directory (sd:/config/playguard/):
//   the state, while it is locked: the Atmosphère version it was locked on;
#define HBL_LOCK_STATE_NAME  "hbmenu_lock.txt"
//   the file as it was before the first lock, kept for a computer-side restore;
#define HBL_LOCK_BACKUP_NAME "override_config.ini.backup"
//   what the sysmodule did at boot, for PlayGuard to tell (and remove).
#define HBL_LOCK_REPORT_NAME "hbmenu_lock_report.txt"

// The largest override_config.ini handled: Atmosphère's own is a few hundred bytes.
#define HBL_LOCK_MAX_FILE 0x4000

typedef enum {
    HblLock_Lock = 0,       // the lock block at the end (any earlier block of PlayGuard's taken away)
    HblLock_Unlock = 1,     // no lock block; Atmosphère's defaults for the keys it set
    HblLock_DropDefaults = 2,   // only the defaults block taken away (at boot)
} HblLockOp;

// Whether `ini` holds PlayGuard's lock block.
bool hbl_lock_locked(const char *ini, size_t len);
// Whether it holds the block of defaults an unlock wrote.
bool hbl_lock_has_defaults(const char *ini, size_t len);

// The file after `op`, into `out` (`size` bytes, NUL-terminated), its length
// in *out_len. The rest of the file is kept byte for byte (a last line
// without its newline gets one when a block follows it). False when it does
// not fit (out then holds "").
bool hbl_lock_apply(const char *ini, size_t len, HblLockOp op, char *out, size_t size, size_t *out_len);

// What Atmosphère makes of `ini` from its own defaults (as after a restart):
// whether the album opens the homebrew menu (a program_id_N slot is the
// album's) and whether R on any game does (override_any_app). PlayGuard's
// check of the lock (Console › Forwarders) and its diagnostic report.
void hbl_lock_effective(const char *ini, size_t len, bool *album, bool *any_app);

// The state file: "ams=1.9.4\n", or "ams=unknown\n" when the version could
// not be read (it is there while PlayGuard holds the lock, whatever it says). Length written, or 0 when it does not fit.
size_t hbl_lock_state_format(uint8_t major, uint8_t minor, uint8_t micro, char *buf, size_t size);
size_t hbl_lock_state_format_unknown(char *buf, size_t size);
// Back from it; false (the outputs unchanged) for anything else.
bool hbl_lock_state_parse(const char *text, size_t len, uint8_t *major, uint8_t *minor, uint8_t *micro);

// Why the sysmodule lifted the lock at boot.
typedef enum {
    HblLockReason_Rescue = 0,      // a RESCUE request (core/rescue.h)
    HblLockReason_Atmosphere = 1,  // Atmosphère updated since it was locked: its signature patches may lag
} HblLockReason;

typedef struct {
    HblLockReason reason;
    bool          ok;          // the file written; else it may still be locked
    char          from[16];    // the versions, for HblLockReason_Atmosphere ("1.9.4")
    char          to[16];
} HblLockReport;

// What the sysmodule does at boot with a locked file: lift it (true, the
// reason and versions in *out) or leave it. `rescue`: a RESCUE request was
// there. `state`: the state file read (`locked_on`, the version it was
// locked on); `now`: the running version read (`ams`). Lifted on a request,
// or on a version that differs from the one it was locked on; never when
// either version is unknown. A child can do neither from the console.
bool hbl_lock_boot_lift(bool rescue, bool state, const uint8_t locked_on[3], bool now, const uint8_t ams[3],
                        HblLockReport *out);

// "reason=atmosphere\nresult=ok\nfrom=1.9.4\nto=1.9.5\n". Length, or 0 when it does not fit.
size_t hbl_lock_report_format(const HblLockReport *r, char *buf, size_t size);
// Back from it: reason and result required, the rest optional, unknown keys
// ignored. False (*out unchanged) for anything else.
bool hbl_lock_report_parse(const char *text, size_t len, HblLockReport *out);

#ifdef __cplusplus
}
#endif
