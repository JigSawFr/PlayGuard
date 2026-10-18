// PlayTimerTab — the week chart (A on a day opens the per-day editor on it),
// daily limit (same for every day / extra time today / weekdays and weekend /
// remove), saved profiles, what happens when the time is up, the bedtime
// alarm, the extra-time settings (amounts offered, the usual limit put back
// the next day by itself) and a line to the advanced (debug-class) actions.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"
#include "view/pt_state_header.hpp"
#include "view/pt_week.hpp"

class PlayTimerTab : public TabBase
{
  public:
    PlayTimerTab();
    static brls::View* create();

  protected:
    void refresh() override;

    PtState pt = {};   // the state the last refresh read

  private:

    void remove_limit();

    BRLS_BIND(brls::DetailCell,  unlocked_banner, "pt_unlocked_banner");
    BRLS_BIND(PtStateHeader,     state_header,    "pt_state_header");
    BRLS_BIND(brls::Header,      week_header,     "pt_week_header");
    BRLS_BIND(PtWeekView,        week,            "pt_week");
    BRLS_BIND(brls::Label,       fw_note,         "pt_fw_note");
    BRLS_BIND(brls::Header,      limit_header,    "pt_limit_header");
    BRLS_BIND(brls::DetailCell,  quick,           "pt_quick");
    BRLS_BIND(brls::DetailCell,  extra,           "pt_extra");
    BRLS_BIND(brls::DetailCell,  stop,            "pt_stop");
    BRLS_BIND(brls::DetailCell,  split,           "pt_split");
    BRLS_BIND(brls::DetailCell,  remove,          "pt_remove");
    BRLS_BIND(brls::DetailCell,  profiles_cell,   "pt_profiles");
    BRLS_BIND(brls::DetailCell,  time_up,         "pt_time_up");
    BRLS_BIND(brls::Label,       limit_note,      "pt_limit_note");
    BRLS_BIND(brls::Header,      bedtime_header,  "pt_bedtime_header");
    BRLS_BIND(brls::DetailCell,  bedtime,         "pt_bedtime");
    BRLS_BIND(brls::DetailCell,  bedtime_reset,   "pt_bedtime_reset");
    BRLS_BIND(brls::Label,       bedtime_note,    "pt_bedtime_note");
    BRLS_BIND(brls::Header,      extra_header,    "pt_extra_header");
    BRLS_BIND(brls::DetailCell,  extra_amounts,   "pt_extra_amounts");
    BRLS_BIND(brls::BooleanCell, extra_auto,      "pt_extra_auto");
    BRLS_BIND(brls::DetailCell,  advanced,        "pt_advanced");
};
