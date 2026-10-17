// Host tests for source/action/notice_logic.cpp: what the notification bell
// lists, in which order, what it counts, and what putting an entry away or
// ignoring one does;
// and the rules action/notices applies: which kinds Preferences › Notifications
// hides, the key an entry read is remembered by, when the backup, console
// lock, SD card and week reminders are due, and the days over the limit.
#include "check.h"
#include <cstring>

#include "action/notice_logic.hpp"

using namespace notice_logic;

static void test_set_and_order()
{
    Store s;
    CHECK(s.visible().empty());
    CHECK(s.set("x_info", Level::Info, "Settings reset", ""));
    CHECK(s.set("x_todo", Level::Action, "Clock off", "3 min"));
    CHECK(s.set("y_todo", Level::Action, "Other", ""));
    const auto v = s.visible();
    if (!CHECK(v.size() == 3)) return;
    // To do first, each kind in the order it was first set (ids with no rank).
    CHECK(v[0].id == "x_todo");
    CHECK(v[1].id == "y_todo");
    CHECK(v[2].id == "x_info");
    CHECK(s.action_count() == 2);
    CHECK(s.info_count() == 1);
    CHECK(s.console_count() == 0);
}

static void test_groups()
{
    // The console's entries: the firmware, the games, the SD card, PlayGuard's
    // files (the prefixes too); anything else is about the parental controls.
    for (const char* id : { "games", "patches", "sd_space", "sd_config", "data_history", "rescue_old", "compat",
                            "sys_firmware", "serial", "other_system", "went_back" })
        CHECK(group_of(id) == Group::Console);
    for (const char* id : { "pin_missing", "unlocked", "clock", "pin_weak", "week", "upload", "update", "crash", "new" })
        CHECK(group_of(id) == Group::Parental);

    Store s;
    s.set("games", Level::Action, "9 games", "");
    s.set("sys_firmware", Level::Info, "23.0.1", "");
    s.set("week", Level::Info, "8 h", "");
    s.set("clock", Level::Action, "Clock off", "");
    const auto v = s.visible();
    if (!CHECK(v.size() == 4)) return;
    // The parental controls' (to do, then for information), then the console's.
    CHECK(v[0].id == "clock" && v[1].id == "week" && v[2].id == "games" && v[3].id == "sys_firmware");
    // The bell counts the parental controls' to do, console_count the console's.
    CHECK(s.action_count() == 1 && s.info_count() == 1 && s.console_count() == 1);
}

static void test_tabs()
{
    // Each entry where it is fixed: the console's and the network clock on
    // the Console tab, the PIN's on the PIN tab, the alarm on the Play timer.
    for (const char* id : { "games", "sd_space", "data_history", "rescue_old", "clock", "clock_off" })
        CHECK(tab_of(id) == "console");
    for (const char* id : { "pin_missing", "unlocked", "pin_weak", "pin_lock_off" }) CHECK(tab_of(id) == "security");
    for (const char* id : { "alarm_off", "not_counting" }) CHECK(tab_of(id) == "play_timer");
    CHECK(tab_of("outside") == "dashboard" && tab_of("extra_restore") == "dashboard");
    CHECK(tab_of("new") == "");

    // A tab counts its entries to do only: not those for information, put
    // away or ignored.
    Store s;
    s.set("unlocked", Level::Action, "Unlocked", "");
    s.set("pin_weak", Level::Action, "PIN", "");
    s.set("pairing", Level::Info, "App", "");
    s.set("games", Level::Action, "9 games", "");
    s.set("clock", Level::Action, "Clock off", "");
    s.set("sys_firmware", Level::Info, "23.0.1", "");
    CHECK(s.tab_count("security") == 2);
    CHECK(s.tab_count("console") == 2);
    CHECK(s.tab_count("play_timer") == 0);
    CHECK(s.ack("pin_weak"));
    CHECK(s.tab_count("security") == 1);
    s.clear("unlocked");
    CHECK(s.tab_count("security") == 0);
}

static void test_fixed_order()
{
    // The order the sources set them in does not matter: a check that ends
    // sooner or later lists the same way.
    Store a, b;
    a.set("pin_weak", Level::Action, "PIN", "");
    a.set("outside", Level::Action, "Outside", "");
    a.set("clock", Level::Action, "Clock", "");
    b.set("clock", Level::Action, "Clock", "");
    b.set("pin_weak", Level::Action, "PIN", "");
    b.set("outside", Level::Action, "Outside", "");
    const auto va = a.visible(), vb = b.visible();
    if (!CHECK(va.size() == 3 && vb.size() == 3)) return;
    for (size_t i = 0; i < 3; i++) CHECK(va[i].id == vb[i].id);
    CHECK(va[0].id == "outside" && va[1].id == "clock" && va[2].id == "pin_weak");
    // A ranked id before one with no rank.
    Store c;
    c.set("unknown", Level::Action, "U", "");
    c.set("pin_weak", Level::Action, "PIN", "");
    CHECK(c.visible()[0].id == "pin_weak");
}

static void test_replace_keeps_place()
{
    Store s;
    s.set("a", Level::Action, "A", "");
    s.set("b", Level::Action, "B", "");
    // The same text again: nothing changed.
    CHECK(!s.set("a", Level::Action, "A", ""));
    // New text: changed, and it keeps its place before b.
    CHECK(s.set("a", Level::Action, "A", "more"));
    const auto v = s.visible();
    if (!CHECK(v.size() == 2)) return;
    CHECK(v[0].id == "a" && v[0].detail == "more");
    // Moving to the other kind moves it to that kind's group.
    CHECK(s.set("a", Level::Info, "A", "more"));
    CHECK(s.visible()[0].id == "b");
    CHECK(s.action_count() == 1);
}

static void test_clear()
{
    Store s;
    CHECK(!s.clear("none"));
    s.set("a", Level::Action, "A", "");
    CHECK(s.has("a"));
    CHECK(s.clear("a"));
    CHECK(!s.has("a"));
    CHECK(s.action_count() == 0);
    CHECK(!s.clear("a"));
}

static void test_dismiss()
{
    Store s;
    s.set("todo", Level::Action, "T", "");
    s.set("info", Level::Info, "I", "x");
    // To do: only fixing it clears it.
    CHECK(!s.dismiss("todo"));
    CHECK(s.has("todo"));
    CHECK(!s.dismiss("unknown"));
    CHECK(s.dismiss("info"));
    CHECK(!s.dismiss("info"));   // already away
    CHECK(!s.has("info"));
    CHECK(s.info_count() == 0);
    CHECK(s.visible().size() == 1);
    // Set again with the same text (every refresh does): stays away.
    CHECK(!s.set("info", Level::Info, "I", "x"));
    CHECK(!s.has("info"));
    // New text: back.
    CHECK(s.set("info", Level::Info, "I", "y"));
    CHECK(s.has("info"));
    // Cleared while away: not shown, so nothing changes on the bell; set
    // again after that, it is new.
    CHECK(s.dismiss("info"));
    CHECK(!s.clear("info"));
    CHECK(s.set("info", Level::Info, "I", "y"));
    CHECK(s.has("info"));
}

static void test_dismissed_moved_to_action()
{
    // Put away as information, then its source says it needs doing: shown.
    Store s;
    s.set("x", Level::Info, "X", "");
    CHECK(s.dismiss("x"));
    CHECK(s.set("x", Level::Action, "X", ""));
    CHECK(s.has("x"));
    CHECK(s.action_count() == 1);
}

static void test_ack()
{
    Store s;
    s.set("pin_weak", Level::Action, "PIN", "");
    s.set("unlocked", Level::Action, "Unlocked", "");
    s.set("sd_space", Level::Action, "SD", "120 MB");
    s.set("week", Level::Info, "8 h", "");
    CHECK(s.action_count() == 2 && s.console_count() == 1);
    // Only the entries to do the parent may ignore.
    CHECK(!s.ack("unlocked"));
    CHECK(!s.ack("week"));
    CHECK(!s.ack("unknown"));
    CHECK(s.ack("pin_weak"));
    CHECK(!s.ack("pin_weak"));   // already ignored
    CHECK(s.acked("pin_weak") && !s.has("pin_weak"));
    // No longer counted on the bell, listed apart.
    CHECK(s.action_count() == 1);
    const auto v = s.visible();
    if (!CHECK(v.size() == 3)) return;
    CHECK(v[0].id == "unlocked" && v[1].id == "week" && v[2].id == "sd_space");
    const auto ign = s.ignored();
    CHECK(ign.size() == 1 && ign[0].id == "pin_weak");
    // A new text (the free space read again) keeps it ignored.
    CHECK(s.ack("sd_space"));
    CHECK(s.console_count() == 0);
    s.set("sd_space", Level::Action, "SD", "90 MB");
    CHECK(s.acked("sd_space") && s.console_count() == 0);
    // Counted again.
    CHECK(s.unack("sd_space"));
    CHECK(!s.unack("sd_space"));
    CHECK(s.console_count() == 1);
    // Cleared while ignored: not shown, nothing changes on the bell; set
    // again, it is new and counted.
    CHECK(!s.clear("pin_weak"));
    CHECK(s.ignored().empty());
    s.set("pin_weak", Level::Action, "PIN", "");
    CHECK(s.has("pin_weak") && s.action_count() == 2);
}

static void test_ack_kind_changes()
{
    // Ignored as something to do, then only for information, then to do
    // again: asked again.
    Store s;
    s.set("pin_weak", Level::Action, "PIN", "");
    CHECK(s.ack("pin_weak"));
    CHECK(s.set("pin_weak", Level::Info, "PIN", ""));
    CHECK(!s.acked("pin_weak") && s.has("pin_weak"));
    CHECK(!s.ack("pin_weak"));   // information: put away, not ignored
    CHECK(s.set("pin_weak", Level::Action, "PIN", ""));
    CHECK(s.has("pin_weak") && s.action_count() == 1);
}

static void test_ackable()
{
    for (const char* id : { "pin_weak", "pin_lock_off", "alarm_off", "sd_space" }) CHECK(ackable(id));
    for (const char* id : { "pin_missing", "unlocked", "outside", "not_counting", "clock", "extra_restore",
                            "data_history", "sd_config", "sd_history", "rescue_old", "rescue_restart", "games",
                            "patches", "week", "pin", "pin_weak_" })
        CHECK(!ackable(id));
}

static void test_ack_keys()
{
    CHECK(ack_key("pin_weak", "4", "2026-10-17") == "pin_weak:4:2026-10-17");
    CHECK(ack_key("alarm_off", "", "2026-10-17") == "alarm_off::2026-10-17");
    std::vector<std::string> keys = { "pin_weak:4:2026-10-17", "alarm_off::2026-09-01", "sd_space::bad-date",
                                      "pin_lock_off::2026-1-1" };
    CHECK(ack_day(keys, "pin_weak", "4") == "2026-10-17");
    CHECK(ack_day(keys, "pin_weak", "6") == "");   // a new PIN, of another length: asked again
    CHECK(ack_day(keys, "pin_weak", "") == "");
    CHECK(ack_day(keys, "alarm_off", "") == "2026-09-01");
    CHECK(ack_day(keys, "sd_space", "") == "");     // not a date
    CHECK(ack_day(keys, "pin_lock_off", "") == "");
    CHECK(ack_day(keys, "pin", "") == "" && ack_day(keys, "pin", "weak:4") == "");
    CHECK(ack_day({}, "pin_weak", "4") == "");

    // For 30 days with the reminder, for good without.
    CHECK(ack_holds(keys, "pin_weak", "4", "2026-10-17", true));
    CHECK(ack_holds(keys, "pin_weak", "4", "2026-11-15", true));
    CHECK(!ack_holds(keys, "pin_weak", "4", "2026-11-16", true));
    CHECK(ack_holds(keys, "pin_weak", "4", "2030-01-01", false));
    CHECK(!ack_holds(keys, "pin_weak", "6", "2026-10-17", false));
    CHECK(!ack_holds(keys, "alarm_off", "", "2026-10-17", true));
    CHECK(ack_until(keys, "pin_weak", "4", true) == "2026-11-16");
    CHECK(ack_until(keys, "pin_weak", "4", false) == "");
    CHECK(ack_until(keys, "pin_weak", "6", true) == "");

    // Forgotten: only that id's, whatever it was about.
    CHECK(ack_forget(keys, "pin_weak"));
    CHECK(!ack_forget(keys, "pin_weak"));
    CHECK(!ack_forget(keys, "pin"));
    CHECK(keys.size() == 3 && ack_day(keys, "alarm_off", "") == "2026-09-01");
}

static void test_order_after_clear()
{
    // Cleared and set again: last of its kind, as a new entry.
    Store s;
    s.set("a", Level::Action, "A", "");
    s.set("b", Level::Action, "B", "");
    s.clear("a");
    s.set("a", Level::Action, "A", "");
    const auto v = s.visible();
    if (!CHECK(v.size() == 2)) return;
    CHECK(v[0].id == "b");
    CHECK(v[1].id == "a");
}

static void test_shown_kind()
{
    Shown all;
    for (const char* id : { "sys_firmware", "week", "over_limit", "backup", "other_system", "console_lock_long",
                            "pin_missing", "sd_space", "crash" })
        CHECK(shown_kind(id, all));
    Shown none;
    none.system = none.summary = none.reminders = false;
    CHECK(!shown_kind("sys_firmware", none) && !shown_kind("sys_hekate", none));
    CHECK(!shown_kind("week", none) && !shown_kind("over_limit", none));
    CHECK(!shown_kind("backup", none) && !shown_kind("other_system", none) && !shown_kind("console_lock_long", none));
    // Everything else always shows.
    for (const char* id : { "pin_missing", "unlocked", "sd_space", "crash", "system", "weekly" })
        CHECK(shown_kind(id, none));
    // Each switch only hides its own kind.
    Shown no_summary;
    no_summary.summary = false;
    CHECK(!shown_kind("week", no_summary) && shown_kind("sys_firmware", no_summary) && shown_kind("backup", no_summary));
}

static void test_read_key()
{
    // FNV-1a over the title then the detail, as 8 hex digits after the id.
    CHECK(read_key("week", "", "") == "week:811c9dc5");
    CHECK(read_key("x", "a", "") == "x:e40c292c");
    CHECK(read_key("x", "a", "") == read_key("x", "", "a"));   // one stream: where it splits does not count
    CHECK(read_key("x", "Title", "Detail") != read_key("x", "Title", "Detail!"));
    CHECK(read_key("x", "Title", "Detail").rfind("x:", 0) == 0 && read_key("x", "Title", "Detail").size() == 10);
}

static void test_dates()
{
    CHECK(backup_date("20261008_142530.json") == "2026-10-08");
    CHECK(backup_date("20261008") == "2026-10-08");
    CHECK(backup_date("2026100") == "" && backup_date("2026-10-08.json") == "" && backup_date("") == "");

    CHECK(days_ago("2026-10-01", 7, "2026-10-08") && days_ago("2026-10-01", 7, "2026-10-09"));
    CHECK(!days_ago("2026-10-01", 7, "2026-10-07"));
    CHECK(days_ago("2026-09-30", 30, "2026-10-30") && !days_ago("2026-09-30", 30, "2026-10-29"));
    CHECK(!days_ago("", 7, "2026-10-08") && !days_ago("junk", 7, "2026-10-08"));

    // No backup at all: due. The newest 30 days old: due; younger, or a
    // name that is no date: not.
    const std::string today = "2026-10-17";
    CHECK(backup_due({}, today));
    CHECK(backup_due({ "20260917_120000.json", "20260101_000000.json" }, today));
    CHECK(!backup_due({ "20260918_120000.json", "20260101_000000.json" }, today));
    CHECK(!backup_due({ "manual.json" }, today));

    CHECK(console_lock_long(true, "2026-10-10", today) && !console_lock_long(true, "2026-10-11", today));
    CHECK(!console_lock_long(false, "2026-01-01", today) && !console_lock_long(true, "", today));

    // The week: some play time, never read or read 7 days ago or more.
    CHECK(week_due(true, 60, "", today) && week_due(true, 60, "2026-10-10", today));
    CHECK(!week_due(true, 60, "2026-10-11", today));
    CHECK(!week_due(true, 0, "", today) && !week_due(false, 60, "", today));
}

static void test_sd_space()
{
    CHECK(sd_space_low(true, 0) && sd_space_low(true, (500ULL << 20) - 1));
    CHECK(!sd_space_low(true, 500ULL << 20) && !sd_space_low(false, 0));
}

static void test_over_limit()
{
    static PlayStats stats;
    std::memset(&stats, 0, sizeof(stats));
    PtState pt;
    std::memset(&pt, 0, sizeof(pt));
    pt.valid = true;
    pt.alarm_only = true;
    for (int d = 0; d < 7; d++) pt.day_min[d] = 60;
    pt.day_min[0] = PT_DAY_NOLIMIT;   // Sunday: no limit
    // Today is a Sunday: day k is (7 - k) % 7.
    for (int k = 0; k < 7; k++) stats.day_wday[k] = (uint8_t)((7 - k) % 7);
    stats.count = 2;
    // Two games, played 40 min each on day 1 (Saturday): 80 min over 60.
    stats.games[0].day_s[1] = 40 * 60;
    stats.games[1].day_s[1] = 40 * 60;
    // Exactly the limit on day 2: not over.
    stats.games[0].day_s[2] = 60 * 60;
    // Way over on Sunday (day 0): no limit that day.
    stats.games[0].day_s[0] = 10 * 3600;
    CHECK(over_limit_days(pt, stats) == 1);
    stats.games[1].day_s[3] = 60 * 60 + 1;
    CHECK(over_limit_days(pt, stats) == 2);
    // The console suspends the game at the limit, or the limits are unread: none.
    pt.alarm_only = false;
    CHECK(over_limit_days(pt, stats) == 0);
    pt.alarm_only = true;
    pt.valid = false;
    CHECK(over_limit_days(pt, stats) == 0);
}

static void test_every()
{
    Every e(60000);
    CHECK(e.due(0));                 // never taken: due at once
    e.taken(1000);
    CHECK(!e.due(1000));
    CHECK(!e.due(6000));             // the Overview's next refresh: kept
    CHECK(!e.due(60999));
    CHECK(e.due(61000));             // a minute later: due
    e.taken(61000);
    CHECK(!e.due(65000));
    e.forget();                      // a backup written: due at once
    CHECK(e.due(65000));
    e.taken(65000);
    CHECK(e.due(64000));             // a clock that went back: not trusted, due
}

int main()
{
    test_every();
    test_set_and_order();
    test_groups();
    test_tabs();
    test_fixed_order();
    test_replace_keeps_place();
    test_clear();
    test_dismiss();
    test_dismissed_moved_to_action();
    test_ack();
    test_ack_kind_changes();
    test_ackable();
    test_ack_keys();
    test_order_after_clear();
    test_shown_kind();
    test_read_key();
    test_dates();
    test_sd_space();
    test_over_limit();
    return CHECK_DONE("notice_logic tests passed");
}
