// Host tests for the simulated console of the desktop and web builds
// (source/sim/, sim_state.h) and its variables (core/sim_env.h):
// - sim_env_flag() and sim_env_int(): the values every variable takes, the
//   web page's changes over the environment's, each value kept once;
// - PLAYGUARD_SIM_KEEP: a change reaches playguard_data/sim_console.bin and
//   the next run starts from it; a file of another layout is not read (each
//   run in a process of its own, the state being set up once per process);
// - the time left today, frozen by PLAYGUARD_SIM_NOW (UTC): the made-up
//   80 minutes against the day's limit, moved on by PLAYGUARD_SIM_SKIP (to
//   the limit and past it, and past midnight), PLAYGUARD_SIM_IDLE,
//   PLAYGUARD_SIM_LEFT_MIN, and "when the time is up" on a warning only;
// - the changes: the read-only switch and the change check first, the play
//   timer's gate, PLAYGUARD_SIM_FAIL;
// - the made-up games: the Activity tab's and the installed ones, one table;
//   a removal kept; accounts sharing the time;
// - the state read and changed from several threads at once (the sanitizers
//   of `make test` watch).
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "core/gamecheck.h"
#include "core/pctl_ops.h"
#include "core/playstats.h"
#include "core/sim_env.h"
#include "core/time_ops.h"
#include "core/write_guard.h"
#include "sim/sim_games.h"
#include "sim/sim_state.h"

// 2026-10-21, a Wednesday (a limit of 120 minutes), 10:00 and 23:30 UTC.
static const char* const WED_10H = "1792576800";
static const char* const WED_23H30 = "1792625400";
static const long long S = 1000000000LL;

static void test_flags_and_numbers()
{
    for (const char* off : {"", "0", "false", "OFF", "No"}) {
        setenv("PLAYGUARD_SIM_TEST", off, 1);
        CHECK(!sim_env_flag("PLAYGUARD_SIM_TEST"));
    }
    for (const char* on : {"1", "yes", "true", "on", "anything"}) {
        setenv("PLAYGUARD_SIM_TEST", on, 1);
        CHECK(sim_env_flag("PLAYGUARD_SIM_TEST"));
    }
    unsetenv("PLAYGUARD_SIM_TEST");
    CHECK(!sim_env_flag("PLAYGUARD_SIM_TEST"));
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 7);
    setenv("PLAYGUARD_SIM_TEST", "4", 1);
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 4);
    setenv("PLAYGUARD_SIM_TEST", "-3", 1);
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 0);
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, -10, 10) == -3);
    setenv("PLAYGUARD_SIM_TEST", "99", 1);
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 10);
    setenv("PLAYGUARD_SIM_TEST", "99999999999999999999999", 1);   // past a long long
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 10);
    setenv("PLAYGUARD_SIM_TEST", "-99999999999999999999999", 1);
    CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 0);
    for (const char* bad : {"", "abc", "12abc", "1.5", " "}) {
        setenv("PLAYGUARD_SIM_TEST", bad, 1);
        CHECK(sim_env_int("PLAYGUARD_SIM_TEST", 7, 0, 10) == 7);
    }

    // The page's change wins over the environment, removes it when empty,
    // and each value is kept once: the same pointer for the same text.
    setenv("PLAYGUARD_SIM_TEST", "env", 1);
    sim_env_set("PLAYGUARD_SIM_TEST", "page");
    CHECK(std::strcmp(sim_env("PLAYGUARD_SIM_TEST"), "page") == 0);
    const char* first = sim_env("PLAYGUARD_SIM_TEST");
    sim_env_set("PLAYGUARD_SIM_TEST", "other");
    CHECK(std::strcmp(first, "page") == 0);   // still valid
    sim_env_set("PLAYGUARD_SIM_TEST", "page");
    CHECK(sim_env("PLAYGUARD_SIM_TEST") == first);
    sim_env_set("PLAYGUARD_SIM_TEST", "");
    CHECK(sim_env("PLAYGUARD_SIM_TEST") == nullptr);
    CHECK(!sim_env_flag("PLAYGUARD_SIM_TEST"));
    unsetenv("PLAYGUARD_SIM_TEST");
}

// ---------------------------------------------------------------- PLAYGUARD_SIM_KEEP

// Runs `run` in a process of its own (a first use of the state there); its
// checks count here through its exit status.
template <typename F> static void in_child(F run)
{
    std::fflush(nullptr);
    const pid_t pid = fork();
    REQUIRE(pid >= 0);
    if (pid == 0) _exit(run() ? 0 : 1);
    int status = 0;
    waitpid(pid, &status, 0);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static std::string read_all(const char* path)
{
    std::string out;
    if (FILE* f = std::fopen(path, "rb")) {
        char buf[4096];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
        std::fclose(f);
    }
    return out;
}

static void test_keep()
{
    TempDir tmp("sim", TempDir::Enter);
    REQUIRE(mkdir("playguard_data", 0700) == 0);
    setenv("PLAYGUARD_SIM_KEEP", "1", 1);
    const char* file = "playguard_data/sim_console.bin";

    // A first run: its state written at once, then its changes.
    in_child([&] {
        PctlStatus st;
        pctl_status_fetch(&st);
        return read_all(file).size() == 8 + 4 + sizeof(SimState) && !st.temp_unlocked &&
               pctl_unlock_restriction_temporarily() == 0 && pctl_set_rating_org(2) == 0 &&
               gamecheck_remove(0x0100A1B2C3D47000ULL) == 0;
    });
    CHECK(read_all(file).compare(0, 8, std::string("PGSIM03", 8)) == 0);
    // The next one starts from it, whatever the variables say.
    in_child([&] {
        setenv("PLAYGUARD_SIM_UNLOCKED", "0", 1);
        const SimState s = sim_state();
        return s.console.temp_unlocked && s.console.rating_org == 2 && s.fixes.removed_count == 1 &&
               s.fixes.removed[0] == 0x0100A1B2C3D47000ULL;
    });
    // A file of another layout: the variables' state.
    std::string other = read_all(file);
    other[6] = '2';
    fs::write(file, other);
    in_child([&] {
        const SimState s = sim_state();
        return !s.console.temp_unlocked && s.console.rating_org == 6 && s.fixes.removed_count == 0;
    });
    // Without PLAYGUARD_SIM_KEEP, nothing read nor written.
    std::remove(file);
    unsetenv("PLAYGUARD_SIM_KEEP");
    in_child([&] { return pctl_relock() == 0 && read_all(file).empty(); });
    CHECK(tmp.remove());
}

// ---------------------------------------------------------------- the time left

static PtState query()
{
    PtState pt;
    pctl_play_timer_query(&pt);
    return pt;
}

static void test_time_left()
{
    setenv("PLAYGUARD_SIM_NOW", WED_10H, 1);
    sim_env_set("PLAYGUARD_SIM_NOT_COUNTING", "1");   // no PlayGuard's own seconds

    // 120 minutes on a Wednesday, 80 played: 40 left, and the 20 s the
    // console adds.
    PtState pt = query();
    CHECK(pt.valid && pt.enabled && !pt.restricted);
    CHECK(pt.day_min[3] == 120 && pt.day_min[6] == 180);
    CHECK(pt.remaining_ns == (40 * 60 + 20) * S);
    CHECK(sim_played_today_min() == 80);

    // Half an hour later, played all along.
    sim_env_set("PLAYGUARD_SIM_SKIP", "1800");
    CHECK(query().remaining_ns == (10 * 60 + 20) * S);
    // Unless no game runs: then the timer has no time left to report.
    sim_env_set("PLAYGUARD_SIM_IDLE", "1");
    CHECK(query().remaining_ns == 0);
    CHECK(sim_played_today_s() == 80 * 60);
    sim_env_set("PLAYGUARD_SIM_IDLE", nullptr);
    // The last minutes, whatever the limit.
    sim_env_set("PLAYGUARD_SIM_LEFT_MIN", "4");
    CHECK(query().remaining_ns == (4 * 60 + 20) * S);
    sim_env_set("PLAYGUARD_SIM_LEFT_MIN", nullptr);

    // An hour later: 140 minutes played, the limit passed.
    sim_env_set("PLAYGUARD_SIM_SKIP", "3600");
    pt = query();
    CHECK(pt.restricted && pt.remaining_ns == 0);
    PtSample sample;
    pctl_play_timer_sample(&sample);
    CHECK(sample.restricted && sample.spent_ns == (u64)(140 * 60) * (u64)S);

    // Past midnight: Thursday, half an hour played since midnight.
    sim_env_set("PLAYGUARD_SIM_NOW", WED_23H30);
    CHECK(sim_played_today_s() == 30 * 60);
    CHECK(query().remaining_ns == (90 * 60 + 20) * S);
    sim_env_set("PLAYGUARD_SIM_NOW", WED_10H);

    // "When the time is up" on a warning only: 20 minutes over, not
    // restricted. The block is changed only unlocked.
    CHECK(pctl_play_timer_set_alarm_only(true) == NXM_RC_WRITE_GATED);
    CHECK(pctl_unlock_restriction_temporarily() == 0);
    CHECK(pctl_play_timer_set_alarm_only(true) == 0);
    CHECK(pctl_relock() == 0);
    pt = query();
    CHECK(pt.alarm_only && !pt.restricted && pt.remaining_ns == -20 * 60 * S);
    CHECK(pctl_unlock_restriction_temporarily() == 0);
    CHECK(pctl_play_timer_set_alarm_only(false) == 0);
    CHECK(pctl_relock() == 0);
    sim_env_set("PLAYGUARD_SIM_SKIP", nullptr);

    // The clocks: the console clock follows the network clock once set.
    TimeSnapshot t;
    time_clock_snapshot(&t);
    CHECK(t.automatic && t.user_time == 1792576800ULL && !t.accuracy);
    TimeApply apply;
    time_clock_apply(1792576800ULL + 600, &apply);
    CHECK(apply.verified && apply.after.user_time == 1792576800ULL + 600 && apply.after.accuracy);
    time_clock_apply(1792576800ULL, &apply);
    CHECK(apply.after.user_time == 1792576800ULL);
}

// ---------------------------------------------------------------- the changes

static bool s_confirm = true;
static bool confirm() { return s_confirm; }

static void test_changes()
{
    const u16 days[7] = { 60, 60, 60, 60, 60, 60, 60 };
    const u16 bad[7] = { 60, 60, 60, 60, 60, 60, 2000 };
    // The play timer counts down: no change to it until unlocked.
    CHECK(pctl_play_timer_set_days(days) == NXM_RC_WRITE_GATED);
    CHECK(pctl_unlock_restriction_temporarily() == 0);
    CHECK(pctl_play_timer_set_days(bad) == NXM_RC_INVALID_ARGUMENT);
    sim_env_set("PLAYGUARD_SIM_FAIL", "write,bedtime");
    CHECK(pctl_play_timer_set_days(days) == SIM_FAIL_RC);
    CHECK(pctl_delete_pairing() == SIM_FAIL_RC);
    sim_env_set("PLAYGUARD_SIM_FAIL", "bedtime");
    PtBedtime bed[7];
    for (auto& b : bed) b = PtBedtime{ true, 20, 30, 7, 0 };
    CHECK(pctl_play_timer_set_bedtime(bed, 3) == NXM_RC_NOT_APPLIED);
    CHECK(query().bedtime_hour == 21);
    sim_env_set("PLAYGUARD_SIM_FAIL", nullptr);
    CHECK(pctl_play_timer_set_bedtime(bed, 3) == 0);
    CHECK(pctl_play_timer_set_days(days) == 0);
    CHECK(pctl_relock() == 0);
    PtState pt = query();
    CHECK(pt.day_min[0] == 60 && pt.bedtime_hour == 20 && pt.bedtime_minute == 30);

    // The read-only switch and the change check come first.
    core_set_read_only(true);
    CHECK(pctl_set_rating_org(1) == NXM_RC_READ_ONLY);
    CHECK(gamecheck_remove(0x0100A1B2C3D48000ULL) == NXM_RC_READ_ONLY);
    core_set_read_only(false);
    core_set_change_check(confirm);
    s_confirm = false;
    CHECK(pctl_set_rating_org(1) == NXM_RC_NOT_CONFIRMED);
    s_confirm = true;
    CHECK(pctl_set_rating_org(1) == 0);
    core_set_change_check(nullptr);
    PctlStatus st;
    pctl_status_fetch(&st);
    CHECK(st.rating_org == 1 && st.pin_length == 4);

    // Custom settings only at the custom level.
    const PctlCustomSettings custom = { 16, false, false };
    CHECK(pctl_set_custom_settings(&custom) == NXM_RC_NOT_CUSTOM);
    CHECK(pctl_set_safety_level(PctlSafetyLevel_Custom) == 0);
    CHECK(pctl_set_custom_settings(&custom) == 0);
    PctlCustomSettings read;
    CHECK(pctl_get_level_settings(PctlSafetyLevel_Custom, &read) == 0 && read.rating_age == 16);
}

// ---------------------------------------------------------------- the games

static void test_games()
{
    // The Activity tab: the played games of the table, in its order.
    PlayStats* stats = new PlayStats;
    playstats_fetch(stats);
    size_t played = 0;
    for (size_t i = 0; i < SIM_GAME_COUNT; i++) played += SIM_GAMES[i].played;
    CHECK(stats->count == played && played == 7);
    CHECK(std::strcmp(stats->games[0].name, "Star Kart Racers") == 0);
    u64 today = 0;
    for (u32 i = 0; i < stats->count; i++) today += stats->games[i].today_s;
    CHECK(today == 80 * 60);

    // Two accounts share a game 2:1, less the time on no account.
    AccountPlay accounts[8];
    Result rc = 1;
    CHECK(playstats_by_account(0x0100A1B2C3D40000ULL, accounts, 8, &rc) == 2 && rc == 0);
    CHECK(accounts[0].total_s + accounts[1].total_s == (3650 - 50) * 60);
    CHECK(accounts[0].total_s == 2 * accounts[1].total_s);
    delete stats;

    // Icons: from the table, none for a deleted game.
    PlayIcon icons[2] = {};
    icons[0].app_id = 0x0100A1B2C3D40000ULL;
    icons[1].app_id = 0x0100A1B2C3D45000ULL;
    playstats_icons(icons, 2);
    CHECK(icons[0].jpeg && icons[0].size > 0 && !icons[1].jpeg && icons[1].size == 0);
    std::free(icons[0].jpeg);

    // The installed games: every installed one of the table, then a broken
    // one removed is gone, and PLAYGUARD_SIM_GAMES_OK leaves the fine ones.
    size_t installed = 0;
    for (size_t i = 0; i < SIM_GAME_COUNT; i++) installed += SIM_GAMES[i].installed;
    std::vector<GameRecord> records(GAMECHECK_MAX);
    u32 count = 0;
    bool truncated = true;
    CHECK(gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated) == 0 && !truncated);
    CHECK(count == installed && count == 15);
    CHECK(gamecheck_remove(0x0100A1B2C3D48000ULL) == 0);
    CHECK(gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated) == 0 && count == installed - 1);
    sim_env_set("PLAYGUARD_SIM_GAMES_OK", "1");
    CHECK(gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated) == 0 && count == 6);
    sim_env_set("PLAYGUARD_SIM_GAMES_OK", nullptr);

    // Checked game by game: the facts of the table's games.
    CHECK(gamecheck_records(records.data(), GAMECHECK_MAX, &count, &truncated) == 0);
    sim_env_set("PLAYGUARD_SIM_CHECK_MS", "0");
    std::vector<GameFacts> facts(count);
    CHECK(gamecheck_check(records.data(), count, facts.data(), nullptr, nullptr) == count);
    bool no_name = false, corrupt_flag = false;
    for (const GameFacts& g : facts) {
        if (g.app_id == 0x0100A1B2C3D46000ULL) no_name = !g.control_ok && !g.name[0];
        if (g.app_id == 0x0100A1B2C3D4E000ULL) corrupt_flag = g.view_flags & GC_VIEW_NEEDS_VERIFY;
    }
    CHECK(no_name && corrupt_flag);
    sim_env_set("PLAYGUARD_SIM_CHECK_MS", nullptr);
    // A check stopped at once.
    bool corrupt = true;
    Result found = 1;
    CHECK(gamecheck_verify(0x0100A1B2C3D4E000ULL, [](void*, u64, u64) { return false; }, nullptr, &corrupt, &found) ==
          NXM_RC_CANCELLED);
}

// ---------------------------------------------------------------- threads

static void test_threads()
{
    std::atomic<bool> stop{ false };
    std::vector<std::thread> readers;
    for (int i = 0; i < 3; i++)
        readers.emplace_back([&] {
            while (!stop) {
                PctlStatus st;
                PtState pt;
                pctl_overview_fetch(&st, &pt);
                TimeSnapshot t;
                time_clock_snapshot(&t);
            }
        });
    bool ok = true;
    for (int i = 0; i < 300; i++) {
        ok = ok && pctl_unlock_restriction_temporarily() == 0 && pctl_relock() == 0;
        sim_env_set("PLAYGUARD_SIM_SKIP", i % 2 ? "60" : nullptr);
    }
    stop = true;
    for (auto& t : readers) t.join();
    CHECK(ok);
    CHECK(!sim_state().console.temp_unlocked);
}

int main()
{
    setenv("TZ", "UTC", 1);
    tzset();
    test_flags_and_numbers();
    test_keep();   // before any use of the state in this process
    test_time_left();
    test_changes();
    test_games();
    test_threads();
    return CHECK_DONE("sim_backend: all tests passed");
}
