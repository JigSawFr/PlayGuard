// ui::hooks — what ui/ and view/ ask of the layers above them (the main
// screen, the tabs, a few screens and flows) without including them: those
// fill this in once at start-up (activity/screen_hooks.cpp), before any
// screen opens. Each hook left empty does nothing (or answers nothing).
// tools/check_sources.py fails on an include of activity/ or tab/ under
// source/ui, source/view or source/util.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>

namespace ui::hooks
{

struct Hooks
{
    // The main screen (activity/main_activity).
    std::function<bool(brls::Activity* activity)> is_main;   // `activity` is it
    std::function<void(brls::Activity* main, int position, const std::string& row)> open_tab;
    std::function<void(brls::Activity* main)> update_title;
    // A new main screen, pushed, on tab `position`, the focus on `row`.
    std::function<void(int position, const std::string& row)> push_main;

    // The tabs (tab/tab_base): TabBase's statics, and whether a view is a tab.
    std::function<void()>                 refresh_shown;
    std::function<std::string()>          shown_screen;
    std::function<std::string()>          main_screen;
    std::function<bool(brls::View* view)> is_tab;

    // Screens and flows.
    std::function<void()>             open_notices;           // the notification bell's list
    std::function<void()>             open_firmware_screen;   // fw_gate's
    std::function<brls::View*()>      make_help_tab;          // Help & diagnostics, for a screen of its own
    std::function<void()>             send_report;            // upload_flow::choose
    std::function<void(const char* what, bool ok)> note_write;   // notices::note_write
    std::function<void()>             mode_changed;           // the recorders after the developer mode
};

// The one set, filled in at start-up.
Hooks& get();

}   // namespace ui::hooks
