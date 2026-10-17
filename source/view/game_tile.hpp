// GameTile — a game as a cover in the Games screen's grid: its icon (a
// placeholder until it arrives, or when the app runs without icons), its name
// on one line and a play time under it.
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
    // As GameCell::set_icon_texture: `owned`, this tile frees it.
    void set_icon_texture(int texture, bool owned);
    bool has_icon() const { return this->iconed; }

  private:
    brls::Image* icon = nullptr;
    brls::Label* name = nullptr;
    brls::Label* detail = nullptr;
    bool iconed = false;
};
