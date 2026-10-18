// RowIcon — an icon on a row (a DetailCell), just after its value: there is
// more to read, Ⓐ on the row shows it (a dialog), so the row keeps one line
// instead of a paragraph under it. An "i" in a disc for a note, an amber
// triangle with "!" for a warning. Gone while there is nothing to say.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class RowIcon : public brls::View
{
  public:
    enum class Kind { Info, Warning };

    explicit RowIcon(Kind kind);

    void set_shown(bool shown);
    void set_kind(Kind kind);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    // The row's, added the first time it is asked for: after its value.
    static RowIcon* on_row(brls::Box* row, Kind kind);

  private:
    Kind kind;
};
