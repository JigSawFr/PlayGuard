// CadranChildActivity — the child screen (Today › Show the child):
// what the parent shows before handing the console over. One big ring of
// the time left today, when it ends (amber, the ring beating, in the last
// five minutes), the bedtime, what the console does when time is up, nothing
// to change; at the same size whatever the text size, read from the couch. A goes back to the parent once the PIN is entered (at
// once without a PIN on the console), B leaves PlayGuard so the child can
// play (asking first to lock again while unlocked, as quitting does).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"
#include "view/pt_ring.hpp"

class CadranChildActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/cadran_child.xml");

    void onContentAvailable() override;

  private:
    brls::RepeatingTimer timer;
    bool was_counting = false;   // the last refresh saw the timer counting (the time up after it: one breath)

    void refresh();

    BRLS_BIND(PtRing,       ring,     "child_ring");
    BRLS_BIND(CadranDigits, bedtime,  "child_bedtime");
    BRLS_BIND(brls::Box,    bed_chip, "child_bed_chip");
    BRLS_BIND(CadranIcon,   bed_icon, "child_bed_icon");
    BRLS_BIND(brls::Label,  time_up,  "child_time_up");
    BRLS_BIND(brls::Box,    end_chip, "child_end_chip");
    BRLS_BIND(CadranIcon,   end_icon, "child_end_icon");
    BRLS_BIND(brls::Label,  end_text, "child_end");
};
