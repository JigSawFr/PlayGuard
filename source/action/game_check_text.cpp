// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_check_text.hpp"
#include "util/title_id.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h>

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

std::string history_label(const GameFacts& g)
{
    return g.name[0] ? std::string(g.name) + " (" + title_id::text(g.app_id) + ")" : title_id::text(g.app_id);
}

std::string console_code(Result rc)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04u-%04u", 2000u + (unsigned)(rc & 0x1FF), (unsigned)((rc >> 9) & 0x1FFF));
    return buf;
}

std::string game_name(const GameFacts& g, const std::function<std::string(const std::string& id)>& unnamed)
{
    return g.name[0] ? std::string(g.name) : unnamed(title_id::text(g.app_id));
}

Sorted sort_broken(const std::vector<GameFacts>& games, u32 hos, const Words& w)
{
    Sorted out;
    for (const GameFacts& g : games) {
        const GameIssue issue = gamecheck_classify(&g, hos);
        if (issue == GC_OK) continue;
        out.all.push_back({ g, issue });
        out.by_group[gamecheck_category(&g, issue, hos)].push_back({ g, issue });
    }
    // In a section, the games with the same problem together, by name.
    for (auto& group : out.by_group)
        std::stable_sort(group.begin(), group.end(), [&w](const Broken& a, const Broken& b) {
            if (a.issue != b.issue) return a.issue < b.issue;
            return strcasecmp(game_name(a.game, w.unnamed).c_str(), game_name(b.game, w.unnamed).c_str()) < 0;
        });
    return out;
}

std::map<u64, std::string> line_names(const std::vector<Broken>& broken, const Words& w)
{
    std::map<std::string, std::vector<const GameFacts*>> same_name;
    for (const Broken& b : broken) same_name[game_name(b.game, w.unnamed)].push_back(&b.game);
    std::map<u64, std::string> out;
    for (const Broken& b : broken) {
        const GameFacts& g = b.game;
        const std::string name = game_name(g, w.unnamed);
        const auto& twins = same_name[name];
        if (twins.size() < 2 || !g.name[0]) {
            out[g.app_id] = name;
            continue;
        }
        bool versions_differ = g.display_version[0] != '\0';
        for (const GameFacts* t : twins)
            if (t->app_id != g.app_id && !std::strcmp(t->display_version, g.display_version)) versions_differ = false;
        out[g.app_id] = name + " · " + (versions_differ ? std::string(g.display_version) : title_id::text(g.app_id));
    }
    return out;
}

std::set<u64> still_selected(const std::set<u64>& selected, const std::vector<Broken>& broken)
{
    std::set<u64> still;
    for (const Broken& b : broken)
        if (selected.count(b.game.app_id)) still.insert(b.game.app_id);
    return still;
}

std::string age_text(int64_t checked_at, int64_t now, const Words& w)
{
    if (checked_at <= 0) return "";
    const int64_t s = now - checked_at;
    if (s < 60) return w.checked_now;
    if (s < 3600) return w.checked_min((int)(s / 60));
    return w.checked_h((int)(s / 3600));
}

std::string result_line(const std::string& found, bool none, const std::string& age,
                        const std::vector<std::string>& notes)
{
    std::string text = found;
    if (!age.empty()) text += (text.empty() ? "" : none ? " " : " · ") + age;
    for (const std::string& n : notes)
        if (!n.empty()) text += (text.empty() ? "" : "\n") + n;
    return text;
}

std::string list_text(const std::vector<GameFacts>& games, u32 hos, const Words& w)
{
    std::string text = w.list_title + "\n\n";
    for (const GameFacts& g : games) {
        const GameIssue issue = gamecheck_classify(&g, hos);
        text += title_id::text(g.app_id) + "  " + game_name(g, w.unnamed) +
                (g.display_version[0] ? " · " + w.version(g.display_version) : "") + " · " + w.problem(g, issue) + "\n";
    }
    return text;
}

std::string name_lines(const std::vector<GameFacts>& games, const Words& w)
{
    std::string names;
    for (size_t i = 0; i < games.size() && i < NAMES_SHOWN; i++) names += "\n" + game_name(games[i], w.unnamed);
    if (games.size() > NAMES_SHOWN) names += "\n…";
    return names;
}

Tally tally(const std::vector<Verified>& results)
{
    Tally t;
    for (const Verified& v : results) {
        if (R_FAILED(v.rc)) t.failed++;
        else if (v.corrupt) t.corrupt.push_back(v.game);
        else t.intact++;
    }
    return t;
}

}   // namespace game_check_text
