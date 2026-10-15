// Host tests for source/action/offline_clock_logic.cpp: when no time server
// answers, when "Set the network clock to the console clock" is offered, and
// what its confirmation must add.
#include "check.h"
#include <cstring>

#include "action/offline_clock_logic.hpp"

using namespace offline_clock_logic;

static const uint64_t NOW = 1791475200;   // 2026-10-08 16:00 UTC

// Every clock read, "Synchronize" off, the network clock 1411 s ahead (the
// test console of docs/parental-controls.md).
static TimeSnapshot offline_console()
{
    TimeSnapshot s;
    std::memset(&s, 0, sizeof(s));
    s.user_time = NOW;
    s.network_time = NOW + 1411;
    s.local_time = NOW;
    s.automatic = false;
    s.accuracy = false;
    return s;
}

static void test_offer()
{
    TimeSnapshot s = offline_console();
    CHECK(offer(true, s) == Offer::None);       // a server answered: write its time
    CHECK(offer(false, s) == Offer::Copy);

    s.network_rc = 0x7C74;                      // never set (cc74): still offered
    CHECK(offer(false, s) == Offer::Copy);

    s = offline_console();
    s.user_rc = 0x0C74;
    CHECK(offer(false, s) == Offer::Unreadable);
    CHECK(offer(true, s) == Offer::None);
    s = offline_console();
    s.service_rc = 0x0C74;
    CHECK(offer(false, s) == Offer::Unreadable);

    s = offline_console();
    s.user_time = PLAUSIBLE_FROM - 1;           // a console clock never set
    CHECK(offer(false, s) == Offer::Implausible);
    s.user_time = PLAUSIBLE_FROM;
    CHECK(offer(false, s) == Offer::Copy);
    s.user_time = 0;
    CHECK(offer(false, s) == Offer::Implausible);
}

static void test_notes()
{
    // Sync off: copy, then turn it on (the console clock then stays put).
    TimeSnapshot s = offline_console();
    Notes n = notes(s);
    CHECK(n.turn_sync_on && !n.follows_network && !n.never_set);
    CHECK(n.network_off_s == 1411);

    // Sync on, both clocks the same: the console clock may follow a wrong one.
    s.automatic = true;
    s.network_time = NOW + 3;
    n = notes(s);
    CHECK(n.follows_network && !n.turn_sync_on);
    CHECK(n.network_off_s == 3);
    s.network_time = NOW - (uint64_t)SAME_S;
    CHECK(notes(s).follows_network);
    s.network_time = NOW - (uint64_t)SAME_S - 1;
    CHECK(!notes(s).follows_network);
    CHECK(notes(s).network_off_s == -SAME_S - 1);

    // Sync on, clocks apart (the network clock not accurate, not followed).
    s.network_time = NOW + 600;
    n = notes(s);
    CHECK(!n.follows_network && !n.turn_sync_on);

    // Sync unknown counts as off: the parent is told to turn it on.
    s.automatic_rc = 0x0C74;
    CHECK(notes(s).turn_sync_on);

    // Network clock never set: no difference, nothing "followed".
    s = offline_console();
    s.automatic = true;
    s.network_rc = 0x7C74;
    n = notes(s);
    CHECK(n.never_set && !n.follows_network && n.network_off_s == 0);

    // Far apart either way: no overflow (UBSan) and no false "same".
    s = offline_console();
    s.automatic = true;
    s.user_time = UINT64_MAX;
    s.network_time = 0;
    CHECK(!notes(s).follows_network);
    s.user_time = 0;
    s.network_time = UINT64_MAX;
    CHECK(!notes(s).follows_network);
}

static void test_worth_writing()
{
    CHECK(!worth_writing(0));
    CHECK(!worth_writing(PLAUSIBLE_FROM - 1));
    CHECK(worth_writing(PLAUSIBLE_FROM));
    CHECK(worth_writing(NOW));
}

int main()
{
    test_offer();
    test_notes();
    test_worth_writing();
    return CHECK_DONE("offline_clock_logic tests passed");
}
