// CoverStrip — the covers of the games being checked, side by side, leafed
// through as the check goes on (Console › Check the installed games): the
// game under way in the middle, full size; the ones checked to its left and
// the ones to come to its right, narrower the further they are, as the pages
// of a book turned or still to turn, the checked ones dimmed. Each game
// checked slides the strip one cover on, eased. A cover is the game's icon
// (action/game_icons), its placeholder until it is read. Focusable without a
// highlight: it holds the focus while the check runs, nothing else showing.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <map>
#include <vector>

#include "ui/cadran_modal.hpp"
#include "util/pctl_ops_c.hpp"

class CoverStrip : public brls::View
{
  public:
    CoverStrip();
    ~CoverStrip() override;

    // The games in the order they are checked; back to the first.
    void set_games(const std::vector<u64>& ids);
    // What became of a game (a removal's): gone, its cover floats away and
    // fades; failed, it stays, a coral "!" on its corner.
    enum class Mark { NONE, GONE, FAILED };
    void mark(size_t index, Mark what);
    // `done` games checked: the next one moves to the middle.
    void set_done(u32 done);
    // The games whose icons are wanted soon (around the middle), for
    // game_icons::load.
    std::vector<u64> wanted() const;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

  private:
    int texture(u64 game);    // its icon, the placeholder, or 0
    float at(brls::Time now) const;   // the position drawn (a game's index, fractional between two)

    std::vector<u64> ids;
    std::vector<Mark> marks;            // by game, as `ids`
    std::vector<brls::Time> marked_at;  // when each was marked (µs)
    std::map<u64, int> owned;   // textures made for this strip, freed with it
    float from = 0, to = 0;     // the position eases from `from` to `to` ...
    brls::Time since = 0;       // ... since then (µs)
};

// A dialog's strip (a batch's: the data checked, the games removed), on top
// of its progress body, not a focus stop (the dialog's buttons keep it).
CoverStrip* cover_strip_above(cadran::Progress& body, const std::vector<u64>& ids);
// The icons the strip is about to show, read (game_icons).
void cover_strip_read(CoverStrip* strip);
