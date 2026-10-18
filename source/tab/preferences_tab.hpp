// PreferencesTab — PlayGuard itself: language, theme, appearance, start tab and the
// intro; the kinds of notifications and the monthly "Support PlayGuard"
// reminder. A setting that shapes one feature lives in that feature's tab;
// the updates and Help & diagnostics are in About.
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
    BRLS_BIND(brls::DetailCell,  appearance,    "pf_appearance");
    BRLS_BIND(brls::DetailCell,  start_tab,     "pf_start_tab");
    BRLS_BIND(brls::BooleanCell, intro,         "pf_intro");
    BRLS_BIND(brls::BooleanCell, support_reminder, "pf_support_reminder");
    BRLS_BIND(brls::BooleanCell, notices_system, "pf_notices_system");
    BRLS_BIND(brls::BooleanCell, notices_summary, "pf_notices_summary");
    BRLS_BIND(brls::BooleanCell, notices_reminders, "pf_notices_reminders");
    BRLS_BIND(brls::BooleanCell, notices_ack_remind, "pf_notices_ack_remind");
};
