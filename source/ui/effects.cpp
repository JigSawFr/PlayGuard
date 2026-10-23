// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/effects.hpp"

#include <chrono>
#include <cstdlib>
#include <string>

#include "core/sim_env.h"
#include "util/config.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace effects
{

namespace
{
constexpr auto READ_EVERY = std::chrono::seconds(5);

// psm opened for the read and closed (as the other services).
bool read_battery()
{
#ifdef __SWITCH__
    if (R_FAILED(psmInitialize())) return false;
    PsmChargerType charger = PsmChargerType_EnoughPower;
    const bool ok = R_SUCCEEDED(psmGetChargerType(&charger));
    psmExit();
    return ok && charger == PsmChargerType_Unconnected;
#else
    return sim_env_flag("PLAYGUARD_SIM_BATTERY");
#endif
}
}   // namespace

bool on_battery()
{
    // Asked at every frame by the views that draw an effect: read again only
    // every few seconds.
    static bool battery = false;
    static std::chrono::steady_clock::time_point at;
    static bool read = false;
    const auto now = std::chrono::steady_clock::now();
    if (!read || now - at > READ_EVERY) {
        battery = read_battery();
        at = now;
        read = true;
    }
    return battery;
}

bool light()
{
    return config::get().effects == "auto" && on_battery();
}

bool reduced()
{
    return config::get().effects == "reduced";
}

bool motion()
{
    // The smoke test's screenshots compare frames: nothing may depend on when
    // they are taken (the same flag holds the focus highlight still).
    static const bool still = sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS");
    if (still) return false;
    const std::string& setting = config::get().effects;
    if (setting == "reduced") return false;
    return setting != "auto" || !on_battery();
}

}   // namespace effects
