// Host tests for util/activity_summary.hpp: the Activity tab's summary over
// each period (average per day, most played game, days played, busiest day,
// average session), what is left out when a figure was not read, the play
// time no account holds, the list's order and the export's table.
#include "check.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "util/activity_summary.hpp"

using namespace activity_summary;

static const uint64_t DAY = 86400, NOW = 1760000000;

static GameStat& add(PlayStats& s, const char* name, uint64_t total_min, uint32_t launches, uint64_t last_days_ago)
{
    GameStat& g = s.games[s.count++];
    g.app_id = 0x0100000000010000ULL + ((uint64_t)s.count << 12);
    std::snprintf(g.name, sizeof g.name, "%s", name);
    g.totals_ok = true;
    g.total_s = total_min * 60;
    g.launches = launches;
    g.last_played = NOW - last_days_ago * DAY;
    g.first_played = NOW - 99 * DAY - 3600;   // 99 whole days and an hour ago: day 100
    return g;
}

static std::unique_ptr<PlayStats> sample()
{
    auto s = std::make_unique<PlayStats>();
    std::memset(s.get(), 0, sizeof *s);
    s->windows_ok = true;
    s->now = NOW;
    for (int k = 0; k < 7; k++) s->day_wday[k] = (u8)((3 - k + 7) % 7);
    GameStat& a = add(*s, "Kart", 600, 20, 0);   // 10 h, this week 70 min (today 30)
    a.today_s = a.day_s[0] = 30 * 60;
    a.day_s[2] = 40 * 60;
    a.week_s = 70 * 60;
    GameStat& b = add(*s, "Farm", 400, 20, 2);   // this week 80 min, all on day 2
    b.day_s[2] = b.week_s = 80 * 60;
    s->month_ok = true;                         // the last 30 days: Kart 5 h, Farm 80 min
    a.month_s = 300 * 60;
    b.month_s = 80 * 60;
    return s;
}

static void test_periods()
{
    auto s = sample();
    Summary t = summarize(*s, Today);
    CHECK(t.known && t.total_s == 30 * 60 && t.top == 0 && t.top_s == 30 * 60);
    CHECK(t.days == 0 && t.days_played == -1 && t.busiest == -1 && t.session_s == 0);

    Summary w = summarize(*s, Week);
    CHECK(w.known && w.total_s == 150 * 60);
    CHECK(w.days == 7 && w.per_day_s == 150 * 60 / 7);
    CHECK(w.top == 1 && w.top_s == 80 * 60);            // Farm, 80 min
    CHECK(w.days_played == 2 && w.busiest == 2 && w.busiest_s == 120 * 60);
    CHECK(w.session_s == 0);

    Summary m = summarize(*s, Month);
    CHECK(m.known && m.total_s == 380 * 60 && m.top == 0 && m.top_s == 300 * 60);
    CHECK(m.days == 30 && m.per_day_s == 380 * 60 / 30);
    CHECK(m.days_played == -1 && m.busiest == -1 && m.session_s == 0);
    s->month_ok = false;                                 // the month not read: unknown
    CHECK(!summarize(*s, Month).known && summarize(*s, Week).known);
    s->month_ok = true;

    // The order periods are offered in: Month between the week and all time.
    CHECK(PERIOD_COUNT == 4 && PERIODS[0] == Today && PERIODS[1] == Week && PERIODS[2] == Month && PERIODS[3] == AllTime);

    Summary all = summarize(*s, AllTime);
    CHECK(all.known && all.total_s == 1000 * 60 && all.top == 0);
    CHECK(all.days == 100 && all.per_day_s == 600);      // 1000 min over 100 days
    CHECK(all.session_s == 1000 * 60 / 40);              // 25 min a launch
    CHECK(all.days_played == -1 && all.busiest == -1);
}

static void test_week_trend()
{
    auto s = sample();   // 150 min over the last 7 days
    int64_t d = 99;
    CHECK(!week_trend_min(*s, d) && d == 99);           // the week before not read
    s->prev_week_ok = true;
    s->prev_week_s = 10 * 60;                           // 140 min more: 20 a day
    CHECK(week_trend_min(*s, d) && d == 20);
    s->prev_week_s = 290 * 60;                          // 140 min fewer
    CHECK(week_trend_min(*s, d) && d == -20);
    s->prev_week_s = 150 * 60;
    CHECK(week_trend_min(*s, d) && d == 0);
    s->windows_ok = false;                              // the last 7 days not read
    CHECK(!week_trend_min(*s, d));
}

static void test_week_trend_words()
{
    auto s = sample();   // 150 min over the last 7 days
    CHECK(!week_trend(*s).known);                       // the week before not read
    s->prev_week_ok = true;
    s->prev_week_s = 10 * 60;                           // 20 min a day more
    WeekTrend t = week_trend(*s);
    CHECK(t.known && t.trend == Trend::More && t.per_day_min == 20 && t.amount_min == 20);
    s->prev_week_s = 290 * 60;                          // 20 min a day fewer
    t = week_trend(*s);
    CHECK(t.known && t.trend == Trend::Less && t.per_day_min == -20 && t.amount_min == 20);
    // Within TREND_SAME_MIN either way: about the same; at it, more or less.
    s->prev_week_s = (150 - 7 * (TREND_SAME_MIN - 1)) * 60;
    CHECK(week_trend(*s).trend == Trend::Same && week_trend(*s).amount_min == TREND_SAME_MIN - 1);
    s->prev_week_s = (150 - 7 * TREND_SAME_MIN) * 60;
    CHECK(week_trend(*s).trend == Trend::More);
    s->prev_week_s = (150 + 7 * (TREND_SAME_MIN - 1)) * 60;
    CHECK(week_trend(*s).trend == Trend::Same && week_trend(*s).per_day_min == -(TREND_SAME_MIN - 1));
    s->prev_week_s = (150 + 7 * TREND_SAME_MIN) * 60;
    CHECK(week_trend(*s).trend == Trend::Less);
    s->prev_week_s = 150 * 60;
    t = week_trend(*s);
    CHECK(t.trend == Trend::Same && t.amount_min == 0);
    // At most a day: more than 24 h a day apart says 24 h.
    s->prev_week_s = 7 * 30 * 3600ULL;
    t = week_trend(*s);
    CHECK(t.trend == Trend::Less && t.per_day_min < -24 * 60 && t.amount_min == 24 * 60);
}

static void test_today_and_days()
{
    auto s = sample();
    TodayPlay t = today_play(*s);
    CHECK(t.total_s == 30 * 60 && t.games == 1);
    s->games[1].today_s = 5;
    t = today_play(*s);
    CHECK(t.total_s == 30 * 60 + 5 && t.games == 2);
    s->count = 0;
    t = today_play(*s);
    CHECK(t.total_s == 0 && t.games == 0);

    s = sample();
    uint64_t days[7];
    std::memset(days, 0xff, sizeof days);   // every day written, not added to
    day_totals(*s, days);
    CHECK(days[0] == 30 * 60 && days[1] == 0 && days[2] == 120 * 60);
    for (int k = 3; k < 7; k++) CHECK(days[k] == 0);
}

static void test_hours()
{
    auto s = sample();
    CHECK(!hours(*s).known);                              // not read
    s->hours_ok = true;
    Hours none = hours(*s);
    CHECK(none.known && none.sessions == 0 && none.busy_from == -1 && none.late_s == 0 && none.longest_day == -1);
    s->hours.sessions = 4;
    s->hours.session_s = 4 * 3000;
    s->hours.longest_s = 5400;
    s->hours.longest_day = 2;
    s->hours.hour_s[0][17] = 1800;                        // today 17:00, 18:00
    s->hours.hour_s[0][18] = 3600;
    s->hours.hour_s[2][18] = 600;                         // two days ago 18:00
    s->hours.hour_s[2][22] = 1200;                        // late: 22:00 and 05:00
    s->hours.hour_s[5][5] = 900;
    s->hours.hour_s[5][6] = 4000;                         // 06:00: not late
    Hours h = hours(*s);
    CHECK(h.sessions == 4 && h.average_s == 3000 && h.longest_s == 5400 && h.longest_day == 2);
    CHECK(h.busy_from == 17 && h.busy_s == 1800 + 3600 + 600);
    CHECK(h.late_s == 1200 + 900);
    s->windows_ok = false;
    CHECK(!hours(*s).known);
}

static void test_edges()
{
    // A tie: the one played last first, as the list.
    auto s = sample();
    s->games[0].last_played = NOW - DAY;
    s->games[1].week_s = 70 * 60;
    s->games[1].last_played = NOW - 3600;
    CHECK(summarize(*s, Week).top == 1);
    s->games[1].last_played = NOW - 5 * DAY;
    CHECK(summarize(*s, Week).top == 0);

    // Nothing played today: no top game.
    s = sample();
    s->games[0].today_s = 0;
    Summary t = summarize(*s, Today);
    CHECK(t.known && t.total_s == 0 && t.top == -1);

    // The log unreadable: today and the week unknown, all time still known.
    s = sample();
    s->windows_ok = false;
    CHECK(!summarize(*s, Today).known && !summarize(*s, Week).known && summarize(*s, AllTime).known);
    // The statistics unreadable: all time unknown.
    s = sample();
    s->stats_rc = 0x1A0C;
    CHECK(!summarize(*s, AllTime).known && summarize(*s, Week).known);
    // The game list unreadable: nothing.
    s = sample();
    s->rc = 0x1A0C;
    for (int p = 0; p < 3; p++) CHECK(!summarize(*s, p).known);

    // A game without totals counts for none of them, and no first play known
    // means no average per day.
    s = sample();
    s->games[1].totals_ok = false;
    for (uint32_t i = 0; i < s->count; i++) s->games[i].first_played = 0;
    Summary all = summarize(*s, AllTime);
    CHECK(all.total_s == 600 * 60 && all.days == 0 && all.per_day_s == 0 && all.session_s == 600 * 60 / 20);
    // No launch counted: no session length.
    s->games[0].launches = 0;
    CHECK(summarize(*s, AllTime).session_s == 0);

    // A first play "in the future" (the clock went back): one day.
    s = sample();
    s->games[0].first_played = s->games[1].first_played = NOW + DAY;
    CHECK(summarize(*s, AllTime).days == 1);
    // First played earlier today: one day.
    s->games[0].first_played = s->games[1].first_played = NOW - 60;
    CHECK(summarize(*s, AllTime).days == 1);

    // Nothing played this week: no busiest day.
    s = sample();
    for (uint32_t i = 0; i < s->count; i++) {
        s->games[i].week_s = s->games[i].today_s = 0;
        std::memset(s->games[i].day_s, 0, sizeof s->games[i].day_s);
    }
    Summary w = summarize(*s, Week);
    CHECK(w.days_played == 0 && w.busiest == -1 && w.per_day_s == 0 && w.top == -1);
}

static void test_unattributed()
{
    AccountPlay a[3] = {};
    a[0].total_s = 3000;
    a[1].total_s = 1000;
    // The game's 5000 s: 1000 s on no account.
    CHECK(unattributed_s(5000, a, 2) == 1000);
    // No account listed: all of it.
    CHECK(unattributed_s(5000, a, 0) == 5000);
    // A few seconds apart: nothing to show.
    CHECK(unattributed_s(4000 + UNATTRIBUTED_MIN_S - 1, a, 2) == 0);
    CHECK(unattributed_s(4000 + UNATTRIBUTED_MIN_S, a, 2) == UNATTRIBUTED_MIN_S);
    // Two accounts open together count the same time twice: never negative.
    CHECK(unattributed_s(3500, a, 2) == 0);
    CHECK(unattributed_s(4000, a, 2) == 0);
    CHECK(unattributed_s(0, a, 2) == 0);
    // No overflow, whatever the figures.
    a[2].total_s = UINT64_MAX;
    CHECK(unattributed_s(UINT64_MAX, a, 3) == 0);
}

static std::string names(const std::vector<const GameStat*>& games)
{
    std::string out;
    for (const GameStat* g : games) out += std::string(out.empty() ? "" : ",") + g->name;
    return out;
}

static void test_sort()
{
    auto s = sample();
    add(*s, "Old", 100, 1, 20);    // nothing this week, more in all, played longer ago
    add(*s, "New", 50, 1, 10);
    std::vector<const GameStat*> games;
    for (uint32_t i = 0; i < s->count; i++) games.push_back(&s->games[i]);

    sort_by_play(games, Week, false);
    CHECK(names(games) == "Farm,Kart,New,Old");   // a tie: the one played last first
    sort_by_play(games, Week, true);
    CHECK(names(games) == "Farm,Kart,Old,New");   // the export: all time first
    sort_by_play(games, Today, false);
    CHECK(names(games) == "Kart,Farm,New,Old");
    sort_by_play(games, AllTime, false);
    CHECK(names(games) == "Kart,Farm,Old,New");
    // All time without its totals: none played, the most recent first.
    for (uint32_t i = 0; i < s->count; i++) s->games[i].totals_ok = false;
    sort_by_play(games, AllTime, true);
    CHECK(names(games) == "Kart,Farm,Old,New");   // total_s still breaks the tie, then the last play
}

static void test_ranked_and_grew()
{
    auto s = sample();
    add(*s, "Old", 100, 1, 20);   // nothing this week
    GameStat& n = add(*s, "New", 50, 1, 10);
    n.week_s = 10 * 60;
    // Only the games played in the period, most played first.
    CHECK(names(ranked_games(*s, Week)) == "Farm,Kart,New");
    CHECK(names(ranked_games(*s, Today)) == "Kart");
    CHECK(names(ranked_games(*s, AllTime)) == "Kart,Farm,Old,New");
    s->games[0].totals_ok = false;   // all time without Kart's totals: left out
    CHECK(names(ranked_games(*s, AllTime)) == "Farm,Old,New");
    s->games[0].totals_ok = true;

    std::vector<const GameStat*> games = ranked_games(*s, Week);
    CHECK(grew_most(*s, games, 4, Week).game == nullptr);   // the week before not read
    s->prev_week_ok = true;
    // Farm 80 min (20 the week before), Kart 70 (69 before), New 10 (0).
    s->games[1].prev_week_s = 20 * 60;
    s->games[0].prev_week_s = 1;
    Grew g = grew_most(*s, games, 4, Week);
    CHECK(g.game == &s->games[0] && g.more_s == 70 * 60 - 1);   // Kart: the most, not the first
    // Only the cards shown count.
    g = grew_most(*s, games, 1, Week);
    CHECK(g.game == &s->games[1] && g.more_s == 60 * 60);
    CHECK(grew_most(*s, games, 0, Week).game == nullptr);
    // GREW_MIN at least: 29 min 59 s more is not enough.
    s->games[0].prev_week_s = s->games[0].week_s;
    s->games[1].prev_week_s = s->games[1].week_s - (uint64_t)GREW_MIN * 60 + 1;
    CHECK(grew_most(*s, games, 4, Week).game == nullptr);
    s->games[1].prev_week_s -= 1;
    g = grew_most(*s, games, 4, Week);
    CHECK(g.game == &s->games[1] && g.more_s == (uint64_t)GREW_MIN * 60);
    // Played less: never.
    s->games[1].prev_week_s = 500 * 60;
    CHECK(grew_most(*s, games, 4, Week).game == nullptr);
    // Another period, or the last 7 days not read: none.
    s->games[1].prev_week_s = 0;
    CHECK(grew_most(*s, games, 4, Week).game == &s->games[1]);
    CHECK(grew_most(*s, games, 4, Today).game == nullptr);
    CHECK(grew_most(*s, games, 4, Month).game == nullptr);
    s->windows_ok = false;
    CHECK(grew_most(*s, games, 4, Week).game == nullptr);
}

static ExportText words()
{
    ExportText t;
    t.title = "T";
    t.subtitle = "S";
    t.sheet = "Sheet";
    t.game = "Game";
    t.id = "ID";
    t.today = "Today";
    t.week = "Week";
    t.month = "Month";
    t.total = "Total";
    t.launches = "Launches";
    t.first = "First";
    t.last = "Last";
    t.day = [](int wday) { return "d" + std::to_string(wday); };
    t.name = [](const GameStat& g) { return g.name[0] ? std::string(g.name) : std::string("(deleted)"); };
    t.time = [](uint64_t posix) {
        CHECK(posix != 0);
        return "t" + std::to_string(posix);
    };
    return t;
}

static void test_export()
{
    auto s = sample();
    GameStat& gone = add(*s, "", 1, 1, 40);   // deleted: its name from the tab
    gone.today_s = 89;                        // 1 min 29 s: 1 min
    gone.day_s[1] = 90;                       // 1 min 30 s: 2 min
    gone.first_played = 0;                    // unknown: an empty cell
    table_export::Table t = export_table(*s, Week, words());
    CHECK(t.title == "T" && t.subtitle == "S" && t.sheet == "Sheet");

    // The columns: the game, today, the six days before (most recent first,
    // named after their weekday), the week, the last 30 days, all time,
    // launches, plays.
    if (!CHECK(t.columns.size() == 15)) return;
    const char* keys[] = { "game", "title_id", "today_min", "day_1_min", "day_2_min", "day_3_min", "day_4_min",
                           "day_5_min", "day_6_min", "week_min", "month_min", "total_min", "launches",
                           "first_played", "last_played" };
    for (size_t i = 0; i < 15; i++) CHECK(t.columns[i].key == keys[i]);
    CHECK(t.columns[0].label == "Game" && t.columns[1].label == "ID" && t.columns[2].label == "Today");
    CHECK(t.columns[3].label == "d2" && t.columns[4].label == "d1" && t.columns[8].label == "d4");
    CHECK(t.columns[9].label == "Week" && t.columns[10].label == "Month" && t.columns[14].label == "Last");
    CHECK(!t.columns[0].numeric && !t.columns[1].numeric && t.columns[2].numeric && t.columns[8].numeric);
    CHECK(t.columns[10].numeric && t.columns[12].numeric && !t.columns[13].numeric && !t.columns[14].numeric);

    // The rows: the export's order, every column.
    if (!CHECK(t.rows.size() == 3)) return;
    for (const auto& r : t.rows) CHECK(r.size() == 15);
    const std::vector<std::string> farm = { "Farm", "0100000000012000", "0", "0", "80", "0", "0", "0", "0",
                                            "80", "80", "400", "20", "t" + std::to_string(NOW - 99 * DAY - 3600),
                                            "t" + std::to_string(NOW - 2 * DAY) };
    CHECK(t.rows[0] == farm);
    CHECK(t.rows[1][0] == "Kart" && t.rows[1][2] == "30" && t.rows[1][4] == "40" && t.rows[1][9] == "70" && t.rows[1][10] == "300");
    CHECK(t.rows[2][0] == "(deleted)" && t.rows[2][2] == "1" && t.rows[2][3] == "2" && t.rows[2][10] == "0");
    CHECK(t.rows[2][13].empty() && t.rows[2][14] == "t" + std::to_string(NOW - 40 * DAY));

    // The log unread: no figure of the week or the month; the statistics
    // unread for a game: none of its totals.
    s->windows_ok = false;
    s->games[0].totals_ok = false;
    t = export_table(*s, AllTime, words());
    if (!CHECK(t.rows.size() == 3)) return;
    CHECK(t.rows[0][0] == "Farm" && t.rows[1][0] == "(deleted)" && t.rows[2][0] == "Kart");
    for (const auto& r : t.rows)
        for (size_t i = 2; i <= 10; i++) CHECK(r[i].empty());
    for (size_t i = 11; i < 15; i++) CHECK(t.rows[2][i].empty());
    CHECK(t.rows[0][11] == "400" && t.rows[0][12] == "20");
    // Only the month unread: its column empty, the week's there.
    s->windows_ok = true;
    s->month_ok = false;
    t = export_table(*s, Week, words());
    CHECK(t.rows[0][9] == "80" && t.rows[0][10].empty());

    // No game: the columns only.
    s->count = 0;
    t = export_table(*s, Week, words());
    CHECK(t.columns.size() == 15 && t.rows.empty());
}

static void test_start_account()
{
    const std::string A(32, 'A'), B(32, 'B');
    const std::vector<std::string> keys = { A, B };
    CHECK(start_account("last", B, keys) == 1);
    CHECK(start_account("last", "", keys) == -1);
    CHECK(start_account("last", std::string(32, 'C'), keys) == -1);   // an account gone since
    CHECK(start_account(A, B, keys) == 0);                             // a fixed one, whatever was shown last
    CHECK(start_account("", B, keys) == -1);                           // every account
    CHECK(start_account(B, "", {}) == -1);                             // no account listed
}

int main()
{
    test_periods();
    test_week_trend();
    test_week_trend_words();
    test_today_and_days();
    test_hours();
    test_edges();
    test_unattributed();
    test_sort();
    test_ranked_and_grew();
    test_export();
    test_start_account();
    return CHECK_DONE("activity_summary assertions passed");
}
