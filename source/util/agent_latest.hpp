// agent_latest — what the agent last saw of the newest versions and of the
// console's clocks: sd:/config/boreale/latest.conf (boreale-link's bl_latest),
// written by the agent only, and only when its settings ask for the checks
// (agent.conf, off by default). PlayGuard reads it to show an update or a
// clock that drifted without asking the network itself; with the agent off,
// not installed or not running, the file is absent or old, and PlayGuard
// checks for itself (sys_update_flow, clock_check).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include "util/sys_updates.hpp"

namespace agent_latest
{

// How long a reading holds before PlayGuard checks for itself: twice what the
// agent waits between two of its checks (12 h, 2 h), so one missed round does
// not make PlayGuard ask too.
constexpr int64_t VERSIONS_FRESH_S = 24 * 3600;
constexpr int64_t CLOCK_FRESH_S    = 4 * 3600;

struct Reading
{
    sys_updates::Latest versions;       // "" when unknown
    std::string integration;            // boreale-ha, "" unknown
    int64_t     checked = 0;            // Unix time the versions were read, 0 never
    int64_t     clock_checked = 0;      // the same for the clocks
    int         clock_network = 0;      // seconds the network clock is ahead of the server (negative: behind)
    int         clock_user = 0;         // the same for the user clock
    std::string clock_server;
};

// The file's text; an empty reading for text of a newer schema (not read, as
// the format's rule says) or that is not the format.
Reading parse(const std::string& text);

// Whether the reading's versions / clocks were taken less than their limit
// before `now` (Unix time). Never taken, or in the future (a clock set back):
// no.
bool versions_fresh(const Reading& r, int64_t now);
bool clock_fresh(const Reading& r, int64_t now);

// The file on the SD card, read at most every 30 s (the tiles ask often);
// an empty reading when it is not there.
Reading read();

}   // namespace agent_latest
