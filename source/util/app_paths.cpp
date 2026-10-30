// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/app_paths.hpp"

#include "util/paths.hpp"

namespace paths
{

std::string play_timer_dir() { return logs_dir() + "/play_timer"; }
std::string profiles_dir()   { return data_dir() + "/profiles"; }
std::string exports_dir()    { return data_dir() + "/exports"; }
std::string backups_dir()    { return nand_dir() + "/backups"; }
std::string state_file()     { return nand_dir() + "/state.json"; }

}   // namespace paths
