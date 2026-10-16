// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/history_logic.hpp"

#include <string>

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
        k == "rescue")
        return Category::Security;
    if (k.rfind("outside_", 0) == 0 || k == "new_account") return Category::Outside;
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
    case Filter::Count: break;
    }
    return false;
}

std::string day_key(const history::Entry& e)
{
    if (e.when.size() < 10 || e.when[4] != '-' || e.when[7] != '-') return "";
    return e.when.substr(0, 10);
}

int weekday_of(const std::string& date)
{
    if (date.size() < 10 || date[4] != '-' || date[7] != '-') return -1;
    for (size_t i : { 0, 1, 2, 3, 5, 6, 8, 9 })
        if (date[i] < '0' || date[i] > '9') return -1;
    int y = std::stoi(date.substr(0, 4));
    const int m = std::stoi(date.substr(5, 2)), d = std::stoi(date.substr(8, 2));
    if (m < 1 || m > 12 || d < 1 || d > 31) return -1;
    // Sakamoto's method.
    static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
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
