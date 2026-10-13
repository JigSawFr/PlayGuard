// CadranTab — "Cadran" prototype of the Overview (PLAYGUARD_SIM_CADRAN=1 on
// the desktop, About › Help & diagnostics › Developer › Cadran prototype on the console), after
// the design boards: today's time left as a
// ring, the two everyday actions under it, three state cards, a line of
// "all fine" pills; the navy theme, an icon rail for a sidebar, the mark and
// a status pill in the header, a glow behind the whole screen. Same readings as DashboardTab, refreshed every
// 5 s. Used to look at the design and to measure its cost against the current
// Overview; the strings are not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/pt_ring.hpp"

namespace cadran
{
// Whether the prototype replaces the current screens: PLAYGUARD_SIM_CADRAN=1
// on the desktop; config cadran (About › Help & diagnostics › Developer) while the developer
// mode is on. Read at start-up: a change takes effect at the next launch.
bool enabled();
// The boards' two palettes on borealis' two variants (dark: navy; light:
// cool white), the rail's metrics and the display font: before any view.
void install_theme();
// A frame's header after the boards: the mark for the app icon, the title
// at 24, a pill at the right (`icon` and `text`; `filled` teal, else a
// bordered one) and, when asked, the date.
void style_header(brls::AppletFrame* frame, const char* icon, const std::string& text, bool filled, bool with_date);
// A TabFrame's sidebar as the boards' rail: an icon before each label, the
// active tab on a teal pill.
void style_rail(brls::View* tab_frame);
// Both, for the main screen.
void style_frame(brls::AppletFrame* frame, brls::View* tab_frame);
// The main screen's header pill: its text, or hidden when empty (each
// screen says what it has to: "Contrôle parental actif", "Minuteur actif"…),
// its icon, and amber for a state that needs the parent ("Pas de code PIN").
void set_pill(const std::string& text, const char* icon = "check", bool warn = false);
// The soft glow of the boards, an absolute full-screen view to add first in
// a frame: a radial gradient of `color_key` centred on (cx, cy), `radius`
// wide. PLAYGUARD_SIM_CADRAN_NOGLOW=1 draws nothing.
brls::View* make_glow(float cx, float cy, float radius, const char* color_key);
}

class CadranTab : public TabBase
{
  public:
    CadranTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    PtState pt = {};
    bool clock_inaccurate = false;
    NVGcolor teal, amber, text, text2, text3, surf, amber_surf, line, amber_line;

    BRLS_BIND(PtRing,       ring,        "cad_ring");
    BRLS_BIND(brls::Label,  kicker,      "cad_kicker");
    BRLS_BIND(CadranDigits, played,      "cad_played");
    BRLS_BIND(brls::Button, extra,       "cad_extra");
    BRLS_BIND(brls::Box,    chip15,      "cad_chip15");
    BRLS_BIND(brls::Box,    chip30,      "cad_chip30");
    BRLS_BIND(brls::Box,    chip60,      "cad_chip60");
    BRLS_BIND(brls::Button, stop,        "cad_stop");
    BRLS_BIND(brls::Button, child,       "cad_child");
    BRLS_BIND(brls::Box,    bed_card,    "cad_bed_card");
    BRLS_BIND(brls::Label,  bed_state,   "cad_bed_state");
    BRLS_BIND(CadranDigits, bed_value,   "cad_bed_value");
    BRLS_BIND(brls::Label,  bed_sub,     "cad_bed_sub");
    BRLS_BIND(brls::Box,    pc_card,     "cad_pc_card");
    BRLS_BIND(brls::Label,  pc_state,    "cad_pc_state");
    BRLS_BIND(CadranDigits, pc_value,    "cad_pc_value");
    BRLS_BIND(brls::Label,  pc_sub,      "cad_pc_sub");
    BRLS_BIND(brls::Box,    clock_card,  "cad_clock_card");
    BRLS_BIND(brls::Label,  clock_title, "cad_clock_title");
    BRLS_BIND(brls::Label,  clock_state, "cad_clock_state");
    BRLS_BIND(CadranDigits, clock_value, "cad_clock_value");
    BRLS_BIND(brls::Box,    clock_key,   "cad_clock_key");
    BRLS_BIND(brls::Label,  clock_sub,   "cad_clock_sub");
    BRLS_BIND(brls::Label,  updated,     "cad_updated");
};
