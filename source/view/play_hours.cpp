// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/play_hours.hpp"

#include <algorithm>
#include <cmath>
#include <fmt/format.h>

#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"

namespace
{
constexpr float ROW_H   = 20.0f;   // a square's height (its width: the room over 24)
constexpr float GAP     = 3.0f;
constexpr float NAME_W  = 58.0f;   // the days' names, before the squares
constexpr float TEXT    = 15.0f;
constexpr int   ORDER[7] = { 1, 2, 3, 4, 5, 6, 0 };   // Monday first
}   // namespace

PlayHoursView::PlayHoursView()
{
    this->setFocusable(false);
    this->font = brls::Application::getDefaultFont();
    // The rows, then the hours under them.
    this->setHeight(std::ceil(7 * (ROW_H + GAP) + TEXT * this->scale() * 1.6f));
}

float PlayHoursView::scale() const { return brls::Label::getFontScale(); }

void PlayHoursView::show(const PlayStats& s)
{
    for (auto& day : this->hour_s) std::fill(std::begin(day), std::end(day), 0u);
    this->top = 0;
    this->today = s.windows_ok ? s.day_wday[0] : -1;
    if (s.windows_ok && s.hours_ok)
        for (int k = 0; k < 7; k++)
            for (int h = 0; h < 24; h++) {
                const uint32_t v = s.hours.hour_s[k][h];
                this->hour_s[s.day_wday[k] % 7][h] = v;
                this->top = std::max(this->top, v);
            }
    this->setHeight(std::ceil(7 * (ROW_H + GAP) + TEXT * this->scale() * 1.6f));
}

void PlayHoursView::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                         brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    (void)height;
    const float s = this->scale();
    const NVGcolor ink = ui::color_text();
    const NVGcolor fill = brls::Application::getTheme()["cadran/teal"];
    const float name_w = NAME_W * s;
    const float cell_w = (width - name_w) / 24.0f;
    nvgFontFaceId(vg, this->font);
    nvgFontSize(vg, TEXT * s);
    for (int r = 0; r < 7; r++) {
        const int d = ORDER[r];
        const float top_y = y + r * (ROW_H + GAP);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, this->a(d == this->today ? fill : nvgTransRGBA(ink, 170)));
        nvgText(vg, x, top_y + ROW_H / 2, brls::getStr(fmt::format("playguard/days_short/{}", d)).c_str(), nullptr);
        for (int h = 0; h < 24; h++) {
            const uint32_t v = this->hour_s[d][h];
            // Nothing played: a faint square; else the stronger, the more.
            const NVGcolor c = v && this->top ? nvgTransRGBA(fill, (unsigned char)(60 + 195.0f * (float)v / (float)this->top))
                                              : nvgTransRGBA(ink, 18);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + name_w + h * cell_w, top_y, cell_w - GAP, ROW_H, 4);
            nvgFillColor(vg, this->a(c));
            nvgFill(vg);
        }
    }
    // The hours under the squares: midnight, 6, noon, 18, midnight.
    const float hours_y = y + 7 * (ROW_H + GAP) + 2;
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgFillColor(vg, this->a(nvgTransRGBA(ink, 150)));
    for (int h = 0; h <= 24; h += 6) {
        const float at = x + name_w + h * cell_w - (h == 24 ? GAP : 0);
        nvgTextAlign(vg, (h == 0 ? NVG_ALIGN_LEFT : h == 24 ? NVG_ALIGN_RIGHT : NVG_ALIGN_CENTER) | NVG_ALIGN_TOP);
        nvgText(vg, at, hours_y, fmt::format("{:02d}:00", h % 24 == 0 && h ? 24 : h).c_str(), nullptr);
    }
}

brls::View* PlayHoursView::create() { return new PlayHoursView(); }
