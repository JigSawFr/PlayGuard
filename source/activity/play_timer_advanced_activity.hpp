// PlayTimerAdvancedActivity — Play timer › Advanced: the switch that shows
// the advanced (debug-class) actions, next to them: the "Time's up" alarm,
// pausing and resuming the countdown. The same switch lets backups and the
// history put the alarm back. In developer mode, the diagnostic too.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include "util/pctl_ops_c.hpp"

class PlayTimerAdvancedActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/play_timer_advanced.xml");

    void onContentAvailable() override;

  private:
    PtState pt = {};

    void refresh();

    AURORE_BIND(aurore::BooleanCell, show,   "pa_show");
    AURORE_BIND(aurore::Label,       note,   "pa_note");
    AURORE_BIND(aurore::BooleanCell, alarm,  "pa_alarm");
    AURORE_BIND(aurore::DetailCell,  pause,  "pa_pause");
    AURORE_BIND(aurore::DetailCell,  resume, "pa_resume");
    AURORE_BIND(aurore::DetailCell,  diag,   "pa_diag");
};
