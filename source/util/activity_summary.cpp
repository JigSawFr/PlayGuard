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
        default:    return g.totals_ok ? g.total_s : 0;
    }
}

Summary summarize(const PlayStats& s, int period)
{
    Summary out;
    out.known = period == AllTime ? R_SUCCEEDED(s.stats_rc) : s.windows_ok;
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

bool week_trend_min(const PlayStats& s, int64_t& per_day_min)
{
    const Summary week = summarize(s, Week);
    if (!week.known || !s.prev_week_ok) return false;
    per_day_min = ((int64_t)week.total_s - (int64_t)s.prev_week_s) / 7 / 60;
    return true;
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
