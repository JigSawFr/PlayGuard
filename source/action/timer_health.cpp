// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/timer_health.hpp"

#include <borealis.hpp>
#include <chrono>

#include "action/session_pause.hpp"
#include "action/timer_health_logic.hpp"
#include "app.hpp"
#include "core/sim_env.h"
#include "ui/ui.hpp"

namespace timer_health
{

namespace
{
timer_health_logic::State s_state;

// How long the time left may stay still on screen: STALL_MS, or
// PLAYGUARD_SIM_STALL_S seconds in the simulator (the smoke test's "outside"
// scenario, which would otherwise wait 90 s).
uint64_t stall_ms()
{
    static const uint64_t ms = [] {
        const long long s = sim_env_int("PLAYGUARD_SIM_STALL_S", 0, 0, 24 * 3600);
        return s > 0 ? (uint64_t)s * 1000 : timer_health_logic::STALL_MS;
    }();
    return ms;
}
}

bool observe(const PtState& pt)
{
    SysInfo si;
    sysinfo_get(&si);   // cached after the first call
    const int wd = ui::today_weekday();
    // Paused for the parent here, the time left stays still on purpose.
    const bool should = !session_pause::active() && timer_health_logic::should_count(pt, wd, si.applet_mode);
    const uint64_t now_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now().time_since_epoch()).count();
    const bool was = s_state.stalled;
    const bool stalled = timer_health_logic::step(s_state, should, should ? (uint64_t)pt.remaining_ns : 0, should ? pt.day_min[wd] : 0,
                                                  now_ms, app::in_focus(), stall_ms());
    if (stalled != was) brls::Logger::info("timer health: {}", stalled ? "not counting" : "counting");
    return stalled;
}

}   // namespace timer_health
