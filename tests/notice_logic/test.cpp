// Host tests for source/action/notice_logic.cpp: what the notification bell
// lists, in which order, what it counts, and what putting an entry away does.
#include "check.h"

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
    // The bell counts the parental controls', the Console tab the console's to do.
    CHECK(s.action_count() == 1 && s.info_count() == 1 && s.console_count() == 1);
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

int main()
{
    test_set_and_order();
    test_groups();
    test_fixed_order();
    test_replace_keeps_place();
    test_clear();
    test_dismiss();
    test_dismissed_moved_to_action();
    test_order_after_clear();
    return CHECK_DONE("notice_logic tests passed");
}
