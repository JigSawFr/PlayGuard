// TitleState — where a state the header's pill says on every tab is undone,
// for a tap on that pill (cadran::refresh_pill): while parental controls are
// unlocked, Today › Turn back on; in read-only mode (an amber "Read-only"
// with a crossed-out pencil, like the other states), the firmware screen
// (read-only for an untested firmware) or the developer switch in Help &
// diagnostics.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace TitleState
{
enum class Kind { UNLOCKED, READ_ONLY };

// Where `kind` is undone, as a tap on the header's pill goes.
void open(Kind kind);
}   // namespace TitleState
