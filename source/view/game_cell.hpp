// GameCell — a game in a list (All games): its icon (when the control data has
// one and the app runs with full memory), its name and a play time. The icon
// slot is kept even before the icon arrives, so the names line up.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <vector>

class GameCell : public aurore::DetailCell
{
  public:
    explicit GameCell(bool with_icon);

    // An NVG image (decoded small by action/game_icons): `owned`, this cell
    // frees it; otherwise someone else owns and keeps it (the icon
    // cache). Nothing happens without an icon slot or with no image.
    void set_icon_texture(int texture, bool owned);
    // The image it shows now (0: none, or no icon slot).
    int icon_texture() const { return this->icon ? this->icon->getTexture() : 0; }

    // A small round mark on the icon's corner: `icon_name` (a Cadran icon)
    // in `color_key`; nullptr takes it away. Nothing without an icon slot.
    void set_badge(const char* icon_name, const char* color_key = "cadran/teal");

    // Small Cadran icons in a row before the detail text, one per thing
    // turned on (a forwarder's rules), after setDetailText(): without text
    // they end the line. An empty list takes them away.
    struct Mark
    {
        const char* icon;
        const char* color_key;
    };
    void set_marks(const std::vector<Mark>& marks);

  private:
    aurore::Image* icon = nullptr;
    aurore::Box*   slot = nullptr;    // the icon and its badge
    aurore::Box*   badge = nullptr;
    aurore::Box*   marks = nullptr;   // set_marks()'s row
};
