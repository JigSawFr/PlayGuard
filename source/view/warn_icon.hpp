// WarnIcon — an amber triangle with "!" on a row (a DetailCell), just before
// its value: there is more to read, Ⓐ on the row shows it (a dialog), so the
// row keeps one line instead of a paragraph under it. Gone while there is
// nothing to warn about.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class WarnIcon : public brls::View
{
  public:
    WarnIcon();

    void set_shown(bool shown);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    // The row's, added the first time it is asked for: between its title and
    // its value.
    static WarnIcon* on_row(brls::Box* row);
};
