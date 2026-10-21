// Host tests for source/core/gate.c: the gate's argv (quoted paths, the
// homebrew's own arguments kept, anything else refused), the free hours
// (a span in the day, one past midnight, no day or an empty span never
// free), the rules file (round trip, the right forwarder found, broken lines
// skipped) and the log (round trip, Windows line ends, unknown results
// refused).
#include "check.h"
#include <stdio.h>
#include <string.h>

#include "gate.h"

static void test_argv(void)
{
    char nro[64];
    const char *argv = NULL;
    CHECK(gate_argv_parse("sdmc:/config/playguard/gate.nro --pin-gate sdmc:/switch/cyberfoil.nro", nro, sizeof(nro), &argv));
    CHECK(strcmp(nro, "sdmc:/switch/cyberfoil.nro") == 0 && strcmp(argv, "sdmc:/switch/cyberfoil.nro") == 0);
    CHECK(gate_argv_parse("\"sdmc:/a b/gate.nro\" --pin-gate \"sdmc:/switch/x y.nro\" -v", nro, sizeof(nro), &argv));
    CHECK(strcmp(nro, "sdmc:/switch/x y.nro") == 0 && strcmp(argv, "\"sdmc:/switch/x y.nro\" -v") == 0);
    CHECK(gate_argv_parse("sdmc:/g.nro --pin-gate sdmc:/x.nro --opt 2", nro, sizeof(nro), &argv) &&
          strcmp(nro, "sdmc:/x.nro") == 0 && strcmp(argv, "sdmc:/x.nro --opt 2") == 0);
    // Not a gate's: started from a menu, another flag, no path, not on the SD card, an unclosed quote.
    CHECK(!gate_argv_parse("sdmc:/switch/playguard/playguard.nro", nro, sizeof(nro), &argv) && nro[0] == 0 && !argv);
    CHECK(!gate_argv_parse("", nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse(NULL, nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse("sdmc:/g.nro --other sdmc:/x.nro", nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse("sdmc:/g.nro --pin-gate ", nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse("sdmc:/g.nro --pin-gate /x.nro", nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse("sdmc:/g.nro --pin-gate \"sdmc:/x.nro", nro, sizeof(nro), &argv));
    CHECK(!gate_argv_parse("\"sdmc:/g.nro --pin-gate sdmc:/x.nro", nro, sizeof(nro), &argv));
    char small[8];
    CHECK(!gate_argv_parse("sdmc:/g.nro --pin-gate sdmc:/switch/long.nro", small, sizeof(small), &argv));
}

static void test_hours(void)
{
    // The weekend (Sunday, Saturday), 9:00 to 20:00.
    const GateHours weekend = { 0x41, 540, 1200 };
    CHECK(gate_hours_free(&weekend, 6, 600) && gate_hours_free(&weekend, 0, 540));
    CHECK(!gate_hours_free(&weekend, 6, 1200) && !gate_hours_free(&weekend, 6, 539));
    CHECK(!gate_hours_free(&weekend, 3, 600));   // a Wednesday
    // Every day, 22:00 to 2:00: the evening, and the morning after.
    const GateHours night = { 0x7F, 1320, 120 };
    CHECK(gate_hours_free(&night, 2, 1400) && gate_hours_free(&night, 2, 60) && !gate_hours_free(&night, 2, 600));
    // Friday evening to Saturday 2:00 only.
    const GateHours friday = { 0x20, 1320, 120 };
    CHECK(gate_hours_free(&friday, 5, 1380) && gate_hours_free(&friday, 6, 60));
    CHECK(!gate_hours_free(&friday, 5, 60) && !gate_hours_free(&friday, 0, 60));
    // All day: 0 to 1440.
    const GateHours all = { 0x3E, 0, 1440 };
    CHECK(gate_hours_free(&all, 1, 0) && gate_hours_free(&all, 5, 1439) && !gate_hours_free(&all, 0, 600));
    // No day, an empty span, out of range: never.
    const GateHours none = { 0, 0, 1440 }, empty = { 0x7F, 600, 600 }, bad = { 0x7F, 0, 2000 };
    CHECK(!gate_hours_free(&none, 1, 600) && !gate_hours_free(&empty, 1, 600) && !gate_hours_free(&bad, 1, 600));
    CHECK(!gate_hours_free(&all, 7, 600) && !gate_hours_free(&all, 1, 1440) && !gate_hours_free(NULL, 1, 600));
}

static void test_rules(void)
{
    char text[512] = "", line[128];
    const GateHours weekend = { 0x41, 540, 1200 }, night = { 0x7F, 1320, 120 };
    CHECK(gate_rule_format(&weekend, "sdmc:/switch/cyberfoil.nro", line, sizeof(line)));
    CHECK(strcmp(line, "41\t540\t1200\tsdmc:/switch/cyberfoil.nro\n") == 0);
    strcat(text, "garbage line\n");
    strcat(text, line);
    CHECK(gate_rule_format(&night, "sdmc:/switch/x y.nro", line, sizeof(line)));
    strcat(text, "zz\t1\t2\tsdmc:/switch/x y.nro\n");   // broken: skipped
    strcat(text, line);
    GateHours h;
    CHECK(gate_rule_find(text, strlen(text), "sdmc:/switch/cyberfoil.nro", &h) && h.days == 0x41 && h.from == 540 && h.to == 1200);
    CHECK(gate_rule_find(text, strlen(text), "sdmc:/switch/x y.nro", &h) && h.days == 0x7F && h.to == 120);
    CHECK(!gate_rule_find(text, strlen(text), "sdmc:/switch/cyberfoil", &h));
    CHECK(gate_rule_find("41\t540\t1200\tsdmc:/a.nro\r\n", 24, "sdmc:/a.nro", &h));
    CHECK(!gate_rule_find("41\t540\t540\tsdmc:/a.nro\n", 23, "sdmc:/a.nro", &h));   // empty span
    CHECK(!gate_rule_find("41\t540\t1600\tsdmc:/a.nro\n", 24, "sdmc:/a.nro", &h));  // past the day
    const GateHours zero = { 0, 0, 60 };
    CHECK(!gate_rule_format(&zero, "sdmc:/a.nro", line, sizeof(line)) && line[0] == 0);
    CHECK(!gate_rule_format(&weekend, "sdmc:/a\tb.nro", line, sizeof(line)));
    CHECK(!gate_rule_format(&weekend, "sdmc:/a.nro", line, 8));
}

static void test_log(void)
{
    char line[128];
    CHECK(gate_log_format("2026-10-20 21:03", GateResult_Refused, "sdmc:/switch/cyberfoil.nro", line, sizeof(line)));
    CHECK(strcmp(line, "2026-10-20 21:03\trefused\tsdmc:/switch/cyberfoil.nro\n") == 0);
    const char *when, *nro;
    size_t wn, nn;
    GateResult r;
    CHECK(gate_log_parse(line, strlen(line) - 1, &when, &wn, &r, &nro, &nn));
    CHECK(wn == 16 && memcmp(when, "2026-10-20 21:03", 16) == 0 && r == GateResult_Refused);
    CHECK(nn == strlen("sdmc:/switch/cyberfoil.nro") && memcmp(nro, "sdmc:/switch/cyberfoil.nro", nn) == 0);
    for (int i = 0; i < 4; i++) {
        CHECK(gate_log_format("t", (GateResult)i, "sdmc:/a.nro", line, sizeof(line)));
        CHECK(gate_log_parse(line, strlen(line) - 1, &when, &wn, &r, &nro, &nn) && r == (GateResult)i);
    }
    CHECK(gate_log_parse("t\tpin\tsdmc:/a.nro\r", 18, &when, &wn, &r, &nro, &nn) && nn == 11);
    CHECK(!gate_log_parse("t\tmaybe\tsdmc:/a.nro", 19, &when, &wn, &r, &nro, &nn));
    CHECK(!gate_log_parse("t\tpin\t", 6, &when, &wn, &r, &nro, &nn));
    CHECK(!gate_log_parse("\tpin\tx", 6, &when, &wn, &r, &nro, &nn));
    CHECK(!gate_log_parse("no tabs", 7, &when, &wn, &r, &nro, &nn));
    CHECK(!gate_log_format("", GateResult_Pin, "sdmc:/a.nro", line, sizeof(line)));
    CHECK(!gate_log_format("t", GateResult_Pin, "sdmc:/a\n.nro", line, sizeof(line)));
    CHECK(strcmp(gate_result_name(GateResult_Free), "free") == 0 && strcmp(gate_result_name((GateResult)9), "error") == 0);
}

static void test_counts(void)
{
    static const char LOG[] = "2026-10-20 21:03\trefused\tsdmc:/s.nro\n"
                              "2026-10-20 21:04\trefused\tsdmc:/s.nro\r\n"
                              "2026-10-19 21:05\trefused\tsdmc:/s.nro\n"
                              "2026-10-20 21:06\tpin\tsdmc:/s.nro\n"
                              "2026-10-20 21:07\trefused\tsdmc:/s.nro.bak\n"
                              "garbage\n"
                              "2026-10-20 21:08\trefused\tsdmc:/s.nro";
    const size_t n = sizeof(LOG) - 1;
    CHECK(gate_log_count(LOG, n, "sdmc:/s.nro", "2026-10-20", GateResult_Refused) == 3);
    CHECK(gate_log_count(LOG, n, "sdmc:/s.nro", "2026-10-19", GateResult_Refused) == 1);
    CHECK(gate_log_count(LOG, n, "sdmc:/s.nro", "2026-10-20", GateResult_Pin) == 1);
    CHECK(gate_log_count(LOG, n, "sdmc:/s.nro", "?", GateResult_Refused) == 0);
    CHECK(gate_log_count(NULL, 0, "sdmc:/s.nro", "2026-10-20", GateResult_Refused) == 0);

    static const char DAYS[] = "Sun,Mon,Tue,Wed,Thu,Fri,Sat";
    char buf[96];
    GateHours h = { 0x41, 540, 1200 };
    CHECK(gate_hours_text(&h, DAYS, buf, sizeof(buf)) && strcmp(buf, "Sat\xE2\x80\x93Sun 09:00\xE2\x80\x93" "20:00") == 0);
    h = (GateHours){ 0x7F, 1320, 120 };
    CHECK(gate_hours_text(&h, DAYS, buf, sizeof(buf)) && strcmp(buf, "Mon\xE2\x80\x93Sun 22:00\xE2\x80\x93" "02:00") == 0);
    h = (GateHours){ 0x0A, 600, 660 };
    CHECK(gate_hours_text(&h, DAYS, buf, sizeof(buf)) && strcmp(buf, "Mon, Wed 10:00\xE2\x80\x93" "11:00") == 0);
    CHECK(!gate_hours_text(&h, "Sun,Mon", buf, sizeof(buf)) && buf[0] == '\0');   // not 7 names
    CHECK(!gate_hours_text(&h, DAYS, buf, 8));                                     // too long
    h.days = 0;
    CHECK(!gate_hours_text(&h, DAYS, buf, sizeof(buf)));
}

int main(void)
{
    test_counts();
    test_argv();
    test_hours();
    test_rules();
    test_log();
    return CHECK_DONE("gate argv, free hours, rules and log assertions passed");
}
