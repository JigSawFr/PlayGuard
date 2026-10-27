// LoadingDial — Cadran's dial on the right of the loading screen, behind the
// icon and the bar, as the PIN gate draws it (core/gate_screen.c's dial()):
// its teal glow, two rings, the minutes, the day's arc, in the theme's
// playguard/loading/* colours (ui/palette.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class LoadingDial : public brls::View
{
  public:
    LoadingDial();

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();
};
