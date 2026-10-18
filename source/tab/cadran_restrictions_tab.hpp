// CadranRestrictionsTab — Restrictions after its board: the restriction
// levels side by side as tiles, each with its age limit and whether social
// media posting and communication are restricted, the one in force in teal
// (A on another asks to set it, as the level picker does). It is
// RestrictionsTab: the level line stays the tab's, out of sight; the custom
// settings, VR mode, the rating organisation and the notes stay lines under
// it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "tab/cadran_list.hpp"
#include "tab/restrictions_tab.hpp"
#include "view/cadran_digits.hpp"

class CadranRestrictionsTab : public RestrictionsTab
{
  public:
    CadranRestrictionsTab();
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
        bool             to_settings = false;   // A goes to Custom's settings
    };
    cadran::List list;
    std::vector<Tile> tiles;

    Tile make_tile(uint32_t level, bool last);
};
