// Host tests for action/rating_logic.hpp: a game's age by an organisation,
// and the games an age restriction blocks.
#include "check.h"
#include <cstring>
#include <vector>

#include "action/rating_logic.hpp"

namespace
{
constexpr uint32_t PEGI = 6, ESRB = 3;

GameFacts game(int pegi, int esrb = -1)
{
    GameFacts g;
    std::memset(&g, 0, sizeof g);
    g.control_ok = true;
    std::memset(g.rating_age, 0xFF, sizeof g.rating_age);
    g.rating_age[PEGI] = (int8_t)pegi;
    g.rating_age[ESRB] = (int8_t)esrb;
    return g;
}
}   // namespace

int main()
{
    // A game's age: the organisation's, -1 when it does not rate it or
    // nothing was read.
    GameFacts g = game(12, 10);
    CHECK(rating_logic::age_of(g, PEGI) == 12 && rating_logic::age_of(g, ESRB) == 10);
    CHECK(rating_logic::age_of(g, 0) == -1 && rating_logic::age_of(g, 0x20) == -1 && rating_logic::age_of(g, 999) == -1);
    g.control_ok = false;
    CHECK(rating_logic::age_of(g, PEGI) == -1);

    // Blocked: rated above the age; the age itself is allowed; 0, no restriction.
    g = game(12);
    CHECK(!rating_logic::blocked(g, PEGI, 12) && rating_logic::blocked(g, PEGI, 7) && !rating_logic::blocked(g, PEGI, 0));
    CHECK(!rating_logic::blocked(g, ESRB, 7));   // not rated by the organisation: not counted
    g = game(0);                                  // rated for every age
    CHECK(!rating_logic::blocked(g, PEGI, 3));

    // The games of a list, in order, and how many the organisation rates.
    const std::vector<GameFacts> games = { game(3), game(16), game(-1, 17), game(18), game(7) };
    rating_logic::Blocked b = rating_logic::blocked_games(games, PEGI, 12);
    CHECK(b.rated == 4 && b.games.size() == 2 && b.games[0] == &games[1] && b.games[1] == &games[3]);
    b = rating_logic::blocked_games(games, PEGI, 0);
    CHECK(b.rated == 4 && b.games.empty());
    b = rating_logic::blocked_games(games, ESRB, 12);
    CHECK(b.rated == 1 && b.games.size() == 1 && b.games[0] == &games[2]);
    b = rating_logic::blocked_games({}, PEGI, 12);
    CHECK(b.rated == 0 && b.games.empty());
    return CHECK_DONE("rating_logic age, blocked and list assertions passed");
}
