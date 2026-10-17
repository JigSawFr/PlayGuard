// NoticeBell — the notification bell in the main screen's header, left of
// the version badge: a bell drawn with nanovg, with the number of entries
// to do on an amber disc. Information only: the bell without a number, a
// small dot. Not focusable: Y opens the list (action/notices), and so does a
// tap on it. Only on the
// main screen, inside the version badge's row so the header keeps its
// layout (the title at the left, the badge at the right).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class NoticeBell : public brls::View
{
  public:
    NoticeBell();
    ~NoticeBell() override;

    // Puts the bell in the main screen's header, when it first takes the
    // focus (after VersionBadge::install's badge). Called once, at start-up.
    static void install();

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;
    // Touched although it is not focusable.
    brls::View* hitTest(brls::Point point) override;

  private:
    int listener = 0;
    int actions  = 0;
    int infos    = 0;

    void update();
};
