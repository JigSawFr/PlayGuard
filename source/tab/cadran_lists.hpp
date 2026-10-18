// The Cadran list screens: the network clock (a screen of its own), PIN,
// Console, History, Preferences and About as they are (same lines, actions,
// strings), restyled by cadran::List after the boards, each with what the
// boards put at its top: the clock, both clocks large; PIN, its three
// states as tiles; Console, the firmware, Atmosphère and compatibility as
// tiles; History, the backup's two actions as tiles; Preferences, the app's
// four choices as tiles; About, the app's name and version. The header pill
// says the state of the screens that have one.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/about_tab.hpp"
#include "tab/cadran_list.hpp"
#include "tab/clock_tab.hpp"
#include "tab/console_tab.hpp"
#include "tab/history_tab.hpp"
#include "tab/preferences_tab.hpp"
#include "tab/security_tab.hpp"
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

class CadranConsoleTab : public ConsoleTab
{
  public:
    CadranConsoleTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
};

class CadranHistoryTab : public HistoryTab
{
  public:
    CadranHistoryTab();
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
