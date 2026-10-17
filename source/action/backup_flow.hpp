// backup_flow — reading the console into a settings backup, and writing a
// backup back: restriction level (1033), the custom settings when the level is
// "Custom" (1036), VR mode (1063), then the per-day limits through the
// play-timer write gate (pt_flow::confirm_write / finish_write).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "util/backup.hpp"

namespace backup_flow
{

// The console's settings now; values that cannot be read stay unset.
backup::Snapshot capture();

// "Back up the settings": capture + save, then a toast with the file.
void save_now();

// The backups on the SD card, file names newest first (backup::list). Listed
// once, then again after a save here (which prunes too) and at each restore
// list (the Overview asks every 5 s, the History tab refreshes often).
const std::vector<std::string>& names();
// How many there are.
size_t count();

// "Restore a backup…": list of the backups → summary → confirmation → write.
void choose_and_restore(std::function<void()> refresh);

// Before "Delete all parental controls": saves a backup, then runs `next`.
// If the backup cannot be made, asks whether to go on without one.
void backup_then(std::function<void()> next);

}   // namespace backup_flow
