// games_logic — the Games screen's layout without Aurore: where the D-pad
// goes in the grid of covers, and which games' icons to read around the one
// selected. Plain C++ so the host tests can run it (tests/games_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <utility>

namespace games_logic
{

// The cell above (`dy` -1) or below (`dy` +1) cell `index` of a grid of
// `count` cells, `columns` a row: the one in the same column, or the last of
// a shorter last row; -1 when there is no row there.
int grid_neighbour(int index, int count, int columns, int dy);

// The cells whose icons are read with cell `focus` selected: from `behind`
// cells before it to `ahead` cells after it, within the `count` cells
// (first, one past the last).
std::pair<size_t, size_t> icon_window(size_t focus, size_t count, size_t behind, size_t ahead);

}   // namespace games_logic
