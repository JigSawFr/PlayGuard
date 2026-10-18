// CadranChildActivity — the Cadran child screen (Overview › Show the child):
// what the parent shows before handing the console over. One big ring of
// the time left today, the bedtime, what the console does when time is up,
// nothing to change. A goes back to the parent once the PIN is entered (at
// once without a PIN on the console), B leaves PlayGuard so the child can
// play (asking first to lock again while unlocked, as quitting does).
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
    BRLS_BIND(brls::Label,  time_up,  "child_time_up");
};
