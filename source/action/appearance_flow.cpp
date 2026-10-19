// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/appearance_flow.hpp"

#include <cstdlib>

#include "tab/cadran_tab.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace appearance_flow
{
namespace
{
// As What's new: the start-up questions are on top by then.
constexpr long START_DELAY_MS = 1500;
}   // namespace

void at_start(brls::Activity* main)
{
    // An appearance forced on the desktop (PLAYGUARD_SIM_APPEARANCE) is no choice.
    if (config::get().appearance_offered || !cadran::enabled() || std::getenv("PLAYGUARD_SIM_APPEARANCE")) return;
    if (!ui::claim_start_screen("the new look")) return;
    brls::delay(START_DELAY_MS, [main]() {
        if (!ui::on_top(main)) return;   // something else first: the next start
        // Said once, whatever the answer (B keeps Cadran).
        config::get().appearance_offered = true;
        ui::save_config();
        auto* dialog = ui::dialog("playguard/cadran/intro/body"_i18n);
        dialog->addButton("playguard/cadran/intro/classic"_i18n, []() {
            config::get().appearance = "classic";
            ui::save_config();
            ui::offer_restart();
        });
        dialog->addButton("playguard/cadran/intro/keep"_i18n, []() {});
        dialog->setCancelable(true);
        cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
        dialog->open();
        // A pressed to close it keeps the new look: the focus on Keep.
        if (brls::View* keep = dialog->getView("brls/dialog/button2")) brls::Application::giveFocus(keep);
    });
}
}   // namespace appearance_flow
