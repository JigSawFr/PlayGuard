// ui — time limits, play time, level and age names as text.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/play_text.hpp"

#include <algorithm>
#include <format>

#include <aurore/cadran/ui/tr.hpp>

#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace
{
// "2 h 30" with no-break spaces: a line never ends on "2 h" before its "30"
// (Aurore, patch 0010).
std::string glued(std::string text)
{
    for (size_t at = text.find(' '); at != std::string::npos; at = text.find(' ', at + 2)) text.replace(at, 1, "\u00A0");
    return text;
}
}   // namespace

namespace ui
{

std::string fmt_limit(uint16_t m)
{
    if (m == PT_DAY_NOLIMIT) return "playguard/common/no_limit"_i18n;
    if (m == 0) return "playguard/common/zero_minutes"_i18n;
    if (m < 60) return glued(aurore::getStr("cadran/common/minutes", (int)m));
    if (m % 60 == 0) return glued(aurore::getStr("cadran/common/hours", (int)(m / 60)));
    return glued(aurore::getStr("cadran/common/hours_minutes", (int)(m / 60), std::format("{:02d}", (int)(m % 60))));
}

std::string days_summary(const uint16_t days[7])
{
    bool uniform = true, any_nolimit = false;
    int lo = -1, hi = -1;
    for (int d = 0; d < 7; d++) {
        uniform &= days[d] == days[0];
        if (days[d] == PT_DAY_NOLIMIT) {
            any_nolimit = true;
            continue;
        }
        lo = lo < 0 ? days[d] : std::min<int>(lo, days[d]);
        hi = hi < 0 ? days[d] : std::max<int>(hi, days[d]);
    }
    if (uniform)
        return days[0] == PT_DAY_NOLIMIT ? "playguard/common/no_limit"_i18n
                                         : aurore::getStr("playguard/play_timer/state/every_day", fmt_limit(days[0]));
    if (lo < 0) return "playguard/common/no_limit"_i18n;
    // Inside the sentence: "1 h to no limit", not "1 h to No limit".
    const std::string top = any_nolimit ? "playguard/common/no_limit_in_text"_i18n : fmt_limit((uint16_t)hi);
    return aurore::getStr("playguard/play_timer/profile_range", fmt_limit((uint16_t)lo), top);
}

std::string fmt_played(uint16_t m)
{
    if (m == 0) return glued(aurore::getStr("cadran/common/minutes", 0));
    return fmt_limit(m);
}

std::string fmt_time_left(uint64_t ns)
{
    uint64_t minutes = (ns + 30000000000ULL) / 60000000000ULL;
    if (minutes > 1440) minutes = 1440;
    return fmt_played((uint16_t)minutes);   // a time left: "0 min", not "0 min (no play)"
}

std::string fmt_per_day_over(uint64_t per_day_s, int days)
{
    const std::string avg = fmt_duration(per_day_s);
    if (days < 60) return aurore::getStr("playguard/activity/summary/per_day_over", avg, days);
    // Rounded to the nearest month (30.44 days) or year (365.25 days).
    if (days < 730) return aurore::getStr("playguard/activity/summary/per_day_over_months", avg, (days * 100 + 1522) / 3044);
    return aurore::getStr("playguard/activity/summary/per_day_over_years", avg, (days * 100 + 18262) / 36525);
}

std::string age_text(int age)
{
    return age == 0 ? "playguard/restrictions/age_none"_i18n : aurore::getStr("playguard/restrictions/age_value", age);
}

std::string level_name(uint32_t level)
{
    if (level > 4) return "?";
    return aurore::getStr(std::format("playguard/restrictions/levels/{}", level));
}

std::string day_short(int wday)
{
    // The keys as they are, without a format: a view draws them at every frame.
    static constexpr const char* KEYS[7] = { "playguard/days_short/0", "playguard/days_short/1", "playguard/days_short/2",
                                             "playguard/days_short/3", "playguard/days_short/4", "playguard/days_short/5",
                                             "playguard/days_short/6" };
    return wday >= 0 && wday < 7 ? aurore::getStr(KEYS[wday]) : std::string();
}

}   // namespace ui
