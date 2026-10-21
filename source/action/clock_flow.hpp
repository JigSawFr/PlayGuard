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
#include <vector>

namespace clock_flow
{

// A measurement stays usable this long (the clock drifts, the user walks away).
constexpr int64_t LIFETIME_S = 120;

// One server's answer, for the result's table.
struct ServerLine
{
    std::string host;
    bool ok = false;
    uint64_t unix_seconds = 0;     // its time at the measurement's `at`
    std::string offset;            // "-12 s" against the network clock, "?" unread
    std::string error;             // what went wrong, when not ok
};

struct Measurement
{
    bool ok = false;               // at least one server answered
    uint64_t unix_seconds = 0;     // median server time at `at`
    std::chrono::steady_clock::time_point at;
    int64_t spread = 0;            // max - min between the servers, seconds
    std::string server;            // the primary server
    std::string report;            // one line per server, then the median and the offset
    std::string warning;           // "" or the servers-disagree line
    std::vector<ServerLine> lines; // the servers asked, the primary first
    bool offset_known = false;     // the network clock was read
    int64_t offset = 0;            // median - network clock, seconds (< 0: ahead)
    uint64_t network_seconds = 0;  // the network clock at `at`
};

// What the network clock's offset amounts to, in words: "12 s ahead",
// "1 min 5 s behind", "on time" (within a second).
std::string offset_text(int64_t offset);

// The NTP server the preferences name (or the console region's default).
std::string current_server();

// Queries `server` and two cross-check servers in parallel, off the UI thread;
// `done(m)` runs on the UI thread (m.ok false when no server answered).
void measure(const std::string& server, std::function<void(const Measurement&)> done);

int64_t  seconds_left(const Measurement& m);   // <= 0: expired
uint64_t projected_now(const Measurement& m);  // the server time, now

// Asks "Set the network clock to …?", saves a "before" report, writes the
// clock, verifies it and saves the "after" report. `done(message)` gets the
// sentence for the user (also shown in a dialog); nothing when cancelled.
void apply(const Measurement& m, std::function<void(const std::string& message)> done);

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
