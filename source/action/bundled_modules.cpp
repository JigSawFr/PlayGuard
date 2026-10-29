// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/bundled_modules.hpp"

#include <aurore.hpp>

#include "core/sim_env.h"

std::string bundled_modules::dir()
{
#ifndef __SWITCH__
    if (const char* sim = sim_env("PLAYGUARD_SIM_BUNDLED")) return sim;
#endif
    return AURORE_ASSET("sysmodules");
}
