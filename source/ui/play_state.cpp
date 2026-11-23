// ui — the unlock the main screen's title says (PlayGuard only).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include <aurore.hpp>

#include "action/sync_flow.hpp"
#include "ui/play_ui.hpp"

namespace ui
{

static bool s_unlocked = false;
static bool s_paused = false;     // that unlock is session_pause's

bool known_unlocked() { return s_unlocked; }

bool known_paused() { return s_paused && s_unlocked; }

void note_paused(bool paused)
{
    if (paused == s_paused) return;
    s_paused = paused;
    aurore::sync([]() { refresh_title(); });
}

void note_unlocked(bool valid, bool unlocked)
{
    if (!valid || unlocked == s_unlocked) return;
    s_unlocked = unlocked;
    sync_flow::changed();
    // Next frame: at start-up the first tab is read before the main screen is
    // on the activity stack.
    aurore::sync([]() { refresh_title(); });
}

}   // namespace ui
