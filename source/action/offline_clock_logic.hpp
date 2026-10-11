// offline_clock_logic — what to offer when no time server answers. A console
// without Internet cannot measure its network clock, and turning on
// "Synchronize Clock via Internet" first makes the console clock jump to that
// (wrong) network clock (docs/parental-controls.md, "Clocks"). The way out is
// the other direction: set the network clock to the console clock, which the
// parent can check. Plain C++ over the service-layer struct so the host tests
// can run it (tests/offline_clock_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>

#include "util/pctl_ops_c.hpp"

namespace offline_clock_logic
{

// A console clock before this (2026-01-01 UTC, before this release) was
// never set, or reset with the battery: copying it would make things worse.
constexpr uint64_t PLAUSIBLE_FROM = 1767225600;
// Two clocks this close count as the same (reads a few IPCs apart).
constexpr int64_t SAME_S = 5;

enum class Offer
{
    None,         // a server answered: its measured time is the one to write
    Copy,         // "Set the network clock to the console clock"
    Unreadable,   // the console clock cannot be read: nothing to copy
    Implausible,  // the console clock reads before PLAUSIBLE_FROM: set it first
};

Offer offer(bool server_answered, const TimeSnapshot& s);

// What the confirmation adds to "Set the network clock to the console clock?".
struct Notes
{
    bool never_set = false;        // the network clock cannot be read (never set)
    // "Synchronize" is on and both clocks agree: the console clock may simply
    // be following a wrong network clock, so the parent must check it (and,
    // if it is wrong, turn the setting off and set the time by hand first).
    bool follows_network = false;
    // "Synchronize" is off (or unknown): turning it on afterwards no longer
    // moves the console clock, both clocks then being the same.
    bool turn_sync_on = false;
    int64_t network_off_s = 0;     // network minus console clock, when both were read
};

Notes notes(const TimeSnapshot& s);

// The time to write is the console clock read in the same time:s session,
// just before the write (time_clock_set_to_user), not the one shown in the
// confirmation, which went on ticking while it was open. This is the check
// on that value: false when it is not worth writing.
bool worth_writing(uint64_t console_now);

}   // namespace offline_clock_logic
