// TitleState — what the user must know whatever the tab, as an icon right of
// the main screen's title: an open padlock (amber) while parental controls
// are temporarily unlocked, a crossed-out pencil in read-only mode. A tap
// goes where it is undone: PIN › Lock again, the firmware screen (read-only
// for an untested firmware) or the developer switch in Help & diagnostics.
// Not focusable, like the bell. MainActivity::update_title shows and hides them.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class TitleState : public brls::View
{
  public:
    enum class Kind { UNLOCKED, READ_ONLY };

    explicit TitleState(Kind kind);

    // The icon of `kind` in `frame`'s title row (both added after the title
    // the first time), shown or hidden.
    static void show(brls::AppletFrame* frame, Kind kind, bool shown);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;
    // Touched although it is not focusable.
    brls::View* hitTest(brls::Point point) override;

  private:
    Kind kind;

    void draw_padlock(NVGcontext* vg, float x, float y, float width, float height);
    void draw_read_only(NVGcontext* vg, float x, float y, float width, float height);
};
