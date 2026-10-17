// GameCell — a game in a list (All games, the games to rediscover): its icon (when the control data has
// one and the app runs with full memory), its name and a play time. The icon
// slot is kept even before the icon arrives, so the names line up.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class GameCell : public brls::DetailCell
{
  public:
    explicit GameCell(bool with_icon);

    // An NVG image (decoded small by action/game_icons): `owned`, this cell
    // frees it; otherwise someone else owns and keeps it (the icon
    // cache). Nothing happens without an icon slot or with no image.
    void set_icon_texture(int texture, bool owned);

  private:
    brls::Image* icon = nullptr;
};
