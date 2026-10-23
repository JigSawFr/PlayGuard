// Simulated core/playstats.h: the activity log of the made-up games
// (sim_games.h), shared between made-up accounts (PLAYGUARD_SIM_ACCOUNTS,
// 2 by default: Alice and Léo), each playing twice what the next one does:
// Alice 2/3 of everything and Léo 1/3 (but the time on no account).
// The figures are made here, not read from a raw play log: so core/
// playstats_logic.c (the log's entries, its walk, the names and icons kept on
// the SD card), which works on that log, is not part of the simulator.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#include "../core/calendar.h"
#include "../core/playstats.h"
#include "../core/sim_env.h"
#include "../core/time_ops.h"
#include "sim_games.h"

static const char *const SIM_ACCOUNT_NAMES[] = { "Alice", "Léo", "Maya", "Noah", "Inès", "Hugo", "Zoé", "Sam" };
#define SIM_ACCOUNTS_MAX (sizeof(SIM_ACCOUNT_NAMES) / sizeof(SIM_ACCOUNT_NAMES[0]))

static size_t sim_accounts(void)
{
    return (size_t)sim_env_int("PLAYGUARD_SIM_ACCOUNTS", 2, 0, SIM_ACCOUNTS_MAX);
}

static bool sim_no_pdm(void)
{
    return sim_env_flag("PLAYGUARD_SIM_NO_PDM");
}

size_t playstats_accounts(PlayAccount *out, size_t max, Result *rc)
{
    if (rc) *rc = 0;
    size_t n = 0;
    for (; n < max && n < sim_accounts(); n++) {
        memset(&out[n], 0, sizeof(out[n]));
        out[n].uid[0] = out[n].uid[1] = n + 1;
        snprintf(out[n].nickname, sizeof(out[n].nickname), "%s", SIM_ACCOUNT_NAMES[n]);
    }
    return n;
}

// The share of `v` the k-th of `n` accounts played (weights 2^(n-1-k)), the
// last one taking what the others leave so that the shares add up to `v`.
static u64 share_of(u64 v, size_t k, size_t n)
{
    const u64 total = (1ULL << n) - 1;
    if (k + 1 < n) return v * (1ULL << (n - 1 - k)) / total;
    u64 given = 0;
    for (size_t j = 0; j + 1 < n; j++) given += v * (1ULL << (n - 1 - j)) / total;
    return v - given;
}

// The share of `v` that `account` played (NULL: all of it).
static u64 share(u64 v, const PlayAccount *account)
{
    if (!account) return v;
    const size_t n = sim_accounts();
    const u64 k = account->uid[0] - 1;
    return k < n ? share_of(v, (size_t)k, n) : 0;
}

// All-time seconds of a game played with no account chosen: in its total,
// in no account's.
static u64 nobody_s(u64 app_id)
{
    const SimGame *g = sim_game(app_id);
    return g ? (u64)g->play.nobody_min * 60 : 0;
}

void playstats_fetch(PlayStats *out)
{
    playstats_fetch_for(out, NULL);
}

// When the made-up week was played: each day's games one after the other,
// half from 16:30 and half from 19:00 on school days, from 10:00 and 15:00 at
// the weekend, today's in one stretch ending before now; folded as the
// console's log is (util/playlog.h).
static void sim_hours(PlayStats *out)
{
    const TimeRule *rule = time_console_rule();
    u64 starts[7];
    for (int k = 0; k < 7; k++) starts[k] = local_midnight(rule, out->now, k);
    enum { GAMES = 16 };
    PlayLogEvent ev[7 * GAMES * 2];
    size_t n = 0;
    for (int k = 6; k >= 0; k--) {
        const bool weekend = out->day_wday[k] == 0 || out->day_wday[k] == 6;
        u64 today_s = 0;
        for (u32 i = 0; i < out->count && i < GAMES; i++) today_s += out->games[i].day_s[0];
        for (int block = 0; block < (k == 0 ? 1 : 2); block++) {
            u64 at;
            if (k == 0) {
                const u64 latest = out->now > today_s + 3600 ? out->now - today_s - 3600 : starts[0];
                at = starts[0] + 13 * 3600 + 1800;
                if (at > latest) at = latest > starts[0] ? latest : starts[0];
            } else {
                at = starts[k] + (u64)(weekend ? (block ? 15 * 60 : 10 * 60) : (block ? 19 * 60 : 16 * 60 + 30)) * 60;
            }
            for (u32 i = 0; i < out->count && i < GAMES; i++) {
                const u32 s = out->games[i].day_s[k];
                if (!s || (k != 0 && (int)(i % 2) != block)) continue;
                ev[n++] = (PlayLogEvent){ out->games[i].app_id, PlayLogEv_Focus, at, 0, { 0, 0 } };
                at += s;
                ev[n++] = (PlayLogEvent){ out->games[i].app_id, PlayLogEv_Unfocus, at, 0, { 0, 0 } };
                at += 120;   // two minutes to change games: the same session
            }
        }
    }
    playlog_fold_hours(ev, n, out->now, starts, &out->hours);
    out->hours_ok = true;
}

// One made-up game's figures (`first`: the game PLAYGUARD_SIM_SKIP's time
// goes to, as the timer counts it), as `account` played it (NULL: everyone).
// False when the account never played it.
static bool sim_stat(const SimGame *s, bool first, bool idle_weeks, const PlayAccount *account, u64 now, GameStat *g)
{
    const SimPlay *p = &s->play;
    g->app_id = s->id;
    snprintf(g->name, sizeof(g->name), "%s", s->name);
    g->totals_ok = p->total_min > 0;
    g->total_s = (u64)p->total_min * 60;
    g->launches = p->launches;
    const u32 days_ago = idle_weeks ? p->days_ago + 30 : p->days_ago;
    g->last_played = g->totals_ok ? now - (u64)days_ago * 86400 - 3600 : 0;
    g->first_played = g->totals_ok ? now - 400ULL * 86400 : 0;
    g->today_s = idle_weeks ? 0 : p->today_min * 60;
    g->week_s = idle_weeks ? 0 : p->week_min * 60;
    if (first && !idle_weeks) {
        const u64 logged = (u64)sim_played_today_min() * 60, played = sim_played_today_s();
        const u64 more = played > logged ? played - logged : 0;
        g->today_s += (u32)more;
        g->week_s += (u32)more;
        g->total_s += more;
    }
    // The rest of the week spread over the six days before, unevenly.
    static const u32 spread[6] = { 30, 0, 25, 15, 0, 30 };
    g->day_s[0] = g->today_s;
    const u32 rest = g->week_s - g->today_s;
    u32 given = 0;
    for (int k = 1; k < 7; k++) {
        g->day_s[k] = k < 6 ? rest * spread[k - 1] / 100 : rest - given;
        given += g->day_s[k];
    }
    // One account: its share of every figure; a game it never played goes.
    g->total_s  = account ? share(g->total_s - nobody_s(g->app_id), account) : g->total_s;
    g->launches = (u32)share(g->launches, account);
    g->today_s  = (u32)share(g->today_s, account);
    g->week_s   = (u32)share(g->week_s, account);
    g->prev_week_s = idle_weeks ? 0 : (u32)share((u64)p->prev_week_min * 60, account);
    // The month: the last 7 days, and the 23 before at the rate of the week
    // before them; 1 h 30 for a game last played 7 to 29 days ago.
    g->month_s = g->week_s + (u32)((u64)g->prev_week_s * 23 / 7);
    if (!g->week_s && !g->prev_week_s && days_ago >= 7 && days_ago < 30) g->month_s = (u32)share(90 * 60, account);
    for (int k = 0; k < 7; k++) g->day_s[k] = (u32)share(g->day_s[k], account);
    return !account || g->week_s || g->total_s;
}

void playstats_fetch_for(PlayStats *out, const PlayAccount *account)
{
    memset(out, 0, sizeof(*out));
    time_local_now(&out->now, NULL);
    if (sim_no_pdm()) {
        out->stats_rc = out->events_rc = (Result)0x1A0C;
        return;
    }
    const bool idle_weeks = sim_env_flag("PLAYGUARD_SIM_NO_PLAY");
    out->windows_ok = true;
    LocalTime today;
    const int wday = time_console_rule()->to_local(NULL, out->now, &today) ? today.wday : 0;
    for (int k = 0; k < 7; k++) out->day_wday[k] = (u8)((wday - k + 7) % 7);
    bool first = true;
    for (size_t i = 0; i < SIM_GAME_COUNT && out->count < PLAYSTATS_MAX; i++) {
        if (!SIM_GAMES[i].played) continue;
        if (sim_stat(&SIM_GAMES[i], first, idle_weeks, account, out->now, &out->games[out->count])) out->count++;
        else memset(&out->games[out->count], 0, sizeof(out->games[out->count]));
        first = false;
    }
    // The week before: 8 h in all (the last 7 days have 9 h 50; Star Kart
    // Racers 2 h more), none after weeks without play.
    out->prev_week_ok = true;
    out->prev_week_s = idle_weeks ? 0 : (u32)share(480ULL * 60, account);
    out->month_ok = true;
    // A large library: filler games, each played a little this week.
    const int n = (int)sim_env_int("PLAYGUARD_SIM_GAMES", 0, 0, 100000);
    int i = 0;
    for (; i < n && out->count < PLAYSTATS_MAX; i++) {
        GameStat *g = &out->games[out->count++];
        g->app_id = 0x0100B00000000000ULL + ((u64)i << 12);
        snprintf(g->name, sizeof(g->name), "Game %03d", i + 1);
        g->totals_ok = true;
        g->week_s = g->month_s = g->day_s[1] = (u32)(60 * (n - i));
        g->total_s = g->week_s * 10;
        g->launches = 3;
        g->last_played = out->now - 86400;
        g->first_played = out->now - 30ULL * 86400;
    }
    out->truncated = i < n;   // more than the list holds, as on a console
    sim_hours(out);
}

// The made-up games have their names already; no SD card icons either.
void playstats_remember(const PlayStats *known) { (void)known; }
void playstats_set_icon_dir(const char *dir) { (void)dir; }
static atomic_bool s_cancel;
void playstats_cancel(void) { atomic_store(&s_cancel, true); }
bool playstats_cancelled(void) { return atomic_load(&s_cancel); }

void playstats_icons(PlayIcon *icons, size_t count)
{
    for (size_t i = 0; i < count; i++) icons[i].jpeg = sim_game_icon(icons[i].app_id, &icons[i].size);
}

size_t playstats_by_account(u64 app_id, AccountPlay *out, size_t max, Result *rc)
{
    if (rc) *rc = sim_no_pdm() ? (Result)0x1A0C : 0;
    const size_t n = sim_accounts();
    if (sim_no_pdm() || max < n) return 0;
    // The made-up accounts sharing the play time 2:1 (none for a deleted
    // game), less the time on no account.
    PlayStats all;
    playstats_fetch(&all);
    for (u32 i = 0; i < all.count; i++) {
        if (all.games[i].app_id != app_id || !all.games[i].totals_ok) continue;
        const u64 t = all.games[i].total_s - nobody_s(app_id);
        for (size_t k = 0; k < n; k++) {
            snprintf(out[k].nickname, sizeof(out[k].nickname), "%s", SIM_ACCOUNT_NAMES[k]);
            out[k].total_s = share_of(t, k, n);
            out[k].launches = (u32)share_of(all.games[i].launches, k, n);
        }
        return n;
    }
    return 0;
}
