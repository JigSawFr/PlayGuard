// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/clock_logic.hpp"

#include <algorithm>

namespace clock_logic
{

int64_t estimate(const ntp::Reply& r, std::chrono::steady_clock::time_point ref)
{
    return (int64_t)r.unix_seconds + std::chrono::duration_cast<std::chrono::seconds>(ref - r.received_at).count();
}

Summary summarize(const std::vector<ntp::Reply>& replies, std::chrono::steady_clock::time_point ref,
                  std::optional<uint64_t> network_time)
{
    Summary s;
    s.offset_known = network_time.has_value();
    std::vector<int64_t> estimates;
    for (const auto& r : replies) {
        Host h;
        h.ok = r.ok;
        if (r.ok) {
            h.estimate = estimate(r, ref);
            if (s.offset_known) h.offset = h.estimate - (int64_t)*network_time;
            estimates.push_back(h.estimate);
        }
        s.hosts.push_back(h);
    }
    if (estimates.empty()) return s;
    std::sort(estimates.begin(), estimates.end());
    s.ok = true;
    // Median; an even count takes the mean of the two middle samples
    // (the difference as unsigned: hi >= lo, so it cannot overflow).
    const size_t mid = estimates.size() / 2;
    int64_t median   = estimates[mid];
    if (estimates.size() % 2 == 0) {
        const int64_t lo = estimates[mid - 1];
        median = lo + (int64_t)(((uint64_t)estimates[mid] - (uint64_t)lo) / 2);
    }
    s.median = (uint64_t)median;
    s.spread = estimates.back() - estimates.front();
    if (s.offset_known) s.offset = (int64_t)s.median - (int64_t)*network_time;
    return s;
}

}   // namespace clock_logic
