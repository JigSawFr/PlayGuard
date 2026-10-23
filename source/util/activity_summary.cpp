// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/activity_summary.hpp"

#include <algorithm>
#include <cstdio>

namespace activity_summary
{

uint64_t value(const GameStat& g, int period)
{
    switch (period) {
        case Today: return g.today_s;
        case Week:  return g.week_s;
        case Month: return g.month_s;
        default:    return g.totals_ok ? g.total_s : 0;
    }
}

bool period_read(const PlayStats& s, int period)
{
    if (period == AllTime) return R_SUCCEEDED(s.stats_rc);
    return s.windows_ok && (period != Month || s.month_ok);
}

Summary summarize(const PlayStats& s, int period)
{
    Summary out;
    out.known = period_read(s, period);
    if (R_FAILED(s.rc) || !out.known) {
        out.known = false;
        return out;
    }
    uint64_t launches = 0, first = 0;
    uint64_t per_day[7] = {};
    for (uint32_t i = 0; i < s.count && i < PLAYSTATS_MAX; i++) {
        const GameStat& g = s.games[i];
        const uint64_t v = value(g, period);
        out.total_s += v;
        // The most played, the latest played first on a tie (as the list).
        if (v > 0 && (out.top < 0 || v > out.top_s || (v == out.top_s && g.last_played > s.games[out.top].last_played))) {
            out.top = (int)i;
            out.top_s = v;
        }
        if (g.totals_ok) {
            launches += g.launches;
            if (g.first_played && (!first || g.first_played < first)) first = g.first_played;
        }
        for (int k = 0; k < 7; k++) per_day[k] += g.day_s[k];
    }
    if (period == Week) {
        out.days = 7;
        out.per_day_s = out.total_s / 7;
        out.days_played = 0;
        for (int k = 0; k < 7; k++) {
            if (per_day[k]) out.days_played++;
            if (per_day[k] > out.busiest_s) {   // the most recent day on a tie
                out.busiest = k;
                out.busiest_s = per_day[k];
            }
        }
    } else if (period == Month) {
        out.days = 30;
        out.per_day_s = out.total_s / 30;
    } else if (period == AllTime) {
        if (first) {
            // Whole days since the first play, today included; a first play
            // "in the future" (the clock went back) counts as today.
            out.days = s.now > first ? (int)((s.now - first) / 86400) + 1 : 1;
            out.per_day_s = out.total_s / (uint64_t)out.days;
        }
        if (launches) out.session_s = out.total_s / launches;
    }
    return out;
}

Hours hours(const PlayStats& s)
{
    Hours out;
    if (R_FAILED(s.rc) || !s.windows_ok || !s.hours_ok) return out;
    out.known = true;
    const PlayLogHours& h = s.hours;
    out.sessions = h.sessions;
    if (h.sessions) {
        out.average_s   = h.session_s / h.sessions;
        out.longest_s   = h.longest_s;
        out.longest_day = h.longest_day;
    }
    uint64_t by_hour[24] = {};
    for (int k = 0; k < 7; k++)
        for (int hr = 0; hr < 24; hr++) {
            by_hour[hr] += h.hour_s[k][hr];
            if (hr >= LATE_FROM || hr < LATE_TO) out.late_s += h.hour_s[k][hr];
        }
    for (int hr = 0; hr < 23; hr++) {   // the earlier two on a tie
        const uint64_t two = by_hour[hr] + by_hour[hr + 1];
        if (two > out.busy_s) {
            out.busy_s = two;
            out.busy_from = hr;
        }
    }
    return out;
}

bool week_trend_min(const PlayStats& s, int64_t& per_day_min)
{
    const Summary week = summarize(s, Week);
    if (!week.known || !s.prev_week_ok) return false;
    per_day_min = ((int64_t)week.total_s - (int64_t)s.prev_week_s) / 7 / 60;
    return true;
}

WeekTrend week_trend(const PlayStats& s)
{
    WeekTrend out;
    int64_t diff = 0;
    out.known = week_trend_min(s, diff);
    if (!out.known) return out;
    out.per_day_min = diff;
    out.amount_min  = (uint16_t)std::min<int64_t>(24 * 60, diff < 0 ? -diff : diff);
    out.trend = diff >= TREND_SAME_MIN ? Trend::More : diff <= -TREND_SAME_MIN ? Trend::Less : Trend::Same;
    return out;
}

TodayPlay today_play(const PlayStats& s)
{
    TodayPlay out;
    for (uint32_t g = 0; g < s.count; g++) {
        out.total_s += s.games[g].today_s;
        out.games += s.games[g].today_s > 0;
    }
    return out;
}

void day_totals(const PlayStats& s, uint64_t (&out)[7])
{
    for (int k = 0; k < 7; k++) out[k] = 0;
    for (uint32_t g = 0; g < s.count; g++)
        for (int k = 0; k < 7; k++) out[k] += s.games[g].day_s[k];
}

std::vector<const GameStat*> ranked_games(const PlayStats& s, int period)
{
    std::vector<const GameStat*> games;
    for (uint32_t g = 0; g < s.count; g++)
        if (value(s.games[g], period) > 0) games.push_back(&s.games[g]);
    std::sort(games.begin(), games.end(), [period](const GameStat* a, const GameStat* b) {
        return value(*a, period) > value(*b, period);
    });
    return games;
}

Grew grew_most(const PlayStats& s, const std::vector<const GameStat*>& ranked, size_t shown, int period)
{
    Grew out;
    if (period != Week || !s.windows_ok || !s.prev_week_ok) return out;
    int64_t best = GREW_MIN * 60 - 1;
    for (size_t i = 0; i < ranked.size() && i < shown; i++) {
        const int64_t d = (int64_t)ranked[i]->week_s - (int64_t)ranked[i]->prev_week_s;
        if (d > best) {
            out.game = ranked[i];
            best     = d;
        }
    }
    if (out.game) out.more_s = (uint64_t)best;
    return out;
}

uint64_t unattributed_s(uint64_t total_s, const AccountPlay* accounts, size_t count)
{
    uint64_t gap = total_s;
    for (size_t i = 0; i < count && gap; i++) gap -= accounts[i].total_s < gap ? accounts[i].total_s : gap;
    return gap >= UNATTRIBUTED_MIN_S ? gap : 0;
}

void sort_by_play(std::vector<const GameStat*>& games, int period, bool by_total)
{
    std::sort(games.begin(), games.end(), [period, by_total](const GameStat* a, const GameStat* b) {
        const uint64_t va = value(*a, period), vb = value(*b, period);
        if (va != vb) return va > vb;
        if (by_total && a->total_s != b->total_s) return a->total_s > b->total_s;
        return a->last_played > b->last_played;
    });
}

table_export::Table export_table(const PlayStats& s, int period, const ExportText& text)
{
    table_export::Table t;
    t.title    = text.title;
    t.subtitle = text.subtitle;
    t.sheet    = text.sheet;
    t.columns  = {
        { "game", text.game, false },
        { "title_id", text.id, false },
        { "today_min", text.today, true },
    };
    // The six days before today, most recent first.
    for (int k = 1; k < 7; k++) t.columns.push_back({ "day_" + std::to_string(k) + "_min", text.day((int)s.day_wday[k]), true });
    t.columns.insert(t.columns.end(), {
        { "week_min", text.week, true },
        { "month_min", text.month, true },
        { "total_min", text.total, true },
        { "launches", text.launches, true },
        { "first_played", text.first, false },
        { "last_played", text.last, false },
    });
    std::vector<const GameStat*> games;
    for (uint32_t i = 0; i < s.count && i < PLAYSTATS_MAX; i++) games.push_back(&s.games[i]);
    sort_by_play(games, period, true);
    auto minutes = [](uint64_t seconds) { return std::to_string((seconds + 30) / 60); };
    for (const GameStat* g : games) {
        char id[24];
        std::snprintf(id, sizeof(id), "%016llX", (unsigned long long)g->app_id);
        t.rows.push_back({
            text.name(*g),
            id,
            s.windows_ok ? minutes(g->today_s) : "",
        });
        for (int k = 1; k < 7; k++) t.rows.back().push_back(s.windows_ok ? minutes(g->day_s[k]) : "");
        t.rows.back().insert(t.rows.back().end(), {
            s.windows_ok ? minutes(g->week_s) : "",
            s.windows_ok && s.month_ok ? minutes(g->month_s) : "",
            g->totals_ok ? minutes(g->total_s) : "",
            g->totals_ok ? std::to_string(g->launches) : "",
            g->totals_ok && g->first_played ? text.time(g->first_played) : "",
            g->totals_ok && g->last_played ? text.time(g->last_played) : "",
        });
    }
    return t;
}

int start_account(const std::string& setting, const std::string& last, const std::vector<std::string>& keys)
{
    const std::string& key = setting == "last" ? last : setting;
    if (key.empty()) return -1;
    for (size_t i = 0; i < keys.size(); i++)
        if (keys[i] == key) return (int)i;
    return -1;
}

}   // namespace activity_summary
