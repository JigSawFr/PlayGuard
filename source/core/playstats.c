// PlayGuard — play data service layer (see playstats.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "playstats.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/playlog.h"
#include "calendar.h"
#include "playstats_logic.h"
#include "time_ops.h"

#define RECORD_CHUNK 64
#define DAY_S        86400u
#define ICONS_MAX_BYTES (16u << 20)          // the icons kept on the SD card, in all
#define ICONS_MAX_FILES (2 * PLAYSTATS_MAX)

// Names read earlier (this run, or the last one's through playstats_remember):
// reading a game's control data is the slow part, about 0.5 s a game on
// 20.0.0+, and names do not change while the app is open.
static PlayNames s_names;
// Games with no readable name this run (deleted), not asked again.
static u64 s_unnamed[PLAYSTATS_MAX];
static u32 s_unnamed_count;
// The games of the earlier read, for the all-time totals.
static u64 s_known[PLAYSTATS_MAX];
static u32 s_known_count;
// Every game the play log names, read once a run (the whole log): a game
// deleted before PlayGuard first read the console is in no earlier read.
#define LOGGED_MAX (4 * PLAYSTATS_MAX)
static u64  s_logged[LOGGED_MAX];
static u32  s_logged_count;
static bool s_logged_read;

static char s_icon_dir[sizeof(((IconStore *)0)->dir)];
static IconStore s_icons;
static bool s_icons_tried;

static atomic_bool s_cancel;

void playstats_cancel(void)
{
    atomic_store(&s_cancel, true);
}

bool playstats_cancelled(void)
{
    return atomic_load(&s_cancel);
}

// Never shown: only a quitting app cancels.
#define RC_CANCELLED MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen)

static Result query_events(s32 index, PdmPlayEvent *events, s32 count, s32 *got)
{
    if (playstats_cancelled()) return RC_CANCELLED;
    return pdmqryQueryPlayEvent(index, events, count, got);
}

// Appends `id` to the array *ids (*count used of *cap, grown as needed).
// False, and nothing added, when it cannot grow.
static bool push_id(u64 **ids, size_t *count, size_t *cap, u64 id)
{
    if (*count == *cap) {
        size_t grown = *cap ? *cap * 2 : 128;
        u64 *bigger = (u64 *)realloc(*ids, grown * sizeof(u64));
        if (!bigger) return false;
        *ids = bigger;
        *cap = grown;
    }
    (*ids)[(*count)++] = id;
    return true;
}

// Adds `id` to the candidates for the all-time totals, once.
static void add_candidate(u64 **ids, size_t *count, size_t *cap, u64 id)
{
    for (size_t i = 0; i < *count; i++)
        if ((*ids)[i] == id) return;
    push_id(ids, count, cap, id);
}

static GameStat *find_or_add(PlayStats *out, u64 id)
{
    for (u32 i = 0; i < out->count; i++)
        if (out->games[i].app_id == id) return &out->games[i];
    if (out->count == PLAYSTATS_MAX) return NULL;
    GameStat *g = &out->games[out->count++];
    memset(g, 0, sizeof(*g));
    g->app_id = id;
    return g;
}

// The log entries since `since` (user clock), oldest first. *events is
// malloc'ed (NULL when empty); the caller frees it.
static Result read_events(u64 since, PlayLogEvent **events, size_t *count, bool *capped)
{
    *events = NULL;
    *count  = 0;
    *capped = false;
    s32 total = 0, start = 0, end = 0;
    Result rc = pdmqryGetAvailablePlayEventRange(&total, &start, &end);
    if (R_FAILED(rc) || total <= 0) return rc;
    return playstats_collect_events(query_events, start, total, since, PLAYSTATS_EVENTS_MAX, events, count, capped);
}

static void open_icon_store(u8 lang)
{
    if (s_icons_tried || !s_icon_dir[0] || !lang) return;
    s_icons_tried = true;
    icon_store_open(&s_icons, s_icon_dir, lang, ICONS_MAX_BYTES, ICONS_MAX_FILES);
}

void playstats_set_icon_dir(const char *dir)
{
    const int n = snprintf(s_icon_dir, sizeof(s_icon_dir), "%s", dir ? dir : "");
    if (n < 0 || (size_t)n >= sizeof(s_icon_dir)) s_icon_dir[0] = '\0';   // too long: none
}

void playstats_remember(const PlayStats *known)
{
    playnames_seed(&s_names, known);
    s_known_count = 0;
    for (u32 i = 0; known && i < known->count && i < PLAYSTATS_MAX; i++) s_known[s_known_count++] = known->games[i].app_id;
}

// Keeps the icon of control data read for something else (one read a game).
static void keep_icon(u64 id, const NsApplicationControlData *cd, u64 size)
{
    if (size > sizeof(cd->nacp)) icon_store_put(&s_icons, id, cd->icon, (size_t)(size - sizeof(cd->nacp)));
}

static void read_name(GameStat *g, NsApplicationControlData **cd, u32 nacp_index)
{
    const char *known = playnames_find(&s_names, g->app_id);
    if (known) {
        nacp_copy_utf8(g->name, sizeof(g->name), known);
        return;
    }
    for (u32 i = 0; i < s_unnamed_count; i++)
        if (s_unnamed[i] == g->app_id) return;
    if (!*cd) *cd = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    u64 size = 0;
    if (*cd && nacp_read_control(g->app_id, *cd, false, &size)) {
        keep_icon(g->app_id, *cd, size);
        if (nacp_name(&(*cd)->nacp, sizeof((*cd)->nacp), nacp_index, g->name, sizeof(g->name))) {
            playnames_add(&s_names, g->app_id, g->name);
            return;
        }
    }
    // Not installed any more, or no name worth showing: the UI shows the title ID.
    if (*cd && s_unnamed_count < PLAYSTATS_MAX) s_unnamed[s_unnamed_count++] = g->app_id;
}

// All-time statistics of one game, for every account or one.
static Result query_totals(u64 app_id, const PlayAccount *account, PdmPlayStatistics *st)
{
    memset(st, 0, sizeof(*st));
    if (!account) return pdmqryQueryPlayStatisticsByApplicationId(app_id, false, st);
    AccountUid uid;
    uid.uid[0] = account->uid[0];
    uid.uid[1] = account->uid[1];
    return pdmqryQueryPlayStatisticsByApplicationIdAndUserAccountId(app_id, uid, false, st);
}

static void set_totals(GameStat *g, const PdmPlayStatistics *st)
{
    g->totals_ok    = true;
    g->total_s      = st->playtime / 1000000000ULL;
    g->launches     = st->total_launches;
    g->first_played = st->first_timestamp_user;
    g->last_played  = st->last_timestamp_user;
}

// PlayTotalsQuery for playstats_add_played: `ctx` is the account, or NULL.
static bool played_totals(void *ctx, u64 app_id, GameStat *g)
{
    if (playstats_cancelled()) return false;
    PdmPlayStatistics st;
    if (R_FAILED(query_totals(app_id, (const PlayAccount *)ctx, &st))) return false;
    if (st.playtime == 0 && st.total_launches == 0) return false;
    set_totals(g, &st);
    return true;
}

void playstats_fetch(PlayStats *out)
{
    playstats_fetch_for(out, NULL);
}

void playstats_fetch_for(PlayStats *out, const PlayAccount *account)
{
    memset(out, 0, sizeof(*out));
    time_local_now(&out->now, NULL);   // live, unlike time() (calendar.h)

    out->rc = nsInitialize();
    if (R_FAILED(out->rc)) return;

    // Installed games.
    u64 *ids = NULL;
    size_t id_count = 0, id_cap = 0;
    NsApplicationRecord records[RECORD_CHUNK];
    for (s32 offset = 0; !playstats_cancelled();) {
        s32 got = 0;
        Result rc = nsListApplicationRecord(records, RECORD_CHUNK, offset, &got);
        if (R_FAILED(rc)) {
            if (offset == 0) out->rc = rc;
            break;
        }
        if (got <= 0) break;
        for (s32 i = 0; i < got; i++)
            if (!push_id(&ids, &id_count, &id_cap, records[i].application_id)) break;
        offset += got;
        if (got < RECORD_CHUNK) break;
    }
    if (R_FAILED(out->rc)) {
        free(ids);
        nsExit();
        return;
    }
    // Then the games of the earlier read that are not installed any more
    // (pdm keeps their statistics), for "all time".
    for (u32 k = 0; k < s_known_count; k++) add_candidate(&ids, &id_count, &id_cap, s_known[k]);

    out->stats_rc  = pdmqryInitialize();
    out->events_rc = out->stats_rc;
    if (R_SUCCEEDED(out->stats_rc)) {
        // And every game the log still names, deleted before PlayGuard first
        // read the console included (read once a run: the log holds years).
        if (!s_logged_read && !playstats_cancelled()) {
            s32 total = 0, start = 0, end = 0;
            Result rc = pdmqryGetAvailablePlayEventRange(&total, &start, &end);
            if (R_SUCCEEDED(rc)) s_logged_count = (u32)playstats_logged_games(query_events, start, total, s_logged, LOGGED_MAX, &rc);
            s_logged_read = R_SUCCEEDED(rc);   // a failed or cancelled read: again next fetch
        }
        for (u32 k = 0; k < s_logged_count; k++) add_candidate(&ids, &id_count, &id_cap, s_logged[k]);
        // Today and the last 7 days. A session can start up to a day before
        // the window and still end inside it.
        // Midnights through the console's rule: a day with a daylight-saving
        // change is 23 or 25 h long, and the week counts it as one day.
        const TimeRule *rule = time_console_rule();
        // The 7 days before too, for the week's trend (Activity).
        u64 day_starts[14];
        for (int k = 0; k < 14; k++) day_starts[k] = local_midnight(rule, out->now, k);
        LocalTime today;   // the weekday of the same instant, not a second read
        const int wday = rule->to_local(rule->ctx, out->now, &today) ? today.wday : 0;
        for (int k = 0; k < 7; k++) out->day_wday[k] = (u8)((wday - k + 7) % 7);
        PlayLogEvent *events = NULL;
        size_t event_count = 0;
        bool capped = false;
        out->events_rc = read_events(day_starts[13] >= DAY_S ? day_starts[13] - DAY_S : 0, &events, &event_count, &capped);
        out->approximate = capped;
        // One account: only the time it was open in the game (in place: the
        // log of a clock set back months can be large).
        if (R_SUCCEEDED(out->events_rc) && account && event_count)
            event_count = playlog_for_account(events, event_count, account->uid, events, event_count);
        if (R_SUCCEEDED(out->events_rc)) {
            // One more than the list holds: that one says it is cut.
            static PlayLogTotal totals[PLAYSTATS_MAX + 1];
            size_t n = playlog_fold_days(events, event_count, out->now, day_starts,
                                         totals, PLAYSTATS_MAX + 1);
            if (n > PLAYSTATS_MAX) {
                out->truncated = true;
                n = PLAYSTATS_MAX;
            }
            for (size_t i = 0; i < n && !playstats_cancelled(); i++) {
                GameStat *g = find_or_add(out, totals[i].app_id);   // also games deleted since
                if (!g) break;
                g->today_s = totals[i].today_s;
                g->week_s  = totals[i].week_s;
                memcpy(g->day_s, totals[i].day_s, sizeof(g->day_s));
                // Its all-time totals too (pdm keeps them for a game deleted since).
                PdmPlayStatistics st;
                if (R_SUCCEEDED(query_totals(g->app_id, account, &st)) && (st.playtime || st.total_launches))
                    set_totals(g, &st);
            }
            out->windows_ok = true;
            // The week before: the same log folded over its days, up to the
            // start of the last 7 (each game's week_s is then that week's).
            size_t prev = playlog_fold_days(events, event_count, day_starts[6], day_starts + 7, totals, PLAYSTATS_MAX + 1);
            u64 prev_s = 0;
            for (size_t i = 0; i < prev; i++) {
                prev_s += totals[i].week_s;
                // Each game of the last 7 days its own (Activity's cards); a
                // game played only the week before is not added for it.
                for (u32 k = 0; k < out->count; k++)
                    if (out->games[k].app_id == totals[i].app_id) out->games[k].prev_week_s = totals[i].week_s;
            }
            out->prev_week_s  = (u32)(prev_s > 0xFFFFFFFFu ? 0xFFFFFFFFu : prev_s);
            out->prev_week_ok = !playstats_cancelled();
        }
        // Then the all-time totals of every other installed game ever played,
        // while there is room: the games of this week come first, so a large
        // library never pushes today's play out of the list (PLAYSTATS_MAX).
        playstats_add_played(out, ids, id_count, played_totals, (void *)account);
        free(events);
        pdmqryExit();
    }
    free(ids);

    // Names: those already known, then ns for the others (and their icons,
    // from the same read).
    u32 nacp_index = 0;
    out->names_lang = nacp_console_language(&nacp_index);
    playnames_use_language(&s_names, out->names_lang);
    open_icon_store(out->names_lang);
    NsApplicationControlData *cd = NULL;
    for (u32 i = 0; i < out->count && !playstats_cancelled(); i++) read_name(&out->games[i], &cd, nacp_index);
    free(cd);
    nsExit();
}

void playstats_icons(PlayIcon *icons, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        icons[i].jpeg = NULL;
        icons[i].size = 0;
    }
    if (!count) return;
    u32 nacp_index = 0;
    const u8 lang = nacp_console_language(&nacp_index);
    open_icon_store(lang);
    // Kept on the SD card by an earlier read: no ns at all.
    size_t missing = 0;
    for (size_t i = 0; i < count && !playstats_cancelled(); i++) {
        icons[i].jpeg = icon_store_get(&s_icons, icons[i].app_id, &icons[i].size);
        if (!icons[i].jpeg) missing++;
    }
    if (!missing || playstats_cancelled() || R_FAILED(nsInitialize())) return;
    playnames_use_language(&s_names, lang);
    NsApplicationControlData *cd = (NsApplicationControlData *)malloc(sizeof(NsApplicationControlData));
    if (cd) {
        for (size_t i = 0; i < count && !playstats_cancelled(); i++) {
            if (icons[i].jpeg) continue;
            u64 got = 0;
            if (!nacp_read_control(icons[i].app_id, cd, true, &got)) continue;
            const size_t size = got - sizeof(cd->nacp);
            if (size > sizeof(cd->icon)) continue;
            icons[i].jpeg = (unsigned char *)malloc(size);
            if (!icons[i].jpeg) break;   // out of memory: the rest stay without an icon
            memcpy(icons[i].jpeg, cd->icon, size);
            icons[i].size = size;
            icon_store_put(&s_icons, icons[i].app_id, icons[i].jpeg, size);
            // Its name from the same read, for the next fetch.
            char name[PLAYSTATS_NAME_LEN];
            if (!playnames_find(&s_names, icons[i].app_id) &&
                nacp_name(&cd->nacp, sizeof(cd->nacp), nacp_index, name, sizeof(name)))
                playnames_add(&s_names, icons[i].app_id, name);
        }
        free(cd);
    }
    nsExit();
}

size_t playstats_accounts(PlayAccount *out, size_t max, Result *rc_out)
{
    size_t n = 0;
    Result rc = accountInitialize(AccountServiceType_Application);
    if (R_SUCCEEDED(rc)) {
        AccountUid uids[PLAYSTATS_MAX_ACCOUNTS];
        s32 count = 0;
        rc = accountListAllUsers(uids, PLAYSTATS_MAX_ACCOUNTS, &count);
        for (s32 i = 0; R_SUCCEEDED(rc) && i < count && n < max; i++) {
            PlayAccount *a = &out[n++];
            memset(a, 0, sizeof(*a));
            a->uid[0] = uids[i].uid[0];
            a->uid[1] = uids[i].uid[1];
            AccountProfile profile;
            AccountProfileBase base;
            if (R_SUCCEEDED(accountGetProfile(&profile, uids[i]))) {
                if (R_SUCCEEDED(accountProfileGet(&profile, NULL, &base)))
                    nacp_copy_utf8(a->nickname, sizeof(a->nickname), base.nickname);
                accountProfileClose(&profile);
            }
        }
        accountExit();
    }
    if (rc_out) *rc_out = rc;
    return n;
}

size_t playstats_by_account(u64 app_id, AccountPlay *out, size_t max, Result *rc_out)
{
    size_t n = 0;
    Result rc = accountInitialize(AccountServiceType_Application);
    if (R_SUCCEEDED(rc)) {
        AccountUid uids[PLAYSTATS_MAX_ACCOUNTS];
        s32 count = 0;
        rc = accountListAllUsers(uids, PLAYSTATS_MAX_ACCOUNTS, &count);
        if (R_SUCCEEDED(rc)) rc = pdmqryInitialize();
        if (R_SUCCEEDED(rc)) {
            Result first_failed = 0;
            for (s32 i = 0; i < count && n < max; i++) {
                PdmPlayStatistics st;
                memset(&st, 0, sizeof(st));
                const Result qrc = pdmqryQueryPlayStatisticsByApplicationIdAndUserAccountId(app_id, uids[i], false, &st);
                if (R_FAILED(qrc) && R_SUCCEEDED(first_failed)) first_failed = qrc;
                if (R_FAILED(qrc) || (st.playtime == 0 && st.total_launches == 0)) continue;
                AccountPlay *a = &out[n];
                memset(a, 0, sizeof(*a));
                a->total_s  = st.playtime / 1000000000ULL;
                a->launches = st.total_launches;
                AccountProfile profile;
                AccountProfileBase base;
                if (R_SUCCEEDED(accountGetProfile(&profile, uids[i]))) {
                    if (R_SUCCEEDED(accountProfileGet(&profile, NULL, &base)))
                        nacp_copy_utf8(a->nickname, sizeof(a->nickname), base.nickname);
                    accountProfileClose(&profile);
                }
                n++;
            }
            rc = first_failed;   // an account left out: its time is not "no account"
            pdmqryExit();
        }
        accountExit();
    }
    if (rc_out) *rc_out = rc;
    return n;
}
