// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_timeline.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace aurore::literals;

namespace
{
constexpr float TRACK_Y   = 9;    // the line's centre, from the top
constexpr float TRACK_H   = 6;
constexpr float MARK_R    = 7;
constexpr float TIME_SIZE = 16;   // the times' size, before the text scale
constexpr float WORD_SIZE = 15;   // the words'
constexpr float GAP       = 8;    // between the line and the times
constexpr int   LOW_MIN   = 15;   // the last minutes, in amber

// "HH:MM", on the stack: the view writes it at every frame.
struct Clock
{
    char text[6];
};

Clock clock_text(int minutes)
{
    minutes = std::max(0, minutes) % (24 * 60);
    const int h = minutes / 60, m = minutes % 60;
    return { { char('0' + h / 10), char('0' + h % 10), ':', char('0' + m / 10), char('0' + m % 10), '\0' } };
}
}   // namespace

CadranTimeline::CadranTimeline()
{
    this->setFocusable(false);
    this->digits_font = aurore::Application::getFont("cadran/digits");
    if (this->digits_font == aurore::FONT_INVALID) this->digits_font = aurore::Application::getDefaultFont();
    this->text_font = aurore::Application::getDefaultFont();
    this->show(0, -1, -1, "", "");
}

float CadranTimeline::scale() const { return aurore::Label::getFontScale(); }

void CadranTimeline::show(int now_min, int time_up_min, int bedtime_min, const std::string& caption,
                          const std::string& bed_caption)
{
    this->now             = now_min;
    this->time_up         = time_up_min;
    this->bedtime         = bedtime_min;
    this->time_up_caption = caption;
    this->bedtime_caption = bed_caption.empty() ? "playguard/cadran/today/tl_bedtime"_i18n : bed_caption;
    // The line, then a time and its word under each mark.
    this->setHeight(std::ceil(TRACK_Y + MARK_R + GAP + TIME_SIZE * this->scale() * 1.3f));
}

void CadranTimeline::draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
                          aurore::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    (void)height;
    aurore::Theme theme = aurore::Application::getTheme();
    const NVGcolor line = theme["cadran/track"], ink = theme["cadran/text"], quiet = theme["cadran/text3"];
    const float s = this->scale();

    // The marks: the time up and the bedtime alarm, in their order.
    struct Mark
    {
        int at;
        std::string word;
        bool bed;
    };
    std::vector<Mark> marks;
    if (this->time_up >= 0) marks.push_back({ this->time_up, this->time_up_caption, false });
    if (this->bedtime >= 0) marks.push_back({ this->bedtime, this->bedtime_caption, true });
    std::sort(marks.begin(), marks.end(), [](const Mark& a, const Mark& b) { return a.at < b.at; });
    if (marks.empty()) return;
    const int first = marks.front().at;

    // A label: the time in the display face, its word after it, quieter.
    const float time_px = TIME_SIZE * s, word_px = WORD_SIZE * s, space = 6 * s;
    const auto advance = [vg](int font, float size, const char* text) {
        float b[4];
        nvgFontFaceId(vg, font);
        nvgFontSize(vg, size);
        return nvgTextBounds(vg, 0, 0, text, nullptr, b);
    };
    const auto label_width = [&](const Mark& m) {
        return advance(this->digits_font, time_px, clock_text(m.at).text) + space +
               advance(this->text_font, word_px, m.word.c_str());
    };
    // At least an hour wide; the last mark short of the right edge by half
    // its label.
    const int span = std::max(60, marks.back().at - this->now);
    const float usable = std::max(1.0f, width - MARK_R - label_width(marks.back()) / 2);
    const auto pos = [&](int at) { return x + MARK_R + usable * (float)(at - this->now) / (float)span; };

    const float cy = y + TRACK_Y;
    // The line, its whole length.
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, cy - TRACK_H / 2, width, TRACK_H, TRACK_H / 2);
    nvgFillColor(vg, this->a(line));
    nvgFill(vg);
    // Play allowed, now to the first mark: amber in its last minutes.
    const NVGcolor fill = first - this->now <= LOW_MIN ? theme["cadran/amber"] : theme["cadran/teal"];
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, cy - TRACK_H / 2, std::max(TRACK_H, pos(first) - x), TRACK_H, TRACK_H / 2);
    nvgFillColor(vg, this->a(fill));
    nvgFill(vg);
    // Now: a small disc at the start.
    nvgBeginPath(vg);
    nvgCircle(vg, x + MARK_R, cy, MARK_R - 2);
    nvgFillColor(vg, this->a(ink));
    nvgFill(vg);

    // Each mark, its label centred under it but kept inside the view and
    // clear of the one before.
    float min_left = x;
    const float ty = cy + MARK_R + GAP;
    for (const Mark& m : marks) {
        const float mx = pos(m.at);
        const bool next = m.at == first;
        nvgBeginPath(vg);
        nvgCircle(vg, mx, cy, MARK_R);
        nvgFillColor(vg, this->a(m.bed ? theme["cadran/surf"] : next ? fill : quiet));
        nvgFill(vg);
        nvgStrokeColor(vg, this->a(m.bed ? (next ? fill : ink) : theme["aurore/background"]));
        nvgStrokeWidth(vg, 2);
        nvgStroke(vg);

        const float w = label_width(m);
        const float left = std::max(min_left, std::min(mx - w / 2, x + width - w));
        const Clock time = clock_text(m.at);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
        nvgFontFaceId(vg, this->digits_font);
        nvgFontSize(vg, time_px);
        nvgFillColor(vg, this->a(next ? ink : quiet));
        const float tw = nvgText(vg, left, ty + time_px, time.text, nullptr) - left;
        nvgFontFaceId(vg, this->text_font);
        nvgFontSize(vg, word_px);
        nvgFillColor(vg, this->a(quiet));
        nvgText(vg, left + tw + space, ty + time_px, m.word.c_str(), nullptr);
        min_left = left + w + 18 * s;
    }
}
