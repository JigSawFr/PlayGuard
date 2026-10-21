// GameCell — a game in a list (All games): its icon (when the control data has
// one and the app runs with full memory), its name and a play time. The icon
// slot is kept even before the icon arrives, so the names line up.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <vector>

class GameCell : public brls::DetailCell
{
  public:
    explicit GameCell(bool with_icon);

    // An NVG image (decoded small by action/game_icons): `owned`, this cell
    // frees it; otherwise someone else owns and keeps it (the icon
    // cache). Nothing happens without an icon slot or with no image.
    void set_icon_texture(int texture, bool owned);

    // A small round mark on the icon's corner: `icon_name` (a Cadran icon)
    // in `color_key`; nullptr takes it away. Nothing without an icon slot.
    void set_badge(const char* icon_name, const char* color_key = "cadran/teal");

    // Small Cadran icons in a row before the detail text, one per thing
    // turned on (a forwarder's rules); an empty list takes them away.
    struct Mark
    {
        const char* icon;
        const char* color_key;
    };
    void set_marks(const std::vector<Mark>& marks);

  private:
    brls::Image* icon = nullptr;
    brls::Box*   slot = nullptr;    // the icon and its badge
    brls::Box*   badge = nullptr;
    brls::Box*   marks = nullptr;   // set_marks()'s row
};
