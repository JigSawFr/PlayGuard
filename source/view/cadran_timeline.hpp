// CadranTimeline — the rest of today on Cadran's Today, as one line: now at
// its left, the moment the time is up and the bedtime alarm as marks along
// it, each with its time and what it is under it ("16:40 time is up"); teal
// up to the first of them (amber in the last 15 minutes), the line's grey
// after. The times are in the display face, the words in the app's; both
// follow the text size (brls::Label's scale), as the view's height does.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

class CadranTimeline : public brls::View
{
  public:
    CadranTimeline();

    // Minutes since midnight: `now`, then when the time is up and when the
    // bedtime alarm rings (-1: not today). `time_up_caption` names the first
    // ("time is up", or "with 15 min more" while an amount is focused),
    // `bedtime_caption` the second ("bedtime" when empty).
    void show(int now, int time_up, int bedtime, const std::string& time_up_caption,
              const std::string& bedtime_caption = "");

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

  private:
    int now = 0, time_up = -1, bedtime = -1;
    std::string time_up_caption, bedtime_caption;
    int digits_font, text_font;

    float scale() const;   // the labels' scale (larger text)
};
