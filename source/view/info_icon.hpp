// InfoIcon — an "i" in a disc on a row (a DetailCell), just before its value:
// there is more to read, Ⓐ on the row shows it (a dialog), so the row keeps
// one line instead of a paragraph under it. Gone while there is nothing to say.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class InfoIcon : public brls::View
{
  public:
    InfoIcon();

    void set_shown(bool shown);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    // The row's, added the first time it is asked for: between its title and
    // its value.
    static InfoIcon* on_row(brls::Box* row);
};
