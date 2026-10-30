// app_paths — PlayGuard's own folders and files, beside the ones every app
// has (util/paths.hpp), in the same namespace.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace paths
{

// In logs_dir(): the developer tools' play-timer files.
std::string play_timer_dir();
std::string profiles_dir();
std::string exports_dir();
// In nand_dir(): the settings backups, and config's per-system part
// (util/config.hpp).
std::string backups_dir();
std::string state_file();

}   // namespace paths
