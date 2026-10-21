// sigpatch_flow — sets aside the sigpatch files (.ips for es, fs and the
// loader) left beside sys-patch (util/patches.hpp: mixed, set_aside): two
// tools patching the same checks, which GBAtemp's sys-patch and sigpatch
// threads give as a cause of forwarders that "could not be started". The
// files are moved to config/playguard/sigpatches-aside/<when>/, never
// deleted, after a confirmation and the PIN; nothing is downloaded or
// installed. The console must restart for sys-patch to patch alone.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

namespace sigpatch_flow
{

// Whether there are sigpatch files to set aside now (patches::can_set_aside).
bool offered();

// Asks, then moves them. `done(moved)` once it is over, also when cancelled.
// `intro`, when given, is said first in the same question (what was found).
void set_aside(std::function<void(bool)> done = nullptr, const std::string& intro = "");

}   // namespace sigpatch_flow
