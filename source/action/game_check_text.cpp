// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_check_text.hpp"

#include <cstdio>

namespace game_check_text
{

const GameCategory GROUPS[GC_CAT_COUNT] = { GC_CAT_FIX, GC_CAT_VERIFY, GC_CAT_REINSTALL, GC_CAT_SYSTEM };

const char* category_key(GameCategory category)
{
    switch (category) {
        case GC_CAT_FIX:       return "fix";
        case GC_CAT_VERIFY:    return "verify";
        case GC_CAT_REINSTALL: return "reinstall";
        default:               return "system";
    }
}

const char* issue_key(GameIssue issue)
{
    switch (issue) {
        case GC_ARCHIVED:            return "archived";
        case GC_STORAGE_UNAVAILABLE: return "storage";
        case GC_NO_BASE:             return "no_base";
        case GC_FILES_MISSING:       return "files";
        case GC_CORRUPT:             return "corrupt";
        case GC_NEEDS_VERIFY:        return "needs_verify";
        case GC_KEY_TOO_NEW:         return "key_too_new";
        case GC_FIRMWARE_TOO_OLD:    return "firmware";
        case GC_TICKET_MISSING:      return "ticket";
        case GC_LAUNCH_VERSION:      return "launch_version";
        case GC_LAUNCH_REFUSED:      return "launch";
        case GC_NO_CONTROL:          return "no_control";
        default:                     return "";
    }
}

std::string issue_text_key(GameIssue issue, const char* part)
{
    return std::string("playguard/games/issues/") + issue_key(issue) + "/" + part;
}

bool warn_only(GameIssue issue)
{
    return issue == GC_STORAGE_UNAVAILABLE || issue == GC_FIRMWARE_TOO_OLD || issue == GC_KEY_TOO_NEW ||
           issue == GC_LAUNCH_VERSION;
}

u32 required_hos(const GameFacts& g)
{
    return g.patch_required_hos > g.base_required_hos ? g.patch_required_hos : g.base_required_hos;
}

std::string fix_key(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    // Only the update too new: playing without it is the other way out.
    if (issue == GC_FIRMWARE_TOO_OLD && g.patch_required_hos > current_hos && g.base_required_hos <= current_hos)
        return "playguard/games/issues/firmware/fix_update";
    if (issue == GC_KEY_TOO_NEW) return "playguard/games/issues/firmware/fix";
    return issue_text_key(issue, "fix");
}

std::string id_text(u64 id)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%016llX", (unsigned long long)id);
    return buf;
}

std::string history_label(const GameFacts& g)
{
    return g.name[0] ? std::string(g.name) + " (" + id_text(g.app_id) + ")" : id_text(g.app_id);
}

std::string console_code(Result rc)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04u-%04u", 2000u + (unsigned)(rc & 0x1FF), (unsigned)((rc >> 9) & 0x1FFF));
    return buf;
}

}   // namespace game_check_text
