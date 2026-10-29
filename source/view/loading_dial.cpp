// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/loading_dial.hpp"

#include <cmath>
#include <utility>

namespace
{
constexpr float PI = 3.14159265f;
// Where the gate has it on its 1280 x 720 screen (core/gate_screen.c).
constexpr float CX = 1196.0f, CY = 340.0f;
}   // namespace

LoadingDial::LoadingDial()
{
    this->setFocusable(false);
}

void LoadingDial::draw(NVGcontext* vg, float x, float y, float width, float height,
                       aurore::Style style, aurore::FrameContext* ctx)
{
    (void)width;
    (void)height;
    (void)style;
    const float cx = x + CX, cy = y + CY;
    // The lines, at a strength of their full colour: a dark one (over a
    // light background) twice as strong, to show as much.
    const NVGcolor ink = ctx->theme["playguard/loading/line"];
    const bool dark_ink = 0.2126f * ink.r + 0.7152f * ink.g + 0.0722f * ink.b < 0.5f;
    const auto line = [ink, dark_ink](float strength) {
        return nvgTransRGBA(ink, (unsigned char)std::lround((dark_ink ? strength * 2.0f : strength) * ink.a * 255.0f));
    };

    // The glow.
    const NVGcolor glow_ink = ctx->theme["playguard/loading/glow"];
    NVGpaint glow = nvgRadialGradient(vg, cx, cy, 0.0f, 290.0f, glow_ink, nvgTransRGBA(glow_ink, 0));
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, 290.0f);
    nvgFillPaint(vg, glow);
    nvgFill(vg);

    // Two rings.
    nvgStrokeWidth(vg, 1.5f);
    for (const auto& [r, a] : { std::pair<float, float>{ 226.0f, 0.05f }, { 138.0f, 0.04f } }) {
        nvgBeginPath(vg);
        nvgCircle(vg, cx, cy, r);
        nvgStrokeColor(vg, line(a));
        nvgStroke(vg);
    }

    // The minutes, longer every five.
    for (int i = 0; i < 60; i++) {
        const float ang = (float)i * 2.0f * PI / 60.0f, len = i % 5 == 0 ? 16.0f : 7.0f;
        const float sx = std::sin(ang), sy = -std::cos(ang);
        nvgBeginPath(vg);
        nvgMoveTo(vg, cx + sx * 235.0f, cy + sy * 235.0f);
        nvgLineTo(vg, cx + sx * (235.0f + len), cy + sy * (235.0f + len));
        const float a = i % 5 == 0 ? 0.12f : 0.06f;
        nvgStrokeColor(vg, line(a));
        nvgStrokeWidth(vg, 1.5f);
        nvgStroke(vg);
    }

    // The day's track, three quarters of it in teal from the top.
    nvgStrokeWidth(vg, 18.0f);
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, 182.0f);
    nvgStrokeColor(vg, ctx->theme["playguard/loading/track"]);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgArc(vg, cx, cy, 182.0f, -PI / 2.0f, -PI / 2.0f + 1.5f * PI, NVG_CW);
    nvgStrokeColor(vg, ctx->theme["playguard/loading/arc"]);
    nvgLineCap(vg, NVG_ROUND);
    nvgStroke(vg);
    nvgLineCap(vg, NVG_BUTT);
}

aurore::View* LoadingDial::create()
{
    return new LoadingDial();
}
