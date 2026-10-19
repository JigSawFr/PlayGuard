// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rating_logic.hpp"

namespace rating_logic
{

int age_of(const GameFacts& g, uint32_t org)
{
    if (!g.control_ok || org >= sizeof(g.rating_age)) return -1;
    return g.rating_age[org] < 0 ? -1 : g.rating_age[org];
}

bool blocked(const GameFacts& g, uint32_t org, uint8_t age)
{
    return age > 0 && age_of(g, org) > (int)age;
}

Blocked blocked_games(const std::vector<GameFacts>& games, uint32_t org, uint8_t age)
{
    Blocked out;
    for (const GameFacts& g : games) {
        if (age_of(g, org) >= 0) out.rated++;
        if (blocked(g, org, age)) out.games.push_back(&g);
    }
    return out;
}

}   // namespace rating_logic
