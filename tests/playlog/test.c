// Host tests for source/util/playlog.c: folding the console's play-event log
// into play time per game for today and the last 7 days.
#include "check.h"
#include <stdio.h>
#include <string.h>

#include "playlog.h"

enum { A = 1, B = 2, C = 3 };
enum { H = 3600, DAY = 86400 };

// "Now" is 18:00 on day 10; today started at midnight; the week six days earlier.
static const uint64_t DAY_START  = 10 * DAY;
static const uint64_t NOW        = 10 * DAY + 18 * H;
static const uint64_t WEEK_START = 4 * DAY;

// playlog_fold_days() with whole days between `day_start` and `week_start`.
static size_t playlog_fold(const PlayLogEvent *events, size_t n, uint64_t now,
                           uint64_t day_start, uint64_t week_start,
                           PlayLogTotal *out, size_t max)
{
    uint64_t starts[7];
    starts[0] = day_start;
    for (int k = 1; k < 7; k++) {
        const uint64_t back = (uint64_t)k * 86400u;
        starts[k] = day_start > week_start + back ? day_start - back : week_start;
    }
    starts[6] = week_start;
    return playlog_fold_days(events, n, now, starts, out, max);
}

static PlayLogEvent ev(uint64_t app, uint8_t kind, uint64_t user)
{
    // Steady clock: the user clock with an arbitrary offset (no clock change).
    PlayLogEvent e = { app, kind, user, user + 5000, { 0, 0 } };
    return e;
}

static size_t fold(const PlayLogEvent *e, size_t n, PlayLogTotal *out)
{
    return playlog_fold(e, n, NOW, DAY_START, WEEK_START, out, 8);
}

static const PlayLogTotal *find(const PlayLogTotal *t, size_t n, uint64_t app)
{
    for (size_t i = 0; i < n; i++)
        if (t[i].app_id == app) return &t[i];
    return NULL;
}

static void test_sessions(void)
{
    PlayLogTotal t[8];
    // Today 9:00-10:00 A; yesterday 20:00-21:30 B; then A again 11:00-11:30.
    const PlayLogEvent log[] = {
        ev(B, PlayLogEv_Focus, 9 * DAY + 20 * H), ev(B, PlayLogEv_Unfocus, 9 * DAY + 21 * H + 1800),
        ev(A, PlayLogEv_Focus, DAY_START + 9 * H),  ev(A, PlayLogEv_Unfocus, DAY_START + 10 * H),
        ev(A, PlayLogEv_Focus, DAY_START + 11 * H), ev(A, PlayLogEv_Unfocus, DAY_START + 11 * H + 1800),
    };
    size_t n = fold(log, 6, t);
    CHECK(n == 2);
    CHECK(find(t, n, A)->today_s == H + 1800 && find(t, n, A)->week_s == H + 1800);
    CHECK(find(t, n, B)->today_s == 0 && find(t, n, B)->week_s == H + 1800);
}

static void test_windows(void)
{
    PlayLogTotal t[8];
    // 23:00 yesterday to 01:00 today: one hour on each side of midnight.
    const PlayLogEvent midnight[] = {
        ev(A, PlayLogEv_Focus, DAY_START - H), ev(A, PlayLogEv_Unfocus, DAY_START + H),
    };
    size_t n = fold(midnight, 2, t);
    CHECK(n == 1 && t[0].today_s == H && t[0].week_s == 2 * H);

    // Across the start of the week window, and entirely before it.
    const PlayLogEvent week[] = {
        ev(A, PlayLogEv_Focus, WEEK_START - 2 * H), ev(A, PlayLogEv_Unfocus, WEEK_START + H),
        ev(B, PlayLogEv_Focus, WEEK_START - 5 * H), ev(B, PlayLogEv_Unfocus, WEEK_START - 4 * H),
    };
    n = fold(week, 4, t);
    CHECK(n == 1 && t[0].app_id == A && t[0].week_s == H && t[0].today_s == 0);
}

static void test_cut_short(void)
{
    PlayLogTotal t[8];
    // A crashes: no "out of focus", the HOME menu gets the focus at 10:00.
    // B is cut short by sleep at 14:00. C gets the focus at 15:00 and is
    // still being played now (18:00).
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Focus, DAY_START + 9 * H),
        ev(0, PlayLogEv_Away,  DAY_START + 10 * H),
        ev(A, PlayLogEv_Unfocus, DAY_START + 12 * H),   // stray: A is not in focus
        ev(B, PlayLogEv_Focus, DAY_START + 13 * H),
        ev(B, PlayLogEv_Focus, DAY_START + 13 * H + 600), // repeated: the first one counts
        ev(0, PlayLogEv_Away,  DAY_START + 14 * H),
        ev(C, PlayLogEv_Focus, DAY_START + 15 * H),
    };
    size_t n = fold(log, 7, t);
    CHECK(n == 3);
    CHECK(find(t, n, A)->today_s == H);
    CHECK(find(t, n, B)->today_s == H);
    CHECK(find(t, n, C)->today_s == 3 * H);

    // Another game getting the focus closes the open one.
    const PlayLogEvent swap[] = {
        ev(A, PlayLogEv_Focus, DAY_START + 9 * H), ev(B, PlayLogEv_Focus, DAY_START + 9 * H + 900),
        ev(B, PlayLogEv_Unfocus, DAY_START + 10 * H),
    };
    n = fold(swap, 3, t);
    CHECK(n == 2 && find(t, n, A)->today_s == 900 && find(t, n, B)->today_s == 2700);
}

static void test_clock_changes(void)
{
    PlayLogTotal t[8];
    // The user clock jumps 2 h forward during the session: the steady clock
    // says 30 min, and the session is placed before its end on the user clock.
    PlayLogEvent jump[] = {
        { A, PlayLogEv_Focus,   DAY_START + 9 * H,              1000, { 0, 0 } },
        { A, PlayLogEv_Unfocus, DAY_START + 9 * H + 2 * H + 1800, 1000 + 1800, { 0, 0 } },
    };
    size_t n = fold(jump, 2, t);
    CHECK(n == 1 && t[0].today_s == 1800);

    // No steady clock: the user clock is used; backwards on both: dropped.
    PlayLogEvent no_steady[] = {
        { A, PlayLogEv_Focus,   DAY_START + 9 * H,  0, { 0, 0 } },
        { A, PlayLogEv_Unfocus, DAY_START + 10 * H, 0, { 0, 0 } },
        { B, PlayLogEv_Focus,   DAY_START + 12 * H, 900, { 0, 0 } },
        { B, PlayLogEv_Unfocus, DAY_START + 11 * H, 800, { 0, 0 } },
    };
    n = fold(no_steady, 4, t);
    CHECK(n == 1 && t[0].app_id == A && t[0].today_s == H);

    // A "session" of more than 24 h is a broken log, not play time.
    const PlayLogEvent broken[] = {
        ev(A, PlayLogEv_Focus, WEEK_START + H), ev(A, PlayLogEv_Unfocus, WEEK_START + 2 * DAY),
    };
    CHECK(fold(broken, 2, t) == 0);

    // Events after "now" (clock moved back since) are clipped to now.
    const PlayLogEvent future[] = {
        ev(A, PlayLogEv_Focus, NOW - H), ev(A, PlayLogEv_Unfocus, NOW + H),
    };
    n = fold(future, 2, t);
    CHECK(n == 1 && t[0].today_s == H);
}

static void test_limits(void)
{
    PlayLogTotal t[2];
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Focus, DAY_START + H), ev(A, PlayLogEv_Unfocus, DAY_START + 2 * H),
        ev(B, PlayLogEv_Focus, DAY_START + 3 * H), ev(B, PlayLogEv_Unfocus, DAY_START + 4 * H),
        ev(C, PlayLogEv_Focus, DAY_START + 5 * H), ev(C, PlayLogEv_Unfocus, DAY_START + 6 * H),
        ev(A, PlayLogEv_Focus, DAY_START + 7 * H), ev(A, PlayLogEv_Unfocus, DAY_START + 8 * H),
    };
    size_t n = playlog_fold(log, 8, NOW, DAY_START, WEEK_START, t, 2);
    CHECK(n == 2 && find(t, n, C) == NULL);   // no room for a third game
    CHECK(find(t, n, A)->today_s == 2 * H);    // known games still add up
    CHECK(playlog_fold(NULL, 0, NOW, DAY_START, WEEK_START, t, 2) == 0);
}

static void test_days(void)
{
    PlayLogTotal t[8];
    // Through playlog_fold: whole days back from today's midnight.
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Focus, 9 * DAY + 23 * H),   ev(A, PlayLogEv_Unfocus, DAY_START + H),        // across midnight
        ev(A, PlayLogEv_Focus, 6 * DAY + 10 * H),   ev(A, PlayLogEv_Unfocus, 6 * DAY + 12 * H),     // 4 days back
        ev(B, PlayLogEv_Focus, WEEK_START + 2 * H), ev(B, PlayLogEv_Unfocus, WEEK_START + 3 * H),   // 6 days back
    };
    size_t n = fold(log, 6, t);
    const PlayLogTotal *a = find(t, n, A), *b = find(t, n, B);
    CHECK(a->day_s[0] == H && a->day_s[1] == H && a->day_s[4] == 2 * H);
    CHECK(a->day_s[2] == 0 && a->day_s[3] == 0 && a->day_s[5] == 0 && a->day_s[6] == 0);
    CHECK(a->today_s == a->day_s[0] && a->week_s == 4 * H);
    CHECK(b->day_s[6] == H && b->week_s == H && b->today_s == 0);

    // Days as the calendar gives them: yesterday was 23 h long (the clocks
    // went forward), so the day before started 23 h before yesterday's start.
    uint64_t starts[7] = { DAY_START, DAY_START - 23 * H };
    for (int k = 2; k < 7; k++) starts[k] = starts[k - 1] - DAY;
    const PlayLogEvent around[] = {
        ev(C, PlayLogEv_Focus, starts[1] - H), ev(C, PlayLogEv_Unfocus, starts[1] + H),
    };
    n = playlog_fold_days(around, 2, NOW, starts, t, 8);
    CHECK(n == 1 && t[0].day_s[2] == H && t[0].day_s[1] == H && t[0].week_s == 2 * H);
    uint32_t sum = 0;
    for (int k = 0; k < 7; k++) sum += t[0].day_s[k];
    CHECK(sum == t[0].week_s);
}

// The week before the last 7 (Activity's trend, core/playstats.c): the same
// log folded over the 7 days before, up to the start of the last 7.
static void test_week_before(void)
{
    PlayLogTotal t[8];
    // Today is day 20 here, so that 14 days back is still after day 0.
    const uint64_t day_start = 20 * DAY, now = day_start + 18 * H;
    uint64_t starts[14];
    for (int k = 0; k < 14; k++) starts[k] = day_start - (uint64_t)k * DAY;
    const PlayLogEvent log[] = {
        ev(C, PlayLogEv_Focus, starts[13] - 2 * H), ev(C, PlayLogEv_Unfocus, starts[13] - H),        // before both
        ev(A, PlayLogEv_Focus, starts[10] + 9 * H), ev(A, PlayLogEv_Unfocus, starts[10] + 11 * H),   // 10 days back
        ev(B, PlayLogEv_Focus, starts[6] - H),      ev(B, PlayLogEv_Unfocus, starts[6] + H),         // across the two weeks
        ev(A, PlayLogEv_Focus, day_start + 9 * H),  ev(A, PlayLogEv_Unfocus, day_start + 10 * H),    // today
        ev(C, PlayLogEv_Focus, now - H),                                                              // still in focus
    };
    size_t n = playlog_fold_days(log, 9, starts[6], starts + 7, t, 8);
    uint32_t prev = 0;
    for (size_t i = 0; i < n; i++) prev += t[i].week_s;
    CHECK(prev == 2 * H + H);   // A's 2 h and B's hour before the last 7 days, nothing of today's
    CHECK(find(t, n, A)->week_s == 2 * H && find(t, n, B)->week_s == H && find(t, n, C) == NULL);
    // And the last 7 days, from the same log: B's other hour, A today, C now.
    n = playlog_fold_days(log, 9, now, starts, t, 8);
    CHECK(find(t, n, B)->week_s == H && find(t, n, A)->week_s == H && find(t, n, C)->today_s == H);
}

static PlayLogEvent acc(uint8_t kind, uint64_t user, uint64_t who)
{
    PlayLogEvent e = { 0, kind, user, user + 5000, { who, who + 100 } };
    return e;
}

static void test_accounts(void)
{
    enum { ALICE = 7, LEO = 9 };
    const uint64_t alice[2] = { ALICE, ALICE + 100 }, leo[2] = { LEO, LEO + 100 };
    const uint64_t T = DAY_START + 9 * H;
    // A starts at 9:00, Alice is picked at 9:05; HOME at 9:30 (the account
    // stays open), back at 9:40; Leo joins at 9:50 (local play); A exits at
    // 10:00 and closes both. Then B launches with Leo only, 11:00-11:30.
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Launch, T),            ev(A, PlayLogEv_Focus, T),
        acc(PlayLogEv_AccountOpen, T + 300, ALICE),
        ev(0, PlayLogEv_Away, T + 1800),       ev(A, PlayLogEv_Focus, T + 2400),
        acc(PlayLogEv_AccountOpen, T + 3000, LEO),
        ev(A, PlayLogEv_Unfocus, T + H),
        acc(PlayLogEv_AccountClose, T + H, ALICE), acc(PlayLogEv_AccountClose, T + H, LEO),
        ev(B, PlayLogEv_Launch, T + 2 * H),    ev(B, PlayLogEv_Focus, T + 2 * H),
        acc(PlayLogEv_AccountOpen, T + 2 * H, LEO),
        ev(B, PlayLogEv_Unfocus, T + 2 * H + 1800),
    };
    const size_t n = sizeof(log) / sizeof(log[0]);
    PlayLogEvent mine[32];
    PlayLogTotal t[8];

    size_t m = playlog_for_account(log, n, alice, mine, 32);
    size_t k = fold(mine, m, t);
    CHECK(k == 1 && find(t, k, A)->today_s == 1500 + 1200);   // 9:05-9:30 and 9:40-10:00

    m = playlog_for_account(log, n, leo, mine, 32);
    k = fold(mine, m, t);
    CHECK(k == 2 && find(t, k, A)->today_s == 600 && find(t, k, B)->today_s == 1800);

    // Everyone: the whole focus time, account events ignored.
    k = fold(log, n, t);
    CHECK(find(t, k, A)->today_s == 1800 + 1200 && find(t, k, B)->today_s == 1800);

    // A game that crashed without closing its account: the next launch does.
    const PlayLogEvent crash[] = {
        ev(A, PlayLogEv_Focus, T), acc(PlayLogEv_AccountOpen, T, ALICE), ev(0, PlayLogEv_Away, T + 600),
        ev(B, PlayLogEv_Launch, T + H), ev(B, PlayLogEv_Focus, T + H), ev(B, PlayLogEv_Unfocus, T + 2 * H),
    };
    m = playlog_for_account(crash, 6, alice, mine, 32);
    k = fold(mine, m, t);
    CHECK(k == 1 && find(t, k, A)->today_s == 600 && find(t, k, B) == NULL);

    // Nothing for an account that never played; the output never overflows.
    const uint64_t nobody[2] = { 1, 2 };
    CHECK(playlog_for_account(log, n, nobody, mine, 32) == 0);
    CHECK(playlog_for_account(log, n, leo, mine, 1) == 1);

    // In place, as playstats.c filters the log it read: the same time, also
    // when the focus goes straight from one game to another.
    const PlayLogEvent swap[] = {
        ev(A, PlayLogEv_Launch, T), acc(PlayLogEv_AccountOpen, T, ALICE), ev(A, PlayLogEv_Focus, T),
        ev(B, PlayLogEv_Focus, T + 600), ev(A, PlayLogEv_Focus, T + 900), ev(A, PlayLogEv_Unfocus, T + 1200),
    };
    const size_t ns = sizeof(swap) / sizeof(swap[0]);
    m = playlog_for_account(swap, ns, alice, mine, 32);
    CHECK(m <= ns);
    k = fold(mine, m, t);
    CHECK(k == 2 && find(t, k, A)->today_s == 600 + 300 && find(t, k, B)->today_s == 300);
    PlayLogEvent same[sizeof(log) / sizeof(log[0])];
    memcpy(same, log, sizeof(log));
    m = playlog_for_account(same, n, alice, same, n);
    k = fold(same, m, t);
    CHECK(k == 1 && find(t, k, A)->today_s == 1500 + 1200);
    memcpy(same, swap, sizeof(swap));
    m = playlog_for_account(same, ns, alice, same, ns);
    k = fold(same, m, t);
    CHECK(k == 2 && find(t, k, A)->today_s == 600 + 300 && find(t, k, B)->today_s == 300);
}

// Each account's days, as the agent publishes them: Alice and Leo in the
// order they were first opened, each day summed over the games.
static void test_account_days(void)
{
    enum { ALICE = 7, LEO = 9 };
    uint64_t starts[7];
    for (int k = 0; k < 7; k++) starts[k] = DAY_START - (uint64_t)k * DAY;
    const uint64_t Y = DAY_START - DAY + 20 * H;   // yesterday, 20:00
    const uint64_t T = DAY_START + 9 * H;
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Launch, Y), acc(PlayLogEv_AccountOpen, Y, LEO), ev(A, PlayLogEv_Focus, Y),
        ev(A, PlayLogEv_Unfocus, Y + 900), acc(PlayLogEv_AccountClose, Y + 900, LEO),
        ev(B, PlayLogEv_Launch, T), acc(PlayLogEv_AccountOpen, T, ALICE), acc(PlayLogEv_AccountOpen, T, LEO),
        ev(B, PlayLogEv_Focus, T), ev(B, PlayLogEv_Unfocus, T + 1200),
        acc(PlayLogEv_AccountClose, T + 1200, ALICE), acc(PlayLogEv_AccountClose, T + 1200, LEO),
        ev(A, PlayLogEv_Launch, T + H), acc(PlayLogEv_AccountOpen, T + H, LEO), ev(A, PlayLogEv_Focus, T + H),
        ev(A, PlayLogEv_Unfocus, T + H + 600),
    };
    const size_t n = sizeof(log) / sizeof(log[0]);
    PlayLogEvent scratch[sizeof(log) / sizeof(log[0])];
    PlayLogTotal t[8];
    PlayLogAccountDays out[4];
    size_t k = playlog_account_days(log, n, NOW, starts, scratch, t, 8, out, 4);
    CHECK(k == 2);
    CHECK(out[0].uid[0] == LEO && out[0].uid[1] == LEO + 100);   // first opened
    CHECK(out[0].day_s[0] == 1200 + 600 && out[0].day_s[1] == 900 && out[0].day_s[2] == 0);
    CHECK(out[1].uid[0] == ALICE && out[1].day_s[0] == 1200 && out[1].day_s[1] == 0);

    // At most `max`; none without an account event.
    CHECK(playlog_account_days(log, n, NOW, starts, scratch, t, 8, out, 1) == 1 && out[0].uid[0] == LEO);
    const PlayLogEvent plain[] = { ev(A, PlayLogEv_Focus, T), ev(A, PlayLogEv_Unfocus, T + 600) };
    CHECK(playlog_account_days(plain, 2, NOW, starts, scratch, t, 8, out, 4) == 0);
}

// When the week's play happened (Activity's hours): each hour of each day,
// and the sessions, games and short breaks together.
static void test_hours(void)
{
    uint64_t starts[7];
    for (int k = 0; k < 7; k++) starts[k] = DAY_START - (uint64_t)k * DAY;
    const PlayLogEvent log[] = {
        ev(A, PlayLogEv_Focus, DAY_START + 9 * H + 1800), ev(B, PlayLogEv_Focus, DAY_START + 10 * H + 600),   // A then B: one session
        ev(0, PlayLogEv_Away, DAY_START + 11 * H),
        ev(A, PlayLogEv_Focus, DAY_START + 11 * H + 240), ev(A, PlayLogEv_Unfocus, DAY_START + 11 * H + 600), // 4 min later: the same
        ev(C, PlayLogEv_Focus, DAY_START - H), ev(C, PlayLogEv_Unfocus, DAY_START + 1200),                  // across midnight
        ev(B, PlayLogEv_Focus, starts[3] + 20 * H), ev(B, PlayLogEv_Unfocus, starts[3] + 22 * H + 1800),    // 3 days back
        ev(A, PlayLogEv_Focus, starts[6] - H), ev(A, PlayLogEv_Unfocus, starts[6] + 1800),                  // from before the week
        ev(A, PlayLogEv_Focus, NOW - 600),                                                                  // still playing
    };
    // Oldest first, as the log is.
    const PlayLogEvent sorted[] = { log[9], log[10], log[7], log[8], log[5], log[6], log[0], log[1], log[2], log[3], log[4], log[11] };
    PlayLogHours h;
    playlog_fold_hours(sorted, 12, NOW, starts, &h);
    CHECK(h.hour_s[0][9] == 1800 && h.hour_s[0][10] == H && h.hour_s[0][11] == 360);
    CHECK(h.hour_s[0][0] == 1200 && h.hour_s[1][23] == H);
    CHECK(h.hour_s[3][20] == H && h.hour_s[3][21] == H && h.hour_s[3][22] == 1800);
    CHECK(h.hour_s[6][0] == 1800);   // only the part inside the week
    CHECK(h.hour_s[0][17] == 600);
    // The sessions: before the week (its part inside), 3 days back, across
    // midnight, the morning's (A, B and A again), the one going on.
    CHECK(h.sessions == 5);
    CHECK(h.longest_s == 2 * H + 1800 && h.longest_day == 3);
    CHECK(h.session_s == 1800 + (2 * H + 1800) + (H + 1200) + (2 * H - 1800 + 600) + 600);
    uint64_t sum = 0;
    for (int k = 0; k < 7; k++)
        for (int hr = 0; hr < 24; hr++) sum += h.hour_s[k][hr];
    CHECK(sum == h.session_s - 240);   // the 4-minute break is not play
}

// Activity's last 30 days: the same fold over a longer window.
static void test_since(void)
{
    PlayLogTotal t[8];
    const uint64_t today = 100 * DAY, now = today + 18 * H;   // far enough from 0 for 40 days back
    const PlayLogEvent log[] = {
        ev(B, PlayLogEv_Focus, today - 40 * DAY), ev(B, PlayLogEv_Unfocus, today - 40 * DAY + H),
        ev(A, PlayLogEv_Focus, today - 20 * DAY), ev(A, PlayLogEv_Unfocus, today - 20 * DAY + H),
        ev(A, PlayLogEv_Focus, today + H), ev(A, PlayLogEv_Unfocus, today + 2 * H),
    };
    const size_t n = playlog_fold_since(log, 6, now, today - 29 * DAY, t, 8);
    CHECK(n == 1 && t[0].app_id == A && t[0].week_s == 2 * H && t[0].today_s == 0 && t[0].day_s[0] == 0);
}

int main(void)
{
    test_sessions();
    test_windows();
    test_cut_short();
    test_clock_changes();
    test_limits();
    test_days();
    test_week_before();
    test_accounts();
    test_account_days();
    test_hours();
    test_since();
    return CHECK_DONE("playlog session, window and per-account assertions passed");
}
