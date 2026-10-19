// TitleState — what the user must know whatever the tab, as an icon right of
// the main screen's title: a crossed-out pencil in read-only mode. A tap goes
// where it is undone: the firmware screen (read-only for an untested
// firmware) or the developer switch in Help & diagnostics. Not focusable,
// like the bell. MainActivity::update_title shows and hides it. While
// parental controls are unlocked, the header's pill says so instead, and its
// tap goes where that is undone (open(Kind::UNLOCKED): PIN › Lock again).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class TitleState : public brls::View
{
  public:
    enum class Kind { UNLOCKED, READ_ONLY };

    TitleState();

    // The read-only icon in `frame`'s title row (added after the title the
    // first time), shown or hidden.
    static void show(brls::AppletFrame* frame, bool shown);
    // Where `kind` is undone, as a tap on its icon (or the header's pill) goes.
    static void open(Kind kind);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;
    // Touched although it is not focusable.
    brls::View* hitTest(brls::Point point) override;

  private:
    void draw_read_only(NVGcontext* vg, float x, float y, float width, float height);
};
