// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/pt_ring.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <yoga/Yoga.h>

namespace
{
// The ring's stroke and radius for its size (the boards: a 20 stroke at 256,
// 26 at 340).
float stroke_of(float size) { return size * 20.0f / 256.0f; }
float radius_of(float size) { return size / 2.0f - stroke_of(size) / 2.0f - 4.0f; }
// The share of the inside's diameter the figure may take: about 30 px of the
// inside left on each side of "40 min" on the Overview's ring.
constexpr float FIGURE_SHARE = 0.70f;
}   // namespace

PtRing::PtRing()
{
    // Looked up once (a theme change needs a restart; each lookup builds a key).
    brls::Theme theme = brls::Application::getTheme();
    this->track = theme["cadran/track"];
    this->teal  = theme["cadran/teal_fill"];
    this->amber = theme["cadran/amber_fill"];
    this->coral = theme["cadran/coral_fill"];

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
    this->caption->setTextColor(theme["cadran/text2"]);
    this->caption->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    this->caption->setMarginTop(2);
    this->addView(this->caption);
}

void PtRing::setFraction(float value) { this->used = value > 1.0f ? 1.0f : value; }
void PtRing::setValue(const std::string& text)
{
    this->value->setText(text);
    this->fit();
}
void PtRing::setCaption(const std::string& text) { this->caption->setText(text); }
void PtRing::setShowLeft(bool left) { this->show_left = left; }
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
    static const bool no_ring = std::getenv("PLAYGUARD_SIM_CADRAN_NORING") != nullptr;
    if (no_ring) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
    const float stroke = stroke_of(std::min(width, height));
    const float cx = x + width / 2.0f, cy = y + height / 2.0f;
    const float r = radius_of(std::min(width, height));

    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgStrokeWidth(vg, stroke);
    nvgStrokeColor(vg, this->track);
    nvgStroke(vg);

    if (this->used > 0.0f) {
        const float f = this->used;
        const NVGcolor color = f >= 1.0f ? this->coral : f >= 0.75f ? this->amber : this->teal;
        const float arc = this->show_left ? 1.0f - f : f;
        if (arc <= 0.0f) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
        nvgBeginPath(vg);
        nvgArc(vg, cx, cy, r, -NVG_PI / 2.0f, -NVG_PI / 2.0f + arc * 2.0f * NVG_PI, NVG_CW);
        nvgLineCap(vg, NVG_ROUND);
        nvgStrokeWidth(vg, stroke);
        nvgStrokeColor(vg, color);
        nvgStroke(vg);
        nvgLineCap(vg, NVG_BUTT);
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);   // the figure and its caption
}

brls::View* PtRing::create() { return new PtRing(); }
