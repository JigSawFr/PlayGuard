// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/pt_ring.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <yoga/Yoga.h>

#include <borealis/cadran/ui/effects.hpp>

#include "core/sim_env.h"

namespace
{
// The ring's stroke and radius for its size (the boards: a 20 stroke at 256,
// 26 at 340).
float stroke_of(float size) { return size * 20.0f / 256.0f; }
float radius_of(float size) { return size / 2.0f - stroke_of(size) / 2.0f - 4.0f; }
// The share of the inside's diameter the figure may take: about 30 px of the
// inside left on each side of "40 min" on Today's ring.
constexpr float FIGURE_SHARE = 0.70f;
}   // namespace

void PtRing::take_colours()
{
    brls::Theme theme = brls::Application::getTheme();
    this->track = theme["cadran/track"];
    this->teal  = theme["cadran/teal_fill"];
    this->amber = theme["cadran/amber_fill"];
    this->coral = theme["cadran/coral_fill"];
    this->tip_light = theme["playguard/ring/tip"];
}

void PtRing::onThemeChanged()
{
    this->take_colours();
    brls::Box::onThemeChanged();
}

PtRing::PtRing()
{
    this->take_colours();

    this->setAxis(brls::Axis::COLUMN);
    this->setJustifyContent(brls::JustifyContent::CENTER);
    this->setAlignItems(brls::AlignItems::CENTER);
    this->setWidth(256);
    this->setHeight(256);
    this->setFocusable(false);

    this->value = new CadranDigits();
    this->value->setSmallUnits(true);   // "40 min": the figures large
    this->value->setFontSize(58);
    this->value->setText("—");
    this->addView(this->value);

    this->caption = new brls::Label();
    this->caption->setFontSize(17);
    this->caption->setThemeColor("textColor", "cadran/text2");
    this->caption->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    this->caption->setMarginTop(2);
    this->addView(this->caption);
}

void PtRing::setFraction(float fraction)
{
    fraction = fraction > 1.0f ? 1.0f : fraction;
    if (fraction == this->used) return;
    // From one known value to another, the arc travels there; from none to
    // the first, it draws itself from nothing (the time left: from the full
    // ring down to it). Not with the motion off.
    const bool known  = this->used >= 0.0f;
    const bool travel = fraction >= 0.0f && effects::motion();
    this->used = fraction;
    if (travel) {
        const float from = known ? this->shown.getValue() : this->show_left ? 1.0f : 0.0f;
        this->shown.stop();
        this->shown.reset(from);
        this->shown.addStep(fraction, known ? 450 : 650, known ? brls::EasingFunction::quadraticOut : brls::EasingFunction::cubicOut);
        this->shown.start();
    } else {
        this->shown.stop();
        this->shown.reset(fraction);
    }
}

void PtRing::setExtra(float fraction) { this->extra = fraction < 0.0f ? 0.0f : fraction > 1.0f ? 1.0f : fraction; }
void PtRing::setValue(const std::string& text)
{
    this->value->setText(text);
    this->fit();
}
void PtRing::setCaption(const std::string& text) { this->caption->setText(text); }
void PtRing::setShowLeft(bool left) { this->show_left = left; }
void PtRing::setPulse(bool on, bool fast)
{
    this->pulse = on;
    this->pulse_fast = fast;
}
void PtRing::setTip(bool on) { this->tip = on; }
void PtRing::setRolls(bool rolls) { this->value->setRolls(rolls); }
void PtRing::setValueSize(float size)
{
    this->value->setFontSize(size);
    this->fit();
}

void PtRing::fit()
{
    const YGValue width = YGNodeStyleGetWidth(this->getYGNode());
    const float size = width.unit == YGUnitPoint ? width.value : 256.0f;
    const float inside = 2.0f * (radius_of(size) - stroke_of(size) / 2.0f);
    this->value->setFitWidth(inside * FIGURE_SHARE);
}
void PtRing::setCaptionSize(float size) { this->caption->setFontSize(size); }

void PtRing::draw(NVGcontext* vg, float x, float y, float width, float height,
                  brls::Style style, brls::FrameContext* ctx)
{
    // PLAYGUARD_SIM_CADRAN_NORING=1 draws the labels alone (cost of the arcs).
    static const bool no_ring = sim_env_flag("PLAYGUARD_SIM_CADRAN_NORING");
    if (no_ring) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
    const float stroke = stroke_of(std::min(width, height));
    const float cx = x + width / 2.0f, cy = y + height / 2.0f;
    const float r = radius_of(std::min(width, height));

    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgStrokeWidth(vg, stroke);
    nvgStrokeColor(vg, this->track);
    nvgStroke(vg);

    // Today's extra time: the end of the limit, before the top, lightly tinted.
    if (this->used >= 0.0f && this->extra > 0.0f && !this->show_left) {
        NVGcolor tint = this->teal;
        tint.a *= 0.35f;
        nvgBeginPath(vg);
        nvgArc(vg, cx, cy, r, -NVG_PI / 2.0f + (1.0f - this->extra) * 2.0f * NVG_PI, -NVG_PI / 2.0f + 2.0f * NVG_PI, NVG_CW);
        nvgStrokeWidth(vg, stroke);
        nvgStrokeColor(vg, tint);
        nvgStroke(vg);
    }

    if (this->used > 0.0f) {
        // The colour of where the arc goes; its length on the way there.
        const float f = this->shown.getValue();
        const float to = this->used;
        NVGcolor color = to >= 1.0f ? this->coral : to >= 0.75f ? this->amber : this->teal;
        if (this->pulse && effects::motion()) {
            // Once every 1.4 s (every second in the last minute: never faster,
            // nothing on screen may flicker), from full to a little over half,
            // and back.
            const unsigned period = this->pulse_fast ? 1000000u : 1400000u;
            const double t = (double)(brls::getCPUTimeUsec() % period) / (double)period;
            color.a *= 0.55f + 0.45f * (0.5f + 0.5f * (float)std::cos(t * 2.0 * NVG_PI));
        }
        const float arc = this->show_left ? 1.0f - f : f;
        if (arc <= 0.0f) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
        const float end = -NVG_PI / 2.0f + arc * 2.0f * NVG_PI;
        nvgBeginPath(vg);
        nvgArc(vg, cx, cy, r, -NVG_PI / 2.0f, end, NVG_CW);
        nvgLineCap(vg, NVG_ROUND);
        nvgStrokeWidth(vg, stroke);
        nvgStrokeColor(vg, color);
        nvgStroke(vg);
        nvgLineCap(vg, NVG_BUTT);
        // Where the time is: a brighter point at the arc's end, a soft light
        // around it (one gradient, a small square).
        if (this->tip) {
            const float tx = cx + r * std::cos(end), ty = cy + r * std::sin(end);
            NVGcolor light = nvgTransRGBA(this->tip_light, 0);
            NVGcolor core  = nvgTransRGBA(this->tip_light, (unsigned char)(color.a * 110.0f));
            nvgBeginPath(vg);
            nvgCircle(vg, tx, ty, stroke * 0.9f);
            nvgFillPaint(vg, nvgRadialGradient(vg, tx, ty, 0.0f, stroke * 0.9f, core, light));
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgCircle(vg, tx, ty, stroke * 0.22f);
            core.a = color.a * 0.85f;
            nvgFillColor(vg, core);
            nvgFill(vg);
        }
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);   // the figure and its caption
}

brls::View* PtRing::create() { return new PtRing(); }
