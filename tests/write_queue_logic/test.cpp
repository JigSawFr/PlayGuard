// Host tests for source/action/write_queue_logic.cpp: the SD-card writes run
// in the order asked, a file written whole keeps only its newest content, a
// waiter sees its write done (running it itself when the runner has not
// started), and nothing queued is lost at exit.
#include "check.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

#include "action/write_queue_logic.hpp"

using write_queue_logic::Queue;
using namespace std::chrono_literals;

namespace
{
// What the jobs did, in order (they may run on another thread).
struct Log
{
    std::mutex lock;
    std::string text;
    void add(const std::string& s)
    {
        std::lock_guard<std::mutex> g(lock);
        text += s;
    }
    std::string get()
    {
        std::lock_guard<std::mutex> g(lock);
        return text;
    }
};
}   // namespace

static void test_order()
{
    Queue q;
    Log log;
    bool start = false;
    q.push("", [&]() { log.add("a"); }, &start);
    CHECK(start);   // the first one asks for a runner ...
    q.push("", [&]() { log.add("b"); }, &start);
    CHECK(!start);   // ... the next ones join it
    q.push("", [&]() { log.add("c"); }, &start);
    CHECK(!start);
    CHECK(log.get().empty());   // nothing runs on push
    q.run();
    CHECK(log.get() == "abc");
    // Idle again: the next push asks for a new runner.
    q.push("", [&]() { log.add("d"); }, &start);
    CHECK(start);
    q.run();
    CHECK(log.get() == "abcd");
    q.run();   // a runner with nothing to do
    CHECK(log.get() == "abcd");
}

static void test_latest_wins()
{
    Queue q;
    Log log;
    bool start = false;
    const Queue::Ticket first = q.push("config", [&]() { log.add("1"); }, &start);
    q.push("", [&]() { log.add("h"); }, &start);
    q.push("other", [&]() { log.add("o"); }, &start);
    const Queue::Ticket second = q.push("config", [&]() { log.add("2"); }, &start);
    CHECK(second > first);
    // The older config.json is not written; the newer one after what came between.
    q.run();
    CHECK(log.get() == "ho2");
    // The replaced job's waiter is not left hanging.
    q.wait(first);
    q.wait(second);
    // Appends (no key) are never merged.
    q.push("", [&]() { log.add("x"); }, &start);
    q.push("", [&]() { log.add("x"); }, &start);
    q.run();
    CHECK(log.get() == "ho2xx");
}

static void test_wait_takes_over()
{
    Queue q;
    Log log;
    bool start = false;
    q.push("", [&]() { log.add("a"); }, &start);
    const Queue::Ticket t = q.push("config", [&]() { log.add("b"); }, &start);
    // The runner was asked for but has not started: the waiter runs the jobs.
    q.wait(t);
    CHECK(log.get() == "ab");
    q.run();   // the runner, late: nothing left, nothing run twice
    CHECK(log.get() == "ab");
    q.wait(t);   // already run: returns at once
    q.wait(0);
    q.settle();
}

static void test_threads()
{
    Queue q;
    Log log;
    std::atomic<bool> started{ false }, release{ false };
    bool start = false;
    q.push("", [&]() {
        started = true;
        while (!release) std::this_thread::sleep_for(1ms);
        log.add("slow");
    }, &start);
    REQUIRE(start);
    std::thread runner([&q]() { q.run(); });
    while (!started) std::this_thread::sleep_for(1ms);   // the runner is on it: queue behind
    const Queue::Ticket t = q.push("config", [&]() { log.add("+cfg"); }, &start);
    CHECK(!start);
    std::thread waiter([&q, t]() { q.wait(t); });
    std::this_thread::sleep_for(20ms);
    CHECK(log.get().empty());   // still behind the slow one
    release = true;
    waiter.join();
    CHECK(log.get() == "slow+cfg");   // in order, done when wait() returned
    q.push("", [&]() { log.add("+h"); }, &start);
    q.settle();   // the runner may have gone: settle runs it here then
    CHECK(log.get() == "slow+cfg+h");
    runner.join();
}

static void test_flush()
{
    // Queued, the runner never started (no thread to be had at exit): run by flush.
    Queue q;
    Log log;
    bool start = false;
    q.push("", [&]() { log.add("h"); }, &start);
    q.push("config", [&]() { log.add("c"); }, &start);
    CHECK(q.flush(100ms));
    CHECK(log.get() == "hc");
    // After the flush, a write runs at once, on the caller's thread.
    const Queue::Ticket t = q.push("", [&]() { log.add("late"); }, &start);
    CHECK(!start);
    CHECK(t == 0);
    CHECK(log.get() == "hclate");
    q.run();   // a runner asked for before the flush: nothing left to do
    CHECK(log.get() == "hclate");

    // A busy runner: flush waits for it, and every job queued behind it runs.
    Queue busy;
    Log done;
    std::atomic<bool> started{ false }, release{ false };
    busy.push("", [&]() {
        started = true;
        while (!release) std::this_thread::sleep_for(1ms);
        done.add("a");
    }, &start);
    std::thread runner([&busy]() { busy.run(); });
    while (!started) std::this_thread::sleep_for(1ms);
    busy.push("", [&]() { done.add("b"); }, &start);
    CHECK(!busy.flush(20ms));   // still busy after `max`
    // Closed with the runner busy: queued for it, not run beside it.
    busy.push("", [&]() { done.add("c"); }, &start);
    CHECK(!start);
    release = true;
    CHECK(busy.flush(5000ms));
    CHECK(done.get() == "abc");
    runner.join();
}

static void test_throwing_job()
{
    Queue q;
    Log log;
    bool start = false;
    q.push("", []() { throw std::runtime_error("SD card gone"); }, &start);
    const Queue::Ticket t = q.push("", [&]() { log.add("next"); }, &start);
    q.run();
    CHECK(log.get() == "next");
    q.wait(t);
}

int main()
{
    test_order();
    test_latest_wins();
    test_wait_takes_over();
    test_threads();
    test_flush();
    test_throwing_job();
    return CHECK_DONE("write_queue_logic: all tests passed");
}
