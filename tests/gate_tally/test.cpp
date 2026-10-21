// Host tests for source/util/gate_tally.cpp: each result counted where it
// belongs, the file's round trip (a damaged one, a bad day or path, dropped),
// four weeks kept, the seven days by forwarder, the refusals climbing.
#include "check.h"

#include "util/gate_tally.hpp"

using gate_tally::Count;
using gate_tally::Tally;

static void test_add()
{
    Tally t;
    gate_tally::add(t, "2026-10-20", "sdmc:/switch/sphaira/sphaira.nro", GateResult_Pin);
    gate_tally::add(t, "2026-10-20", "sdmc:/switch/sphaira/sphaira.nro", GateResult_Free);
    gate_tally::add(t, "2026-10-20", "sdmc:/switch/sphaira/sphaira.nro", GateResult_Refused);
    for (GateResult r : { GateResult_Paused, GateResult_Closed, GateResult_Limit, GateResult_Error })
        gate_tally::add(t, "2026-10-20", "sdmc:/switch/sphaira/sphaira.nro", r);
    const Count c = t["2026-10-20"]["sdmc:/switch/sphaira/sphaira.nro"];
    CHECK(c.opened == 2 && c.refused == 1 && c.blocked == 4);
    // Not a day, not a path on the SD card: nothing.
    gate_tally::add(t, "?", "sdmc:/x.nro", GateResult_Pin);
    gate_tally::add(t, "2026-02-30", "sdmc:/x.nro", GateResult_Pin);
    gate_tally::add(t, "2026-10-20", "/x.nro", GateResult_Pin);
    CHECK(t.size() == 1 && t["2026-10-20"].size() == 1);
}

static void test_file()
{
    Tally t;
    gate_tally::add(t, "2026-10-19", "sdmc:/a.nro", GateResult_Refused);
    gate_tally::add(t, "2026-10-20", "sdmc:/b.nro", GateResult_Pin);
    CHECK(gate_tally::parse(gate_tally::text(t)) == t);
    CHECK(gate_tally::parse("{not json").empty() && gate_tally::parse("[]").empty() && gate_tally::parse("{}").empty());
    const Tally odd = gate_tally::parse(R"({"days":{"2026-10-20":{"sdmc:/a.nro":{"opened":2,"refused":-3},
        "/b.nro":{"opened":1},"sdmc:/c.nro":{"opened":0}},"yesterday":{"sdmc:/a.nro":{"opened":1}}}})");
    CHECK(odd.size() == 1 && odd.at("2026-10-20").size() == 1 && odd.at("2026-10-20").at("sdmc:/a.nro").opened == 2 &&
          odd.at("2026-10-20").at("sdmc:/a.nro").refused == 0);
}

static void test_trim()
{
    Tally t;
    for (const char* d : { "2026-09-22", "2026-09-23", "2026-10-20", "2026-10-21", "2026-10-22" })
        gate_tally::add(t, d, "sdmc:/a.nro", GateResult_Pin);
    gate_tally::trim(t, "2026-10-21");
    // Four weeks to the 21st: from the 24th of September; none after today.
    CHECK(t.size() == 2 && t.count("2026-10-20") && t.count("2026-10-21"));
    gate_tally::add(t, "2026-09-24", "sdmc:/a.nro", GateResult_Pin);
    gate_tally::trim(t, "2026-10-21");
    CHECK(t.count("2026-09-24") == 1);
}

static void test_week()
{
    Tally t;
    // sphaira refused 4 times over the week, Star Atlas once; one nine days ago left out.
    for (const char* d : { "2026-10-15", "2026-10-19", "2026-10-21", "2026-10-21" })
        gate_tally::add(t, d, "sdmc:/sphaira.nro", GateResult_Refused);
    gate_tally::add(t, "2026-10-21", "sdmc:/sphaira.nro", GateResult_Pin);
    gate_tally::add(t, "2026-10-20", "sdmc:/atlas.nro", GateResult_Refused);
    gate_tally::add(t, "2026-10-12", "sdmc:/old.nro", GateResult_Refused);
    const std::vector<gate_tally::Week> w = gate_tally::week(t, "2026-10-21");
    CHECK(w.size() == 2 && w[0].nro == "sdmc:/sphaira.nro" && w[0].total.refused == 4 && w[0].total.opened == 1);
    CHECK(w[0].days[0].refused == 1 && w[0].days[4].refused == 1 && w[0].days[6].refused == 2 && w[0].days[6].opened == 1);
    CHECK(w[1].nro == "sdmc:/atlas.nro" && w[1].days[5].refused == 1);
    int now = 0, before = 0;
    gate_tally::refusals(t, "2026-10-21", &now, &before);
    CHECK(now == 5 && before == 1);
    CHECK(gate_tally::rising(5, 2) && !gate_tally::rising(5, 3) && !gate_tally::rising(4, 0) && gate_tally::rising(9, 4));
    CHECK(gate_tally::week(Tally(), "2026-10-21").empty());
}

int main()
{
    test_add();
    test_file();
    test_trim();
    test_week();
    return CHECK_DONE("gate_tally counts, file, four weeks, the week and the climb assertions passed");
}
