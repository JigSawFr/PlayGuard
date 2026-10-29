// PlayTimerTab — the week's limits in a sentence, the profile and "same limit
// every day" chips, the week as the editor (CadranWeekView: A on a day edits
// it with the stick, the changes are saved together, as in the per-day
// editor; the time played in its bars), then bedtime, time's up and the
// removal as chips. Under them: weekdays and weekend, a line to Today (extra
// time, no more play today), "Block all games" (every day's limit at 0),
// when play is allowed again after bedtime, the extra-time
// settings (amounts offered, the usual limit put back the next day by
// itself) and a line to the advanced (debug-class) actions.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>
#include <aurore/cadran/view/cadran_digits.hpp>
#include <aurore/cadran/view/cadran_icon.hpp>

#include "tab/tab_base.hpp"
#include "view/cadran_week.hpp"
#include "view/pt_state_header.hpp"

class PlayTimerTab : public TabBase
{
  public:
    PlayTimerTab();
    ~PlayTimerTab() override;
    static aurore::View* create();

  protected:
    void refresh() override;   // the console read again; edits not saved yet stay

  private:
    PtState pt = {};   // the state the last refresh read
    cadran::List list;
    uint16_t     pending[7] = {};
    aurore::ActionIdentifier save_action = 0;   // + while there is something to save
    int          play_listener = -1;            // play_data: the week's time played
    uint32_t     played_min[7] = {};            // its minutes, per weekday
    bool         played_ok = false;

    void remove_limit();
    bool changes() const;
    void show_played();
    void show_played_key(int day);   // the focused day's figures, else the key
    void rerender();
    void save();
    void discard();

    AURORE_BIND(PtStateHeader,     state_header,    "pt_state_header");
    AURORE_BIND(aurore::Box,         top,             "pt_top");
    AURORE_BIND(aurore::Label,       fw_note,         "pt_fw_note");
    AURORE_BIND(aurore::Header,      limit_header,    "pt_limit_header");
    AURORE_BIND(aurore::DetailCell,  split,           "pt_split");
    AURORE_BIND(aurore::DetailCell,  today,           "pt_today");
    AURORE_BIND(aurore::Label,       limit_note,      "pt_limit_note");
    AURORE_BIND(aurore::Header,      lock_header,     "pt_lock_header");
    AURORE_BIND(aurore::DetailCell,  console_lock_cell, "pt_console_lock");
    AURORE_BIND(aurore::Label,       console_lock_note, "pt_console_lock_note");
    AURORE_BIND(aurore::Header,      bedtime_header,  "pt_bedtime_header");
    AURORE_BIND(aurore::DetailCell,  bedtime_reset,   "pt_bedtime_reset");
    AURORE_BIND(aurore::Label,       bedtime_note,    "pt_bedtime_note");
    AURORE_BIND(aurore::Header,      extra_header,    "pt_extra_header");
    AURORE_BIND(aurore::DetailCell,  extra_amounts,   "pt_extra_amounts");
    AURORE_BIND(aurore::BooleanCell, extra_auto,      "pt_extra_auto");
    AURORE_BIND(aurore::DetailCell,  advanced,        "pt_advanced");
    AURORE_BIND(aurore::DetailCell,  remove_cell,     "pt_remove");
    AURORE_BIND(aurore::Label,       kicker,          "pt_kicker");
    AURORE_BIND(CadranDigits,      summary,         "pt_week_summary");
    AURORE_BIND(aurore::Box,         profile_chip,    "pt_profile");
    AURORE_BIND(aurore::Label,       profile_name,    "pt_profile_name");
    AURORE_BIND(aurore::Box,         uniform_chip,    "pt_uniform");
    AURORE_BIND(CadranWeekView,    week_editor,     "cpt_week");
    AURORE_BIND(aurore::Box,         keys_idle,       "cpt_keys_idle");
    AURORE_BIND(aurore::Box,         played_key,      "cpt_played_key");
    AURORE_BIND(aurore::Box,         over_key,        "cpt_over_key");
    AURORE_BIND(aurore::Label,       played_text,     "cpt_played_text");
    AURORE_BIND(aurore::Box,         keys_edit,       "cpt_keys_edit");
    AURORE_BIND(aurore::Rectangle,   pending_dot,     "pt_pending_dot");
    AURORE_BIND(aurore::Label,       pending_text,    "pt_pending");
    AURORE_BIND(aurore::Button,      discard_button,  "pt_discard");
    AURORE_BIND(aurore::Button,      save_button,     "pt_save");
    AURORE_BIND(aurore::Box,         bed_chip,        "pt_bed");
    AURORE_BIND(CadranDigits,      bed_value,       "pt_bed_value");
    AURORE_BIND(CadranIcon,        bed_arrow,       "pt_bed_arrow");
    AURORE_BIND(CadranDigits,      bed_end,         "pt_bed_end");
    AURORE_BIND(aurore::Box,         time_up_chip,    "pt_time_up");
    AURORE_BIND(aurore::Label,       time_up_value,   "pt_time_up_value");
};
