// others_update — the update check made for all the Boréale homebrews
// installed: their registry's repositories (config/boreale/updates.conf,
// boreale-link's bl_updates), at most once a day whoever made the last one,
// so the bells show another homebrew's update without the network.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "util/update.hpp"

namespace others_update
{

// After PlayGuard's own check (`own` its result), on the check's thread
// (blocking is fine): PlayGuard's newest release goes in with the others'.
void check(const update::Result& own);

}   // namespace others_update
