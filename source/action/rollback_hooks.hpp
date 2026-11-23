// rollback_hooks — what PlayGuard adds to the kit's rollback_flow: the
// data's layout (its migrations), and the PIN asked before the previous build
// is put back. Installed first thing in main(), before rollback_flow::at_start.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace rollback_hooks
{

void install();

}   // namespace rollback_hooks
