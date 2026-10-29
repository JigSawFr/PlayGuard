// PtStateHeader — at the top of the Play timer tab and the per-day editor, a
// warning line while the day's limit is reached, and one while the console is
// not counting play time (action/timer_health, fed by each show()); nothing
// otherwise (the state itself is the header's pill and the week's).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include "util/pctl_ops_c.hpp"

class PtStateHeader : public aurore::Box
{
  public:
    PtStateHeader();

    void show(const PtState& pt);      // renders a state its screen read

    static aurore::View* create();

  private:
    AURORE_BIND(aurore::Label, alert,   "pt_alert");
    AURORE_BIND(aurore::Label, health,  "pt_health");
};
