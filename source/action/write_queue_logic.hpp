// write_queue_logic — the SD-card writes of the UI (config.json, the change
// history, the play-timer log, the exports) in one queue, run one after the
// other off the UI thread, in the order they were asked for. It has no thread
// of its own: push() says when one must be started to run() it (ui uses
// in_background). Host-tested (tests/write_queue_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace write_queue_logic
{

class Queue
{
public:
    using Job    = std::function<void()>;
    using Ticket = std::uint64_t;   // 0: already run

    // Adds `job` at the back. A `key` that is not empty stands for a file
    // written whole (config.json): a job of the same key still waiting is
    // dropped, as this one writes newer content, and its ticket now waits for
    // this one. *start: no runner is on the queue, the caller must call run()
    // on a thread of its own. After flush(), `job` runs here and now.
    Ticket push(const std::string& key, Job job, bool* start);
    // The runner: runs the jobs until there are none left. Returns at once
    // when the queue was taken over (wait, flush) before it started.
    void run();
    // Until the job of `ticket` (or the newer one that replaced it) has run.
    // A runner asked for but not started yet: the jobs run here instead (a
    // waiter must not wait for a thread that may come late).
    void wait(Ticket ticket);
    // Until every job pushed so far has run.
    void settle();
    // At exit: waits up to `max` for the runner, runs here what is still
    // queued, and from then on runs each push() at once. False when the
    // runner was still busy after `max`.
    bool flush(std::chrono::milliseconds max);

private:
    struct Item
    {
        std::string key;
        Job job;
        std::vector<Ticket> tickets;
    };
    enum class Runner { None, Asked, Running };

    void run_locked(std::unique_lock<std::mutex>& lock);

    std::mutex lock_;
    std::condition_variable changed_;
    std::deque<Item> items_;
    std::set<Ticket> pending_;   // tickets whose job has not run yet
    Ticket last_   = 0;
    Runner runner_ = Runner::None;
    bool closed_   = false;
};

}   // namespace write_queue_logic
