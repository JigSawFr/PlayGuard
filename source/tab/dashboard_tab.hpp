// DashboardTab — Today: today's time left as a ring with the everyday actions
// beside it (extra time and its amounts, stop for today, show the child,
// today's limit, the profile in force), what stops play first, three state
// cards (bedtime, parental controls, network clock), the console's state as
// pills. Above them what needs the parent first: the first steps until the
// PIN and the limits are set, the changes seen outside PlayGuard
// (action/outside_watch); under them, the extra time to put back and the
// alarm turned off. Refreshed every 5 s while shown. When the timer reports
// no time left (no game running yet, or no limit today), today's play time
// comes from the activity log (play_data, read in the background at most
// every few minutes). The console not counting play time
// (action/timer_health) is said under the time played.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_list.hpp>
#include <borealis/cadran/view/cadran_digits.hpp>
#include <borealis/cadran/view/cadran_icon.hpp>

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_timeline.hpp"
#include "view/pt_ring.hpp"

class DashboardTab : public TabBase
{
  public:
    DashboardTab();
    ~DashboardTab() override;
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    // The last readings.
    PtState    pt = {};
    PctlStatus status = {};
    SysInfo    sys = {};
    Result     accuracy_rc = 0;
    bool       accurate = false;
    bool       clock_inaccurate = false;   // last read: A on the clock card offers to fix it
    int        listener = 0;               // play_data::listen id

    // Today's play time from the activity log, for when the timer does not say.
    struct TodayLog
    {
        bool     ok         = false;
        uint64_t played_s   = 0;
        uint16_t played_min = 0;   // rounded, at most a day
    };
    // A pill of the console's state.
    struct Pill
    {
        brls::Box*       box  = nullptr;
        cadran::kit::Mark* dot = nullptr;
        brls::Label*     text = nullptr;
        int              rank = 1;   // 0 needs the parent (amber, coral), 1 fine (teal), 2 for the record (grey)
        bool             on   = false;   // has something to say (shown, unless folded into fine_pill)
    };
    cadran::List list;
    // The firmware's compatibility, the phone app, the serial number, the
    // game patches; A opens the tab where each is set.
    Pill compat_pill, pairing_pill, serial_pill, patches_pill;
    // Atmosphère's DNS sending Nintendo's servers elsewhere (nintendo_dns):
    // a pill of its own, after the serial number's.
    Pill dns_pill;
    // Opened over a game (Ⓐ says it at length); a game left open on HOME,
    // which keeps counting.
    Pill counted_pill;
    Pill home_pill;
    // What is fine and what is only said, folded into one pill ("✓ 4 checks
    // fine") under the ones that need the parent; Ⓐ unfolds them, for as
    // long as the tab is open.
    Pill fine_pill;
    bool fine_open = false;
    // What changed outside PlayGuard: a card of its own above the ring, its
    // lines rebuilt when the notice's text changes.
    brls::Box*  outside_card  = nullptr;
    brls::Box*  outside_lines = nullptr;
    std::string outside_shown;
    // First steps, while the PIN or the limits are missing: the three steps
    // as a list to tick, each one done on Ⓐ.
    struct Step
    {
        brls::Box*   row  = nullptr;
        CadranIcon*  done = nullptr;   // a check once done
        brls::Box*   todo = nullptr;   // an empty ring until then
        brls::Label* state = nullptr;
    };
    brls::Box*   steps_card  = nullptr;
    brls::Label* steps_count = nullptr;
    Step         steps[3];
    bool         steps_shown = false;      // the card is up (it bows out once all is done)
    bool         steps_done[3] = {};       // each step as last shown (a step just done: its check pops)
    bool         steps_read  = false;      // steps_done read once (the first reading pops nothing)
    bool         low_lit     = false;      // the first amount stands out (show_low; lit anew: one breath)
    // What stops play first today, under the time played (show_next).
    brls::Box*   next      = nullptr;
    CadranIcon*  next_icon = nullptr;
    brls::Label* next_text = nullptr;
    // Or, before the time up or the bedtime alarm, the rest of the day as a line.
    CadranTimeline* timeline = nullptr;
    // The extra time whose chip has the focus (show_next says where it ends),
    // and whether parental controls were unlocked at the last refresh.
    int  preview_extra = 0;
    bool unlocked_now  = false;
    // The extra-time amounts' chips, the first one standing out when little
    // time is left (show_low).
    std::vector<brls::Box*> amount_chips;

    // refresh()'s sections.
    static TodayLog read_today_log();
    void show_headline(int today, bool unlocked, const TodayLog& log);
    void show_ring(bool unlocked);
    void show_next(bool unlocked);
    void show_low(bool unlocked);
    void show_cards(bool unlocked, bool has_pin);
    void show_pills();
    void order_pills();
    void show_profile();
    void show_steps_card(bool has_pin);
    void show_outside_card(const std::vector<std::string>& lines);
    void fit_text_size();
    void choose_today_limit();
    // A game is open on HOME, its time still counting (not while unlocked:
    // the headline says the time may not count).
    bool home_counts() const;
    Pill make_pill(const std::string& text, const char* dot);
    void build_pills();
    void build_outside_card(brls::Box* column, int at);
    void build_steps_card(brls::Box* column, int at);

    BRLS_BIND(brls::Label,      applet,        "dash_applet");
    BRLS_BIND(brls::Label,      headline,      "dash_gauge_text");
    BRLS_BIND(brls::Label,      not_counting,  "dash_not_counting");
    BRLS_BIND(brls::DetailCell, extra_pending, "dash_extra_pending");
    BRLS_BIND(brls::DetailCell, alarm,         "dash_alarm");
    BRLS_BIND(PtRing,       ring,        "dash_ring");
    BRLS_BIND(brls::Label,  kicker,      "dash_kicker");
    BRLS_BIND(brls::Box,    today_text,  "dash_today_text");
    BRLS_BIND(brls::Box,    actions,     "dash_actions");
    BRLS_BIND(brls::Button, relock,      "dash_relock");
    BRLS_BIND(brls::Button, setup,       "dash_setup");
    BRLS_BIND(brls::Box,    chips,       "dash_chips");
    BRLS_BIND(brls::Button, stop,        "dash_stop");
    BRLS_BIND(brls::Button, child,       "dash_child");
    BRLS_BIND(brls::Box,    actions2,    "dash_actions2");
    BRLS_BIND(brls::Button, limit,       "dash_limit");
    BRLS_BIND(brls::Button, profile,     "dash_profile");
    BRLS_BIND(brls::Box,    bed_card,    "dash_bed_card");
    BRLS_BIND(brls::Label,  bed_state,   "dash_bed_state");
    BRLS_BIND(CadranDigits, bed_value,   "dash_bed_value");
    BRLS_BIND(brls::Label,  bed_sub,     "dash_bed_sub");
    BRLS_BIND(brls::Box,    bed_key,     "dash_bed_key");
    BRLS_BIND(brls::Box,    pc_card,     "dash_pc_card");
    BRLS_BIND(brls::Label,  pc_state,    "dash_pc_state");
    BRLS_BIND(CadranDigits, pc_value,    "dash_pc_value");
    BRLS_BIND(brls::Label,  pc_sub,      "dash_pc_sub");
    BRLS_BIND(brls::Box,    clock_card,  "dash_clock_card");
    BRLS_BIND(brls::Label,  clock_title, "dash_clock_title");
    BRLS_BIND(brls::Label,  clock_state, "dash_clock_state");
    BRLS_BIND(CadranDigits, clock_value, "dash_clock_value");
    BRLS_BIND(brls::Box,    clock_key,   "dash_clock_key");
    BRLS_BIND(brls::Label,  clock_sub,   "dash_clock_sub");
    BRLS_BIND(brls::Box,    pills,       "dash_pills");
};
