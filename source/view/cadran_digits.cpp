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

void CadranDigits::setFitWidth(float width)
{
    this->fitWidth = width;
    this->measure();
}

void CadranDigits::measure()
{
    NVGcontext* vg = brls::Application::getNVGContext();
    nvgFontFaceId(vg, this->font);
    nvgFontSize(vg, this->fontSize);
    nvgTextLetterSpacing(vg, -0.02f * this->fontSize);
    float bounds[4] = { 0, 0, 0, 0 };
    float advance = nvgTextBounds(vg, 0, 0, this->text.c_str(), nullptr, bounds);
    this->drawSize = this->fontSize;
    if (this->fitWidth > 0.0f && advance > this->fitWidth) {
        this->drawSize = std::max(10.0f, this->fontSize * this->fitWidth / advance);
        nvgFontSize(vg, this->drawSize);
        nvgTextLetterSpacing(vg, -0.02f * this->drawSize);
        advance = nvgTextBounds(vg, 0, 0, this->text.c_str(), nullptr, bounds);
    }
    nvgTextLetterSpacing(vg, 0);
    this->setWidth(std::ceil(advance));
    this->setHeight(std::ceil(this->fontSize * 1.15f));
    this->invalidate();
}

void CadranDigits::draw(NVGcontext* vg, float x, float y, float width, float height,
                        brls::Style style, brls::FrameContext* ctx)
{
    (void)width;
    (void)style;
    (void)ctx;
    nvgFontFaceId(vg, this->font);
    nvgFontSize(vg, this->drawSize);
    nvgTextLetterSpacing(vg, -0.02f * this->drawSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, this->color);
    nvgText(vg, x, y + height / 2.0f, this->text.c_str(), nullptr);
    nvgTextLetterSpacing(vg, 0);
}

brls::View* CadranDigits::create() { return new CadranDigits(); }
