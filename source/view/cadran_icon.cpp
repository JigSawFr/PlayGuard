// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_icon.hpp"

CadranIcon::CadranIcon()
{
    this->color = brls::Application::getTheme()["brls/text"];
    this->setWidth(22);
    this->setHeight(22);
    this->setFocusable(false);
    this->registerStringXMLAttribute("icon", [this](std::string value) { this->setIcon(value); });
    this->registerColorXMLAttribute("color", [this](NVGcolor value) { this->setColor(value); });
}

void CadranIcon::setIcon(const std::string& name) { this->icon = name; }
void CadranIcon::setColor(NVGcolor value) { this->color = value; }

static void mark(NVGcontext* vg, float w, float h)
{
    // branding/mark.svg: a play button inside the daily play-time ring (75 %).
    nvgScale(vg, w / 256.0f, h / 256.0f);
    nvgBeginPath(vg);
    nvgCircle(vg, 128, 128, 104);
    nvgFillColor(vg, nvgRGB(0x0B, 0x14, 0x24));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgCircle(vg, 128, 128, 74);
    nvgStrokeWidth(vg, 20);
    nvgStrokeColor(vg, nvgRGB(0x1F, 0x33, 0x55));
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgArc(vg, 128, 128, 74, -NVG_PI / 2.0f, -NVG_PI / 2.0f + 0.75f * 2.0f * NVG_PI, NVG_CW);
    nvgLineCap(vg, NVG_ROUND);
    nvgStrokeColor(vg, nvgRGB(0x2E, 0xC4, 0xA6));
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 110, 98);
    nvgLineTo(vg, 162, 128);
    nvgLineTo(vg, 110, 158);
    nvgClosePath(vg);
    nvgFillColor(vg, nvgRGB(0xE8, 0xEE, 0xF7));
    nvgFill(vg);
    nvgStrokeWidth(vg, 10);
    nvgLineJoin(vg, NVG_ROUND);
    nvgStrokeColor(vg, nvgRGB(0xE8, 0xEE, 0xF7));
    nvgStroke(vg);
}

void CadranIcon::draw(NVGcontext* vg, float x, float y, float width, float height,
                      brls::Style style, brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    if (this->icon == "mark") {
        mark(vg, width, height);
        nvgRestore(vg);
        return;
    }
    // The boards' icons: a 24-unit box, 2.2-unit round strokes.
    nvgScale(vg, width / 24.0f, height / 24.0f);
    nvgStrokeColor(vg, this->color);
    nvgFillColor(vg, this->color);
    nvgStrokeWidth(vg, 2.2f);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    const std::string& i = this->icon;
    if (i == "today") {
        nvgBeginPath(vg); nvgCircle(vg, 12, 12, 9); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 10, 9); nvgLineTo(vg, 15, 12); nvgLineTo(vg, 10, 15); nvgClosePath(vg);
        nvgFill(vg); nvgStroke(vg);
    } else if (i == "timer") {
        nvgBeginPath(vg); nvgCircle(vg, 12, 12, 9); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 12, 7); nvgLineTo(vg, 12, 12); nvgLineTo(vg, 15, 14); nvgStroke(vg);
    } else if (i == "activity") {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 5, 20); nvgLineTo(vg, 5, 12);
        nvgMoveTo(vg, 12, 20); nvgLineTo(vg, 12, 5);
        nvgMoveTo(vg, 19, 20); nvgLineTo(vg, 19, 14);
        nvgStroke(vg);
    } else if (i == "shield") {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 12, 3); nvgLineTo(vg, 19, 6); nvgLineTo(vg, 19, 12);
        nvgBezierTo(vg, 19, 16.5f, 16, 19.5f, 12, 21);
        nvgBezierTo(vg, 8, 19.5f, 5, 16.5f, 5, 12);
        nvgLineTo(vg, 5, 6); nvgClosePath(vg); nvgStroke(vg);
    } else if (i == "lock") {
        nvgBeginPath(vg); nvgRoundedRect(vg, 5, 11, 14, 10, 2); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 8, 11); nvgLineTo(vg, 8, 8);
        nvgArc(vg, 12, 8, 4, NVG_PI, 0, NVG_CW); nvgLineTo(vg, 16, 11); nvgStroke(vg);
    } else if (i == "globe") {
        nvgBeginPath(vg); nvgCircle(vg, 12, 12, 9); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 3, 12); nvgLineTo(vg, 21, 12); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 12, 3); nvgBezierTo(vg, 15, 6.5f, 15, 17.5f, 12, 21); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 12, 3); nvgBezierTo(vg, 9, 6.5f, 9, 17.5f, 12, 21); nvgStroke(vg);
    } else if (i == "settings") {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 4, 7); nvgLineTo(vg, 13, 7); nvgMoveTo(vg, 19, 7); nvgLineTo(vg, 20, 7);
        nvgMoveTo(vg, 4, 17); nvgLineTo(vg, 7, 17); nvgMoveTo(vg, 13, 17); nvgLineTo(vg, 20, 17);
        nvgStroke(vg);
        nvgBeginPath(vg); nvgCircle(vg, 16, 7, 2.5f); nvgStroke(vg);
        nvgBeginPath(vg); nvgCircle(vg, 10, 17, 2.5f); nvgStroke(vg);
    } else if (i == "tools") {
        nvgBeginPath(vg); nvgCircle(vg, 16.5f, 7.5f, 4); nvgStroke(vg);
        nvgBeginPath(vg); nvgStrokeWidth(vg, 3); nvgMoveTo(vg, 13.5f, 10.5f); nvgLineTo(vg, 4.5f, 19.5f); nvgStroke(vg);
    } else if (i == "info") {
        nvgBeginPath(vg); nvgCircle(vg, 12, 12, 9); nvgStroke(vg);
        nvgBeginPath(vg); nvgMoveTo(vg, 12, 11); nvgLineTo(vg, 12, 16.5f); nvgStroke(vg);
        nvgBeginPath(vg); nvgCircle(vg, 12, 8, 1.3f); nvgFill(vg);
    } else if (i == "plus") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 3);
        nvgMoveTo(vg, 12, 5); nvgLineTo(vg, 12, 19); nvgMoveTo(vg, 5, 12); nvgLineTo(vg, 19, 12); nvgStroke(vg);
    } else if (i == "stop") {
        nvgBeginPath(vg); nvgRoundedRect(vg, 5, 5, 14, 14, 3); nvgFill(vg);
    } else if (i == "eye") {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 2, 12);
        nvgBezierTo(vg, 2, 12, 6, 5, 12, 5);
        nvgBezierTo(vg, 18, 5, 22, 12, 22, 12);
        nvgBezierTo(vg, 22, 12, 18, 19, 12, 19);
        nvgBezierTo(vg, 6, 19, 2, 12, 2, 12);
        nvgClosePath(vg); nvgStroke(vg);
        nvgBeginPath(vg); nvgCircle(vg, 12, 12, 3); nvgStroke(vg);
    } else if (i == "moon") {
        nvgBeginPath(vg);
        nvgArc(vg, 12, 12, 9, 0.11f, 4.60f, NVG_CW);
        nvgArc(vg, 16, 8, 7.07f, 3.93f, 0.785f, NVG_CCW);
        nvgClosePath(vg); nvgStroke(vg);
    } else if (i == "pause") {
        nvgBeginPath(vg); nvgMoveTo(vg, 9, 5); nvgLineTo(vg, 9, 19); nvgMoveTo(vg, 15, 5); nvgLineTo(vg, 15, 19); nvgStroke(vg);
    } else if (i == "list") {
        nvgBeginPath(vg); nvgMoveTo(vg, 4, 6); nvgLineTo(vg, 20, 6); nvgMoveTo(vg, 4, 12); nvgLineTo(vg, 20, 12);
        nvgMoveTo(vg, 4, 18); nvgLineTo(vg, 14, 18); nvgStroke(vg);
    } else if (i == "lines") {
        nvgBeginPath(vg); nvgMoveTo(vg, 3, 12); nvgLineTo(vg, 11, 12); nvgMoveTo(vg, 3, 6); nvgLineTo(vg, 21, 6);
        nvgMoveTo(vg, 3, 18); nvgLineTo(vg, 15, 18); nvgStroke(vg);
    } else if (i == "download") {
        nvgBeginPath(vg); nvgMoveTo(vg, 12, 3); nvgLineTo(vg, 12, 15); nvgMoveTo(vg, 7, 10); nvgLineTo(vg, 12, 15); nvgLineTo(vg, 17, 10);
        nvgMoveTo(vg, 4, 20); nvgLineTo(vg, 20, 20); nvgStroke(vg);
    } else if (i == "chevron-down") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 2.5f); nvgMoveTo(vg, 6, 9); nvgLineTo(vg, 12, 15); nvgLineTo(vg, 18, 9); nvgStroke(vg);
    } else if (i == "chevron-right") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 2.5f); nvgMoveTo(vg, 9, 6); nvgLineTo(vg, 15, 12); nvgLineTo(vg, 9, 18); nvgStroke(vg);
    } else if (i == "caret-up") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 3); nvgMoveTo(vg, 4, 15); nvgLineTo(vg, 12, 7); nvgLineTo(vg, 20, 15); nvgStroke(vg);
    } else if (i == "caret-down") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 3); nvgMoveTo(vg, 4, 9); nvgLineTo(vg, 12, 17); nvgLineTo(vg, 20, 9); nvgStroke(vg);
    } else if (i == "check") {
        nvgBeginPath(vg); nvgStrokeWidth(vg, 3);
        nvgMoveTo(vg, 4, 12); nvgLineTo(vg, 9, 17); nvgLineTo(vg, 20, 7); nvgStroke(vg);
    }
    nvgRestore(vg);
}

brls::View* CadranIcon::create() { return new CadranIcon(); }
