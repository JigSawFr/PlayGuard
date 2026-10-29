// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/play_days.hpp"

#include <algorithm>
#include <fmt/format.h>

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "ui/ui.hpp"

using namespace aurore::literals;

namespace
{
constexpr float BAR_MAX   = 56.0f;
constexpr float BAR_MIN   = 4.0f;
constexpr float BAR_WIDTH = 30.0f;
constexpr float LANE_WIDTH = BAR_WIDTH + 16.0f;   // the limit line overhangs the bar
constexpr float LABEL_FONT = 20.0f;   // as big as the text around it
constexpr float LEGEND_FONT = 18.0f;

float bar_height(uint64_t seconds, uint64_t top)
{
    return seconds ? BAR_MIN + (BAR_MAX - BAR_MIN) * (float)seconds / (float)top : BAR_MIN;
}
}   // namespace

PlayDaysView::PlayDaysView()
{
    this->setAxis(aurore::Axis::COLUMN);
    this->setAlignItems(aurore::AlignItems::STRETCH);
    this->setFocusable(false);

    auto* row = new aurore::Box(aurore::Axis::ROW);
    row->setJustifyContent(aurore::JustifyContent::SPACE_AROUND);
    row->setAlignItems(aurore::AlignItems::FLEX_END);
    this->addView(row);

    for (auto& c : this->cols) {
        auto* col = new aurore::Box(aurore::Axis::COLUMN);
        col->setAlignItems(aurore::AlignItems::CENTER);
        col->setJustifyContent(aurore::JustifyContent::FLEX_END);
        col->setGrow(1.0f);

        c.value = new aurore::Label();
        c.value->setFontSize(LABEL_FONT);
        c.value->setHorizontalAlign(aurore::HorizontalAlign::CENTER);
        c.value->setMarginBottom(8);   // clear of a limit line at the top of the lane
        // A fixed lane: the bar grows from its bottom, the limit line is
        // placed in it by height, whatever the bar.
        auto* lane = new aurore::Box(aurore::Axis::COLUMN);
        lane->setWidth(LANE_WIDTH);
        lane->setHeight(BAR_MAX);
        lane->setAlignItems(aurore::AlignItems::CENTER);
        lane->setJustifyContent(aurore::JustifyContent::FLEX_END);
        c.bar = new aurore::Rectangle();
        c.bar->setWidth(BAR_WIDTH);
        c.bar->setHeight(BAR_MIN);
        c.bar->setCornerRadius(4);
        c.limit = new aurore::Rectangle();
        c.limit->setWidth(LANE_WIDTH);
        c.limit->setHeight(2);
        c.limit->setPositionType(aurore::PositionType::ABSOLUTE);
        c.limit->setPositionLeft(0);
        c.limit->setPositionBottom(0);
        c.limit->setVisibility(aurore::Visibility::GONE);
        lane->addView(c.bar);
        lane->addView(c.limit);
        c.day = new aurore::Label();
        c.day->setFontSize(LABEL_FONT);
        c.day->setHorizontalAlign(aurore::HorizontalAlign::CENTER);
        c.day->setMarginTop(6);
        c.mark = new aurore::Rectangle();
        c.mark->setThemeColor("color", "cadran/text2");
        c.mark->setWidth(BAR_WIDTH);
        c.mark->setHeight(3);
        c.mark->setCornerRadius(1.5f);
        c.mark->setMarginTop(3);

        col->addView(c.value);
        col->addView(lane);
        col->addView(c.day);
        col->addView(c.mark);
        row->addView(col);
    }

    this->legend = new aurore::Label();
    this->legend->setFontSize(LEGEND_FONT);
    this->legend->setHorizontalAlign(aurore::HorizontalAlign::CENTER);
    this->legend->setThemeColor("textColor", "cadran/text3");
    this->legend->setMarginTop(10);
    this->addView(this->legend);
}

void PlayDaysView::show(const PlayStats& s, const uint16_t limits[7])
{
    uint32_t total[7] = {};
    for (uint32_t i = 0; i < s.count; i++)
        for (int k = 0; k < 7; k++) total[k] += s.games[i].day_s[k];
    this->show(total, s.day_wday, limits);
}

void PlayDaysView::show(const uint32_t day_s[7], const uint8_t day_wday[7], const uint16_t limits[7])
{
    uint64_t top = 3600;   // the busiest day (or the highest limit) sets the scale, at least 1 h
    for (int k = 0; k < 7; k++) {
        top = std::max<uint64_t>(top, day_s[k]);
        if (limits && limits[k] != PT_DAY_NOLIMIT) top = std::max<uint64_t>(top, (uint64_t)limits[k] * 60);
    }

    bool any_over = false;
    for (int col = 0; col < 7; col++) {
        const int k = 6 - col;   // days back
        Column& c = this->cols[col];
        const bool today = k == 0;
        const bool has_limit = limits && limits[k] != PT_DAY_NOLIMIT;
        const bool over = has_limit && day_s[k] > (uint64_t)limits[k] * 60;
        c.bar->setHeight(bar_height(day_s[k], top));
        // Played in the teal it is everywhere else (the week, Activity's cards);
        // nothing played: a faint stub. The faint ones through the view's
        // alpha, so the colour stays the theme's key.
        c.bar->setThemeColor("color", over ? "cadran/amber" : "cadran/teal_fill");
        c.bar->setAlpha(!day_s[k] ? 40 / 255.0f : !today && !over ? 120 / 255.0f : 1.0f);
        c.limit->setVisibility(has_limit ? aurore::Visibility::VISIBLE : aurore::Visibility::GONE);
        if (has_limit) {
            // 0 min (no play) sits on the floor of the lane.
            const float h = limits[k] ? bar_height((uint64_t)limits[k] * 60, top) : 0.0f;
            c.limit->setPositionBottom(std::max(0.0f, h - 1.0f));
            c.limit->setThemeColor("color", over ? "cadran/amber" : "cadran/text");
            c.limit->setAlpha(over ? 1.0f : 150 / 255.0f);
        }
        any_over = any_over || over;
        // Over the limit is marked by "!", not by the colour alone.
        const std::string played = day_s[k] ? ui::fmt_duration(day_s[k]) : "—";
        c.value->setText(over ? "! " + played : played);
        c.value->setThemeColor("textColor", over ? "cadran/amber" : today ? "cadran/text" : "cadran/text3");
        c.day->setText(aurore::getStr(fmt::format("playguard/days_short/{}", (int)day_wday[k])));
        c.day->setThemeColor("textColor", today ? "cadran/text2" : "cadran/text3");
        c.mark->setAlpha(today ? 1.0f : 0.0f);
    }

    // The order differs from the play timer's week (Monday first): say so,
    // and what the line and the amber mean when they are shown.
    std::string text = "playguard/chart/last_days"_i18n;
    if (limits) text += "\n" + (any_over ? "playguard/chart/limit_over"_i18n : "playguard/chart/limit_line"_i18n);
    this->legend->setText(text);
}

aurore::View* PlayDaysView::create()
{
    return new PlayDaysView();
}
