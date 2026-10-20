// clock_flow — measuring the network clock against public NTP servers and
// setting it (or, with no server, setting it to the console clock), shared
// by Console › Network clock, Today's network clock card and the
// first-steps screen (one implementation of the measurement, the 2-minute
// validity and the write with its before / after report).
// NTP sampling and the time:s write are adapted from anbingxi's fork.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

#include "core/time_ops.h"

namespace clock_flow
{

// A measurement stays usable this long (the clock drifts, the user walks away).
constexpr int64_t LIFETIME_S = 120;

struct Measurement
{
    bool ok = false;               // at least one server answered
    uint64_t unix_seconds = 0;     // median server time at `at`
    std::chrono::steady_clock::time_point at;
    int64_t spread = 0;            // max - min between the servers, seconds
    std::string server;            // the primary server
    std::string report;            // one line per server, then the median and the offset
    std::string warning;           // "" or the servers-disagree line
    bool offset_known = false;     // the network clock could be read
    int64_t offset_s = 0;          // the server time minus the network clock, at `at`
};

// The last measurement any screen made (or the launch API), kept in
// nand_cache_dir()/clock_measure.json: for GET /v1/clock?cached=1.
struct LastMeasure
{
    int64_t offset_s = 0;
    std::string server;
    uint64_t at_utc = 0;   // the server time when it was measured
};
bool last_measure(LastMeasure* out);

// The NTP server the preferences name (or the console region's default).
std::string current_server();

// Queries `server` and two cross-check servers in parallel, off the UI thread;
// `done(m)` runs on the UI thread (m.ok false when no server answered).
// `timeout_ms`: the longest each server is waited for (half per address,
// two addresses at most).
void measure(const std::string& server, std::function<void(const Measurement&)> done, int timeout_ms = 5000);

int64_t  seconds_left(const Measurement& m);   // <= 0: expired
uint64_t projected_now(const Measurement& m);  // the server time, now

// Asks "Set the network clock to …?", saves a "before" report, writes the
// clock, verifies it and saves the "after" report. `done(message)` gets the
// sentence for the user (also shown in a dialog); nothing when cancelled.
void apply(const Measurement& m, std::function<void(const std::string& message)> done);

// The launch API's write (action/api_flow): no question (the PIN was asked
// before), the same reports and history entry as apply(). `done(a, message)`
// on the UI thread.
void write(const Measurement& m, std::function<void(const TimeApply& a, const std::string& message)> done);

// For a console no server answers (offline_clock_logic): asks "Set the
// network clock to the console clock, …?" with what the parent must check
// first, then the same reports and write as apply(), from the console clock.
// Also with "Synchronize" off: set first, turning it on no longer moves the
// console clock. Writes the clock only, not the parental controls.
void apply_console_clock(std::function<void(const std::string& message)> done);

// The guided path from Today and the first steps: offers to measure
// with the current server (a spinner while it runs, which Cancel or B
// closes, the late result then dropped), then to set the clock
// from the result, or from the console clock when no server answered.
// `done` runs after the clock changed (to refresh).
void guided(std::function<void()> done);

}   // namespace clock_flow
