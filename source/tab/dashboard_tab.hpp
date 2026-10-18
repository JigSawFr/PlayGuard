// DashboardTab — overview: today's play time, parental-control state, system
// warnings. Refreshed every 5 s while shown; A on a line opens the matching tab.
// When the timer reports no time left (no game running yet, or no limit
// today), today's play time comes from the activity log (play_data, read in
// the background at most every few minutes). Two amber notices: the console
// not counting play time (action/timer_health) and changes seen outside
// PlayGuard (action/outside_watch).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"
#include "util/patches.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/pt_gauge.hpp"

class DashboardTab : public TabBase
{
  public:
    DashboardTab();
    ~DashboardTab() override;
    static brls::View* create();

  protected:
    void refresh() override;

    // The last readings, for a look that shows them its own way (CadranTab).
    PtState    pt = {};
    PctlStatus status = {};
    SysInfo    sys = {};
    Result     accuracy_rc = 0;
    bool       accurate = false;
    bool clock_inaccurate = false;   // last read: A on the clock line offers to fix it

    void open_today_limit();

  private:
    int  listener = 0;               // play_data::listen id

    // Today's play time from the activity log, for when the timer does not say.
    struct TodayLog
    {
        bool     ok         = false;
        uint64_t played_s   = 0;
        uint16_t played_min = 0;   // rounded, at most a day
    };
    // What the System section flags: shown here only then (the Console tab always).
    struct SystemIssues
    {
        bool compat  = false;
        bool serial  = false;
        bool patches = false;
    };

    // refresh()'s sections, in the order it runs them.
    static TodayLog read_today_log();
    bool show_outside(const PtState& pt);
    void show_today(const PtState& pt, int today, bool unlocked, const TodayLog& log);
    void show_remaining(const PtState& pt, int today, bool unlocked, const TodayLog& log);
    void show_bedtime(const PtState& pt);
    void show_controls(const PctlStatus& s, bool unlocked, bool has_pin);
    SystemIssues show_system(const PctlStatus& s, const SysInfo& si, Result accuracy_rc, bool accurate);

    BRLS_BIND(brls::DetailCell, unlocked_banner, "dash_unlocked_banner");
    BRLS_BIND(brls::DetailCell, pc,          "dash_pc");
    BRLS_BIND(brls::DetailCell, pin,         "dash_pin");
    BRLS_BIND(brls::DetailCell, level,       "dash_level");
    BRLS_BIND(PtGauge,          gauge,       "dash_gauge");
    BRLS_BIND(brls::Label,      gauge_text,  "dash_gauge_text");
    BRLS_BIND(brls::Label,      not_counting, "dash_not_counting");
    BRLS_BIND(brls::Label,      home_hint,   "dash_home_hint");
    BRLS_BIND(brls::Label,      outside_text, "dash_outside_text");
    BRLS_BIND(brls::DetailCell, outside,     "dash_outside");
    BRLS_BIND(brls::DetailCell, today_limit, "dash_today_limit");
    BRLS_BIND(brls::DetailCell, extra,       "dash_extra");
    BRLS_BIND(brls::DetailCell, stop,        "dash_stop");
    BRLS_BIND(brls::DetailCell, extra_pending, "dash_extra_pending");
    BRLS_BIND(brls::DetailCell, remaining,   "dash_remaining");
    BRLS_BIND(brls::DetailCell, bedtime,     "dash_bedtime");
    BRLS_BIND(brls::DetailCell, alarm,       "dash_alarm");
    BRLS_BIND(brls::DetailCell, clock,       "dash_clock");
    BRLS_BIND(brls::DetailCell, pairing,     "dash_pairing");
    BRLS_BIND(brls::DetailCell, serial,      "dash_serial");
    BRLS_BIND(brls::DetailCell, game_patches, "dash_patches");
    BRLS_BIND(brls::DetailCell, fw,          "dash_fw");
    BRLS_BIND(brls::DetailCell, compat,      "dash_compat");
    BRLS_BIND(brls::DetailCell, first_steps, "dash_first_steps");
    BRLS_BIND(brls::Label,      applet,      "dash_applet");
    BRLS_BIND(brls::Label,      counted,     "dash_counted");
    BRLS_BIND(brls::Label,      updated,     "dash_updated");
};
