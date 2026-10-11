// nand — which system PlayGuard runs on, so what belongs to one system stays
// with it: the sysMMC, or one of the emuMMCs hekate can keep on the SD card
// (each with its own parental controls). Its data folder is
// data_dir()/nand/<key> (paths::nand_dir()): the change history, watch.json,
// the backups, the play-data cache and state.json (util/config.hpp). The
// preferences, the profiles, the exports and the logs are shared.
// Plain C++ (no libnx): host-tested (tests/log_files).
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

// The emuMMC an emummc.ini (hekate's, sd:/emuMMC/emummc.ini) describes, by
// where it is: its path without the leading "emuMMC/" ("SD00", "RAW1"), else
// its start sector ("sector-80000"); lower case, letters, digits, "-" and
// "_" only. "" when the file names neither.
std::string emummc_where(const std::string& ini_text);

// "sysmmc", "emummc-<where>" ("emummc" when emummc.ini says nothing usable)
// or "unknown". `ini_text`: emummc.ini's content ("" when unreadable).
std::string key(Storage storage, const std::string& ini_text);

}   // namespace nand
