// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_kit.hpp"

#include <cstdlib>
#include <fmt/format.h>
#include <yoga/Yoga.h>

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
    box->setPadding(18, 20, 18, 20);
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

brls::Box* pill(const std::string& text, const char* dot_key)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setHeight(30);
    box->setPaddingLeft(12);
    box->setPaddingRight(12);
    box->setCornerRadius(15);
    box->setBorderColor(theme["cadran/line"]);
    box->setBorderThickness(1);
    box->setMarginRight(10);
    box->setMarginBottom(8);
    box->setHighlightCornerRadius(15);
    auto* dot = new brls::Rectangle(theme[dot_key]);
    dot->setWidth(8);
    dot->setHeight(8);
    dot->setCornerRadius(4);
    dot->setMarginRight(7);
    box->addView(dot);
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
    box->setPaddingLeft(18);
    box->setPaddingRight(18);
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
    box->addView(digits);
    return box;
}

void wrap(brls::View* row)
{
    if (row) YGNodeStyleSetFlexWrap(row->getYGNode(), YGWrapWrap);
}

bool press(brls::View* line)
{
    if (!line) return false;
    for (const std::shared_ptr<brls::Action>& action : line->getActions()) {
        if (!(*action == brls::BUTTON_A) || !action->isAvailable()) continue;
        action->getActionListener()(line);
        return true;
    }
    return false;
}

void follow(brls::View* view, brls::View* line)
{
    if (!view || !line) return;
    ui::set_visible(view, line->getVisibility() == brls::Visibility::VISIBLE);
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

std::string long_date()
{
    const std::string date = ui::today_date();   // "2026-10-08"
    int month = 0, day = 0;
    if (date.size() >= 10) {
        month = std::atoi(date.substr(5, 2).c_str());
        day = std::atoi(date.substr(8, 2).c_str());
    }
    if (month < 1 || month > 12 || day < 1) return date;
    // {0} the day's name, {1} the same in lower case, {2} the day, {3} the month.
    const int wd = ui::today_weekday();
    return brls::getStr("playguard/cadran/date", brls::getStr(fmt::format("playguard/days/{}", wd)),
                        brls::getStr(fmt::format("playguard/days_lower/{}", wd)), day,
                        brls::getStr(fmt::format("playguard/cadran/months/{}", month)));
}

}   // namespace cadran::kit
