// CadranChildActivity — the child screen (Today › Show the child):
// what the parent shows before handing the console over. One big ring of
// the time left today, when it ends (amber, the ring beating, in the last
// five minutes), the bedtime, what the console does when time is up, nothing
// to change; at the same size whatever the text size, read from the couch. A goes back to the parent once the PIN is entered (at
// once without a PIN on the console), B leaves PlayGuard so the child can
// play (asking first to lock again while unlocked, as quitting does).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include <aurore/cadran/view/cadran_digits.hpp>
#include <aurore/cadran/view/cadran_icon.hpp>

#include "action/session_pause.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/pt_ring.hpp"

class CadranChildActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/cadran_child.xml");

    void onContentAvailable() override;

  private:
    aurore::RepeatingTimer timer;
    session_pause::Hold hold;   // the child's: play counts as usual while it is up
    bool was_counting = false;   // the last refresh saw the timer counting (the time up after it: one breath)

    void refresh();

    AURORE_BIND(PtRing,       ring,     "child_ring");
    AURORE_BIND(CadranDigits, bedtime,  "child_bedtime");
    AURORE_BIND(aurore::Box,    bed_chip, "child_bed_chip");
    AURORE_BIND(CadranIcon,   bed_icon, "child_bed_icon");
    AURORE_BIND(aurore::Label,  time_up,  "child_time_up");
    AURORE_BIND(aurore::Box,    end_chip, "child_end_chip");
    AURORE_BIND(CadranIcon,   end_icon, "child_end_icon");
    AURORE_BIND(aurore::Label,  end_text, "child_end");
};
