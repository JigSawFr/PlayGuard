// ActivityTab — the accounts as chips, the period and the export / share, three
// figures (today, the last seven days with their bars, all time) from the
// console's play data, the four games most played over the period as cards
// with their week as a sparkline (A: the game's own screen). Under them the
// chart, the summary over the chosen period (util/activity_summary.hpp), the
// way to every game of the period (activity/games_activity: a list or
// covers) and when the week was played. The export / share chip writes to
// the SD card (CSV, JSON, XLSX or PDF) or shows the week as a QR code. Read-only, so also in read-only
// mode. The data comes from play_data (read off the main thread, shared with
// Today, and kept on the SD card for the next run): what is known
// shows at once, and is read again in the background when over a minute old,
// without a spinner; Ⓧ reads it again with one. The chart marks each day's
// current limit (every account only). It starts on the account shown last,
// or one chosen under "Account by default" (config's activity_account). A
// user account new to PlayGuard is flagged under the account filter
// (action/outside_watch).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_list.hpp>
#include <borealis/cadran/view/cadran_digits.hpp>

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/play_days.hpp"
#include "view/play_hours.hpp"

class ActivityTab : public TabBase
{
  public:
    ActivityTab();
    ~ActivityTab() override;
    static brls::View* create();

    // Asks for a format, then writes every game of `data` (most played over
    // `period` first) to the exports folder; `account` says whose figures
    // they are (nullptr: every account).
    static void export_stats(std::shared_ptr<const PlayStats> data, int period, const PlayAccount* account);

  protected:
    void refresh() override;

  private:
    enum Period { Today = 0, Week = 1, AllTime = 2, Month = 3 };   // as activity_summary's
    int period = Week;
    // A game card: its icon, name, time today and over the period, its week.
    struct Card
    {
        uint64_t      app_id = 0;
        brls::Box*    box    = nullptr;
        brls::Image*  icon   = nullptr;
        brls::Box*    icon_fallback = nullptr;
        brls::Label*  name   = nullptr;
        CadranDigits* today  = nullptr;
        brls::Label*  rest   = nullptr;
        brls::Box*    grew_row = nullptr; // the game played most more than the week before: ▲ and how much
        brls::Label*  grew   = nullptr;
        brls::Box*    bars[7] = {};
    };
    cadran::List list;
    std::vector<brls::Box*> chips;   // every account, then each one
    std::vector<Card> cards;
    // Nothing played over the period: why, and all time when it has some.
    cadran::kit::Empty empty;
    brls::Box* see_all = nullptr;

    // Shows this account (-1: every account), remembered for the next run
    // when Activity starts on the last one shown; its data read again when
    // over a minute old.
    void show_account(int account);
    // The account shown (-1: every account) and its data (nullptr: not read yet).
    static int shown_account();
    static std::shared_ptr<const PlayStats> shown_data();
    // The screen again from the data at hand (a read arrived, a choice changed).
    void rebuild();
    // The console's play-timer state for the chart's limit line: read when
    // the tab is made (each time the sidebar reaches it, so after a change on
    // another screen), on Ⓧ and once a minute, not at every rebuild (a pctl
    // session and about ten requests on the UI thread).
    const PtState& play_timer();

    // Reads the data again. `shown`: with the spinner, even over a list (Ⓧ);
    // otherwise only when there is nothing to show yet.
    void fetch(bool shown);
    // rebuild()'s sections: the totals and the chart, and the line under All
    // games.
    void show_totals(const PlayStats& s, const PlayAccount* who);
    void show_status(const PlayStats& s, bool none_played);
    // The summary over the period.
    void show_summary(const std::shared_ptr<const PlayStats>& data);
    // When the last 7 days were played: the hours, the sessions (whatever
    // the period).
    void show_hours(const std::shared_ptr<const PlayStats>& data);
    void export_to_sd() const;
    void choose_period();
    void build_cards();
    void build_chips();
    // The top: the chips, the three figures and the game cards.
    void show_top();
    void show_icons();

    int listener = 0;                                 // play_data::listen id
    // Data arrived while another screen (a game's own screen, All games, a
    // dialog) was on top, or All games changed the period: the tab is rebuilt
    // once it is back.
    bool stale = false;
    bool spinner = false;   // Ⓧ pressed: the spinner stays until its read ends
    PtState pt = {};
    bool pt_read = false;
    std::chrono::steady_clock::time_point pt_at;

    BRLS_BIND(PlayDaysView,     days,        "ac_days");
    BRLS_BIND(brls::DetailCell, account_default, "ac_account_default");
    BRLS_BIND(brls::Label,      new_account, "ac_new_account");
    BRLS_BIND(brls::DetailCell, per_day,     "ac_per_day");
    BRLS_BIND(brls::DetailCell, top,         "ac_top");
    BRLS_BIND(brls::DetailCell, days_played, "ac_days_played");
    BRLS_BIND(brls::DetailCell, busiest,     "ac_busiest");
    BRLS_BIND(brls::DetailCell, session,     "ac_session");
    BRLS_BIND(brls::Header,     hours_header, "ac_hours_header");
    BRLS_BIND(PlayHoursView,    hours,       "ac_hours");
    BRLS_BIND(brls::Label,      hours_legend, "ac_hours_legend");
    BRLS_BIND(brls::DetailCell, sessions,    "ac_sessions");
    BRLS_BIND(brls::DetailCell, longest,     "ac_longest");
    BRLS_BIND(brls::DetailCell, busy_hours,  "ac_busy_hours");
    BRLS_BIND(brls::DetailCell, late,        "ac_late");
    BRLS_BIND(brls::DetailCell, games,       "ac_games");
    BRLS_BIND(brls::Box,        progress,    "ac_progress");
    BRLS_BIND(brls::Label,      capped,      "ac_capped");
    BRLS_BIND(brls::Label,      status,      "ac_status");
    BRLS_BIND(brls::Label,      note,        "ac_note");
    BRLS_BIND(brls::Box,        accounts_row, "ac_accounts");
    BRLS_BIND(brls::Box,        period_chip,  "ac_period");
    BRLS_BIND(brls::Label,      period_label, "ac_period_label");
    BRLS_BIND(brls::Box,        export_chip,  "ac_export");
    BRLS_BIND(brls::Label,      games_title,  "ac_games_title");
    BRLS_BIND(CadranDigits,     today_value,  "ac_today");
    BRLS_BIND(brls::Label,      today_sub,    "ac_today_sub");
    BRLS_BIND(CadranDigits,     week_value,   "ac_week");
    BRLS_BIND(brls::Label,      week_sub,     "ac_week_sub");
    BRLS_BIND(brls::Label,      week_trend,   "ac_week_trend");
    BRLS_BIND(brls::Box,        week_trend_row, "ac_week_trend_row");
    BRLS_BIND(CadranIcon,       week_trend_icon, "ac_week_trend_icon");
    BRLS_BIND(brls::Box,        week_bars,    "ac_week_bars");
    BRLS_BIND(CadranDigits,     total_value,  "ac_total");
    BRLS_BIND(brls::Label,      total_sub,    "ac_total_sub");
    BRLS_BIND(brls::Box,        games_grid,   "ac_cards");
    BRLS_BIND(brls::Label,      games_note,   "ac_games_note");
};
