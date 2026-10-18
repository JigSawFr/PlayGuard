// Cadran — PlayGuard's look after the 2026 design boards (Preferences ›
// Appearance, the default; "Classic" keeps the lists of 1.x): the navy and
// cool-white palettes, an icon rail for a sidebar, the mark and a status pill
// in the header, cards in place of lists, the figures in Sora.
//
// CadranTab — the Overview after its board: today's time left as a ring with
// the everyday actions beside it, three state cards (bedtime, parental
// controls, network clock), the console's state as pills; under them the
// Overview's own lines that have no place up there (today's limit, the
// alarm, the notices). It is DashboardTab: the same readings every 5 s, the
// same actions; the lines the top shows its own way are kept out of sight
// and their actions run from the cards and buttons.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "tab/cadran_list.hpp"
#include "tab/dashboard_tab.hpp"
#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"
#include "view/pt_ring.hpp"

namespace cadran
{
// Whether the app wears Cadran (Preferences › Appearance, config appearance,
// the default) rather than the classic lists; PLAYGUARD_SIM_APPEARANCE=classic
// or =cadran forces it on the desktop. Read once, at start-up: a change takes
// effect at the next launch.
bool enabled();
// The boards' two palettes on borealis' two variants (dark: navy; light:
// cool white), the rail's metrics and the display font: before any view.
void install_theme();
// A frame's header after the boards: the mark for the app icon, the title
// at 24, a pill at the right (`icon` and `text`; `filled` teal, else a
// bordered one) and, when asked, the date.
void style_header(brls::AppletFrame* frame, const char* icon, const std::string& text, bool filled, bool with_date);
// From then on, every screen but the main one gets the header after the
// boards (the mark, the title) and its list in cards (cadran::List), the
// first time it takes the focus.
void install_screens();
// A TabFrame's sidebar as the boards' rail: an icon before each label, the
// active tab on a teal pill.
void style_rail(brls::View* tab_frame);
// Both, for the main screen.
void style_frame(brls::AppletFrame* frame, brls::View* tab_frame);
// The main screen's header pill: its text, or hidden when empty (each
// screen says what it has to: "Contrôle parental actif", "Minuteur actif"…),
// its icon, and amber for a state that needs the parent ("Pas de code PIN").
// While parental controls are known to be unlocked (ui::known_unlocked), it
// says that instead, whatever the screen: the header's padlock in Classic.
void set_pill(const std::string& text, const char* icon = "check", bool warn = false);
// The pill again after the unlocked state changed (MainActivity::update_title).
void refresh_pill();
// The soft glow of the boards, an absolute full-screen view to add first in
// a frame: a radial gradient of `color_key` centred on (cx, cy), `radius`
// wide. PLAYGUARD_SIM_CADRAN_NOGLOW=1 draws nothing.
brls::View* make_glow(float cx, float cy, float radius, const char* color_key);
}

class CadranTab : public DashboardTab
{
  public:
    CadranTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    // A pill of the console's state, standing for one of the Overview's lines.
    struct Pill
    {
        const char*      line = nullptr;
        brls::Box*       box  = nullptr;
        brls::Rectangle* dot  = nullptr;
        brls::Label*     text = nullptr;
    };
    cadran::List list;
    std::vector<Pill> pill_slots;
    // What stops play first today, under the time played (show_next).
    brls::Box*   next      = nullptr;
    CadranIcon*  next_icon = nullptr;
    brls::Label* next_text = nullptr;
    // The extra time whose chip has the focus (show_next says where it ends),
    // and whether parental controls were unlocked at the last refresh.
    int  preview_extra = 0;
    bool unlocked_now  = false;

    void show_ring(bool unlocked);
    void show_next(bool unlocked);
    void show_cards(bool unlocked, bool has_pin);
    void show_pills();

    BRLS_BIND(PtRing,       ring,        "cad_ring");
    BRLS_BIND(brls::Label,  kicker,      "cad_kicker");
    BRLS_BIND(brls::Box,    today_text,  "cad_today_text");
    BRLS_BIND(brls::Box,    actions,     "cad_actions");
    BRLS_BIND(brls::Button, extra,       "cad_extra");
    BRLS_BIND(brls::Box,    chips,       "cad_chips");
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
    BRLS_BIND(brls::Box,    pills,       "cad_pills");
};
