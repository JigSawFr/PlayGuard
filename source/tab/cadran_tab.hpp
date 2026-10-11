// CadranTab — "Cadran" prototype of the Overview (desktop simulation only, with
// PLAYGUARD_SIM_CADRAN=1): today's time left as a ring, the two everyday
// actions under it, three state cards, a line of "all fine" pills. Same
// readings as DashboardTab, refreshed every 5 s. Used to measure the cost of
// the design against the current Overview; the strings are not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/pt_ring.hpp"

namespace cadran
{
// The navy theme (both variants) and the slimmer sidebar: before any view.
void install_theme();
}

class CadranTab : public TabBase
{
  public:
    CadranTab();
    static brls::View* create();

    // The soft glow behind everything, then the children.
    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

  protected:
    void refresh() override;

  private:
    PtState pt = {};
    bool clock_inaccurate = false;
    NVGcolor glow, clear, teal, amber, text2, surf, amber_surf, line, amber_line;

    BRLS_BIND(PtRing,       ring,        "cad_ring");
    BRLS_BIND(brls::Label,  kicker,      "cad_kicker");
    BRLS_BIND(brls::Label,  played,      "cad_played");
    BRLS_BIND(brls::Button, extra,       "cad_extra");
    BRLS_BIND(brls::Button, stop,        "cad_stop");
    BRLS_BIND(brls::Box,    bed_card,    "cad_bed_card");
    BRLS_BIND(brls::Label,  bed_state,   "cad_bed_state");
    BRLS_BIND(brls::Label,  bed_value,   "cad_bed_value");
    BRLS_BIND(brls::Label,  bed_sub,     "cad_bed_sub");
    BRLS_BIND(brls::Box,    pc_card,     "cad_pc_card");
    BRLS_BIND(brls::Label,  pc_state,    "cad_pc_state");
    BRLS_BIND(brls::Label,  pc_value,    "cad_pc_value");
    BRLS_BIND(brls::Label,  pc_sub,      "cad_pc_sub");
    BRLS_BIND(brls::Box,    clock_card,  "cad_clock_card");
    BRLS_BIND(brls::Label,  clock_title, "cad_clock_title");
    BRLS_BIND(brls::Label,  clock_state, "cad_clock_state");
    BRLS_BIND(brls::Label,  clock_value, "cad_clock_value");
    BRLS_BIND(brls::Label,  clock_sub,   "cad_clock_sub");
    BRLS_BIND(brls::Label,  updated,     "cad_updated");
};
