// profile_flow — a saved profile (util/profiles) put in force on the console:
// nothing written when its days are the console's already (a toast says so),
// else the one confirmation a week's change asks (pt_flow::confirm_write,
// the days changed under the question), then the write. Profiles' own screen
// and Cadran's Today use it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>

#include "util/profiles.hpp"

namespace profile_flow
{
// Puts `p`'s days in force; `after` once they are written (not when nothing
// had to be, nor on a refusal).
void apply(const profiles::Profile& p, std::function<void()> after);

// The saved profiles to pick one from, the one in force checked, each with
// its days; the last line opens Play timer › Profiles. Then apply().
void pick(std::function<void()> after);
}   // namespace profile_flow
