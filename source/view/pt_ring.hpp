// PtRing — Cadran: today's play time as a ring (the brand mark),
// the time left in big digits in the middle, made smaller when they would
// come near the ring (a margin inside it). Teal → amber (≥ 75 %) → coral
// (limit reached); the numbers are always written too, never colour alone.
// A change of the time played moves the arc there rather than jumping, and
// the first value known draws the arc from nothing; the extra time given
// today tints the ring's end, the part of the limit it is.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <string>

#include <aurore/cadran/view/cadran_digits.hpp>

class PtRing : public aurore::Box
{
  public:
    PtRing();

    // used: 0..1 fraction of today's limit already played. Negative == unknown
    // (the track alone is drawn). From one known value to another, the arc
    // travels there; from unknown to the first one, it draws itself from
    // nothing (not with the motion off: ui/effects).
    void setFraction(float used);
    // extra: the fraction of today's limit that is extra time (0: none).
    void setExtra(float extra);
    void setValue(const std::string& text);     // the big figure ("40 min")
    void setCaption(const std::string& text);   // under it ("restantes")
    // The arc shows what is left rather than what was played (the child
    // screen); the colour still follows what was played.
    void setShowLeft(bool left);
    // The last minutes (the child screen): the arc beats, slowly (every
    // 1.4 s; `fast`, the last minute: every second); steady with the
    // motion off.
    void setPulse(bool pulse, bool fast = false);
    // A brighter point at the arc's end, where the time is (the child screen).
    void setTip(bool tip);
    // The figure's digits roll when they change (ui/motion).
    void setRolls(bool rolls);
    void setValueSize(float size);
    void setCaptionSize(float size);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              aurore::Style style, aurore::FrameContext* ctx) override;
    void onThemeChanged() override;

    static aurore::View* create();

  private:
    float used = -1.0f;
    float extra = 0.0f;
    aurore::Animatable shown;   // the arc as drawn, on its way to `used`
    // The figure's width at most: what the ring's inside leaves at its height,
    // less a margin (from the ring's size, as set).
    void fit();
    bool show_left = false;
    bool pulse = false;
    bool pulse_fast = false;
    bool tip = false;
    CadranDigits* value  = nullptr;
    aurore::Label* caption = nullptr;
    // The arc's colours, looked up when the theme changes rather than at
    // each frame (each lookup builds a key).
    void take_colours();
    NVGcolor track, teal, amber, coral, tip_light;
};
