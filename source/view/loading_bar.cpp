// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/loading_bar.hpp"

#include <algorithm>

namespace
{
// The brand palette (branding/*.svg).
const NVGcolor TRACK      = nvgRGB(0x1F, 0x33, 0x55);
const NVGcolor TEAL       = nvgRGB(0x2E, 0xC4, 0xA6);
const NVGcolor TEAL_LIGHT = nvgRGB(0x9A, 0xF0, 0xDC);

// How long the fill takes to reach a new value, in µs.
constexpr float EASE_US = 350000.0f;

float eased(float t)   // ease-out cubic, 0..1
{
    const float r = 1.0f - t;
    return 1.0f - r * r * r;
}
}   // namespace

LoadingBar::LoadingBar()
{
    this->setFocusable(false);
}

void LoadingBar::setProgress(float value)
{
    value = std::clamp(value, 0.0f, 1.0f);
    // From where the fill is drawn now, not from the last value asked.
    const brls::Time now = brls::getCPUTimeUsec();
    const float t = this->since ? std::min(1.0f, (float)(now - this->since) / EASE_US) : 1.0f;
    this->from  = this->from + (this->to - this->from) * eased(t);
    this->to    = value;
    this->since = now;
}

void LoadingBar::draw(NVGcontext* vg, float x, float y, float width, float height,
                      brls::Style style, brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    const float radius = height / 2.0f;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, radius);
    nvgFillColor(vg, TRACK);
    nvgFill(vg);

    const float t = this->since ? std::min(1.0f, (float)(brls::getCPUTimeUsec() - this->since) / EASE_US) : 1.0f;
    const float value = this->from + (this->to - this->from) * eased(t);
    if (value <= 0.0f) return;
    const float w = std::max(height, width * value);
    // Teal, lighter toward its end: the end that moves.
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, height, radius);
    nvgFillPaint(vg, nvgLinearGradient(vg, x, y, x + w, y, TEAL, TEAL_LIGHT));
    nvgFill(vg);
}

brls::View* LoadingBar::create()
{
    return new LoadingBar();
}
