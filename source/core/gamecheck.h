// PlayGuard — the installed games the HOME menu cannot start or draw: an
// icon that keeps loading, a "!" or a dotted frame instead of the game. For
// each game the console lists (ns), what is installed of it (ns, ncm): the
// game itself, its update and add-ons, where, whether their files are there,
// the system version and key generation they ask for, their tickets (es),
// the launch version the console expects (avm) and how the HOME menu sees it
// (its view flags). gamecheck_classify() turns that into one problem and the
// UI into a cause and a fix; the actions below repair what can be repaired.
// The checks and repairs follow what Goldleaf (XorTroll), sphaira
// (ITotalJustice) and DBI do, and switchbrew's NS, ES and AVM pages.
//
// The scan only reads, one session per service for the whole list, closed
// before it returns; it reads every game, so the UI runs it off the main
// thread. Every action but the integrity check is a change: it goes through
// core_change_allowed() (read-only mode, the PIN asked before a change).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once
#include "nx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GAMECHECK_MAX          512
#define GAMECHECK_NAME_LEN     128
#define GAMECHECK_MAX_CONTENTS 16   // contents kept per game for the actions (the game, its update, add-ons)

// NcmContentMetaType / NcmStorageId values, so the UI and the tests need no libnx.
#define GC_META_APPLICATION 0x80
#define GC_META_PATCH       0x81
#define GC_META_ADDON       0x82
#define GC_STORAGE_GAMECARD 2
#define GC_STORAGE_NAND     4
#define GC_STORAGE_SD       5

// NsApplicationView flags qlaunch acts on (switchbrew, libnx-ext).
#define GC_VIEW_HAS_MAIN_CONTENTS (1u << 1)
#define GC_VIEW_CAN_LAUNCH        (1u << 8)
#define GC_VIEW_NEEDS_VERIFY      (1u << 13)

// One content of a game, as ns lists it.
typedef struct {
    u64 id;        // its own ID: the game's, an update's (…800), an add-on's
    u32 version;
    u8  type;      // GC_META_*
    u8  storage;   // GC_STORAGE_*
} GameContent;

// What was found of one game. Filled by gamecheck_check(); the host tests and
// the simulator fill it by hand.
typedef struct {
    u64  app_id;
    char name[GAMECHECK_NAME_LEN];   // UTF-8, console language; empty when unreadable
    char display_version[0x10];      // "1.4.0": the installed version as the game shows it; empty when unreadable
    bool control_ok;                 // name and icon read (what the HOME menu draws)
    u8   record_event;               // the HOME menu entry's last event (ns ApplicationRecord)
    u64  record_updated;             // when the entry last changed (its LastUpdated): a cached check of it is stale otherwise

    u32  meta_count;                 // contents the console lists for it (game, update, add-ons)
    GameContent contents[GAMECHECK_MAX_CONTENTS];
    u32  content_count;              // kept in `contents`, at most GAMECHECK_MAX_CONTENTS
    bool has_base;                   // the game itself
    bool has_patch;                  // an update
    u32  base_version, patch_version;
    bool card_only;                  // every content on a game card (one not inserted is normal)
    bool storage_unavailable;        // a content on a storage that could not be opened (SD card)
    bool meta_missing;               // a listed content the storage does not know
    bool files_missing;              // a file of the game or its update missing from its storage
    u32  base_required_hos;          // MAKEHOSVERSION the game asks for, 0 when unknown
    u32  patch_required_hos;         // the same for its update
    u8   key_generation;             // the highest of the game's and its update's files, 0 when unknown
    bool needs_ticket;               // a file of the game or its update is encrypted with a title key
    bool ticket_missing;             // … and the console has no ticket for it (only when tickets were listed)

    bool view_ok;                    // view_flags read
    u32  view_flags;                 // GC_VIEW_*
    bool launch_checked;             // launch_required read (avm, 6.0.0+)
    u32  launch_required;            // the version the console wants before it starts the game
    Result launch_rc;                // nsCheckApplicationLaunchVersion: why the console would refuse it

    // What PlayGuard's own integrity check (gamecheck_verify) found. Never
    // read from the console: the scan leaves it at GC_VERIFIED_NONE and the
    // cache carries it from one check to the next while the same version is
    // installed (util/game_cache.hpp).
    u8   verified;                   // GC_VERIFIED_*
    u32  verified_version;           // gamecheck_installed_version() it was checked at
    u64  verified_at;                // when, POSIX (user clock)
    Result verified_code;            // the code the console gave for the corrupt data, 0 when none
} GameFacts;

#define GC_VERIFIED_NONE    0
#define GC_VERIFIED_INTACT  1   // read to its end: nothing corrupt
#define GC_VERIFIED_CORRUPT 2   // read to its end: corrupt data

// The one problem shown for a game, the first that applies in this order: an
// earlier one explains the later ones (no files: no name either).
typedef enum {
    GC_OK = 0,
    GC_ARCHIVED,             // listed with nothing installed: the dotted frame
    GC_STORAGE_UNAVAILABLE,  // on the SD card, which cannot be read
    GC_NO_BASE,              // an update or add-on without the game: "!"
    GC_FILES_MISSING,        // installed, files missing or unknown to the storage
    GC_CORRUPT,              // PlayGuard's integrity check found corrupt data
    GC_NEEDS_VERIFY,         // the console found corrupt data (view flag), not checked since
    GC_KEY_TOO_NEW,          // encrypted with a key this firmware does not have
    GC_FIRMWARE_TOO_OLD,     // needs a newer system version than this console's
    GC_TICKET_MISSING,       // needs a ticket the console does not have
    GC_LAUNCH_VERSION,       // the console refuses it: it wants a newer version than the one installed
    GC_LAUNCH_REFUSED,       // the console refuses to start it for another reason
    GC_NO_CONTROL,           // everything there but the name and icon: an icon that keeps loading
    GC_ISSUE_COUNT
} GameIssue;

// `current_hos`: the console's MAKEHOSVERSION.
GameIssue gamecheck_classify(const GameFacts *g, u32 current_hos);

// The highest version of the game and its update installed: what the launch
// version is put back to.
u32 gamecheck_installed_version(const GameFacts *g);

// What PlayGuard offers for a game with `issue` (GA_* bits).
#define GA_REMOVE        (1u << 0)   // the whole game: not for one that is fine or whose SD card cannot be read
#define GA_REMOVE_UPDATE (1u << 1)   // only its update (not on a game card)
#define GA_REMOVE_ADDON  (1u << 2)   // one of its add-ons (not on a game card)
#define GA_RESET_LAUNCH  (1u << 3)   // the launch version back to the installed one
#define GA_VERIFY        (1u << 4)   // the console's own integrity check (reads only)
u32 gamecheck_actions(const GameFacts *g, GameIssue issue);

// The one action that fixes `issue`, if one does (a GA_* bit of
// gamecheck_actions(), 0 when none): accept the installed version again,
// remove the update that needs a newer firmware, check the data the console
// found corrupt or cannot read the name of (not once checked intact), or
// remove a game found corrupt or with nothing usable behind it.
u32 gamecheck_recommended(const GameFacts *g, GameIssue issue, u32 current_hos);

// The screen's sections: what to do about `issue`.
typedef enum {
    GC_CAT_FIX = 0,    // PlayGuard fixes it here (its recommended action does)
    GC_CAT_VERIFY,     // its data is to be checked first: it says whether to install it again
    GC_CAT_REINSTALL,  // the game is to be installed again (or its entry cleaned)
    GC_CAT_SYSTEM,     // the fix is outside the game: the firmware, the SD card
    GC_CAT_COUNT
} GameCategory;
GameCategory gamecheck_category(const GameFacts *g, GameIssue issue, u32 current_hos);

// A content meta's RequiredSystemVersion (major << 26 | minor << 20 |
// micro << 16 | relstep) as MAKEHOSVERSION; 0 stays 0.
u32 gamecheck_sysver_to_hos(u32 required_system_version);

// The highest key generation (an NCA header's KeyGeneration) the firmware
// `hos` has the keys for; 0 when the firmware is newer than the table.
u8 gamecheck_max_key_generation(u32 hos);

// One HOME menu entry, as the console lists it (quick: no game is read).
typedef struct {
    u64 app_id;
    u64 updated;   // LastUpdated
    u8  event;     // LastEvent
} GameRecord;

// The console's entries, at most `max` (*truncated when there are more).
Result gamecheck_records(GameRecord *out, u32 max, u32 *count, bool *truncated);

// Checks the games of `records`, one after the other, into `out` (as many
// facts as records), with one session per service for them all: the slow
// part, a fraction of a second per game. `on_game` (may be NULL) is called
// after each game with how many are done; returning false stops there.
// Returns how many games were checked.
u32 gamecheck_check(const GameRecord *records, u32 count, GameFacts *out,
                    bool (*on_game)(void *ctx, const GameFacts *game, u32 done), void *ctx);

// Where the last gamecheck_check() spent its time, by part (the log and the
// developer mode say it): opening the services, the tickets and the view of
// every game (setup, the views counted in state too), each game's name and
// icon (control), its contents, files, keys and tickets (contents), its
// launch check and launch version (state).
typedef struct {
    u32 games;
    u64 total_ns, setup_ns, control_ns, contents_ns, state_ns;
} GameCheckTiming;
void gamecheck_last_timing(GameCheckTiming *out);

// Deletes the game, its update and add-ons, and its entry on the HOME menu
// (nsDeleteApplicationCompletely, then the entry itself if one is left, as
// Goldleaf does). Save data stays. NXM_RC_READ_ONLY / NXM_RC_NOT_CONFIRMED
// before any session is opened.
Result gamecheck_remove(u64 app_id);

// Deletes one content (an update or an add-on; not the game itself, not on a
// game card): its files and its entry in the storage (ncm), then the HOME
// menu entry is rebuilt without it (ns ListApplicationRecordContentMeta,
// DeleteApplicationRecord, PushApplicationRecord). An update removed: the
// launch version goes back to what is left (avm, 6.0.0+).
Result gamecheck_remove_content(u64 app_id, const GameContent *content);

// The launch version back to `version` (avm PushLaunchVersion, 6.0.0+), then
// read again: NXM_RC_NOT_APPLIED when the console still wants more.
Result gamecheck_reset_launch_version(u64 app_id, u32 version);

// Whether `rc` says the data read is corrupt rather than that a request
// failed: ns 2016-0601 / 2016-0602 (Nintendo: corrupt data when a game
// starts) and fs' DataCorrupted results (2002-4000 to 2002-4999, such as
// 2002-4153).
bool gamecheck_is_corruption(Result rc);

// The console's integrity check (System Settings › Data Management), which
// reads every file of the game. `progress` is called while it runs (current
// and total bytes, total 0 while unknown); returning false cancels the check
// (NXM_RC_CANCELLED). Returns 0 when the check ran to its end, whatever it
// found: *corrupt is then set when it found corrupt data, and *found holds
// the code the console gave for it (0 when none). Any other result: the
// check could not run.
Result gamecheck_verify(u64 app_id, bool (*progress)(void *ctx, u64 current, u64 total), void *ctx, bool *corrupt,
                        Result *found);

#ifdef __cplusplus
}
#endif
