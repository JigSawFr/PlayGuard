// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/worker_threads.hpp"

#include <exception>
#include <iterator>

WorkerThreads::~WorkerThreads()
{
    std::list<Worker> over;
    {
        std::unique_lock<std::mutex> lock(lock_);
        changed_.wait(lock, [this]() { return running_locked() == 0; });
        over = take_over_locked();
    }
    join(over);
}

bool WorkerThreads::start(Task task, std::string* error)
{
    std::list<Worker> over;
    bool ok = true;
    {
        std::lock_guard<std::mutex> lock(lock_);
        over = take_over_locked();
        // Its place in the list first: once the thread runs, nothing here may
        // throw before it is kept (a std::thread destroyed joinable ends the
        // app).
        workers_.emplace_back();
        Worker& w = workers_.back();
        w.done    = std::make_shared<bool>(false);
        try {
            w.thread = std::thread([this, task, done = w.done]() {
                task();
                // Said under the lock, so a wait sees the task over and the
                // thread about to end together.
                std::lock_guard<std::mutex> ended(lock_);
                *done = true;
                changed_.notify_all();
            });
        } catch (const std::exception& e) {   // std::system_error: no thread to be had
            workers_.pop_back();
            if (error) *error = e.what();
            ok = false;
        }
    }
    // Joined outside the lock: what a task's end destroys may start a task.
    join(over);
    return ok;
}

int WorkerThreads::running()
{
    std::lock_guard<std::mutex> lock(lock_);
    return running_locked();
}

bool WorkerThreads::wait_all(std::chrono::milliseconds max)
{
    std::list<Worker> over;
    bool done = false;
    {
        std::unique_lock<std::mutex> lock(lock_);
        done = changed_.wait_for(lock, max, [this]() { return running_locked() == 0; });
        over = take_over_locked();
    }
    join(over);
    return done;
}

int WorkerThreads::running_locked() const
{
    int n = 0;
    for (const Worker& w : workers_)
        if (!*w.done) n++;
    return n;
}

std::list<WorkerThreads::Worker> WorkerThreads::take_over_locked()
{
    std::list<Worker> over;
    for (auto it = workers_.begin(); it != workers_.end();) {
        const auto next = std::next(it);
        if (*it->done) over.splice(over.end(), workers_, it);
        it = next;
    }
    return over;
}

void WorkerThreads::join(std::list<Worker>& workers)
{
    // Each task is over: its thread ends at once.
    for (Worker& w : workers) w.thread.join();
}
