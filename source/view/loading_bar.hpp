// LoadingBar — the loading screen's progress bar: a rounded navy track, filled
// in the brand teal up to the start-up step reached. The fill eases to each
// new value instead of jumping (the next step's frames show it moving). Brand
// colours, not the theme's (see LoadingMark).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class LoadingBar : public brls::View
{
  public:
    LoadingBar();

    // 0..1: how much of the start-up is done.
    void setProgress(float value);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();

  private:
    float from = 0.0f, to = 0.0f;   // the fill eases from `from` to `to` ...
    brls::Time since = 0;           // ... starting then (µs)
};
