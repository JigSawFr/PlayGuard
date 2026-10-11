// PtRing — "Cadran" prototype: today's play time as a ring (the brand mark),
// the time left in big digits in the middle. Teal → amber (≥ 75 %) → coral
// (limit reached); the numbers are always written too, never colour alone.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

#include "view/cadran_digits.hpp"

class PtRing : public brls::Box
{
  public:
    PtRing();

    // used: 0..1 fraction of today's limit already played. Negative == unknown
    // (the track alone is drawn).
    void setFraction(float used);
    void setValue(const std::string& text);     // the big figure ("40 min")
    void setCaption(const std::string& text);   // under it ("restantes")

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();

  private:
    float used = -1.0f;
    CadranDigits* value  = nullptr;
    brls::Label* caption = nullptr;
    NVGcolor track, teal, amber, coral;
};
