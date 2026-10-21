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

static void test_pause(void)
{
    char out[1024], next[1024];
    // No file: 5 tries, 15 minutes.
    GatePauseRule rule = gate_pause_rule(NULL, 0);
    CHECK(rule.tries == GATE_PAUSE_TRIES && rule.minutes == GATE_PAUSE_MINUTES);
    // Refused four times: counted; the fifth pauses it from then, the count back to 0.
    size_t n = 0;
    out[0] = '\0';
    for (int i = 0; i < 4; i++) {
        n = gate_pause_update(out, n, "sdmc:/s.nro", GateResult_Refused, 1000 + (uint64_t)i, next, sizeof(next));
        CHECK(n > 0);
        memcpy(out, next, n + 1);
    }
    unsigned streak;
    uint64_t until;
    CHECK(gate_pause_find(out, n, "sdmc:/s.nro", &streak, &until) && streak == 4 && until == 0);
    n = gate_pause_update(out, n, "sdmc:/s.nro", GateResult_Refused, 2000, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(gate_pause_find(out, n, "sdmc:/s.nro", &streak, &until) && streak == 0 && until == 2000 + 15 * 60);
    // A try while paused leaves it; another forwarder has its own line.
    n = gate_pause_update(out, n, "sdmc:/s.nro", GateResult_Paused, 2100, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(gate_pause_find(out, n, "sdmc:/s.nro", &streak, &until) && until == 2900);
    n = gate_pause_update(out, n, "sdmc:/o.nro", GateResult_Refused, 2100, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(gate_pause_find(out, n, "sdmc:/o.nro", &streak, &until) && streak == 1);
    // Refused after the pause: counted again from 1.
    n = gate_pause_update(out, n, "sdmc:/s.nro", GateResult_Refused, 3000, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(gate_pause_find(out, n, "sdmc:/s.nro", &streak, &until) && streak == 1 && until == 0);
    // Opened: its line gone, the other's kept.
    n = gate_pause_update(out, n, "sdmc:/s.nro", GateResult_Pin, 3100, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(!gate_pause_find(out, n, "sdmc:/s.nro", &streak, &until) && gate_pause_find(out, n, "sdmc:/o.nro", NULL, NULL));

    // PlayGuard's rule: 3 tries, an hour; written first, the lines kept.
    const GatePauseRule three = { 3, 60 };
    n = gate_pause_edit(out, n, &three, NULL, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(strncmp(out, "rule\t3\t60\n", 10) == 0 && gate_pause_find(out, n, "sdmc:/o.nro", NULL, NULL));
    rule = gate_pause_rule(out, n);
    CHECK(rule.tries == 3 && rule.minutes == 60);
    n = gate_pause_update(out, n, "sdmc:/o.nro", GateResult_Refused, 5000, next, sizeof(next));
    memcpy(out, next, n + 1);
    n = gate_pause_update(out, n, "sdmc:/o.nro", GateResult_Refused, 5001, next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(gate_pause_find(out, n, "sdmc:/o.nro", &streak, &until) && until == 5001 + 3600);
    CHECK(strncmp(out, "rule\t3\t60\n", 10) == 0);   // the rule kept by the gate
    // Lifted: its line gone, the rule kept.
    n = gate_pause_edit(out, n, NULL, "sdmc:/o.nro", next, sizeof(next));
    memcpy(out, next, n + 1);
    CHECK(!gate_pause_find(out, n, "sdmc:/o.nro", NULL, NULL) && gate_pause_rule(out, n).tries == 3);
    // Off: never paused.
    const GatePauseRule off = { 0, 0 };
    n = gate_pause_edit(out, n, &off, NULL, next, sizeof(next));
    memcpy(out, next, n + 1);
    for (int i = 0; i < 20; i++) {
        n = gate_pause_update(out, n, "sdmc:/o.nro", GateResult_Refused, 6000, next, sizeof(next));
        memcpy(out, next, n + 1);
    }
    CHECK(gate_pause_find(out, n, "sdmc:/o.nro", &streak, &until) && streak == 20 && until == 0);
    // Damaged lines dropped, a tiny buffer refused.
    static const char BAD[] = "rule\tx\t5\nzz\tq\tsdmc:/a.nro\n1\t0\tsdmc:/b.nro\n";
    CHECK(gate_pause_rule(BAD, sizeof(BAD) - 1).tries == GATE_PAUSE_TRIES);
    n = gate_pause_edit(BAD, sizeof(BAD) - 1, NULL, NULL, next, sizeof(next));
    CHECK(strcmp(next, "1\t0\tsdmc:/b.nro\n") == 0);
    CHECK(gate_pause_update(BAD, sizeof(BAD) - 1, "sdmc:/b.nro", GateResult_Refused, 1, next, 4) == 0);
    // The two new results in the log.
    char line[128];
    const char *when, *nro;
    size_t wn, nn;
    GateResult r;
    CHECK(gate_log_format("2026-10-20 21:00", GateResult_Paused, "sdmc:/a.nro", line, sizeof(line)));
    CHECK(gate_log_parse(line, strlen(line) - 1, &when, &wn, &r, &nro, &nn) && r == GateResult_Paused);
    CHECK(gate_log_format("2026-10-20 21:00", GateResult_Closed, "sdmc:/a.nro", line, sizeof(line)));
    CHECK(gate_log_parse(line, strlen(line) - 1, &when, &wn, &r, &nro, &nn) && r == GateResult_Closed);
}

static void test_carried(void)
{
    char nro[64];
    const char *argv = NULL;
    GateRules r;
    // Each kind of token, then the homebrew's path and its own argv.
    static const char RAW[] = "sdmc:/config/playguard/gate-1a2b.nro --pin-gate --gate-rules --gate-free=3e,960,1140 "
                              "--gate-free=41,540,1200 --gate-closed=7f,1260,420 --gate-pause=3,30 --gate-limit "
                              "\"sdmc:/switch/x y.nro\" -v";
    CHECK(gate_argv_rules(RAW, nro, sizeof(nro), &argv, &r));
    CHECK(strcmp(nro, "sdmc:/switch/x y.nro") == 0 && strcmp(argv, "\"sdmc:/switch/x y.nro\" -v") == 0);
    CHECK(r.carried && r.limit && r.free_n == 2 && r.closed_n == 1 && r.has_pause);
    CHECK(r.free[0].days == 0x3E && r.free[0].from == 960 && r.free[0].to == 1140 && r.free[1].days == 0x41);
    CHECK(r.closed[0].from == 1260 && r.closed[0].to == 420 && r.pause_tries == 3 && r.pause_minutes == 30);
    // The plain parser skips them; the same homebrew.
    CHECK(gate_argv_parse(RAW, nro, sizeof(nro), &argv) && strcmp(nro, "sdmc:/switch/x y.nro") == 0);
    // Weekdays 16:00-19:00 or the weekend 9:00-20:00.
    CHECK(gate_spans_hold(r.free, r.free_n, 3, 1000) && !gate_spans_hold(r.free, r.free_n, 3, 600));
    CHECK(gate_spans_hold(r.free, r.free_n, 6, 600) && !gate_spans_hold(r.free, 0, 6, 600));
    // Round trip.
    char tokens[256];
    const size_t n = gate_rules_format(&r, tokens, sizeof(tokens));
    CHECK(n > 0 && strcmp(tokens, " --gate-rules --gate-free=3e,960,1140 --gate-free=41,540,1200 "
                                  "--gate-closed=7f,1260,420 --gate-pause=3,30 --gate-limit") == 0);
    char again[320];
    snprintf(again, sizeof(again), "sdmc:/g.nro --pin-gate%s sdmc:/x.nro", tokens);
    GateRules back;
    CHECK(gate_argv_rules(again, nro, sizeof(nro), &argv, &back) && memcmp(&back, &r, sizeof(r)) == 0);
    // A forwarder made before: none carried; nothing after the rules: not a gate's.
    CHECK(gate_argv_rules("sdmc:/g.nro --pin-gate sdmc:/x.nro", nro, sizeof(nro), &argv, &r) && !r.carried && !r.free_n);
    CHECK(!gate_argv_rules("sdmc:/g.nro --pin-gate --gate-rules", nro, sizeof(nro), &argv, &r));
    // A span it cannot read, an unknown word, too many spans: skipped, the rest kept.
    CHECK(gate_argv_rules("sdmc:/g.nro --pin-gate --gate-rules --gate-free=zz,1,2 --gate-free=7f,600,600 --gate-new=1 "
                          "--gate-free=7f,0,1440,9 --gate-pause=4 sdmc:/x.nro",
                          nro, sizeof(nro), &argv, &r));
    CHECK(r.carried && r.free_n == 0 && !r.has_pause && strcmp(nro, "sdmc:/x.nro") == 0);
    CHECK(gate_argv_rules("sdmc:/g.nro --pin-gate --gate-free=7f,0,60 --gate-free=7f,60,120 --gate-free=7f,120,180 "
                          "--gate-free=7f,180,240 --gate-free=7f,240,300 sdmc:/x.nro",
                          nro, sizeof(nro), &argv, &r) && r.free_n == GATE_SPANS && !r.carried);
    // A pause of nothing is none; none at all, never.
    CHECK(gate_argv_rules("sdmc:/g.nro --pin-gate --gate-rules --gate-pause=5,0 sdmc:/x.nro", nro, sizeof(nro), &argv, &r) &&
          r.has_pause && r.pause_tries == 0);
    const GateRules off = { true, { { 0 } }, 0, { { 0 } }, 0, true, 0, 0, false };
    CHECK(gate_rules_format(&off, tokens, sizeof(tokens)) && strcmp(tokens, " --gate-rules --gate-pause=0,0") == 0);
    CHECK(gate_rules_format(&r, tokens, 8) == 0 && tokens[0] == 0);
    // The pause by the carried rule, not the file's.
    static const char FILE_RULE[] = "rule\t10\t60\n";
    const GatePauseRule two = { 2, 5 };
    char next[256];
    gate_pause_update_with(FILE_RULE, sizeof(FILE_RULE) - 1, "sdmc:/x.nro", GateResult_Refused, 1000, &two, next, sizeof(next));
    char after[256];
    gate_pause_update_with(next, strlen(next), "sdmc:/x.nro", GateResult_Refused, 1001, &two, after, sizeof(after));
    uint64_t until = 0;
    CHECK(gate_pause_find(after, strlen(after), "sdmc:/x.nro", NULL, &until) && until == 1001 + 300);
    // The limit in the log; spans as text.
    char line[128];
    const char *when, *lnro;
    size_t wn, nn;
    GateResult res;
    CHECK(gate_log_format("2026-10-20 21:00", GateResult_Limit, "sdmc:/a.nro", line, sizeof(line)) &&
          strstr(line, "\tlimit\t"));
    CHECK(gate_log_parse(line, strlen(line) - 1, &when, &wn, &res, &lnro, &nn) && res == GateResult_Limit);
    char text[256];
    static const char DAYS[] = "Sun,Mon,Tue,Wed,Thu,Fri,Sat";
    CHECK(gate_spans_text(back.free, back.free_n, DAYS, text, sizeof(text)) &&
          strcmp(text, "Mon\xE2\x80\x93" "Fri 16:00\xE2\x80\x93" "19:00, Sat\xE2\x80\x93" "Sun 09:00\xE2\x80\x93" "20:00") == 0);
}

int main(void)
{
    test_carried();
    test_pause();
    test_counts();
    test_argv();
    test_hours();
    test_rules();
    test_log();
    return CHECK_DONE("gate argv, free hours, rules and log assertions passed");
}
