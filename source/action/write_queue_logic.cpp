// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/write_queue_logic.hpp"

namespace write_queue_logic
{

Queue::Ticket Queue::push(const std::string& key, Job job, bool* start)
{
    *start = false;
    std::unique_lock<std::mutex> lock(lock_);
    if (closed_ && runner_ == Runner::None) {
        // Past the exit flush: no runner will come.
        lock.unlock();
        job();
        return 0;
    }
    Item item{ key, std::move(job), { ++last_ } };
    if (!key.empty()) {
        for (auto it = items_.begin(); it != items_.end(); ++it) {
            if (it->key != key) continue;
            item.tickets.insert(item.tickets.end(), it->tickets.begin(), it->tickets.end());
            items_.erase(it);
            break;
        }
    }
    pending_.insert(last_);
    items_.push_back(std::move(item));
    if (runner_ == Runner::None && !closed_) {
        runner_ = Runner::Asked;
        *start  = true;
    }
    return last_;
}

void Queue::run_locked(std::unique_lock<std::mutex>& lock)
{
    runner_ = Runner::Running;
    while (!items_.empty()) {
        Item item = std::move(items_.front());
        items_.pop_front();
        lock.unlock();
        // One job failing must not stop the ones behind it.
        try {
            item.job();
        } catch (...) {
        }
        lock.lock();
        for (Ticket t : item.tickets) pending_.erase(t);
        changed_.notify_all();
    }
    runner_ = Runner::None;
    changed_.notify_all();
}

void Queue::run()
{
    std::unique_lock<std::mutex> lock(lock_);
    if (runner_ != Runner::Asked) return;   // taken over by wait() or flush()
    run_locked(lock);
}

void Queue::wait(Ticket ticket)
{
    std::unique_lock<std::mutex> lock(lock_);
    if (!pending_.count(ticket)) return;
    if (runner_ == Runner::Asked) run_locked(lock);
    changed_.wait(lock, [this, ticket]() { return !pending_.count(ticket); });
}

void Queue::settle()
{
    Ticket last;
    {
        std::lock_guard<std::mutex> lock(lock_);
        last = last_;
    }
    // The newest job is the last in the queue: once it has run, so has every other.
    wait(last);
}

bool Queue::flush(std::chrono::milliseconds max)
{
    std::unique_lock<std::mutex> lock(lock_);
    closed_ = true;
    if (runner_ == Runner::Asked) run_locked(lock);
    return changed_.wait_for(lock, max, [this]() { return runner_ == Runner::None; });
}

}   // namespace write_queue_logic
