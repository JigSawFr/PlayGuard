// CadranActivityTab — Activity after its board: the accounts as chips, the
// period and the export, three figures (today, the last seven days with
// their bars, all time), the four games most played over the period as cards
// with their week as a sparkline (A: the game's own screen). It is
// ActivityTab: the same play data, account, period and export; the lines a
// chip or a figure stands for stay the tab's, out of sight; the chart, the
// summary, All games and the notes stay lines under it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <memory>
#include <vector>

#include "tab/activity_tab.hpp"
#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "view/cadran_digits.hpp"

class CadranActivityTab : public ActivityTab
{
  public:
    CadranActivityTab();
    ~CadranActivityTab() override;
    static brls::View* create();

  protected:
    void refresh() override;
    void rebuild() override;

  private:
    struct Card
    {
        uint64_t      app_id = 0;
        brls::Box*    box    = nullptr;
        brls::Image*  icon   = nullptr;
        brls::Box*    icon_fallback = nullptr;
        brls::Label*  name   = nullptr;
        CadranDigits* today  = nullptr;
        brls::Label*  rest   = nullptr;
        brls::Box*    bars[7] = {};
    };
    cadran::List list;
    std::vector<brls::Box*> chips;   // every account, then each one
    std::vector<Card> cards;
    // Nothing played over the period: why, and all time when it has some.
    cadran::kit::Empty empty;
    brls::Box* see_all = nullptr;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    void build_chips();
    void show_hero();
    void show_icons();

    BRLS_BIND(brls::Box,    accounts_row, "cac_accounts");
    BRLS_BIND(brls::Box,    period_chip,  "cac_period");
    BRLS_BIND(brls::Label,  period_label, "cac_period_label");
    BRLS_BIND(brls::Box,    export_chip,  "cac_export");
    BRLS_BIND(brls::Label,  games_title,  "cac_games_title");
    BRLS_BIND(CadranDigits, today_value,  "cac_today");
    BRLS_BIND(brls::Label,  today_sub,    "cac_today_sub");
    BRLS_BIND(CadranDigits, week_value,   "cac_week");
    BRLS_BIND(brls::Label,  week_sub,     "cac_week_sub");
    BRLS_BIND(brls::Box,    week_bars,    "cac_week_bars");
    BRLS_BIND(CadranDigits, total_value,  "cac_total");
    BRLS_BIND(brls::Label,  total_sub,    "cac_total_sub");
    BRLS_BIND(brls::Box,    games_grid,   "cac_games");
    BRLS_BIND(brls::Label,  games_note,   "cac_games_note");
};
