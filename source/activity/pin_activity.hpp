// PinActivity — Security › PIN: set or change the PIN, show it, protect
// PlayGuard with it, ask it again, let it pause the parental controls.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>

#include <memory>

#include "ui/play_ui.hpp"

class PinActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/pin.xml");

    void onContentAvailable() override;
    void onResume() override;

  private:
    std::unique_ptr<cadran::List> list;
    // False once the screen is gone: a flow ending after it touches nothing.
    ui::Alive alive = ui::make_alive();

    void refresh();
    void change_pin();

    AURORE_BIND(aurore::DetailCell,  set_pin,   "pn_set");
    AURORE_BIND(aurore::DetailCell,  show_pin,  "pn_show");
    AURORE_BIND(aurore::DetailCell,  lock_cell, "pn_lock");
    AURORE_BIND(aurore::BooleanCell, ask_again, "pn_ask_again");
    AURORE_BIND(aurore::BooleanCell, pauses,    "pn_pauses");
    AURORE_BIND(aurore::Label,       lock_note, "pn_lock_note");
};
