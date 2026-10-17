// Host tests for source/action/games_logic.cpp: the D-pad in the Games
// screen's grid of covers, and the icons read around the selected game.
#include "check.h"

#include "action/games_logic.hpp"

using namespace games_logic;

static void test_grid()
{
    // 14 covers, 6 a row: 0-5, 6-11, 12-13.
    CHECK(grid_neighbour(2, 14, 6, 1) == 8);
    CHECK(grid_neighbour(8, 14, 6, -1) == 2);
    CHECK(grid_neighbour(2, 14, 6, -1) == -1);   // the first row: nothing above
    CHECK(grid_neighbour(13, 14, 6, 1) == -1);   // the last row: nothing below
    // Down to a shorter last row: its last cover.
    CHECK(grid_neighbour(10, 14, 6, 1) == 13);
    CHECK(grid_neighbour(7, 14, 6, 1) == 13);
    CHECK(grid_neighbour(6, 14, 6, 1) == 12);
    CHECK(grid_neighbour(13, 14, 6, -1) == 7);
    // A full last row, a single row, and what makes no grid.
    CHECK(grid_neighbour(5, 12, 6, 1) == 11);
    CHECK(grid_neighbour(3, 4, 6, 1) == -1);
    CHECK(grid_neighbour(0, 0, 6, 1) == -1);
    CHECK(grid_neighbour(14, 14, 6, -1) == -1);
    CHECK(grid_neighbour(-1, 14, 6, 1) == -1);
    CHECK(grid_neighbour(1, 14, 0, 1) == -1);
    // One column: the list's order.
    CHECK(grid_neighbour(3, 5, 1, 1) == 4);
    CHECK(grid_neighbour(3, 5, 1, -1) == 2);
}

static void test_icon_window()
{
    using W = std::pair<size_t, size_t>;
    CHECK(icon_window(0, 100, 12, 30) == W(0, 31));
    CHECK(icon_window(50, 100, 12, 30) == W(38, 81));
    CHECK(icon_window(95, 100, 12, 30) == W(83, 100));
    CHECK(icon_window(5, 8, 12, 30) == W(0, 8));
    CHECK(icon_window(0, 0, 12, 30) == W(0, 0));
}

int main()
{
    test_grid();
    test_icon_window();
    return CHECK_DONE("games_logic tests passed");
}
