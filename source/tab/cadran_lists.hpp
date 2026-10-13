// The "Cadran" prototype of the list screens: Clock, Security, Preferences,
// Tools and About as they are (same lines, actions, i18n strings), restyled
// by cadran::List after the boards, each with what the boards put at its top:
// Clock, both clocks large; Security, its three states as tiles; Preferences,
// the app's three choices as tiles; Tools, its everyday actions as tiles and
// the console's readings; About, the app's name and version. The header
// pill says each screen's state. Strings added here are not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/about_tab.hpp"
#include "tab/cadran_list.hpp"
#include "tab/clock_tab.hpp"
#include "tab/preferences_tab.hpp"
#include "tab/security_tab.hpp"
#include "tab/tools_tab.hpp"
#include "view/cadran_digits.hpp"

class CadranClockTab : public ClockTab
{
  public:
    CadranClockTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    brls::Box*    network_card = nullptr;
    CadranDigits* user_time = nullptr;
    CadranDigits* network_time = nullptr;
    brls::Label*  user_date = nullptr;
    brls::Label*  network_state = nullptr;
    brls::Rectangle* network_dot = nullptr;
};

class CadranSecurityTab : public SecurityTab
{
  public:
    CadranSecurityTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
};

class CadranPreferencesTab : public PreferencesTab
{
  public:
    CadranPreferencesTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
};

class CadranToolsTab : public ToolsTab
{
  public:
    CadranToolsTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
};

class CadranAboutTab : public AboutTab
{
  public:
    CadranAboutTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    brls::Label* version_line = nullptr;
};
