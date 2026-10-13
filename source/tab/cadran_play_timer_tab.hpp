// CadranPlayTimerTab — "Cadran" prototype of Play timer, after the board: the
// week is the editor (CadranWeekView, the stick sets each day), a live
// summary, the profile and "same limit every day" chips, bedtime / pause /
// alarm / remove chips, + saves. PLAYGUARD_SIM_CADRAN=1 on the desktop,
// Tools › Developer › Cadran prototype on the console; strings not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"
#include "view/cadran_week.hpp"

class CadranPlayTimerTab : public TabBase
{
  public:
    CadranPlayTimerTab();
    static brls::View* create();

  protected:
    void refresh() override;   // re-reads the console; pending edits are kept

  private:
    PtState  live = {};
    uint16_t pending[7] = {};
    bool     edited = false;   // pending differs from live somewhere

    bool has_changes() const;
    void rerender();
    void save();

    BRLS_BIND(CadranDigits,   summary,      "pt_summary");
    BRLS_BIND(brls::Box,      profile_chip, "pt_profile");
    BRLS_BIND(brls::Label,    profile_name, "pt_profile_name");
    BRLS_BIND(brls::Box,      uniform_chip, "pt_uniform");
    BRLS_BIND(CadranWeekView, week,         "pt_week");
    BRLS_BIND(brls::Rectangle, pending_dot, "pt_pending_dot");
    BRLS_BIND(brls::Label,    pending_text, "pt_pending");
    BRLS_BIND(brls::Box,      bed_chip,     "pt_bed");
    BRLS_BIND(CadranDigits,   bed_value,    "pt_bed_value");
    BRLS_BIND(CadranIcon,     bed_arrow,    "pt_bed_arrow");
    BRLS_BIND(CadranDigits,   bed_end,      "pt_bed_end");
    BRLS_BIND(brls::Box,      pause_chip,   "pt_pause");
    BRLS_BIND(brls::Label,    pause_text,   "pt_pause_text");
    BRLS_BIND(brls::Box,      alarm_chip,   "pt_alarm");
    BRLS_BIND(brls::Label,    alarm_state,  "pt_alarm_state");
    BRLS_BIND(brls::Box,      remove_chip,  "pt_remove");
};
