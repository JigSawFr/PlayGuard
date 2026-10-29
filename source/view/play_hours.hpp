// PlayHoursView — when the last 7 days' play happened, every game together:
// a row a day, Monday first (today's name in the accent), of 24 squares, one
// an hour, the stronger the more played (Cadran's teal, or the values'
// colour); 0, 6, 12, 18 and 24 under them.
// Read from the Activity tab's data (PlayStats.hours); its words follow the
// text size, as the view's height does.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <cstdint>

#include "util/pctl_ops_c.hpp"

class PlayHoursView : public aurore::View
{
  public:
    PlayHoursView();

    void show(const PlayStats& stats);   // hours not read: every square empty

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              aurore::Style style, aurore::FrameContext* ctx) override;

    static aurore::View* create();

  private:
    uint32_t hour_s[7][24] = {};   // by weekday, 0 = Sunday
    uint32_t top = 0;              // the most played hour
    int      today = -1;           // its weekday
    int      font;

    float scale() const;
};
