// Host tests for source/action/pin_lock_logic.cpp: the change check (modes,
// the PIN holding until PlayGuard closes or 5 minutes with "Ask for the PIN
// again", the 3-second hold after a refusal), what
// moving between the modes needs, and "open" locking again after time away.
#include "check.h"
#include <cstdio>
#include <string>

#include "action/pin_lock_logic.hpp"

using pin_lock_logic::Change;
using pin_lock_logic::Check;
using pin_lock_logic::Clock;
using std::chrono::minutes;
using std::chrono::seconds;

// A moment well after the clock's epoch, as on a console that has been on a while.
static const Clock::time_point T0 = Clock::time_point{} + std::chrono::hours(10);

static void test_rank()
{
    CHECK(pin_lock_logic::rank("off") == 0);
    CHECK(pin_lock_logic::rank("changes") == 1);
    CHECK(pin_lock_logic::rank("open") == 2);
    CHECK(pin_lock_logic::rank("") == 0);           // unknown: off
    CHECK(pin_lock_logic::rank("Changes") == 0);
    for (int i = 0; i < 3; i++) CHECK(pin_lock_logic::rank(pin_lock_logic::MODES[i]) == i);
}

static void test_holds()
{
    const Clock::time_point never = pin_lock_logic::NEVER;
    // Never entered: holds nowhere, whatever the setting.
    CHECK(!pin_lock_logic::holds(never, T0, false));
    CHECK(!pin_lock_logic::holds(never, T0, true));
    // Entered at T0: until PlayGuard closes by default...
    CHECK(pin_lock_logic::holds(T0, T0, false));
    CHECK(pin_lock_logic::holds(T0, T0 + std::chrono::hours(5), false));
    // ...5 minutes with "Ask for the PIN again".
    CHECK(pin_lock_logic::holds(T0, T0 + minutes(4) + seconds(59), true));
    CHECK(!pin_lock_logic::holds(T0, T0 + minutes(5), true));
    // A clock that has not gone far since its epoch: still not "entered".
    CHECK(!pin_lock_logic::holds(never, Clock::time_point{}, false));
}

static void test_check()
{
    const Clock::time_point never = pin_lock_logic::NEVER;
    // Only "changes" asks: "open" asks once at the start, not per change.
    for (bool again : { false, true }) {
        CHECK(pin_lock_logic::check("off", T0, never, again, never) == Check::Allow);
        CHECK(pin_lock_logic::check("open", T0, never, again, never) == Check::Allow);
        CHECK(pin_lock_logic::check("bogus", T0, never, again, never) == Check::Allow);
        CHECK(pin_lock_logic::check("changes", T0, never, again, never) == Check::Ask);
        // Even with a refusal held: other modes never refuse.
        CHECK(pin_lock_logic::check("off", T0, never, again, T0 + seconds(1)) == Check::Allow);
    }

    // The PIN entered at T0: not asked again until PlayGuard closes.
    CHECK(pin_lock_logic::check("changes", T0, T0, false, never) == Check::Allow);
    CHECK(pin_lock_logic::check("changes", T0 + std::chrono::hours(3), T0, false, never) == Check::Allow);

    // With "Ask for the PIN again": no asking for 5 minutes, then again.
    CHECK(pin_lock_logic::check("changes", T0, T0, true, never) == Check::Allow);
    CHECK(pin_lock_logic::check("changes", T0 + minutes(4) + seconds(59), T0, true, never) == Check::Allow);
    CHECK(pin_lock_logic::check("changes", T0 + minutes(5), T0, true, never) == Check::Ask);
    CHECK(pin_lock_logic::check("changes", T0 + minutes(6), T0, true, never) == Check::Ask);

    // Refused at T0: the next changes of the same action are refused without
    // asking, for 3 seconds.
    const Clock::time_point refused = T0 + pin_lock_logic::REFUSAL_HOLDS;
    CHECK(pin_lock_logic::check("changes", T0, never, false, refused) == Check::Refuse);
    CHECK(pin_lock_logic::check("changes", T0 + seconds(2), never, false, refused) == Check::Refuse);
    CHECK(pin_lock_logic::check("changes", T0 + seconds(3), never, false, refused) == Check::Ask);

    // A PIN that still holds wins over a refusal (it was entered since).
    CHECK(pin_lock_logic::check("changes", T0, T0, true, refused) == Check::Allow);
    CHECK(pin_lock_logic::check("changes", T0, T0, false, refused) == Check::Allow);
}

static void test_change()
{
    // Turning it on needs a PIN on the console.
    CHECK(pin_lock_logic::change("off", 1, false) == Change::NoPin);
    CHECK(pin_lock_logic::change("off", 2, false) == Change::NoPin);
    CHECK(pin_lock_logic::change("changes", 2, false) == Change::NoPin);
    CHECK(pin_lock_logic::change("open", 1, false) == Change::NoPin);   // down, but still on
    // More protection: no PIN asked.
    CHECK(pin_lock_logic::change("off", 1, true) == Change::Allowed);
    CHECK(pin_lock_logic::change("off", 2, true) == Change::Allowed);
    CHECK(pin_lock_logic::change("changes", 2, true) == Change::Allowed);
    // Less protection: the PIN first.
    CHECK(pin_lock_logic::change("open", 1, true) == Change::AskPin);
    CHECK(pin_lock_logic::change("open", 0, true) == Change::AskPin);
    CHECK(pin_lock_logic::change("changes", 0, true) == Change::AskPin);
    // Turning it off with no PIN left on the console: nothing to ask.
    CHECK(pin_lock_logic::change("changes", 0, false) == Change::Allowed);
    CHECK(pin_lock_logic::change("open", 0, false) == Change::Allowed);
    // An unknown setting counts as off.
    CHECK(pin_lock_logic::change("bogus", 0, true) == Change::Allowed);
    CHECK(pin_lock_logic::change("bogus", 1, true) == Change::Allowed);
}

static void test_lock_again()
{
    const Clock::time_point never = pin_lock_logic::NEVER;
    const Clock::time_point pin_before = T0 - minutes(30);   // entered on the lock screen at start
    // By default, entered once is enough: no lock screen again, however long away.
    CHECK(!pin_lock_logic::lock_again("open", T0, T0 + minutes(5), pin_before, false));
    CHECK(!pin_lock_logic::lock_again("open", T0, T0 + std::chrono::hours(3), pin_before, false));
    // "Ask for the PIN again": five minutes or more out of focus brings it back.
    CHECK(pin_lock_logic::lock_again("open", T0, T0 + minutes(5), pin_before, true));
    CHECK(pin_lock_logic::lock_again("open", T0, T0 + std::chrono::hours(3), pin_before, true));
    CHECK(pin_lock_logic::lock_again("open", T0, T0 + minutes(5), never, true));
    // A shorter look at the HOME menu does not.
    CHECK(!pin_lock_logic::lock_again("open", T0, T0 + minutes(4) + seconds(59), pin_before, true));
    CHECK(!pin_lock_logic::lock_again("open", T0, T0, pin_before, true));
    // The PIN entered while away (its own screen takes the focus): no lock.
    CHECK(!pin_lock_logic::lock_again("open", T0, T0 + minutes(8), T0 + minutes(7), true));
    CHECK(!pin_lock_logic::lock_again("open", T0, T0 + minutes(8), T0, true));
    // The other modes never show the lock screen ("changes" asks after its grace).
    CHECK(!pin_lock_logic::lock_again("changes", T0, T0 + minutes(30), pin_before, true));
    CHECK(!pin_lock_logic::lock_again("off", T0, T0 + minutes(30), pin_before, true));
    CHECK(!pin_lock_logic::lock_again("bogus", T0, T0 + minutes(30), pin_before, true));
}

int main()
{
    test_rank();
    test_holds();
    test_check();
    test_change();
    test_lock_again();
    return CHECK_DONE("pin_lock_logic mode, hold, grace, refusal hold, mode-change and lock-again assertions passed");
}
