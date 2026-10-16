// PreferencesTab — PlayGuard itself: language, theme and start tab; the
// intro and the monthly "Support PlayGuard" reminder; the kinds of
// notifications; the daily update check and how to update. A setting that
// shapes one feature lives in that feature's tab.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"

class PreferencesTab : public TabBase
{
  public:
    PreferencesTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    BRLS_BIND(brls::DetailCell,  language,      "pf_language");
    BRLS_BIND(brls::DetailCell,  theme,         "pf_theme");
    BRLS_BIND(brls::DetailCell,  start_tab,     "pf_start_tab");
    BRLS_BIND(brls::BooleanCell, intro,         "pf_intro");
    BRLS_BIND(brls::BooleanCell, support_reminder, "pf_support_reminder");
    BRLS_BIND(brls::BooleanCell, notices_system, "pf_notices_system");
    BRLS_BIND(brls::BooleanCell, notices_summary, "pf_notices_summary");
    BRLS_BIND(brls::BooleanCell, notices_reminders, "pf_notices_reminders");
    BRLS_BIND(brls::BooleanCell, update_daily,  "pf_update_daily");
    BRLS_BIND(brls::DetailCell,  update_via,    "pf_update_via");
    BRLS_BIND(brls::Label,       note,          "pf_note");
};
