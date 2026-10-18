// GameTile — a game as a cover in a grid (All games, the installed games to
// check): its icon (a placeholder until it arrives, or when the app runs
// without icons), its name on one line and a value under it (a play time, a
// problem).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

class GameTile : public brls::Box
{
  public:
    // Its width, margins included: the grid fits as many a row as its width
    // takes.
    static constexpr float WIDTH = 176;

    GameTile();

    void set_name(const std::string& text);
    void set_detail(const std::string& text);
    void set_name_color(NVGcolor color);
    void set_detail_color(NVGcolor color);
    // As GameCell::set_icon_texture: `owned`, this tile frees it.
    void set_icon_texture(int texture, bool owned);
    bool has_icon() const { return this->iconed; }
    // Back to the placeholder: the tile shows another game.
    void clear_icon();

  private:
    brls::Image* icon = nullptr;
    brls::Label* name = nullptr;
    brls::Label* detail = nullptr;
    bool iconed = false;
};
