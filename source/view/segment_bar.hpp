// SegmentBar — a removal's bar for a small batch: one segment a game, side by
// side, the well while it waits, teal once removed, coral when it stays.
// Each segment eases into its colour. In place of the bar of a progress
// dialog (cadran::progress_body), as high.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <vector>

class SegmentBar : public brls::View
{
  public:
    enum class State { WAITING, DONE, FAILED };

    explicit SegmentBar(size_t count);
    void set(size_t index, State state);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    // Up to this many games, a removal shows its segments; past it, the bar.
    static constexpr size_t MAX = 20;

  private:
    std::vector<State> states;
    std::vector<brls::Time> since;   // when each segment changed (µs)
};
