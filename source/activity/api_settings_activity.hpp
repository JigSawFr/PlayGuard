// ApiSettingsActivity — Preferences › Other homebrews (action/api_flow,
// docs/api.md): whether other homebrews may call PlayGuard, each group of
// routes' permission (the groups this build answers, each level said in
// words), a HOME-menu icon that sets the clock in one press, the last calls
// as cards by day (a press for the details), one press to set every group
// back to "Not allowed" (and to put them back while the screen is open),
// and the API's page for developers as a QR code. Opened behind the PIN.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <map>
#include <string>
#include <vector>

class ApiSettingsActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/api_settings.xml");

    void onContentAvailable() override;

  private:
    std::vector<std::pair<std::string, brls::DetailCell*>> groups;
    // What "Turn it all off" replaced, for "Put them back" (empty: nothing to put back).
    std::map<std::string, std::string> before_off;

    void refresh();
    void refresh_calls();
    void choose(const std::string& group);
    void turn_off_or_back();
    void add_clock_icon();

    BRLS_BIND(brls::BooleanCell, enabled, "as_enabled");
    BRLS_BIND(brls::Label,       note,    "as_note");
    BRLS_BIND(brls::Box,         perms,   "as_perms");
    BRLS_BIND(brls::Box,         actions, "as_actions");
    BRLS_BIND(brls::Box,         calls,   "as_calls");
    BRLS_BIND(brls::Box,         more,    "as_more");
    brls::DetailCell* home    = nullptr;   // in `actions`
    brls::DetailCell* all_off = nullptr;
    brls::DetailCell* docs    = nullptr;   // in `more`
};
