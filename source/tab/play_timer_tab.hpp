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

#include "ui/cadran_list.hpp"
#include "tab/tab_base.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"
#include "view/cadran_week.hpp"
#include "view/pt_state_header.hpp"

class PlayTimerTab : public TabBase
{
  public:
    PlayTimerTab();
    ~PlayTimerTab() override;
    static brls::View* create();

  protected:
    void refresh() override;   // the console read again; edits not saved yet stay

  private:
    PtState pt = {};   // the state the last refresh read
    cadran::List list;
    uint16_t     pending[7] = {};
    brls::ActionIdentifier save_action = 0;   // + while there is something to save
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

    BRLS_BIND(PtStateHeader,     state_header,    "pt_state_header");
    BRLS_BIND(brls::Box,         top,             "pt_top");
    BRLS_BIND(brls::Label,       fw_note,         "pt_fw_note");
    BRLS_BIND(brls::Header,      limit_header,    "pt_limit_header");
    BRLS_BIND(brls::DetailCell,  split,           "pt_split");
    BRLS_BIND(brls::DetailCell,  today,           "pt_today");
    BRLS_BIND(brls::Label,       limit_note,      "pt_limit_note");
    BRLS_BIND(brls::Header,      lock_header,     "pt_lock_header");
    BRLS_BIND(brls::DetailCell,  console_lock_cell, "pt_console_lock");
    BRLS_BIND(brls::Label,       console_lock_note, "pt_console_lock_note");
    BRLS_BIND(brls::Header,      bedtime_header,  "pt_bedtime_header");
    BRLS_BIND(brls::DetailCell,  bedtime_reset,   "pt_bedtime_reset");
    BRLS_BIND(brls::Label,       bedtime_note,    "pt_bedtime_note");
    BRLS_BIND(brls::Header,      extra_header,    "pt_extra_header");
    BRLS_BIND(brls::DetailCell,  extra_amounts,   "pt_extra_amounts");
    BRLS_BIND(brls::BooleanCell, extra_auto,      "pt_extra_auto");
    BRLS_BIND(brls::DetailCell,  advanced,        "pt_advanced");
    BRLS_BIND(brls::DetailCell,  remove_cell,     "pt_remove");
    BRLS_BIND(brls::Label,       kicker,          "pt_kicker");
    BRLS_BIND(CadranDigits,      summary,         "pt_week_summary");
    BRLS_BIND(brls::Box,         profile_chip,    "pt_profile");
    BRLS_BIND(brls::Label,       profile_name,    "pt_profile_name");
    BRLS_BIND(brls::Box,         uniform_chip,    "pt_uniform");
    BRLS_BIND(CadranWeekView,    week_editor,     "cpt_week");
    BRLS_BIND(brls::Box,         keys_idle,       "cpt_keys_idle");
    BRLS_BIND(brls::Box,         played_key,      "cpt_played_key");
    BRLS_BIND(brls::Box,         over_key,        "cpt_over_key");
    BRLS_BIND(brls::Label,       played_text,     "cpt_played_text");
    BRLS_BIND(brls::Box,         keys_edit,       "cpt_keys_edit");
    BRLS_BIND(brls::Rectangle,   pending_dot,     "pt_pending_dot");
    BRLS_BIND(brls::Label,       pending_text,    "pt_pending");
    BRLS_BIND(brls::Button,      discard_button,  "pt_discard");
    BRLS_BIND(brls::Button,      save_button,     "pt_save");
    BRLS_BIND(brls::Box,         bed_chip,        "pt_bed");
    BRLS_BIND(CadranDigits,      bed_value,       "pt_bed_value");
    BRLS_BIND(CadranIcon,        bed_arrow,       "pt_bed_arrow");
    BRLS_BIND(CadranDigits,      bed_end,         "pt_bed_end");
    BRLS_BIND(brls::Box,         time_up_chip,    "pt_time_up");
    BRLS_BIND(brls::Label,       time_up_value,   "pt_time_up_value");
};
