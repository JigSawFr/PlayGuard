// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_digits.hpp"

#include <algorithm>
#include <cmath>

CadranDigits::CadranDigits()
{
    this->color = brls::Application::getTheme()["brls/text"];
    this->font = brls::Application::getFont("cadran/digits");
    if (this->font == brls::FONT_INVALID) this->font = brls::Application::getDefaultFont();
    this->setFocusable(false);
    this->registerStringXMLAttribute("text", [this](std::string value) { this->setText(value); });
    this->registerFloatXMLAttribute("fontSize", [this](float value) { this->setFontSize(value); });
    this->registerColorXMLAttribute("textColor", [this](NVGcolor value) { this->setTextColor(value); });
    this->registerFloatXMLAttribute("fitWidth", [this](float value) { this->setFitWidth(value); });
    this->registerBoolXMLAttribute("followsTextSize", [this](bool value) { this->setFollowsTextSize(value); });
    this->measure();
}

void CadranDigits::setText(const std::string& value)
{
    this->text = value;
    this->measure();
}

void CadranDigits::setFontSize(float value)
{
    this->fontSize = value;
    this->measure();
}

void CadranDigits::setTextColor(NVGcolor value) { this->color = value; }

void CadranDigits::setFollowsTextSize(bool follows)
{
    this->followsTextSize = follows;
    this->measure();
}

void CadranDigits::setFitWidth(float width)
{
    this->fitWidth = width;
    this->measure();
}

void CadranDigits::setSmallUnits(bool small)
{
    this->smallUnits = small;
    this->measure();
}

namespace
{
constexpr float UNIT_SHARE = 0.5f;   // a word's size beside the figures
}   // namespace

std::vector<CadranDigits::Run> CadranDigits::runs() const
{
    std::vector<Run> out;
    const bool any = this->smallUnits && std::any_of(this->text.begin(), this->text.end(),
                                                     [](char c) { return c >= '0' && c <= '9'; });
    if (!any) return { { this->text, true } };
    for (char c : this->text) {
        const bool figure = c >= '0' && c <= '9';
        if (out.empty() || out.back().figure != figure) out.push_back({ "", figure });
        out.back().text += c;   // a word's UTF-8 bytes stay together: none is a figure
    }
    return out;
}

float CadranDigits::advance(NVGcontext* vg, float size) const
{
    float total = 0.0f, bounds[4] = { 0, 0, 0, 0 };
    for (const Run& run : this->runs()) {
        const float s = run.figure ? size : size * UNIT_SHARE;
        nvgFontSize(vg, s);
        nvgTextLetterSpacing(vg, -0.02f * s);
        total += nvgTextBounds(vg, 0, 0, run.text.c_str(), nullptr, bounds);
    }
    nvgTextLetterSpacing(vg, 0);
    return total;
}

void CadranDigits::measure()
{
    NVGcontext* vg = brls::Application::getNVGContext();
    nvgFontFaceId(vg, this->font);
    const float size = this->fontSize * (this->followsTextSize ? brls::Label::getFontScale() : 1.0f);
    float width = this->advance(vg, size);
    this->drawSize = size;
    if (this->fitWidth > 0.0f && width > this->fitWidth) {
        this->drawSize = std::max(10.0f, size * this->fitWidth / width);
        width = this->advance(vg, this->drawSize);
    }
    this->setWidth(std::ceil(width));
    this->setHeight(std::ceil(size * 1.15f));
    this->invalidate();
}

void CadranDigits::draw(NVGcontext* vg, float x, float y, float width, float height,
                        brls::Style style, brls::FrameContext* ctx)
{
    (void)width;
    (void)style;
    (void)ctx;
    nvgFontFaceId(vg, this->font);
    nvgFillColor(vg, this->color);
    const std::vector<Run> runs = this->runs();
    if (runs.size() == 1) {
        nvgFontSize(vg, this->drawSize);
        nvgTextLetterSpacing(vg, -0.02f * this->drawSize);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(vg, x, y + height / 2.0f, this->text.c_str(), nullptr);
    } else {
        // One baseline, the figures centred as a single-size text would be.
        float ascender = 0.0f, descender = 0.0f, line = 0.0f;
        nvgFontSize(vg, this->drawSize);
        nvgTextMetrics(vg, &ascender, &descender, &line);
        const float baseline = y + height / 2.0f + (ascender + descender) / 2.0f;
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
        float at = x;
        for (const Run& run : runs) {
            const float s = run.figure ? this->drawSize : this->drawSize * UNIT_SHARE;
            nvgFontSize(vg, s);
            nvgTextLetterSpacing(vg, -0.02f * s);
            at = nvgText(vg, at, baseline, run.text.c_str(), nullptr);
        }
    }
    nvgTextLetterSpacing(vg, 0);
}

brls::View* CadranDigits::create() { return new CadranDigits(); }
