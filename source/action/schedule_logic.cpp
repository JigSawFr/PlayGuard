// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/schedule_logic.hpp"

#include "util/date.hpp"

namespace schedule_logic
{

Step step(const Schedule& s, const std::string& today)
{
    if (!valid_period(s.from, s.to)) return Step::None;
    int to_start = 0, to_end = 0;
    if (!date::days_between(s.from, today, &to_start) || !date::days_between(s.to, today, &to_end)) return Step::None;
    const bool over = to_end > 0;
    if (s.applied) return over ? Step::Revert : Step::None;
    if (over) return Step::Drop;
    return to_start >= 0 ? Step::Apply : Step::None;
}

std::vector<std::string> dates_from(const std::string& first, int count)
{
    std::vector<std::string> out;
    for (int n = 0; n < count; n++) {
        const std::string d = date::add_days(first, n);
        if (d.empty()) return {};
        out.push_back(d);
    }
    return out;
}

bool valid_period(const std::string& from, const std::string& to)
{
    int days = 0;
    return date::parse_valid(from) && date::parse_valid(to) && date::days_between(from, to, &days) && days >= 0;
}

}   // namespace schedule_logic
