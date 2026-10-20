// ApiSettingsActivity — Preferences › Other homebrews (action/api_flow,
// docs/api.md): whether other homebrews may call PlayGuard, each group of
// routes' permission (the groups this build answers), the last calls, and
// one press to set every group back to "Not allowed". Opened behind the PIN.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>
#include <vector>

class ApiSettingsActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/api_settings.xml");

    void onContentAvailable() override;

  private:
    std::vector<std::pair<std::string, brls::DetailCell*>> groups;

    void refresh();
    void choose(const std::string& group);

    BRLS_BIND(brls::BooleanCell, enabled, "as_enabled");
    BRLS_BIND(brls::Label,       note,    "as_note");
    BRLS_BIND(brls::Box,         perms,   "as_perms");
    BRLS_BIND(brls::Box,         calls,   "as_calls");
    BRLS_BIND(brls::DetailCell,  all_off, "as_all_off");
};
