// bundled_modules — where the copies of the optional sysmodules PlayGuard
// carries are (cmake/bundle_sysmodules.cmake): romfs:/sysmodules/, or on the
// desktop the folder PLAYGUARD_SIM_BUNDLED names. Console › Optional
// modules, the recovery module's bell entry and the agent's update read them.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace bundled_modules
{

std::string dir();

}   // namespace bundled_modules
