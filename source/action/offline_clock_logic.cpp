// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/offline_clock_logic.hpp"

namespace offline_clock_logic
{

bool worth_writing(uint64_t console_now)
{
    return console_now >= PLAUSIBLE_FROM;
}

Offer offer(bool server_answered, const TimeSnapshot& s)
{
    if (server_answered) return Offer::None;
    if (R_FAILED(s.service_rc) || R_FAILED(s.user_rc)) return Offer::Unreadable;
    if (!worth_writing(s.user_time)) return Offer::Implausible;
    return Offer::Copy;
}

Notes notes(const TimeSnapshot& s)
{
    Notes n;
    const bool user_ok = R_SUCCEEDED(s.service_rc) && R_SUCCEEDED(s.user_rc);
    const bool network_ok = R_SUCCEEDED(s.service_rc) && R_SUCCEEDED(s.network_rc);
    const bool sync_on = R_SUCCEEDED(s.automatic_rc) && s.automatic;
    n.never_set = !network_ok;
    n.turn_sync_on = !sync_on;
    if (user_ok && network_ok) {
        // The difference as unsigned first: it cannot overflow there.
        n.network_off_s = (int64_t)(s.network_time - s.user_time);
        const uint64_t gap = s.network_time >= s.user_time ? s.network_time - s.user_time
                                                           : s.user_time - s.network_time;
        n.follows_network = sync_on && gap <= (uint64_t)SAME_S;
    }
    return n;
}

}   // namespace offline_clock_logic
