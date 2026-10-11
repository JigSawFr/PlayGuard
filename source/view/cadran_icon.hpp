// CadranIcon — the "Cadran" prototype's icons, drawn as nanovg paths from
// the design boards' 24-unit SVG strokes (no SVG loader in this borealis):
// today, timer, activity, shield, lock, globe, settings, tools, info, plus,
// stop, eye, check, and "mark", the PlayGuard logo (branding/mark.svg).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

class CadranIcon : public brls::View
{
  public:
    CadranIcon();

    void setIcon(const std::string& name);
    void setColor(NVGcolor color);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();

  private:
    std::string icon = "today";
    NVGcolor color;
};
