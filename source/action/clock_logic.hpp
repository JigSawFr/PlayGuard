// clock_logic — the maths of a network clock measurement: every server's
// answer brought to the same instant, their median, how far apart they are
// and the network clock's offset from them. Plain C++ over ntp::Reply so the
// host tests can run it (tests/clock_logic); the words stay in clock_flow.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "util/ntp_client.hpp"

namespace clock_logic
{

// One server's answer, in the order asked.
struct Host
{
    bool ok = false;
    int64_t estimate = 0;   // its time at the reference instant, when ok
    int64_t offset = 0;     // estimate - network clock, when ok and the clock was read
};

struct Summary
{
    bool ok = false;               // at least one server answered
    uint64_t median = 0;           // the answers' median at the reference instant
    int64_t spread = 0;            // max - min between the answers, seconds
    bool offset_known = false;     // the network clock was read
    int64_t offset = 0;            // median - network clock, seconds (< 0: ahead)
    std::vector<Host> hosts;       // one per reply
};

// A reply's time at `ref`: its time plus how long ago it came in.
int64_t estimate(const ntp::Reply& r, std::chrono::steady_clock::time_point ref);

// The replies brought to `ref`, against the network clock read then
// (`network_time` empty when it could not be read).
Summary summarize(const std::vector<ntp::Reply>& replies, std::chrono::steady_clock::time_point ref,
                  std::optional<uint64_t> network_time);

}   // namespace clock_logic
