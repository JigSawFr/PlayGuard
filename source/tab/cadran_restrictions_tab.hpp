// CadranRestrictionsTab — "Cadran" prototype of Restrictions, after the
// board: the level as four tiles that say what they impose (the console's
// own presets), the other settings as three cards. PLAYGUARD_SIM_CADRAN=1 on
// the desktop, Tools › Developer › Cadran prototype on the console; strings
// not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "tab/tab_base.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"

class CadranRestrictionsTab : public TabBase
{
  public:
    CadranRestrictionsTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    struct Tile
    {
        uint32_t        level = 0;
        brls::Box*      box   = nullptr;
        brls::Box*      badge = nullptr;
        CadranDigits*   age   = nullptr;
        brls::Label*    note  = nullptr;
        brls::Label*    sns   = nullptr;
        brls::Label*    comm  = nullptr;
        brls::Rectangle* sns_dot = nullptr;
        brls::Rectangle* comm_dot = nullptr;
    };
    PctlStatus st = {};
    std::vector<Tile> tiles;

    Tile make_tile(uint32_t level, const char* name, bool last);
    void choose_level(uint32_t level);

    BRLS_BIND(CadranDigits, level_value, "rs_level");
    BRLS_BIND(brls::Box,    tiles_row,   "rs_tiles");
    BRLS_BIND(brls::Box,    vr_card,     "rs_vr");
    BRLS_BIND(brls::Label,  vr_sub,      "rs_vr_sub");
    BRLS_BIND(brls::Box,    vr_toggle,   "rs_vr_toggle");
    BRLS_BIND(brls::Box,    vr_knob,     "rs_vr_knob");
    BRLS_BIND(brls::Box,    org_card,    "rs_org");
    BRLS_BIND(brls::Label,  org_sub,     "rs_org_sub");
    BRLS_BIND(brls::Box,    comm_card,   "rs_comm");
    BRLS_BIND(brls::Label,  comm_sub,    "rs_comm_sub");
};
