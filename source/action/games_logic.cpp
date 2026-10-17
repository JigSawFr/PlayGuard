// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/games_logic.hpp"

#include <algorithm>

namespace games_logic
{

int grid_neighbour(int index, int count, int columns, int dy)
{
    if (columns <= 0 || index < 0 || index >= count) return -1;
    const int row = index / columns + dy;
    const int rows = (count + columns - 1) / columns;
    if (row < 0 || row >= rows) return -1;
    return std::min(row * columns + index % columns, count - 1);
}

std::pair<size_t, size_t> icon_window(size_t focus, size_t count, size_t behind, size_t ahead)
{
    const size_t first = focus > behind ? focus - behind : 0;
    const size_t end = std::min(count, focus + ahead + 1);
    return { std::min(first, end), end };
}

}   // namespace games_logic
