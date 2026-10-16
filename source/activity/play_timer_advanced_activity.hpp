// PlayTimerAdvancedActivity — Play timer › Advanced: the switch that shows
// the advanced (debug-class) actions, next to them: the "Time's up" alarm,
// pausing and resuming the countdown. The same switch lets backups and the
// history put the alarm back. In developer mode, the diagnostic too.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/pctl_ops_c.hpp"

class PlayTimerAdvancedActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/play_timer_advanced.xml");

    void onContentAvailable() override;

  private:
    PtState pt = {};

    void refresh();

    BRLS_BIND(brls::BooleanCell, show,   "pa_show");
    BRLS_BIND(brls::Label,       note,   "pa_note");
    BRLS_BIND(brls::BooleanCell, alarm,  "pa_alarm");
    BRLS_BIND(brls::DetailCell,  pause,  "pa_pause");
    BRLS_BIND(brls::DetailCell,  resume, "pa_resume");
    BRLS_BIND(brls::DetailCell,  diag,   "pa_diag");
};
