// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/title_state.hpp"

#include <borealis.hpp>

#include "action/fw_gate.hpp"
#include "ui/hooks.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace TitleState
{

static void open_unlocked()
{
    // Today, on "Turn back on".
    ui::open_tab(ui::tab::dashboard, "dash_relock");
}

static void open_read_only()
{
    // Untested firmware: the screen that chose read-only, where the choice changes.
    const ui::hooks::Hooks& hooks = ui::hooks::get();
    if (fw_gate::needed()) {
        if (hooks.open_firmware_screen) hooks.open_firmware_screen();
        return;
    }
    // Else the developer switch turned it on: Help & diagnostics, on that switch.
    if (!hooks.make_help_tab) return;
    brls::View* help = hooks.make_help_tab();
    ui::open_screen(help, "playguard/prefs/help"_i18n);
    brls::sync([help]() {
        brls::View* cell = help->getView("hp_dev_read_only");
        if (cell && cell->getVisibility() == brls::Visibility::VISIBLE) brls::Application::giveFocus(cell);
    });
}

void open(Kind kind)
{
    if (kind == Kind::UNLOCKED) open_unlocked();
    else open_read_only();
}

}   // namespace TitleState
