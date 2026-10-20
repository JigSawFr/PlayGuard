// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_kit.hpp"

#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <yoga/Yoga.h>

#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "view/cadran_digits.hpp"

namespace cadran::kit
{

brls::Label* label(const std::string& text, float size, const char* color_key)
{
    auto* l = new brls::Label();
    l->setText(text);
    l->setFontSize(size);
    l->setTextColor(brls::Application::getTheme()[color_key]);
    return l;
}

CadranIcon* icon(const char* name, float size, const char* color_key)
{
    auto* view = new CadranIcon();
    view->setIcon(name);
    view->setWidth(size);
    view->setHeight(size);
    view->setColor(brls::Application::getTheme()[color_key]);
    return view;
}

brls::Box* card(bool focusable, bool grow)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setBackgroundColor(theme["cadran/surf"]);
    box->setBorderColor(theme["cadran/line"]);
    box->setBorderThickness(1);
    box->setCornerRadius(20);
    box->setPadding(20, 24, 20, 24);
    if (focusable) {
        box->setFocusable(true);
        box->setHideHighlightBackground(true);
        box->setHighlightCornerRadius(20);
    }
    if (grow) {
        box->setGrow(1.0f);
        box->setWidth(0);
    }
    return box;
}

brls::Box* top_card(bool grow)
{
    brls::Box* box = card(false, grow);
    box->setCornerRadius(22);
    box->setPadding(22, 26, 22, 26);
    return box;
}

brls::Box* kicker(const char* icon_name, const std::string& title)
{
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    head->addView(icon(icon_name, 18, "cadran/text2"));
    brls::Label* text = label(cadran::upper(title), 16, "cadran/text2");
    text->setSingleLine(true);
    text->setMarginLeft(8);
    head->addView(text);
    return head;
}

Mark::Mark(const char* color_key)
{
    this->setFocusable(false);
    this->setWidth(12);
    this->setHeight(12);
    this->set(color_key);
}

void Mark::set(const char* color_key)
{
    const std::string key = color_key;
    this->color = brls::Application::getTheme()[color_key];
    this->shape = key.rfind("cadran/teal", 0) == 0                                          ? Shape::CHECK
                  : key.rfind("cadran/amber", 0) == 0 || key.rfind("cadran/coral", 0) == 0 ? Shape::WARN
                                                                                           : Shape::DOT;
}

void Mark::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style, brls::FrameContext*)
{
    const float cx = x + width / 2, cy = y + height / 2;
    nvgBeginPath(vg);
    switch (this->shape) {
        case Shape::DOT:
            nvgCircle(vg, cx, cy, 4);
            nvgFillColor(vg, this->a(this->color));
            nvgFill(vg);
            break;
        case Shape::CHECK:
            nvgMoveTo(vg, x + 1.5f, cy + 0.5f);
            nvgLineTo(vg, x + 4.8f, cy + 3.6f);
            nvgLineTo(vg, x + width - 1.2f, y + 2.6f);
            nvgStrokeColor(vg, this->a(this->color));
            nvgStrokeWidth(vg, 2.2f);
            nvgLineCap(vg, NVG_ROUND);
            nvgLineJoin(vg, NVG_ROUND);
            nvgStroke(vg);
            break;
        case Shape::WARN: {
            // A rounded triangle, an exclamation mark cut out of it.
            nvgMoveTo(vg, cx, y + 0.5f);
            nvgLineTo(vg, x + width, y + height - 0.5f);
            nvgLineTo(vg, x, y + height - 0.5f);
            nvgClosePath(vg);
            nvgFillColor(vg, this->a(this->color));
            nvgFill(vg);
            const NVGcolor cut = this->a(brls::Application::getTheme()["brls/background"]);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, cx - 0.8f, y + 4.2f, 1.6f, 3.8f, 0.8f);
            nvgCircle(vg, cx, y + height - 2.6f, 0.95f);
            nvgFillColor(vg, cut);
            nvgFill(vg);
            break;
        }
    }
}

brls::Box* pill(const std::string& text, const char* dot_key)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setHeight(30);
    box->setPaddingLeft(14);
    box->setPaddingRight(16);
    box->setCornerRadius(15);
    box->setBorderColor(theme["cadran/line"]);
    box->setBorderThickness(1);
    box->setMarginRight(10);
    box->setMarginBottom(8);
    box->setHighlightCornerRadius(15);
    auto* mark = new Mark(dot_key);
    mark->setMarginRight(7);
    box->addView(mark);
    brls::Label* l = label(text, 15, "cadran/text2");
    l->setSingleLine(true);
    box->addView(l);
    return box;
}

brls::Box* chip(const std::string& text, float height)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setHeight(height);
    box->setPaddingLeft(22);
    box->setPaddingRight(22);
    box->setCornerRadius(height / 2);
    box->setBackgroundColor(theme["cadran/surf2"]);
    box->setBorderColor(theme["cadran/line"]);
    box->setBorderThickness(1);
    box->setFocusable(true);
    box->setHideHighlightBackground(true);
    box->setHighlightCornerRadius(height / 2);
    auto* digits = new CadranDigits();
    digits->setText(text);
    digits->setFontSize(17);
    digits->setFollowsTextSize(true);
    box->addView(digits);
    return box;
}

Empty empty(const char* icon_name)
{
    brls::Theme theme = brls::Application::getTheme();
    Empty e;
    e.box = card(false, false);
    e.box->setAlignItems(brls::AlignItems::CENTER);
    e.box->setPadding(32, 40, 32, 40);
    auto* circle = new brls::Box(brls::Axis::ROW);
    circle->setWidth(64);
    circle->setHeight(64);
    circle->setCornerRadius(32);
    circle->setBackgroundColor(theme["cadran/teal_soft"]);
    circle->setAlignItems(brls::AlignItems::CENTER);
    circle->setJustifyContent(brls::JustifyContent::CENTER);
    circle->addView(icon(icon_name, 28, "cadran/teal"));
    e.box->addView(circle);
    e.title = label("", 19, "cadran/text");
    e.title->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    e.title->setAlignSelf(brls::AlignSelf::STRETCH);   // wraps at the card's width
    e.title->setMarginTop(16);
    e.box->addView(e.title);
    e.text = label("", 15.5f, "cadran/text2");
    e.text->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    e.text->setAlignSelf(brls::AlignSelf::STRETCH);
    e.text->setMarginTop(6);
    e.box->addView(e.text);
    return e;
}

void wrap(brls::View* row)
{
    if (row) YGNodeStyleSetFlexWrap(row->getYGNode(), YGWrapWrap);
}

void draw_more_below(NVGcontext* vg, brls::ScrollingFrame* frame, float x, float y, float width, float height,
                     const char* color_key)
{
    // The content: the frame's tallest child (the other is its scroll bar).
    brls::View* content = nullptr;
    if (frame)
        for (brls::View* child : frame->getChildren())
            if (!content || child->getHeight() > content->getHeight()) content = child;
    if (!content) return;
    const float below = content->getHeight() - frame->getContentOffsetY() - height;
    if (below <= 1.0f) return;
    constexpr float FADE = 32.0f;
    NVGcolor solid = brls::Application::getTheme()[color_key];
    solid.a *= frame->getAlpha();
    NVGcolor clear = solid;
    clear.a = 0.0f;
    const float top = y + height - FADE;
    nvgBeginPath(vg);
    nvgRect(vg, x, top, width, FADE);
    nvgFillPaint(vg, nvgLinearGradient(vg, x, top, x, y + height, clear, solid));
    nvgFill(vg);
}

std::string without_chevron(const std::string& value)
{
    static const std::string chevron = "›";
    if (value.size() < chevron.size() || value.compare(value.size() - chevron.size(), chevron.size(), chevron) != 0)
        return value;
    std::string out = value.substr(0, value.size() - chevron.size());
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

const char* dot_of(NVGcolor c)
{
    const auto same = [&c](NVGcolor o) { return c.r == o.r && c.g == o.g && c.b == o.b; };
    if (same(ui::color_ok())) return "cadran/teal_fill";
    if (same(ui::color_warn())) return "cadran/amber_fill";
    if (same(ui::color_bad())) return "cadran/coral_fill";
    return "cadran/text3";
}

namespace
{
// {0} the day's name, {1} the same in lower case, {2} the day, {3} the month.
std::string date_text(int weekday, int day, int month)
{
    return brls::getStr("playguard/cadran/date", brls::getStr(fmt::format("playguard/days/{}", weekday)),
                        brls::getStr(fmt::format("playguard/days_lower/{}", weekday)), day,
                        brls::getStr(fmt::format("playguard/cadran/months/{}", month)));
}
}   // namespace

std::string long_date()
{
    const std::string date = ui::today_date();   // "2026-10-08"
    int month = 0, day = 0;
    if (date.size() >= 10) {
        month = std::atoi(date.substr(5, 2).c_str());
        day = std::atoi(date.substr(8, 2).c_str());
    }
    if (month < 1 || month > 12 || day < 1) return date;
    return date_text(ui::today_weekday(), day, month);
}

std::string long_date(const std::string& ymd)
{
    if (ymd.size() != 10 || ymd[4] != '-' || ymd[7] != '-') return ymd;
    std::tm tm = {};
    tm.tm_year = std::atoi(ymd.substr(0, 4).c_str()) - 1900;
    tm.tm_mon = std::atoi(ymd.substr(5, 2).c_str()) - 1;
    tm.tm_mday = std::atoi(ymd.substr(8, 2).c_str());
    tm.tm_hour = 12;   // clear of any change of hour
    if (tm.tm_mon < 0 || tm.tm_mon > 11 || tm.tm_mday < 1 || std::mktime(&tm) == (std::time_t)-1) return ymd;
    return date_text(tm.tm_wday, tm.tm_mday, tm.tm_mon + 1);
}

}   // namespace cadran::kit
