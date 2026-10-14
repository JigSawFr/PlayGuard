// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/history_logic.hpp"

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

Day day_of(const std::string& when, const std::string& today, const std::string& yesterday)
{
    // The date is the stamp's first ten characters; a shorter one is no date.
    if (when.size() < 10) return Day::Other;
    const std::string date = when.substr(0, 10);
    if (!today.empty() && date == today) return Day::Today;
    if (!yesterday.empty() && date == yesterday) return Day::Yesterday;
    return Day::Other;
}

}   // namespace history_logic
