// GameTile — a game as a cover in a grid (All games, the installed games to
// check): its icon (a placeholder until it arrives, or when the app runs
// without icons), its name on one line and a value under it (a play time, a
// problem).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <string>

class GameTile : public aurore::Box
{
  public:
    // Its width, margins included: the grid fits as many a row as its width
    // takes.
    static constexpr float WIDTH = 176;

    GameTile();

    void set_name(const std::string& text);
    void set_detail(const std::string& text);
    void set_name_color(const char* key);
    void set_detail_color(const char* key);
    // As GameCell::set_icon_texture: `owned`, this tile frees it.
    void set_icon_texture(int texture, bool owned);
    bool has_icon() const { return this->iconed; }
    // Back to the placeholder: the tile shows another game.
    void clear_icon();

    void draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
              aurore::FrameContext* ctx) override;

  private:
    aurore::Image* icon = nullptr;
    aurore::Label* name = nullptr;
    aurore::Label* detail = nullptr;
    bool iconed = false;
    aurore::Time icon_since = 0;   // the icon arrived then (µs): it fades in; 0 at rest
};
