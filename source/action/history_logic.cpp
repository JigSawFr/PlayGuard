// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/history_logic.hpp"

#include <algorithm>
#include <string>

#include <aurore/cadran/util/text.hpp>

#include "util/date.hpp"

namespace history_logic
{

Undo undo_action(const history::Entry& e, const std::vector<int>& now, bool read_only, bool advanced)
{
    if (!now.empty() && now == e.before) return Undo::AlreadyBack;
    if (read_only) return Undo::ReadOnly;
    if (e.kind == "alarm" && !advanced) return Undo::NeedsAdvanced;
    return Undo::Offer;
}

bool changed_since(const history::Entry& e, const std::vector<int>& now)
{
    return !now.empty() && now != e.after;
}

std::vector<int> undo_replaces(const history::Entry& e, const std::vector<int>& now)
{
    return now.empty() ? e.after : now;
}

std::vector<int> changed_days(const history::Entry& e)
{
    std::vector<int> days;
    if ((e.kind != "limits" && e.kind != "outside_limits") || e.before.size() != 7 || e.after.size() != 7) return days;
    for (int d = 0; d < 7; d++)
        if (e.before[d] != e.after[d]) days.push_back(d);
    return days;
}

Category category(const history::Entry& e)
{
    const std::string& k = e.kind;
    if (k == "limits" || k == "alarm" || k == "time_up" || k == "bedtime" || k == "countdown" || k == "console_lock")
        return Category::PlayTime;
    if (k == "level" || k == "org" || k == "custom" || k == "vr") return Category::Restrictions;
    if (k == "pin" || k == "pin_shown" || k == "unlock" || k == "relock" || k == "unlink" || k == "delete" ||
        k == "rescue" || k == "module")
        return Category::Security;
    if (text::starts_with(k, "outside_") || k == "new_account") return Category::Outside;
    if (text::ends_with(k, "_forwarder") || k == "forwarder_gone" || text::starts_with(k, "gate_") ||
        k == "lock_hbmenu" || k == "unlock_hbmenu" || text::starts_with(k, "hbmenu_"))
        return Category::Homebrew;
    return Category::Other;
}

bool matches(const history::Entry& e, Filter f)
{
    switch (f) {
    case Filter::All: return true;
    case Filter::PlayTime: return category(e) == Category::PlayTime;
    case Filter::Restrictions: return category(e) == Category::Restrictions;
    case Filter::Security: return category(e) == Category::Security;
    case Filter::Outside: return category(e) == Category::Outside;
    case Filter::Failed: return !e.result.empty();
    case Filter::Homebrew: return category(e) == Category::Homebrew;
    case Filter::Count: break;
    }
    return false;
}

std::string day_key(const history::Entry& e)
{
    if (e.when.size() < 10 || e.when[4] != '-' || e.when[7] != '-') return "";
    return e.when.substr(0, 10);
}

int weekday_of(const std::string& day) { return date::weekday(day); }

Day day_of(const std::string& when, const std::string& today, const std::string& yesterday)
{
    // The date is the stamp's first ten characters; a shorter one is no date.
    if (when.size() < 10) return Day::Other;
    const std::string date = when.substr(0, 10);
    if (!today.empty() && date == today) return Day::Today;
    if (!yesterday.empty() && date == yesterday) return Day::Yesterday;
    return Day::Other;
}

namespace
{
const char* const KINDS[] = { "limits", "level", "custom", "org", "vr", "alarm", "pin", "pin_shown",
                              "unlock", "relock", "unlink", "delete", "clock", "restore", "rescue",
                              "console_lock", "bedtime", "config_reset", "outside_reset", "outside_clock",
                              "outside_limits", "new_account", "time_up", "countdown", "remove_game",
                              "remove_update", "remove_addon", "launch_version", "add_forwarder",
                              "remove_forwarder", "protect_forwarder", "unprotect_forwarder",
                              "forwarder_gone", "lock_hbmenu", "unlock_hbmenu", "hbmenu_restored",
                              "hbmenu_lock_removed", "gate_open", "gate_free", "gate_refused", "gate_hours", "gate_paused", "gate_closed",
                              "gate_closed_hours", "gate_pause", "gate_pause_lifted", "gate_limit", "gate_limit_set",
                              "forwarder_options", "module" };
const char* const SOURCES[] = { "uniform", "day", "per_day", "extra", "stop", "restore_extra", "bonus", "restore_bonus", "schedule_end",
                                "remove", "backup", "undo", "first_steps", "overview", "console_lock", "notices",
                                "rescue", "remote", "remote_extra", "remote_stop", "remote_console_lock",
                                "remote_restore_extra" };

bool listed(const std::vector<std::string>& list, const std::string& s)
{
    return std::ranges::contains(list, s);
}
}   // namespace

const std::vector<std::string>& known_kinds()
{
    static const std::vector<std::string> kinds(std::begin(KINDS), std::end(KINDS));
    return kinds;
}

bool known_kind(const std::string& kind) { return listed(known_kinds(), kind); }

const std::vector<std::string>& known_sources()
{
    static const std::vector<std::string> sources(std::begin(SOURCES), std::end(SOURCES));
    return sources;
}

bool known_source(const std::string& source) { return listed(known_sources(), source); }

}   // namespace history_logic
