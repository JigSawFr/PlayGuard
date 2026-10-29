// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/title_state.hpp"

#include <aurore.hpp>

#include "action/console_actions.hpp"
#include "action/fw_gate.hpp"
#include "ui/app_hooks.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

namespace TitleState
{

static void open_unlocked()
{
    // Today, on "Turn back on".
    ui::open_tab(ui::tab::dashboard, "dash_relock");
}

static void open_paused()
{
    // Nothing to undo: it ends by itself as PlayGuard is left. Said, with a
    // way to turn them back on now (until the PIN is entered again).
    ui::confirm("playguard/cadran/paused/body"_i18n, "playguard/cadran/paused/relock"_i18n,
                []() { console_actions::relock("", []() { ui::refresh_shown_tab(); }); });
}

static void open_read_only()
{
    // Untested firmware: the screen that chose read-only, where the choice changes.
    const ui::app_hooks::Hooks& hooks = ui::app_hooks::get();
    if (fw_gate::needed()) {
        if (hooks.open_firmware_screen) hooks.open_firmware_screen();
        return;
    }
    // Else the developer switch turned it on: Help & diagnostics, on that switch.
    if (!hooks.make_help_tab) return;
    aurore::View* help = hooks.make_help_tab();
    ui::open_screen(help, "playguard/prefs/help"_i18n);
    aurore::sync([help]() {
        aurore::View* cell = help->getView("hp_dev_read_only");
        if (cell && cell->getVisibility() == aurore::Visibility::VISIBLE) aurore::Application::giveFocus(cell);
    });
}

void open(Kind kind)
{
    if (kind == Kind::UNLOCKED) open_unlocked();
    else if (kind == Kind::PAUSED) open_paused();
    else open_read_only();
}

}   // namespace TitleState
