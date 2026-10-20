// PtStateHeader — at the top of the Play timer tab and the per-day editor, a
// warning line while the day's limit is reached, and one while the console is
// not counting play time (action/timer_health, fed by each show()); nothing
// otherwise (the state itself is the header's pill and the week's).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/pctl_ops_c.hpp"

class PtStateHeader : public brls::Box
{
  public:
    PtStateHeader();

    void show(const PtState& pt);      // renders a state its screen read

    static brls::View* create();

  private:
    BRLS_BIND(brls::Label, alert,   "pt_alert");
    BRLS_BIND(brls::Label, health,  "pt_health");
};
