// BorealeHubTab — what PlayGuard shares with the other Boréale homebrews:
// Remote access (the MQTT / Home Assistant link), the optional modules (the
// Boréale agent, the recovery module) and the homebrews installed on the
// console, listed: PlayGuard first, each with its version; Ⓐ on another one
// closes PlayGuard and starts it (hbloader's next load), or says to open it
// from the homebrew menu when it cannot be started from here (an applet, no
// .nro known). They were in Console and About.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/boreale_apps_logic.hpp"
#include "tab/tab_base.hpp"

class BorealeHubTab : public TabBase
{
  public:
    BorealeHubTab();
    static aurore::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    std::string seen;   // what the homebrews list shows: built again when it changes

    aurore::DetailCell* homebrew_cell(const boreale_apps_logic::App& app);
    void refresh_homebrews();
    AURORE_BIND(aurore::DetailCell, remote,      "bh_remote");
    AURORE_BIND(aurore::DetailCell, rescue,      "bh_rescue");
    AURORE_BIND(aurore::DetailCell, agent,       "bh_agent");
    AURORE_BIND(aurore::Box,        homebrews,   "bh_list");
};
