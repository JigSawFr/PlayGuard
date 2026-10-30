// Host tests for source/action/forwarder_rules_logic.cpp: a protected
// forwarder's free and closed hours as remembered, the closed hours of the
// forwarders made before they carried their rules, the gate's log folded
// (one entry per kind, forwarder and day; names; refusals; ways back seen
// working; the week's tally) and the times and days the lines show.
#include "check.h"
#include <string>
#include <vector>

#include "action/forwarder_rules_logic.hpp"

using namespace forwarder_rules_logic;

static forwarders::Made made(const char* name, const char* nro, bool pin)
{
    forwarders::Made m;
    m.name = name;
    m.nro = nro;
    m.pin = pin;
    return m;
}

static void test_hours()
{
    forwarders::Made m;
    set_free(m, 0x3E, 540, 1200, 0x41, 600, 1260);
    CHECK(m.free_days == 0x3E && m.free_from == 540 && m.free_to == 1200);
    CHECK(m.free2_days == 0x41 && m.free2_from == 600 && m.free2_to == 1260);
    // No second span: its hours dropped; no first span: nothing at all.
    set_free(m, 0x3E, 540, 1200, 0, 600, 1260);
    CHECK(m.free_days == 0x3E && m.free2_days == 0 && m.free2_from == 0 && m.free2_to == 0);
    set_free(m, 0, 540, 1200, 0x41, 600, 1260);
    CHECK(m.free_days == 0 && m.free_from == 0 && m.free_to == 0 && m.free2_days == 0 && m.free2_from == 0);

    set_closed(m, 0x7F, 1320, 420);
    CHECK(m.closed_days == 0x7F && m.closed_from == 1320 && m.closed_to == 420);
    set_closed(m, 0, 1320, 420);
    CHECK(m.closed_days == 0 && m.closed_from == 0 && m.closed_to == 0);
}

static void test_closed_rules()
{
    std::vector<forwarders::Made> list = { made("Old", "sdmc:/switch/old.nro", true),
                                           made("New", "sdmc:/switch/new.nro", true),
                                           made("Open", "sdmc:/switch/open.nro", false),
                                           made("None", "sdmc:/switch/none.nro", true) };
    for (auto& m : list) set_closed(m, 0x41, 1320, 420);
    list[1].carried = true;    // carries its rules: not in the file
    list[3].closed_days = 0;   // no closed hours
    CHECK(closed_rules(list) == "41\t1320\t420\tsdmc:/switch/old.nro\n");
    list[0].carried = true;
    CHECK(closed_rules(list).empty());
    CHECK(closed_rules({}).empty());
}

static void test_fold_log()
{
    std::vector<forwarders::Made> list = { made("Menu", "sdmc:/hbmenu.nro", true), made("Files", "sdmc:/switch/files.nro", true) };
    list[1].verified = true;
    const std::string text = "2026-10-20 21:03\trefused\tsdmc:/hbmenu.nro\n"
                             "garbage\n"
                             "2026-10-20 21:04\trefused\tsdmc:/hbmenu.nro\n"
                             "2026-10-20 21:05\tpin\tsdmc:/hbmenu.nro\n"
                             "2026-10-21 08:00\trefused\tsdmc:/hbmenu.nro\n"
                             "2026-10-21 08:01\tfree\tsdmc:/switch/files.nro\n"
                             "?\terror\tsdmc:/switch/other/ftpd.nro\n"
                             "?\terror\tsdmc:/switch/other/ftpd.nro\n"   // no time: never joined
                             "2026-10-21 09:00\tlater\tsdmc:/switch/files.nro";   // a newer gate's result
    gate_tally::Tally tally;
    const Log log = fold_log(text, list, tally);
    if (!CHECK(log.seen.size() == 7)) return;
    CHECK(std::string(log.seen[0].kind) == "gate_refused" && log.seen[0].when == "2026-10-20 21:03" && log.seen[0].detail == "Menu");
    CHECK((log.seen[0].times == std::vector<std::string>{ "21:03", "21:04" }));   // the same day: one entry
    CHECK(std::string(log.seen[1].kind) == "gate_open" && log.seen[1].times.size() == 1);
    CHECK(std::string(log.seen[2].kind) == "gate_refused" && log.seen[2].when == "2026-10-21 08:00");   // another day
    CHECK(std::string(log.seen[3].kind) == "gate_free" && log.seen[3].detail == "Files");
    // An .nro PlayGuard did not make: its file name; an error says so.
    CHECK(std::string(log.seen[4].kind) == "gate_refused" && log.seen[4].detail == "ftpd (error)" && log.seen[4].when.empty());
    CHECK(log.seen[5].detail == "ftpd (error)" && log.seen[5].times == std::vector<std::string>{ "" });
    // A result this version does not know: kept, as an "other change".
    CHECK(std::string(log.seen[6].kind) == "gate_other" && log.seen[6].detail == "Files");

    if (!CHECK(log.refused.size() == 3)) return;
    CHECK(log.refused[0].first == "Menu" && log.refused[0].second == "2026-10-20 21:03");
    CHECK(log.refused[2].second == "2026-10-21 08:00");
    // Opened through the gate: Menu (protected, not verified yet); Files was already.
    CHECK(log.seen_working == std::vector<size_t>{ 0 });

    // The tally: by day and forwarder, the tries with no time left out.
    CHECK(tally.size() == 2);
    CHECK(tally["2026-10-20"]["sdmc:/hbmenu.nro"].refused == 2 && tally["2026-10-20"]["sdmc:/hbmenu.nro"].opened == 1);
    CHECK(tally["2026-10-21"]["sdmc:/hbmenu.nro"].refused == 1 && tally["2026-10-21"]["sdmc:/switch/files.nro"].opened == 1);

    // At most LOG_MAX tries at a time.
    std::string many;
    for (int i = 0; i < LOG_MAX + 5; i++) many += "2026-10-20 21:03\trefused\tsdmc:/hbmenu.nro\n";
    gate_tally::Tally t2;
    const Log capped = fold_log(many, list, t2);
    CHECK(capped.refused.size() == (size_t)LOG_MAX && capped.seen.size() == 1);
    CHECK(fold_log("", list, t2).seen.empty());
}

static void test_until_text()
{
    CHECK(until_text("2026-10-21 07:00:12", "2026-10-21") == "07:00");
    CHECK(until_text("2026-10-22 07:00:12", "2026-10-21") == "2026-10-22 07:00");
    CHECK(until_text("2026-10-22 07:00", "2026-10-22") == "07:00");
    CHECK(until_text("?", "2026-10-21") == "?");   // too short: as it is
}

static void test_refused_by_day()
{
    const std::vector<std::string> names = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    gate_tally::Week w;
    // 2026-10-21 is a Wednesday: days[0] the Thursday before.
    w.days[0].refused = 2;
    w.days[5].refused = 1;
    w.days[6].opened = 3;
    CHECK(refused_by_day(w, "2026-10-21", names) == "Thu 2, Tue 1");
    CHECK(refused_by_day(gate_tally::Week(), "2026-10-21", names).empty());
    CHECK(refused_by_day(w, "not a date", names) == "? 2, ? 1");
}

int main()
{
    test_hours();
    test_closed_rules();
    test_fold_log();
    test_until_text();
    test_refused_by_day();
    return CHECK_DONE("forwarder_rules_logic tests passed");
}
