// RestrictionsTab — the restriction levels side by side as tiles, each with
// its age limit, whether social media posting and communication are
// restricted and how many installed games it blocks, the one in force in
// teal (A on another asks to set it, saying what it restricts). Under them,
// Custom's settings (editable while it is in force), VR mode, the rating
// organisation, the free communication exceptions and how many installed
// games the level blocks by their age rating (the last game check's,
// action/rating_flow), A listing them. Same settings as System Settings.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "tab/cadran_list.hpp"
#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"

class RestrictionsTab : public TabBase
{
  public:
    RestrictionsTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    struct Tile
    {
        uint32_t         level = 0;
        brls::Box*       box   = nullptr;
        brls::Box*       badge = nullptr;
        CadranDigits*    age   = nullptr;
        brls::Label*     note  = nullptr;
        brls::Rectangle* sns_dot  = nullptr;
        brls::Label*     sns      = nullptr;
        brls::Rectangle* comm_dot = nullptr;
        brls::Label*     comm     = nullptr;
        brls::Box*       games_row = nullptr;   // the installed games it blocks (a game check made)
        brls::Rectangle* games_dot = nullptr;
        brls::Label*     games     = nullptr;
        bool             to_settings = false;   // A goes to Custom's settings
    };

    cadran::List list;
    std::vector<Tile> tiles;
    PctlStatus st = {};   // what the last refresh read
    int age_now = -1;     // the age the level in force restricts to (0 none), -1 unknown

    Tile make_tile(uint32_t level, bool last);
    // Asks to set this restriction level (0 none … 4 custom), saying what a
    // preset restricts, then sets it.
    void choose_level(int level);
    // `loosens`: what the change allows; asked first when not empty (B or
    // Cancel puts the switch back). Tightening applies at once.
    void write_custom(const PctlCustomSettings& s, const std::string& loosens = "");
    // "Restrict software by age rating: Up to 12 years (PEGI)" and the two others.
    std::string settings_text(const PctlCustomSettings& s) const;
    void show_tiles(bool custom);

    BRLS_BIND(brls::Header,      custom_header, "rs_custom_header");
    BRLS_BIND(brls::DetailCell,  age,           "rs_age");
    BRLS_BIND(brls::BooleanCell, sns,           "rs_sns");
    BRLS_BIND(brls::BooleanCell, comm,          "rs_comm");
    BRLS_BIND(brls::Label,       custom_note,   "rs_custom_note");
    BRLS_BIND(brls::BooleanCell, vr,            "rs_vr");
    BRLS_BIND(brls::DetailCell,  org,           "rs_org");
    BRLS_BIND(brls::DetailCell,  blocked,       "rs_blocked");
    BRLS_BIND(brls::Label,       blocked_note,  "rs_blocked_note");
    BRLS_BIND(brls::DetailCell,  free_comm,     "rs_free_comm");
    BRLS_BIND(brls::Label,       note,          "rs_note");
};
