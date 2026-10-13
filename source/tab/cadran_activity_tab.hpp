// CadranActivityTab — "Cadran" prototype of Activity, after the board: the
// accounts as chips, three figures, the games of the last seven days as
// cards with a sparkline, the games to rediscover. The same play data as
// the Activity tab (action/play_data), the same period (today, the last
// seven days, all time; kept in config like there) and the same export.
// PLAYGUARD_SIM_CADRAN=1 on the desktop, Tools › Developer › Cadran prototype
// on the console; strings not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <map>
#include <memory>
#include <vector>

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"

class CadranActivityTab : public TabBase
{
  public:
    CadranActivityTab();
    ~CadranActivityTab() override;
    static brls::View* create();

  protected:
    void refresh() override;

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
    int account = -1;    // index in play_data::accounts(), -1 every account
    int period = 1;      // activity_summary::Period: the games' order and figures
    int listener = 0;
    std::vector<brls::Box*> chips;
    std::vector<Card> cards;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    std::shared_ptr<const PlayStats> stats() const;
    void build_chips();
    void load_icons();
    void apply_icons();

    BRLS_BIND(brls::Box,    accounts_row, "ac_accounts");
    BRLS_BIND(brls::Box,    period_chip,  "ac_period");
    BRLS_BIND(brls::Label,  period_label, "ac_period_label");
    BRLS_BIND(brls::Box,    export_chip,  "ac_export");
    BRLS_BIND(brls::Label,  games_title,  "ac_games_title");
    BRLS_BIND(CadranDigits, today_value,  "ac_today");
    BRLS_BIND(brls::Label,  today_sub,    "ac_today_sub");
    BRLS_BIND(CadranDigits, week_value,   "ac_week");
    BRLS_BIND(brls::Label,  week_sub,     "ac_week_sub");
    BRLS_BIND(brls::Box,    week_bars,    "ac_week_bars");
    BRLS_BIND(CadranDigits, total_value,  "ac_total");
    BRLS_BIND(brls::Label,  total_sub,    "ac_total_sub");
    BRLS_BIND(brls::Box,    games_grid,   "ac_games");
    BRLS_BIND(brls::Label,  games_note,   "ac_games_note");
    BRLS_BIND(brls::Box,    rediscover,   "ac_rediscover");
};
