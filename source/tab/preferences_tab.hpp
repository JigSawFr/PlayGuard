// PreferencesTab — PlayGuard itself: language, theme, text size and start
// tab as tiles, visual effects, the sidebar and the intro; the kinds of notifications and
// the monthly "Support PlayGuard" reminder; how PlayGuard updates; the first
// steps again. A setting that shapes one feature lives in that feature's
// tab; the change history and backups have their own (History), checking
// for updates now and Help & diagnostics are in About.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

#include "tab/tab_base.hpp"

class PreferencesTab : public TabBase
{
  public:
    PreferencesTab();
    static aurore::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    AURORE_BIND(aurore::DetailCell,  language,      "pf_language");
    AURORE_BIND(aurore::DetailCell,  theme,         "pf_theme");
    AURORE_BIND(aurore::DetailCell,  palette,       "pf_palette");
    AURORE_BIND(aurore::DetailCell,  start_tab,     "pf_start_tab");
    AURORE_BIND(aurore::DetailCell,  text_size,     "pf_text_size");
    AURORE_BIND(aurore::DetailCell,  effects,       "pf_effects");
    AURORE_BIND(aurore::DetailCell,  rail,          "pf_rail");
    AURORE_BIND(aurore::BooleanCell, intro,         "pf_intro");
    AURORE_BIND(aurore::BooleanCell, support_reminder, "pf_support_reminder");
    AURORE_BIND(aurore::BooleanCell, notices_system, "pf_notices_system");
    AURORE_BIND(aurore::BooleanCell, notices_summary, "pf_notices_summary");
    AURORE_BIND(aurore::BooleanCell, notices_reminders, "pf_notices_reminders");
    AURORE_BIND(aurore::BooleanCell, notices_ack_remind, "pf_notices_ack_remind");
    AURORE_BIND(aurore::BooleanCell, notices_others, "pf_notices_others");
    AURORE_BIND(aurore::BooleanCell, look_shared, "pf_look_shared");
    AURORE_BIND(aurore::BooleanCell, update_daily,   "pf_update_daily");
    AURORE_BIND(aurore::DetailCell,  update_via,     "pf_update_via");
    AURORE_BIND(aurore::DetailCell,  first_steps,    "pf_first_steps");
};
