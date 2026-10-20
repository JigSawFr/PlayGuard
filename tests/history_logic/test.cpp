// Host tests for source/action/history_logic.cpp: what an undoable entry's
// dialog offers given the console's value now, what the undo records, the
// days a limits change touched, the relative day of an entry, and the kinds
// and sources the History screen names (tools/check_resources.py checks each
// has its name in the catalog).
#include "check.h"
#include <cstdio>
#include <vector>

#include "action/history_logic.hpp"

using history_logic::Undo;

static history::Entry entry(const char* kind, std::vector<int> before, std::vector<int> after)
{
    history::Entry e;
    e.when = "2026-10-08 18:30";
    e.kind = kind;
    e.before = std::move(before);
    e.after = std::move(after);
    return e;
}

static void test_action()
{
    const history::Entry level = entry("level", { 1 }, { 3 });
    CHECK(history_logic::undo_action(level, { 3 }, false, false) == Undo::Offer);
    CHECK(history_logic::undo_action(level, { 1 }, false, false) == Undo::AlreadyBack);
    // The console's value unread: still offered.
    CHECK(history_logic::undo_action(level, {}, false, false) == Undo::Offer);
    // Read-only: nothing to offer, but "already back" says more and comes first.
    CHECK(history_logic::undo_action(level, { 3 }, true, false) == Undo::ReadOnly);
    CHECK(history_logic::undo_action(level, {}, true, false) == Undo::ReadOnly);
    CHECK(history_logic::undo_action(level, { 1 }, true, false) == Undo::AlreadyBack);

    // The alarm only in advanced mode; read-only is said first.
    const history::Entry alarm = entry("alarm", { 0 }, { 1 });
    CHECK(history_logic::undo_action(alarm, { 1 }, false, false) == Undo::NeedsAdvanced);
    CHECK(history_logic::undo_action(alarm, { 1 }, false, true) == Undo::Offer);
    CHECK(history_logic::undo_action(alarm, { 1 }, true, false) == Undo::ReadOnly);
    CHECK(history_logic::undo_action(alarm, { 0 }, false, false) == Undo::AlreadyBack);

    const history::Entry limits = entry("limits", std::vector<int>(7, 120), std::vector<int>(7, 60));
    CHECK(history_logic::undo_action(limits, std::vector<int>(7, 60), false, false) == Undo::Offer);
    CHECK(history_logic::undo_action(limits, std::vector<int>(7, 120), false, false) == Undo::AlreadyBack);
    std::vector<int> other(7, 120);
    other[3] = 90;                                         // one day differs: not back
    CHECK(history_logic::undo_action(limits, other, false, false) == Undo::Offer);
}

static void test_changed_since()
{
    const history::Entry level = entry("level", { 1 }, { 3 });
    CHECK(!history_logic::changed_since(level, { 3 }));   // still the entry's value
    CHECK(history_logic::changed_since(level, { 2 }));    // changed again since
    CHECK(!history_logic::changed_since(level, {}));      // unread: nothing said
}

static void test_replaces()
{
    const history::Entry custom = entry("custom", { 12, 1, 0 }, { 16, 0, 0 });
    // The undo records the value it replaced: the console's when it was read.
    CHECK((history_logic::undo_replaces(custom, { 18, 1, 1 }) == std::vector<int>{ 18, 1, 1 }));
    CHECK((history_logic::undo_replaces(custom, {}) == std::vector<int>{ 16, 0, 0 }));
}

static void test_changed_days()
{
    std::vector<int> before(7, 195), after(7, 195);
    after[3] = 180;   // Wednesday only
    CHECK((history_logic::changed_days(entry("limits", before, after)) == std::vector<int>{ 3 }));
    after[0] = 0;
    CHECK((history_logic::changed_days(entry("outside_limits", before, after)) == std::vector<int>{ 0, 3 }));
    CHECK(history_logic::changed_days(entry("limits", before, before)).empty());
    // Not a week of limits: none.
    CHECK(history_logic::changed_days(entry("level", { 1 }, { 3 })).empty());
    CHECK(history_logic::changed_days(entry("limits", { 1, 2 }, after)).empty());
}

static void test_day_of()
{
    using history_logic::Day;
    CHECK(history_logic::day_of("2026-10-14 21:29", "2026-10-14", "2026-10-13") == Day::Today);
    CHECK(history_logic::day_of("2026-10-13 01:25", "2026-10-14", "2026-10-13") == Day::Yesterday);
    CHECK(history_logic::day_of("2026-10-12 02:10", "2026-10-14", "2026-10-13") == Day::Other);
    // An unknown date (the clock unread) or a damaged stamp: no relative day.
    CHECK(history_logic::day_of("2026-10-14 21:29", "", "") == Day::Other);
    CHECK(history_logic::day_of("", "2026-10-14", "2026-10-13") == Day::Other);
    CHECK(history_logic::day_of("2026-10", "2026-10-14", "2026-10-13") == Day::Other);
}

static void test_categories()
{
    using history_logic::Category;
    using history_logic::Filter;
    CHECK(history_logic::category(entry("limits", {}, {})) == Category::PlayTime);
    CHECK(history_logic::category(entry("countdown", { 1 }, { 0 })) == Category::PlayTime);
    CHECK(history_logic::category(entry("bedtime", {}, {})) == Category::PlayTime);
    CHECK(history_logic::category(entry("console_lock", { 0 }, { 1 })) == Category::PlayTime);
    CHECK(history_logic::category(entry("custom", {}, {})) == Category::Restrictions);
    CHECK(history_logic::category(entry("vr", {}, {})) == Category::Restrictions);
    CHECK(history_logic::category(entry("unlink", {}, {})) == Category::Security);
    CHECK(history_logic::category(entry("pin_shown", {}, {})) == Category::Security);
    CHECK(history_logic::category(entry("outside_clock", {}, {})) == Category::Outside);
    CHECK(history_logic::category(entry("new_account", {}, {})) == Category::Outside);
    CHECK(history_logic::category(entry("clock", {}, {})) == Category::Other);
    CHECK(history_logic::category(entry("from_a_newer_version", {}, {})) == Category::Other);

    history::Entry refused = entry("level", { 1 }, { 3 });
    refused.result = "0x0001088E";
    const history::Entry done = entry("level", { 1 }, { 3 });
    CHECK(history_logic::matches(done, Filter::All) && history_logic::matches(refused, Filter::All));
    CHECK(history_logic::matches(refused, Filter::Restrictions) && history_logic::matches(refused, Filter::Failed));
    CHECK(!history_logic::matches(done, Filter::Failed) && !history_logic::matches(done, Filter::PlayTime));
    CHECK(history_logic::matches(entry("relock", {}, {}), Filter::Security));
    CHECK(history_logic::matches(entry("outside_limits", {}, {}), Filter::Outside));
    // "Everything" is the only filter that shows a kind of no category.
    for (int f = 1; f < (int)Filter::Count; f++) CHECK(!history_logic::matches(entry("clock", {}, {}), (Filter)f));
    CHECK(!history_logic::matches(done, Filter::Count));
}

static void test_days()
{
    CHECK(history_logic::day_key(entry("pin", {}, {})) == "2026-10-08");
    history::Entry odd = entry("pin", {}, {});
    odd.when = "?";
    CHECK(history_logic::day_key(odd).empty());
    odd.when = "2026/10/08 18:30";
    CHECK(history_logic::day_key(odd).empty());

    CHECK(history_logic::weekday_of("2026-10-08") == 4);   // a Thursday
    CHECK(history_logic::weekday_of("2026-10-11") == 0);   // a Sunday
    CHECK(history_logic::weekday_of("2024-02-29") == 4);   // a leap day
    CHECK(history_logic::weekday_of("2000-01-01") == 6);
    CHECK(history_logic::weekday_of("2026-13-01") == -1 && history_logic::weekday_of("2026-1-01") == -1);
    CHECK(history_logic::weekday_of("") == -1 && history_logic::weekday_of("abcd-ef-gh") == -1);
}

static void test_known()
{
    for (const char* k : { "limits", "level", "pin", "unlock", "relock", "console_lock", "bedtime", "time_up",
                           "countdown", "remove_game", "launch_version", "outside_limits", "new_account",
                           "protect_forwarder", "unprotect_forwarder", "forwarder_gone", "lock_hbmenu",
                           "unlock_hbmenu", "hbmenu_restored", "hbmenu_lock_removed", "gate_open",
                           "gate_free", "gate_refused", "gate_hours" })
        CHECK(history_logic::known_kind(k));
    // Written by a newer version, or nothing: unknown.
    CHECK(!history_logic::known_kind("") && !history_logic::known_kind("teleport") && !history_logic::known_kind("Limits"));
    CHECK(history_logic::known_kinds().size() == 41);
    for (const std::string& k : history_logic::known_kinds()) CHECK(history_logic::known_kind(k));

    for (const char* s : { "uniform", "per_day", "extra", "stop", "undo", "backup", "overview", "console_lock" })
        CHECK(history_logic::known_source(s));
    // No longer written, still in older histories: still named.
    CHECK(history_logic::known_source("day"));
    // "profile" is named apart (with the profile's name), others not at all.
    CHECK(!history_logic::known_source("profile") && !history_logic::known_source("") &&
          !history_logic::known_source("telepathy"));
    for (const std::string& s : history_logic::known_sources()) CHECK(history_logic::known_source(s));
}

int main()
{
    test_action();
    test_changed_since();
    test_replaces();
    test_changed_days();
    test_day_of();
    test_categories();
    test_days();
    test_known();
    return CHECK_DONE("history_logic undo offer, changed-since, recorded-value, changed-day, day and known kind and source assertions passed");
}
