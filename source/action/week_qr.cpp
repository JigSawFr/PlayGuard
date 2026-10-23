// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/week_qr.hpp"

#include <algorithm>
#include <borealis.hpp>
#include <fmt/format.h>
#include <vector>

#include "ui/ui.hpp"
#include "util/activity_summary.hpp"

using namespace brls::literals;

namespace week_qr
{

namespace
{
constexpr size_t TOP_GAMES  = 3;    // the games most played
constexpr size_t NAME_BYTES = 28;   // bytes of a game's name kept: the code stays small enough to read

std::string line(const std::string& label, const std::string& value)
{
    return brls::getStr("playguard/common/line", label, value);
}

// At most NAME_BYTES bytes, not cutting a character.
std::string short_name(const GameStat& g)
{
    std::string name = g.name[0] ? g.name : fmt::format("{:016X}", (unsigned long long)g.app_id);
    if (name.size() <= NAME_BYTES) return name;
    size_t cut = NAME_BYTES;
    while (cut > 0 && (name[cut] & 0xC0) == 0x80) cut--;
    return name.substr(0, cut) + "…";
}
}   // namespace

std::string text(const PlayStats& s, const std::string& who)
{
    std::vector<std::string> lines = { "PlayGuard · " + who + " · " + ui::today_date() };
    const auto week = activity_summary::summarize(s, activity_summary::Week);
    lines.push_back(line("playguard/activity/week"_i18n, ui::fmt_play_time(week.total_s) + " · " +
                                                            brls::getStr("playguard/cadran/activity/per_day",
                                                                         ui::fmt_play_time(week.per_day_s))));
    using activity_summary::Trend;
    const auto trend = activity_summary::week_trend(s);
    if (trend.known)
        lines.push_back(trend.trend == Trend::More   ? brls::getStr("playguard/cadran/activity/trend_up", ui::fmt_minutes(trend.amount_min))
                        : trend.trend == Trend::Less ? brls::getStr("playguard/cadran/activity/trend_down", ui::fmt_minutes(trend.amount_min))
                                                     : "playguard/cadran/activity/trend_same"_i18n);
    // Each day, the oldest first.
    uint64_t day_total[7];
    activity_summary::day_totals(s, day_total);
    std::string days;
    for (int k = 6; k >= 0; k--) {
        if (!days.empty()) days += ", ";
        days += brls::getStr(fmt::format("playguard/days_short/{}", (int)s.day_wday[k] % 7)) + " " + ui::fmt_play_time(day_total[k]);
    }
    lines.push_back(line("playguard/activity/qr_days"_i18n, days));
    // The games most played.
    std::vector<const GameStat*> games;
    for (uint32_t g = 0; g < s.count; g++)
        if (s.games[g].week_s) games.push_back(&s.games[g]);
    activity_summary::sort_by_play(games, activity_summary::Week, false);
    std::string top;
    for (size_t i = 0; i < games.size() && i < TOP_GAMES; i++) {
        if (!top.empty()) top += ", ";
        top += short_name(*games[i]) + " " + ui::fmt_play_time(games[i]->week_s);
    }
    if (!top.empty()) lines.push_back(line("playguard/activity/summary/top"_i18n, top));
    // When it was played.
    const auto h = activity_summary::hours(s);
    if (h.known && h.sessions) {
        lines.push_back(line("playguard/activity/hours/sessions"_i18n,
                             brls::getStr("playguard/activity/hours/sessions_value", (int)h.sessions, ui::fmt_play_time(h.average_s))));
        lines.push_back(line("playguard/activity/hours/longest"_i18n,
                             ui::fmt_play_time(h.longest_s) + " · " + ui::day_name((int)s.day_wday[h.longest_day] % 7)));
        lines.push_back(line("playguard/activity/hours/late"_i18n, ui::fmt_play_time(h.late_s)));
    }
    std::string out;
    for (const std::string& l : lines) out += (out.empty() ? "" : "\n") + l;
    return out;
}

void show(const PlayStats& s, const std::string& who)
{
    if (R_FAILED(s.rc) || !s.windows_ok) {
        ui::notify("playguard/activity/export_not_ready"_i18n);
        return;
    }
    ui::qr_text_dialog("playguard/activity/qr_body"_i18n, text(s, who));
}

}   // namespace week_qr
