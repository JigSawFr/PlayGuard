// CadranPlayTimerTab — Play timer after its board: the week's limits in a
// sentence, the profile and "same limit every day" chips, the week as the
// editor (CadranWeekView: A on a day edits it with the stick, the changes are
// saved together, as in the per-day editor), then bedtime, time's up and the
// removal as chips. It is PlayTimerTab: the lines a chip stands for stay the
// tab's, out of sight, so the chip runs their code and checks; the others
// (extra time, stop for today, weekdays and weekend, when play is allowed
// again, the extra-time settings, Advanced) stay lines under it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/cadran_list.hpp"
#include "tab/play_timer_tab.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_week.hpp"

class CadranPlayTimerTab : public PlayTimerTab
{
  public:
    CadranPlayTimerTab();
    static brls::View* create();

  protected:
    void refresh() override;   // the console read again; edits not saved yet stay

  private:
    cadran::List list;
    brls::View*  hero = nullptr;
    uint16_t     pending[7] = {};
    brls::ActionIdentifier save_action = 0;   // + while there is something to save

    bool changes() const;
    void rerender();
    void save();
    void discard();

    BRLS_BIND(brls::Label,     kicker,        "cpt_kicker");
    BRLS_BIND(CadranDigits,    summary,       "cpt_summary");
    BRLS_BIND(brls::Box,       profile_chip,  "cpt_profile");
    BRLS_BIND(brls::Label,     profile_name,  "cpt_profile_name");
    BRLS_BIND(brls::Box,       uniform_chip,  "cpt_uniform");
    BRLS_BIND(CadranWeekView,  week_editor,   "cpt_week");
    BRLS_BIND(brls::Box,       keys_idle,     "cpt_keys_idle");
    BRLS_BIND(brls::Box,       keys_edit,     "cpt_keys_edit");
    BRLS_BIND(brls::Rectangle, pending_dot,   "cpt_pending_dot");
    BRLS_BIND(brls::Label,     pending_text,  "cpt_pending");
    BRLS_BIND(brls::Button,    discard_button, "cpt_discard");
    BRLS_BIND(brls::Button,    save_button,   "cpt_save");
    BRLS_BIND(brls::Box,       bed_chip,      "cpt_bed");
    BRLS_BIND(CadranDigits,    bed_value,     "cpt_bed_value");
    BRLS_BIND(CadranIcon,      bed_arrow,     "cpt_bed_arrow");
    BRLS_BIND(CadranDigits,    bed_end,       "cpt_bed_end");
    BRLS_BIND(brls::Box,       time_up_chip,  "cpt_time_up");
    BRLS_BIND(brls::Label,     time_up_value, "cpt_time_up_value");
    BRLS_BIND(brls::Box,       remove_chip,   "cpt_remove");
};
