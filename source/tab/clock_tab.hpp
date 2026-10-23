// ClockTab — shows the system clocks, both large, the network one amber while
// it is inaccurate, and sets the network clock from a public NTP server (the
// play timer relies on the network clock). The measurement and the write are
// clock_flow's; this screen adds the server choice, the result text, the
// 2-minute countdown, the measurement in a dialog (a spinner, then the
// result) and, when no server answers, "Set the network clock to the console
// clock".
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "action/clock_flow.hpp"
#include "tab/cadran_list.hpp"
#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"

class ClockTab : public TabBase
{
  public:
    ClockTab();
    // The network clock on a screen of its own (Console › Network clock).
    static void open_screen();

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
    bool busy = false;
    bool autosync_off = false;   // "Synchronise Clock via Internet" read as off
    bool offline = false;        // the last measurement: no server answered
    // The tab ticks every second; the clocks are read every SNAPSHOT_EVERY and
    // moved forward by the steady clock in between (a snapshot is ~16 IPCs).
    TimeSnapshot snap = {};
    std::chrono::steady_clock::time_point snap_at;
    bool snap_ok = false;
    std::string region_id;
    std::string server;
    clock_flow::Measurement last;

    void choose_region();
    void choose_server();
    void enter_custom();
    void set_server(const std::string& host, const std::string& region);
    // Measures with a spinner in a dialog, then shows the result in another
    // (show_measurement); the screen keeps it in one line.
    void measure();
    // The result as a dialog: the offset in words, large; the servers' time
    // against the network clock; each server's answer; what needs the parent
    // in amber; then Set the clock (or, no server answering, the console clock).
    void show_measurement(const clock_flow::Measurement& m, const std::string& warn);
    static size_t answered(const clock_flow::Measurement& m);   // servers that answered
    void apply();
    void copy();
    // The result under Measure: plain text, or, no server answering, an
    // amber card that says so first, the servers' lines under it.
    void show_result(const std::string& text, bool offline = false);
    // Both clocks large, the network one amber while it is inaccurate
    // (`known` false: not read).
    void show_clocks(const std::string& user_text, const std::string& network_text, bool known, bool accurate);

    BRLS_BIND(brls::DetailCell, autosync, "ck_auto");
    BRLS_BIND(brls::DetailCell, zone,     "ck_zone");
    BRLS_BIND(brls::Label,      why,      "ck_why");
    BRLS_BIND(brls::DetailCell, region,   "ck_region");
    BRLS_BIND(brls::DetailCell, server_cell, "ck_server");
    BRLS_BIND(brls::DetailCell, custom,   "ck_custom");
    BRLS_BIND(brls::DetailCell, measure_cell, "ck_measure");
    BRLS_BIND(brls::Label,      result,   "ck_result");
    BRLS_BIND(brls::Label,      result_title, "ck_result_title");
    BRLS_BIND(brls::Box,        result_card,  "ck_result_card");
    BRLS_BIND(brls::Label,      warning,  "ck_warning");
    BRLS_BIND(brls::DetailCell, apply_cell, "ck_apply");
    BRLS_BIND(brls::DetailCell, copy_cell, "ck_copy");
    BRLS_BIND(brls::BooleanCell, check_at_start, "ck_check_at_start");
};
