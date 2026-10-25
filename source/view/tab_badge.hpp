// TabBadge — the number of entries to do of a tab, at the right of its name
// in the main screen's sidebar: an amber disc, as the notification bell's,
// so what needs the parent shows where it is fixed (notice_logic::tab_of).
// The same in the tab, on the row where they are fixed (TabBase::mark),
// at its right end. Gone while there is nothing to do: the
// name keeps the whole row. With the rail on its icons (Preferences ›
// Sidebar), a smaller disc on the icon's corner instead (dock).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class TabBadge : public brls::View
{
  public:
    TabBadge();

    void set_count(int count);
    // The sidebar's: `open` 1 at the right of the name, 0 on the icon's top
    // right corner, smaller, and in between while the rail opens or folds.
    // `floating`: out of the item's row, for a rail too narrow for it.
    void dock(float open, bool floating);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    // The sidebar item's badge, added the first time it is asked for.
    static TabBadge* of(brls::SidebarItem* item);
    // Its badge if it has one yet, else nullptr.
    static TabBadge* find(brls::SidebarItem* item);
    // A row's (a DetailCell: its title, then its value): after the value.
    static TabBadge* on_row(brls::Box* row);

  private:
    int count = 0;
    brls::Time pop_since = 0;   // the number changed: the disc pops (µs; 0 at rest)
    float open = 1.0f;
    bool floating = false;
};
