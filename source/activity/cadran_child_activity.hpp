// CadranChildActivity — "Cadran" prototype, the child screen: what the parent
// shows before handing the console over. One big ring of the time left
// today, the bedtime, nothing to change. A goes back to the parent once the
// PIN is entered (at once without a PIN on the console), B leaves PlayGuard
// so the child can play. PLAYGUARD_SIM_CADRAN=1 on the desktop, Tools ›
// Developer › Cadran prototype on the console; strings not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/pt_ring.hpp"

class CadranChildActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/cadran_child.xml");

    void onContentAvailable() override;

  private:
    brls::RepeatingTimer timer;

    void refresh();

    BRLS_BIND(PtRing,       ring,     "child_ring");
    BRLS_BIND(CadranDigits, bedtime,  "child_bedtime");
    BRLS_BIND(brls::Box,    bed_chip, "child_bed_chip");
};
