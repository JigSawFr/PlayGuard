// appearance_flow — after an update from a version before the Cadran
// appearance: the new look said once, with Classic one press away, so the
// parent who knew the lists is not left looking for them.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

namespace appearance_flow
{
// At the main screen's start, once (config appearance_offered), when no other
// screen of its own comes first this start.
void at_start(brls::Activity* main);
}   // namespace appearance_flow
