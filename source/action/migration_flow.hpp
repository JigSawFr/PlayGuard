// migration_flow — the steps of util/migrations.hpp, with what they move
// (util/data_move.hpp, data_move::logs), in the log: those of the start,
// before anything is written to the data folder (the session file goes to
// the logs/ they leave), and the system's, once its folder is known. Then
// the developer tools' "Run the migrations again": the steps chosen in a
// list (all at first), on the SD-card write queue, after a confirmation. It
// moves files on the SD card only: allowed in read-only mode.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "util/migrations.hpp"

namespace migration_flow
{

// The steps not done yet of the start: the data folder of the versions that
// kept it next to the .nro, logs/ before its subfolders.
void at_start();

// What the versions before the per-system folders kept for every system, to
// this one's: the first system started takes it, once. After
// paths::set_nand().
void for_system();

// Going back to the previous build, which knows the steps up to `level` (-1:
// not known): each step done above it undoes what it moved, when it can
// (util/migrations.hpp undo_to), then every step runs again at the next start
// of a later build. None of PlayGuard's steps has an undo yet: the next step
// that moves something brings its own.
void going_back(int level, const migrations::Log& log);

// Developer tools › Run the migrations again: a line per step to check or
// uncheck (all checked at first), then one that runs those checked,
// whatever migrations.json records (it is never lowered); a dialog then says
// how many steps ran and how many entries they moved.
void replay();

}   // namespace migration_flow
