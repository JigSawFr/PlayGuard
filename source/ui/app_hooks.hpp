// ui::app_hooks — what ui/ and view/ ask of the layers above them (a few
// screens and flows) without including them, besides what Aurore's Cadran
// asks (ui::hooks): those fill this in once at start-up
// (activity/screen_hooks.cpp), before any screen opens. Each hook left empty
// does nothing (or answers nothing). tools/check_sources.py fails on an
// include of activity/ or tab/ under source/ui, source/view or source/util.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>

namespace ui::app_hooks
{

struct Hooks
{
    std::function<void()>             open_notices;           // the notification bell's list
    std::function<void()>             open_firmware_screen;   // fw_gate's
    std::function<brls::View*()>      make_help_tab;          // Help & diagnostics, for a screen of its own
    std::function<void(const char* what, bool ok)> note_write;   // notices::note_write
    std::function<void()>             mode_changed;           // the recorders after the developer mode
    std::function<bool()>             lock_screen_up;         // activity/lock_activity's screen is open
};

// The one set, filled in at start-up.
Hooks& get();

}   // namespace ui::app_hooks
