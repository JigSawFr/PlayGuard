// Host tests for source/action/session_pause_logic.cpp: when PlayGuard's own
// pause of parental controls is wanted.
#include "check.h"

#include "action/session_pause_logic.hpp"

using session_pause_logic::Inputs;
using session_pause_logic::wanted;

static Inputs parent_in()
{
    Inputs in;
    in.enabled = true;
    in.pin_holds = true;
    in.in_focus = true;
    in.application = true;
    return in;
}

int main()
{
    // The parent in PlayGuard, PIN entered: paused.
    CHECK(wanted(parent_in()));

    // Off by default: the PIN alone pauses nothing.
    CHECK(!wanted(Inputs()));
    Inputs in = parent_in();
    in.enabled = false;
    CHECK(!wanted(in));

    in = parent_in();
    in.pin_holds = false;   // no PIN entered yet, or no longer holding
    CHECK(!wanted(in));

    in = parent_in();
    in.in_focus = false;   // the HOME menu, sleep
    CHECK(!wanted(in));

    in = parent_in();
    in.holds = 1;   // the child's screen
    CHECK(!wanted(in));
    in.holds = 2;
    CHECK(!wanted(in));

    in = parent_in();
    in.application = false;   // from the album
    CHECK(!wanted(in));

    in = parent_in();
    in.read_only = true;
    CHECK(!wanted(in));

    in = parent_in();
    in.quitting = true;
    CHECK(!wanted(in));

    return CHECK_DONE("session_pause_logic assertions passed");
}
