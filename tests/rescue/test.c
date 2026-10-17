// Host tests for source/core/rescue.c: the request's mode, the report's
// format and parsing (round trip, any line order, Windows line ends,
// malformed values refused), and its proof (the challenge, the message the
// MAC covers, the proof's lines; the MAC itself is libnx's or util/sha256's,
// tested in tests/dev_builds).
#include "check.h"
#include <stdio.h>
#include <string.h>

#include "rescue.h"

static RescueMode mode_of(const char *s)
{
    return rescue_request_mode(s, strlen(s));
}

static void test_request_mode(void)
{
    CHECK(mode_of("") == RescueMode_Unlock);
    CHECK(rescue_request_mode(NULL, 0) == RescueMode_Unlock);
    CHECK(mode_of("unlock") == RescueMode_Unlock);
    CHECK(mode_of("please help") == RescueMode_Unlock);
    // A line that is the word alone, in any case, spaces around it ignored.
    CHECK(mode_of("delete") == RescueMode_Delete);
    CHECK(mode_of("DELETE\n") == RescueMode_Delete);
    CHECK(mode_of("DELETE\r\n") == RescueMode_Delete);
    CHECK(mode_of("  delete \r\n") == RescueMode_Delete);
    CHECK(mode_of("\tDelete\t") == RescueMode_Delete);
    CHECK(mode_of("unlock\ndelete") == RescueMode_Delete);
    CHECK(mode_of("please\r\n\r\n delete\r\nthanks\r\n") == RescueMode_Delete);
    CHECK(mode_of("\xEF\xBB\xBF" "delete") == RescueMode_Delete);   // Notepad's byte-order mark
    CHECK(mode_of("\xEF\xBB\xBF" "Delete\r\n") == RescueMode_Delete);
    // The word inside other text is not enough.
    CHECK(mode_of("don't delete anything") == RescueMode_Unlock);
    CHECK(mode_of("undelete") == RescueMode_Unlock);
    CHECK(mode_of("delete everything") == RescueMode_Unlock);
    CHECK(mode_of("  delete everything") == RescueMode_Unlock);
    CHECK(mode_of("deleted") == RescueMode_Unlock);
    CHECK(mode_of("delet") == RescueMode_Unlock);
    CHECK(mode_of("del ete") == RescueMode_Unlock);
    CHECK(mode_of("\n\n") == RescueMode_Unlock);
    // A byte-order mark only counts at the very start.
    CHECK(mode_of("x\n\xEF\xBB\xBF" "delete") == RescueMode_Unlock);
    // Only the first `len` bytes count.
    CHECK(rescue_request_mode("delete", 5) == RescueMode_Unlock);
    CHECK(rescue_request_mode("deletex", 6) == RescueMode_Delete);
    CHECK(rescue_request_mode("\xEF\xBB", 2) == RescueMode_Unlock);
}

static void test_round_trip(void)
{
    const RescueReport all[] = {
        { RescueMode_Unlock, RescueResult_Ok, 0, 1, RescueRequest_Removed },
        { RescueMode_Unlock, RescueResult_NoPin, 0, 0, RescueRequest_Renamed },
        { RescueMode_Unlock, RescueResult_Failed, 0xC8A0F, 0, RescueRequest_Kept },
        { RescueMode_Delete, RescueResult_Ok, 0, 0, RescueRequest_Renamed },
        { RescueMode_Delete, RescueResult_Failed, 0xFFFFFFFFu, 4294967295u, RescueRequest_Removed },
        { RescueMode_Delete, RescueResult_Refused, 0, 0, RescueRequest_Kept },
    };
    for (size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
        char buf[128];
        const size_t n = rescue_report_format(&all[i], buf, sizeof(buf));
        CHECK(n > 0 && n == strlen(buf));
        RescueReport back;
        memset(&back, 0x5A, sizeof(back));
        CHECK(rescue_report_parse(buf, n, &back));
        CHECK(back.mode == all[i].mode && back.result == all[i].result);
        CHECK(back.rc == all[i].rc && back.unlocks == all[i].unlocks);
        CHECK(back.request == all[i].request);
    }

    char buf[128];
    const RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 2, RescueRequest_Removed };
    rescue_report_format(&r, buf, sizeof(buf));
    CHECK(strcmp(buf, "mode=unlock\nresult=ok\nrc=0x00000000\nunlocks=2\nrequest=removed\n") == 0);
}

static void test_format_too_small(void)
{
    const RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 0, RescueRequest_Removed };
    char buf[10];
    memset(buf, 'x', sizeof(buf));
    CHECK(rescue_report_format(&r, buf, sizeof(buf)) == 0 && buf[0] == '\0');
    CHECK(rescue_report_format(&r, buf, 0) == 0);
    CHECK(rescue_report_format(NULL, buf, sizeof(buf)) == 0 && buf[0] == '\0');
}

static bool parses(const char *text, RescueReport *out)
{
    return rescue_report_parse(text, strlen(text), out);
}

static void test_parse(void)
{
    RescueReport r;
    // Any order, CR LF, unknown keys, blank lines, no trailing newline.
    CHECK(parses("unlocks=3\r\nrc=12\r\n\r\nresult=failed\r\nfuture=1\r\nmode=delete", &r));
    CHECK(r.mode == RescueMode_Delete && r.result == RescueResult_Failed && r.rc == 12 && r.unlocks == 3);
    // rc, unlocks and request are optional (a report from before request=).
    CHECK(parses("mode=unlock\nresult=no_pin\n", &r));
    CHECK(r.result == RescueResult_NoPin && r.rc == 0 && r.unlocks == 0 && r.request == RescueRequest_Removed);
    CHECK(parses("mode=delete\nresult=refused\nrequest=kept\n", &r));
    CHECK(r.result == RescueResult_Refused && r.request == RescueRequest_Kept);

    // Refused, and *out left as it was.
    const char *bad[] = {
        "",
        "result=ok\n",                              // no mode
        "mode=unlock\n",                            // no result
        "mode=wipe\nresult=ok\n",
        "mode=unlock\nresult=maybe\n",
        "mode=unlock\nresult=ok\nrc=0xZZ\n",
        "mode=unlock\nresult=ok\nrc=0x1FFFFFFFF\n",  // over 32 bits
        "mode=unlock\nresult=ok\nunlocks=-1\n",
        "mode=unlock\nresult=ok\nunlocks=\n",
        "mode=Unlock\nresult=ok\n",                 // written by the sysmodule: exact
        "mode= unlock\nresult=ok\n",
        "mode=unlock\nresult=ok\nrequest=gone\n",
        "mode=unlock\nresult=ok\nrequest=\n",
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        memset(&r, 0x5A, sizeof(r));
        RescueReport before = r;
        CHECK(!parses(bad[i], &r));
        CHECK(memcmp(&r, &before, sizeof(r)) == 0);
    }
    CHECK(!rescue_report_parse(NULL, 0, &r));
    CHECK(!rescue_report_parse("mode=unlock\nresult=ok\n", 22, NULL));
    // Only `len` bytes are read.
    CHECK(!rescue_report_parse("mode=unlock\nresult=ok\n", 12, &r));
}

static void test_names(void)
{
    CHECK(strcmp(rescue_mode_name(RescueMode_Unlock), "unlock") == 0);
    CHECK(strcmp(rescue_mode_name(RescueMode_Delete), "delete") == 0);
    CHECK(strcmp(rescue_result_name(RescueResult_Ok), "ok") == 0);
    CHECK(strcmp(rescue_result_name(RescueResult_NoPin), "no_pin") == 0);
    CHECK(strcmp(rescue_result_name(RescueResult_Failed), "failed") == 0);
    CHECK(strcmp(rescue_result_name(RescueResult_Refused), "refused") == 0);
    CHECK(strcmp(rescue_request_name(RescueRequest_Removed), "removed") == 0);
    CHECK(strcmp(rescue_request_name(RescueRequest_Renamed), "renamed") == 0);
    CHECK(strcmp(rescue_request_name(RescueRequest_Kept), "kept") == 0);
}

// A report is only as good as the console state behind it: one written by
// hand on the SD card must not open the recovery actions.
static void test_confirmed(void)
{
    RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 1, RescueRequest_Removed };
    CHECK(rescue_report_confirmed(&r, 4, true));     // unlocked with a PIN: what the sysmodule did
    CHECK(!rescue_report_confirmed(&r, 4, false));   // still locked: written by hand
    CHECK(!rescue_report_confirmed(&r, 0, true));    // no PIN: there was nothing to unlock

    r.mode = RescueMode_Delete;
    CHECK(rescue_report_confirmed(&r, 0, false));    // the PIN is gone, as a delete leaves it
    CHECK(!rescue_report_confirmed(&r, 4, false));   // a PIN is still set: not deleted
    CHECK(!rescue_report_confirmed(&r, 4, true));

    r.mode = RescueMode_Unlock;
    r.result = RescueResult_NoPin;
    CHECK(rescue_report_confirmed(&r, 0, false));
    CHECK(!rescue_report_confirmed(&r, 6, false));

    r.result = RescueResult_Failed;                   // nothing was done: never confirmed
    CHECK(!rescue_report_confirmed(&r, 4, true));
    CHECK(!rescue_report_confirmed(&r, 0, false));
    r.result = RescueResult_Refused;
    r.mode = RescueMode_Delete;
    CHECK(!rescue_report_confirmed(&r, 4, false));
    CHECK(!rescue_report_confirmed(&r, 0, false));

    CHECK(!rescue_report_confirmed(NULL, 0, true));
}

static const uint8_t NONCE[RESCUE_NONCE_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                   0x08, 0x09, 0x0a, 0x0b, 0xfc, 0xfd, 0xfe, 0xff };

static void test_needs_proof(void)
{
    RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 1, RescueRequest_Removed };
    CHECK(rescue_report_needs_proof(&r));   // the PIN stays set: the actions go around it
    r.result = RescueResult_NoPin;
    CHECK(!rescue_report_needs_proof(&r));
    r.result = RescueResult_Failed;
    CHECK(!rescue_report_needs_proof(&r));
    r.mode = RescueMode_Delete;
    r.result = RescueResult_Ok;
    CHECK(!rescue_report_needs_proof(&r));   // no PIN left to go around
    CHECK(!rescue_report_needs_proof(NULL));
}

static void test_nonce(void)
{
    char buf[64];
    CHECK(rescue_nonce_format(NONCE, buf, sizeof(buf)) == 33);
    CHECK(strcmp(buf, "000102030405060708090a0bfcfdfeff\n") == 0);
    uint8_t back[RESCUE_NONCE_SIZE];
    CHECK(rescue_nonce_parse(buf, strlen(buf), back) && memcmp(back, NONCE, sizeof(back)) == 0);
    CHECK(rescue_nonce_parse(" 000102030405060708090A0BFCFDFEFF\r\n", 35, back) && memcmp(back, NONCE, sizeof(back)) == 0);
    CHECK(rescue_nonce_format(NONCE, buf, 33) == 0 && buf[0] == '\0');   // no room for the NUL

    const char *bad[] = { "", "00", "000102030405060708090a0bfcfdfe", "000102030405060708090a0bfcfdfeff00",
                          "000102030405060708090a0bfcfdfeg0", "0001020304050607 08090a0bfcfdfeff" };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        memset(back, 0x5A, sizeof(back));
        CHECK(!rescue_nonce_parse(bad[i], strlen(bad[i]), back) && back[0] == 0x5A);
    }
    CHECK(!rescue_nonce_parse(NULL, 0, back));
}

static void test_mac_message(void)
{
    const RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 1, RescueRequest_Removed };
    char buf[256];
    const char *want = "playguard-rescue/1\nnonce=000102030405060708090a0bfcfdfeff\n"
                       "mode=unlock\nresult=ok\nrc=0x00000000\nunlocks=1\nrequest=removed\n";
    CHECK(rescue_mac_message(&r, NONCE, buf, sizeof(buf)) == strlen(want) && strcmp(buf, want) == 0);
    // A report parsed back from the file gives the same message, whatever
    // the file's order of lines or its proof lines.
    RescueReport back;
    const char *file = "request=removed\r\nunlocks=1\r\nrc=0\r\nresult=ok\r\nmode=unlock\r\nnonce=00\r\nmac=11\r\n";
    CHECK(parses(file, &back));
    char again[256];
    CHECK(rescue_mac_message(&back, NONCE, again, sizeof(again)) == strlen(want) && strcmp(again, want) == 0);
    // Too small: nothing.
    CHECK(rescue_mac_message(&r, NONCE, buf, 40) == 0 && buf[0] == '\0');
    CHECK(rescue_mac_message(&r, NONCE, buf, strlen(want)) == 0 && buf[0] == '\0');
    CHECK(rescue_mac_message(NULL, NONCE, buf, sizeof(buf)) == 0);
    CHECK(rescue_mac_message(&r, NULL, buf, sizeof(buf)) == 0);
}

static void test_proof(void)
{
    RescueProof p;
    memset(&p, 0, sizeof(p));
    p.present = true;
    memcpy(p.nonce, NONCE, sizeof(p.nonce));
    for (size_t i = 0; i < RESCUE_MAC_SIZE; i++) p.mac[i] = (uint8_t)(0xA0 + i);

    // After the report, as the sysmodule writes it; its reader ignores the lines.
    char buf[256];
    const RescueReport r = { RescueMode_Unlock, RescueResult_Ok, 0, 1, RescueRequest_Removed };
    size_t n = rescue_report_format(&r, buf, sizeof(buf));
    const size_t m = rescue_proof_format(&p, buf + n, sizeof(buf) - n);
    CHECK(m == 6 + 32 + 1 + 4 + 64 + 1);
    CHECK(strncmp(buf + n, "nonce=000102030405060708090a0bfcfdfeff\nmac=a0a1a2", 49) == 0);
    n += m;
    RescueReport back;
    CHECK(rescue_report_parse(buf, n, &back) && back.mode == RescueMode_Unlock && back.result == RescueResult_Ok);
    RescueProof q;
    rescue_proof_parse(buf, n, &q);
    CHECK(q.present && memcmp(q.nonce, p.nonce, sizeof(q.nonce)) == 0 && memcmp(q.mac, p.mac, sizeof(q.mac)) == 0);

    // Absent: an older sysmodule's report, a line missing, malformed or twice.
    char mac_hex[2 * RESCUE_MAC_SIZE + 1];
    memcpy(mac_hex, strstr(buf, "mac=") + 4, 2 * RESCUE_MAC_SIZE);
    mac_hex[2 * RESCUE_MAC_SIZE] = '\0';
    char text[512];
    const char *nonce_line = "nonce=000102030405060708090a0bfcfdfeff\n";
    const char *cases[][2] = {
        { "mode=unlock\nresult=ok\n", "" },
        { nonce_line, "" },
        { "", "mac=" },
        { "nonce=0001\n", "mac=" },
        { nonce_line, "mac=zz" },
        { nonce_line, "mac=" },     // twice, below
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        snprintf(text, sizeof(text), "mode=unlock\nresult=ok\n%s%s%s\n", cases[i][0], cases[i][1],
                 strcmp(cases[i][1], "mac=") == 0 ? mac_hex : "");
        if (i == 5) strcat(text, "mac=00\n");
        memset(&q, 0x5A, sizeof(q));
        rescue_proof_parse(text, strlen(text), &q);
        CHECK(!q.present && q.nonce[0] == 0 && q.mac[0] == 0);
    }
    snprintf(text, sizeof(text), "mode=unlock\r\nresult=ok\r\n%smac=%s\r\n", nonce_line, mac_hex);
    rescue_proof_parse(text, strlen(text), &q);
    CHECK(q.present);   // Windows line ends too

    p.present = false;
    CHECK(rescue_proof_format(&p, buf, sizeof(buf)) == 0 && buf[0] == '\0');
    p.present = true;
    CHECK(rescue_proof_format(&p, buf, 100) == 0 && buf[0] == '\0');
    CHECK(rescue_proof_format(NULL, buf, sizeof(buf)) == 0);
    rescue_proof_parse(NULL, 0, &q);
    CHECK(!q.present);
}

static void test_mac_equal_and_pin_length(void)
{
    uint8_t a[RESCUE_MAC_SIZE], b[RESCUE_MAC_SIZE];
    memset(a, 7, sizeof(a));
    memcpy(b, a, sizeof(b));
    CHECK(rescue_mac_equal(a, b));
    b[31] ^= 1;
    CHECK(!rescue_mac_equal(a, b));
    b[31] ^= 1;
    b[0] ^= 0x80;
    CHECK(!rescue_mac_equal(a, b));
    CHECK(!rescue_mac_equal(a, NULL) && !rescue_mac_equal(NULL, b));

    char pin[32] = "1234";
    CHECK(rescue_pin_length(pin, sizeof(pin), 0) == 4);
    CHECK(rescue_pin_length(pin, sizeof(pin), 4) == 4);
    CHECK(rescue_pin_length(pin, sizeof(pin), 2) == 2);   // 1208's count, when shorter
    CHECK(rescue_pin_length(pin, sizeof(pin), 9) == 4);
    CHECK(rescue_pin_length(pin, 3, 0) == 3);             // no NUL in the buffer
    CHECK(rescue_pin_length("", 32, 4) == 0);
    CHECK(rescue_pin_length(NULL, 32, 4) == 0);
}

int main(void)
{
    test_request_mode();
    test_round_trip();
    test_format_too_small();
    test_parse();
    test_names();
    test_confirmed();
    test_needs_proof();
    test_nonce();
    test_mac_message();
    test_proof();
    test_mac_equal_and_pin_length();
    return CHECK_DONE("rescue request, report and proof assertions passed");
}
