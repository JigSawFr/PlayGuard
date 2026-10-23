// Host tests for source/action/clock_logic.cpp: the servers' answers brought
// to one instant, their median (an even count without overflow), their
// spread and the network clock's offset, with and without that clock.
#include "check.h"
#include <cstdint>

#include "action/clock_logic.hpp"

using namespace clock_logic;
using std::chrono::seconds;

static const uint64_t NOW = 1791475200;   // 2026-10-08 16:00 UTC
static const auto REF = std::chrono::steady_clock::time_point(seconds(100000));

// A server that answered `t`, `ago` seconds before REF.
static ntp::Reply answer(uint64_t t, int ago = 0)
{
    ntp::Reply r;
    r.ok = true;
    r.unix_seconds = t;
    r.received_at = REF - seconds(ago);
    return r;
}

static ntp::Reply no_answer()
{
    ntp::Reply r;
    r.kind = ntp::Error::Timeout;
    r.error = "timeout";
    return r;
}

static void test_estimate()
{
    CHECK(estimate(answer(NOW), REF) == (int64_t)NOW);
    CHECK(estimate(answer(NOW, 3), REF) == (int64_t)NOW + 3);
    // Received less than a second before: rounded down, nothing added.
    ntp::Reply r = answer(NOW);
    r.received_at = REF - std::chrono::milliseconds(900);
    CHECK(estimate(r, REF) == (int64_t)NOW);
}

static void test_none()
{
    Summary s = summarize({}, REF, NOW);
    CHECK(!s.ok && s.hosts.empty());
    s = summarize({ no_answer(), no_answer() }, REF, NOW);
    CHECK(!s.ok && s.median == 0 && s.spread == 0 && s.offset == 0);
    CHECK(s.offset_known);
    if (!CHECK(s.hosts.size() == 2)) return;
    CHECK(!s.hosts[0].ok && !s.hosts[1].ok);
}

static void test_median()
{
    // Odd count: the middle one, whatever the order; each brought to REF.
    Summary s = summarize({ answer(NOW + 10), answer(NOW - 2, 2), answer(NOW + 1) }, REF, NOW - 20);
    CHECK(s.ok && s.median == NOW + 1);
    CHECK(s.spread == 10);
    CHECK(s.offset_known && s.offset == 21);
    if (!CHECK(s.hosts.size() == 3)) return;
    CHECK(s.hosts[0].ok && s.hosts[0].estimate == (int64_t)NOW + 10 && s.hosts[0].offset == 30);
    CHECK(s.hosts[1].estimate == (int64_t)NOW && s.hosts[1].offset == 20);

    // Even count: the mean of the two middle ones, rounded down; a server that
    // did not answer is left out of it but keeps its place.
    s = summarize({ answer(NOW + 3), no_answer(), answer(NOW) }, REF, NOW + 5);
    CHECK(s.ok && s.median == NOW + 1 && s.spread == 3);
    CHECK(s.offset == -4);   // the network clock ahead
    if (!CHECK(s.hosts.size() == 3)) return;
    CHECK(!s.hosts[1].ok && s.hosts[1].estimate == 0);
    CHECK(s.hosts[2].offset == -5);

    // One answer: itself, no spread.
    s = summarize({ answer(NOW) }, REF, NOW);
    CHECK(s.ok && s.median == NOW && s.spread == 0 && s.offset == 0);
}

static void test_no_overflow()
{
    // Two middle samples near the top of int64_t: their sum would overflow (UBSan).
    const uint64_t hi = (uint64_t)INT64_MAX;
    Summary s = summarize({ answer(hi), answer(hi - 1) }, REF, std::nullopt);
    CHECK(s.ok && s.median == hi - 1);
    s = summarize({ answer(hi), answer(hi) }, REF, std::nullopt);
    CHECK(s.median == hi);
}

static void test_network_unread()
{
    Summary s = summarize({ answer(NOW), answer(NOW + 2) }, REF, std::nullopt);
    CHECK(s.ok && !s.offset_known && s.offset == 0 && s.median == NOW + 1);
    if (!CHECK(s.hosts.size() == 2)) return;
    CHECK(s.hosts[0].offset == 0 && s.hosts[1].offset == 0);
}

int main()
{
    test_estimate();
    test_none();
    test_median();
    test_no_overflow();
    test_network_unread();
    return CHECK_DONE("clock_logic tests passed");
}
