// BorealeHubTab — what PlayGuard shares with the other Boréale homebrews:
// Remote access (the MQTT / Home Assistant link), the optional modules (the
// Boréale agent, the recovery module) and the homebrews installed on the
// console. They were in Console and About.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

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
    AURORE_BIND(aurore::DetailCell, remote,      "bh_remote");
    AURORE_BIND(aurore::DetailCell, rescue,      "bh_rescue");
    AURORE_BIND(aurore::DetailCell, agent,       "bh_agent");
    AURORE_BIND(aurore::DetailCell, boreale,     "bh_boreale");
};
