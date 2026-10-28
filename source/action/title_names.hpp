// title_names — what each title the play log names is, and what to call
// it, for the screens and the remote link: util/titles' record (the
// forwarders, PlayGuard's own and the others, kept with their names after
// they are deleted) read once, brought up to date after each read of the
// play log and when PlayGuard makes a forwarder, and saved off the UI thread
// with the agent's copy (nand/<system>/sync/titles.txt). UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include "sync/sync_titles.h"
#include "util/pctl_ops_c.hpp"

namespace title_names
{

// A game, a homebrew (a forwarder; PlayGuard's own told apart) or the
// console's own program.
SyncTitleKind kind(uint64_t id);
// The name the record keeps (a forwarder deleted since), "" none.
std::string recorded(uint64_t id);

// After a read of the play log: what it names (the forwarders' names from
// their control data), the installed titles (the last check of them) and the
// forwarders PlayGuard remembers making.
void learn(const PlayStats& stats);
// A forwarder PlayGuard just made: a homebrew by this name (`self`:
// PlayGuard's own).
void learn_forwarder(uint64_t id, const std::string& name, bool self);

}   // namespace title_names
