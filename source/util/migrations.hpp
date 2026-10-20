// migrations — the moves of the data folder's earlier layouts
// (util/data_move.hpp, log_files::migrate), run once each instead of at
// every start: the last one done is kept in data_dir()/migrations (a number),
// and a start runs only the steps above it, in order. On the SD card each
// look for a file costs about a millisecond, and the old names are missing at
// nearly every start. Plain C++ (no borealis): host-tested (tests/migrations).
//
// A new layout is a new step with the next number; a step whose shim goes
// (data_move's and log_files::migrate's, after April 2027) leaves its number
// unused, never given to another step. A file missing or unreadable reads as
// 0: every step again, which is harmless (each one moves only what it finds).
// A number above every step (a later build ran, then this one again: the
// previous build of action/rollback_flow.hpp) runs none and is kept as it is.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace migrations
{

// The steps, in the order they run. Their numbers never change.
enum Level : int
{
    None      = 0,
    DataDir   = 1,   // sd:/switch/playguard to sd:/config/playguard (data_move::run, the console only)
    LogsDirs  = 2,   // logs/ before its subfolders (log_files::migrate)
    NandDirs  = 3,   // what every system shared, to the first one's nand/<key> (data_move::to_nand)
    Latest    = NandDirs,
};

struct Step
{
    int                     level;
    const char*             name;   // for the log
    std::function<size_t()> run;    // how many entries it moved
};

// The last step done, from `file`: 0 when it is missing or unreadable.
int done(const std::string& file);

// Each step of `steps` (in ascending order) above the one `file` records,
// then the last one run written to `file` (once, after them; not when none
// ran). `log(step, moved)` after each one, when given. Returns how many steps
// ran. A step runs again at the next start when the file could not be
// written.
size_t run(const std::string& file, const std::vector<Step>& steps,
           const std::function<void(const Step&, size_t)>& log = nullptr);

}   // namespace migrations
