// PlayGuard — what a game's facts mean (see gamecheck.h). No console calls:
// compiled for the Switch, the desktop simulator and the host tests.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gamecheck.h"

u32 gamecheck_sysver_to_hos(u32 v)
{
    if (!v) return 0;
    return MAKEHOSVERSION((v >> 26) & 0x3F, (v >> 20) & 0x3F, (v >> 16) & 0xF);
}

// Each key generation and the first firmware with it (switchbrew, NCA:
// KeyGeneration). A new firmware with new keys adds a line.
static const struct { u8 key_generation; u8 major, minor, micro; } KEY_GENERATIONS[] = {
    { 0x00, 1, 0, 0 },  { 0x01, 1, 0, 0 },  { 0x02, 3, 0, 0 },  { 0x03, 3, 0, 1 },  { 0x04, 4, 0, 0 },  { 0x05, 5, 0, 0 },
    { 0x06, 6, 0, 0 },  { 0x07, 6, 2, 0 },  { 0x08, 7, 0, 0 },  { 0x09, 8, 1, 0 },  { 0x0A, 9, 0, 0 },
    { 0x0B, 9, 1, 0 },  { 0x0C, 12, 1, 0 }, { 0x0D, 13, 0, 0 }, { 0x0E, 14, 0, 0 }, { 0x0F, 15, 0, 0 },
    { 0x10, 16, 0, 0 }, { 0x11, 17, 0, 0 }, { 0x12, 18, 0, 0 }, { 0x13, 19, 0, 0 }, { 0x14, 20, 0, 0 },
    { 0x15, 21, 0, 0 }, { 0x16, 22, 0, 0 }, { 0x17, 23, 0, 0 },
};
#define KEY_GENERATION_COUNT (sizeof(KEY_GENERATIONS) / sizeof(KEY_GENERATIONS[0]))
// The last firmware the table is known to be complete for: on a newer one, a
// higher key generation may be fine.
#define KEY_TABLE_LAST_HOS MAKEHOSVERSION(23, 0, 1)

u8 gamecheck_max_key_generation(u32 hos)
{
    if (!hos || hos > KEY_TABLE_LAST_HOS) return 0;
    u8 max = 0;
    for (size_t i = 0; i < KEY_GENERATION_COUNT; i++) {
        const u32 first = MAKEHOSVERSION(KEY_GENERATIONS[i].major, KEY_GENERATIONS[i].minor, KEY_GENERATIONS[i].micro);
        if (first <= hos) max = KEY_GENERATIONS[i].key_generation;
    }
    return max;
}

u32 gamecheck_key_generation_firmware(u8 key_generation)
{
    for (size_t i = 0; i < KEY_GENERATION_COUNT; i++)
        if (KEY_GENERATIONS[i].key_generation >= key_generation)
            return MAKEHOSVERSION(KEY_GENERATIONS[i].major, KEY_GENERATIONS[i].minor, KEY_GENERATIONS[i].micro);
    return 0;
}

u32 gamecheck_installed_version(const GameFacts *g)
{
    u32 v = g->has_base ? g->base_version : 0;
    if (g->has_patch && g->patch_version > v) v = g->patch_version;
    return v;
}

// The console wants a newer version than the one installed, and refuses to
// start the game for it (a version it only knows of is no problem).
static bool launch_version_stale(const GameFacts *g)
{
    return g->launch_checked && g->has_base && R_FAILED(g->launch_rc) &&
           g->launch_required > gamecheck_installed_version(g);
}

GameIssue gamecheck_classify(const GameFacts *g, u32 current_hos)
{
    if (g->meta_count == 0) return GC_ARCHIVED;
    if (g->card_only) return GC_OK;   // the card is not inserted, or reads fine
    if (g->storage_unavailable) return GC_STORAGE_UNAVAILABLE;
    if (!g->has_base) return GC_NO_BASE;
    if (g->meta_missing || g->files_missing) return GC_FILES_MISSING;
    if (g->view_ok && (g->view_flags & GC_VIEW_NEEDS_VERIFY)) return GC_NEEDS_VERIFY;
    const u8 max_key = gamecheck_max_key_generation(current_hos);
    if (g->key_generation && max_key && g->key_generation > max_key) return GC_KEY_TOO_NEW;
    const u32 required = g->patch_required_hos > g->base_required_hos ? g->patch_required_hos : g->base_required_hos;
    if (required && current_hos && required > current_hos) return GC_FIRMWARE_TOO_OLD;
    if (g->ticket_missing) return GC_TICKET_MISSING;
    if (launch_version_stale(g)) return GC_LAUNCH_VERSION;
    if (R_FAILED(g->launch_rc)) return GC_LAUNCH_REFUSED;
    if (!g->control_ok) return GC_NO_CONTROL;
    return GC_OK;
}

u32 gamecheck_actions(const GameFacts *g, GameIssue issue)
{
    if (issue == GC_OK || issue == GC_STORAGE_UNAVAILABLE || issue >= GC_ISSUE_COUNT) return 0;
    u32 actions = GA_REMOVE;
    if (g->has_base) {   // without the game, removing it all is the cleaning
        for (u32 i = 0; i < g->content_count; i++) {
            const GameContent *c = &g->contents[i];
            if (c->storage != GC_STORAGE_NAND && c->storage != GC_STORAGE_SD) continue;
            if (c->type == GC_META_PATCH) actions |= GA_REMOVE_UPDATE;
            if (c->type == GC_META_ADDON) actions |= GA_REMOVE_ADDON;
        }
    }
    if (launch_version_stale(g)) actions |= GA_RESET_LAUNCH;
    if (g->has_base && issue != GC_ARCHIVED && issue != GC_NO_BASE) actions |= GA_VERIFY;
    return actions;
}

u32 gamecheck_recommended(const GameFacts *g, GameIssue issue, u32 current_hos)
{
    const u32 actions = gamecheck_actions(g, issue);
    u32 pick = 0;
    switch (issue) {
        case GC_LAUNCH_VERSION:
            pick = GA_RESET_LAUNCH;
            break;
        case GC_FIRMWARE_TOO_OLD:   // only the update needs it: the game runs without it
            if (current_hos && g->patch_required_hos > current_hos && g->base_required_hos <= current_hos)
                pick = GA_REMOVE_UPDATE;
            break;
        case GC_NEEDS_VERIFY:
        case GC_NO_CONTROL:
            pick = GA_VERIFY;
            break;
        case GC_ARCHIVED:
        case GC_NO_BASE:
        case GC_FILES_MISSING:
            pick = GA_REMOVE;
            break;
        default:
            break;
    }
    return actions & pick;
}
