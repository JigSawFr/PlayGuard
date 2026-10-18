// ActivityTab — play time (today, last 7 days, all time) from the console's
// play data, for every account or one, its summary over the chosen period
// (util/activity_summary.hpp), the way to every game
// of the period (activity/games_activity: a list or covers) and its export
// to the SD card (CSV, JSON, XLSX or PDF). Read-only, so also in read-only
// mode. The data comes from play_data (read off the main thread, shared with
// the Overview, and kept on the SD card for the next run): what is known
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

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/play_days.hpp"

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

    enum Period { Today = 0, Week = 1, AllTime = 2 };
    int period = Week;

    // Shows this account (-1: every account), remembered for the next run
    // when Activity starts on the last one shown; its data read again when
    // over a minute old.
    void show_account(int account);
    // The account shown (-1: every account) and its data (nullptr: not read yet).
    static int shown_account();
    static std::shared_ptr<const PlayStats> shown_data();
    // The screen again from the data at hand (a read arrived, a choice changed).
    virtual void rebuild();
    // The console's play-timer state for the chart's limit line: read when
    // the tab is made (each time the sidebar reaches it, so after a change on
    // another screen), on Ⓧ and once a minute, not at every rebuild (a pctl
    // session and about ten requests on the UI thread).
    const PtState& play_timer();

  private:
    // Reads the data again. `shown`: with the spinner, even over a list (Ⓧ);
    // otherwise only when there is nothing to show yet.
    void fetch(bool shown);
    // rebuild()'s sections: the totals and the chart, and the line under All
    // games.
    void show_totals(const PlayStats& s, const PlayAccount* who);
    void show_status(const PlayStats& s, bool empty);
    // The summary over the period.
    void show_summary(const std::shared_ptr<const PlayStats>& data);
    void export_to_sd() const;

    int listener = 0;                                 // play_data::listen id
    // Data arrived while another screen (a game's own screen, All games, a
    // dialog) was on top, or All games changed the period: the tab is rebuilt
    // once it is back.
    bool stale = false;
    bool spinner = false;   // Ⓧ pressed: the spinner stays until its read ends
    PtState pt = {};
    bool pt_read = false;
    std::chrono::steady_clock::time_point pt_at;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    BRLS_BIND(PlayDaysView,     days,        "ac_days");
    BRLS_BIND(brls::DetailCell, today,       "ac_today");
    BRLS_BIND(brls::DetailCell, week,        "ac_week");
    BRLS_BIND(brls::DetailCell, total,       "ac_total");
    BRLS_BIND(brls::DetailCell, account,     "ac_account");
    BRLS_BIND(brls::DetailCell, account_default, "ac_account_default");
    BRLS_BIND(brls::Label,      new_account, "ac_new_account");
    BRLS_BIND(brls::DetailCell, sort,        "ac_period");
    BRLS_BIND(brls::DetailCell, per_day,     "ac_per_day");
    BRLS_BIND(brls::DetailCell, top,         "ac_top");
    BRLS_BIND(brls::DetailCell, days_played, "ac_days_played");
    BRLS_BIND(brls::DetailCell, busiest,     "ac_busiest");
    BRLS_BIND(brls::DetailCell, session,     "ac_session");
    BRLS_BIND(brls::DetailCell, games,       "ac_games");
    BRLS_BIND(brls::DetailCell, export_cell, "ac_export");
    BRLS_BIND(brls::Box,        progress,    "ac_progress");
    BRLS_BIND(brls::Label,      capped,      "ac_capped");
    BRLS_BIND(brls::Label,      status,      "ac_status");
    BRLS_BIND(brls::Label,      note,        "ac_note");
};
