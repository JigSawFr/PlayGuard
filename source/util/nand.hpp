// nand — which system PlayGuard runs on, so what belongs to one system stays
// with it: the sysMMC or the emuMMC, each with its own parental controls.
// Its data folder is data_dir()/nand/<key> (paths::nand_dir()): the change
// history, watch.json, the backups, the play-data cache and state.json
// (util/config.hpp). The preferences, the profiles, the exports and the logs
// are shared. Plain C++ (no libnx): host-tested (tests/log_files).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace nand
{

enum class Storage
{
    Unknown,   // Atmosphère not detected, or the emuMMC state not read
    SysMMC,
    EmuMMC,
};

// "sysmmc", "emummc" or "unknown". Every emuMMC of an SD card shares
// "emummc": hekate can keep several, but which one runs is not worth telling
// apart here.
std::string key(Storage storage);

}   // namespace nand
