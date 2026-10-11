// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/pt_ring.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

PtRing::PtRing()
{
    // Looked up once (a theme change needs a restart; each lookup builds a key).
    brls::Theme theme = brls::Application::getTheme();
    this->track = theme["cadran/track"];
    this->teal  = theme["cadran/teal"];
    this->amber = theme["cadran/amber"];
    this->coral = theme["cadran/coral"];

    this->setAxis(brls::Axis::COLUMN);
    this->setJustifyContent(brls::JustifyContent::CENTER);
    this->setAlignItems(brls::AlignItems::CENTER);
    this->setWidth(256);
    this->setHeight(256);
    this->setFocusable(false);

    this->value = new brls::Label();
    this->value->setFontSize(58);
    this->value->setHorizontalAlign(brls::HorizontalAlign::CENTER);
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
void PtRing::setValue(const std::string& text) { this->value->setText(text); }
void PtRing::setCaption(const std::string& text) { this->caption->setText(text); }

void PtRing::draw(NVGcontext* vg, float x, float y, float width, float height,
                  brls::Style style, brls::FrameContext* ctx)
{
    // PLAYGUARD_SIM_CADRAN_NORING=1 draws the labels alone (cost of the arcs).
    static const bool no_ring = std::getenv("PLAYGUARD_SIM_CADRAN_NORING") != nullptr;
    if (no_ring) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
    const float stroke = 20.0f;
    const float cx = x + width / 2.0f, cy = y + height / 2.0f;
    const float r = std::min(width, height) / 2.0f - stroke / 2.0f - 4.0f;

    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgStrokeWidth(vg, stroke);
    nvgStrokeColor(vg, this->track);
    nvgStroke(vg);

    if (this->used > 0.0f) {
        const float f = this->used;
        const NVGcolor color = f >= 1.0f ? this->coral : f >= 0.75f ? this->amber : this->teal;
        nvgBeginPath(vg);
        nvgArc(vg, cx, cy, r, -NVG_PI / 2.0f, -NVG_PI / 2.0f + f * 2.0f * NVG_PI, NVG_CW);
        nvgLineCap(vg, NVG_ROUND);
        nvgStrokeWidth(vg, stroke);
        nvgStrokeColor(vg, color);
        nvgStroke(vg);
        nvgLineCap(vg, NVG_BUTT);
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);   // the two labels
}

brls::View* PtRing::create() { return new PtRing(); }
