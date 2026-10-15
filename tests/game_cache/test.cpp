// Host tests for util/game_cache.hpp: the cached check of the installed
// games (round trip, another layout or a cut file refused, strings cut at
// their size) and what to check again (nothing while fresh and unchanged; a
// new game, a changed entry; every game once a day old, on another firmware,
// with the clock gone back or no full check yet; games gone dropped, the
// rest in the console's order); put() in place or added.
#include "check.h"
#include <cstdio>
#include <cstring>

#include "util/game_cache.hpp"

namespace
{
const uint32_t HOS = MAKEHOSVERSION(22, 1, 0);
const int64_t  NOW = 1791471600;

GameFacts game(uint64_t id, uint64_t updated, uint8_t event = 3)
{
    GameFacts g;
    std::memset(&g, 0, sizeof g);
    g.app_id = id;
    g.record_updated = updated;
    g.record_event = event;
    std::snprintf(g.name, sizeof g.name, "Game %llx", (unsigned long long)id);
    return g;
}

GameRecord record(uint64_t id, uint64_t updated, uint8_t event = 3)
{
    GameRecord r;
    std::memset(&r, 0, sizeof r);
    r.app_id = id;
    r.updated = updated;
    r.event = event;
    return r;
}

std::vector<uint64_t> ids(const std::vector<GameRecord>& v)
{
    std::vector<uint64_t> out;
    for (const GameRecord& r : v) out.push_back(r.app_id);
    return out;
}

std::vector<uint64_t> ids(const std::vector<GameFacts>& v)
{
    std::vector<uint64_t> out;
    for (const GameFacts& g : v) out.push_back(g.app_id);
    return out;
}

game_cache::Cache fresh_cache()
{
    game_cache::Cache c;
    c.checked_at = NOW - 3600;
    c.hos = HOS;
    c.games = { game(1, 100), game(2, 200), game(3, 300) };
    return c;
}

void test_file()
{
    game_cache::Cache c = fresh_cache();
    c.truncated = true;
    c.games[1].files_missing = true;
    std::strcpy(c.games[1].display_version, "1.4.0");
    const std::string bytes = game_cache::encode(c);

    game_cache::Cache back;
    CHECK(game_cache::decode(bytes, &back));
    CHECK(back.checked_at == c.checked_at && back.hos == HOS && back.truncated);
    CHECK(back.games.size() == 3 && back.games[1].files_missing && std::strcmp(back.games[1].display_version, "1.4.0") == 0);
    CHECK(std::memcmp(back.games.data(), c.games.data(), 3 * sizeof(GameFacts)) == 0);

    // Empty: fine.
    game_cache::Cache empty;
    CHECK(game_cache::decode(game_cache::encode(empty), &back) && back.games.empty() && back.checked_at == 0);

    // Cut, lengthened, another magic, version or layout: refused, `out` untouched.
    back = fresh_cache();
    back.checked_at = 42;
    CHECK(!game_cache::decode(bytes.substr(0, bytes.size() - 1), &back));
    CHECK(!game_cache::decode(bytes + "x", &back));
    CHECK(!game_cache::decode("", &back));
    CHECK(!game_cache::decode(bytes.substr(0, 8), &back));
    std::string other = bytes;
    other[0] = 'X';
    CHECK(!game_cache::decode(other, &back));
    other = bytes;
    other[4] = 9;   // version
    CHECK(!game_cache::decode(other, &back));
    other = bytes;
    other[8] ^= 1;   // sizeof(GameFacts)
    CHECK(!game_cache::decode(other, &back));
    CHECK(back.checked_at == 42);

    // Strings without their end: cut at their size; a content count too big: bounded.
    game_cache::Cache bad = fresh_cache();
    std::memset(bad.games[0].name, 'A', sizeof bad.games[0].name);
    std::memset(bad.games[0].display_version, 'B', sizeof bad.games[0].display_version);
    bad.games[0].content_count = 999;
    CHECK(game_cache::decode(game_cache::encode(bad), &back));
    CHECK(std::strlen(back.games[0].name) == sizeof back.games[0].name - 1);
    CHECK(std::strlen(back.games[0].display_version) == sizeof back.games[0].display_version - 1);
    CHECK(back.games[0].content_count == GAMECHECK_MAX_CONTENTS);
}

void test_expired()
{
    game_cache::Cache c = fresh_cache();
    CHECK(!game_cache::expired(c, NOW, HOS));
    CHECK(game_cache::expired(c, NOW, MAKEHOSVERSION(23, 0, 0)));            // another firmware
    CHECK(game_cache::expired(c, c.checked_at + game_cache::MAX_AGE_S, HOS)); // a day old
    CHECK(!game_cache::expired(c, c.checked_at + game_cache::MAX_AGE_S - 1, HOS));
    CHECK(game_cache::expired(c, c.checked_at - 1, HOS));                     // the clock went back
    c.checked_at = 0;
    CHECK(game_cache::expired(c, NOW, HOS));                                  // never fully checked
}

void test_to_check()
{
    // Fresh, nothing changed: nothing to check, everything kept.
    game_cache::Cache c = fresh_cache();
    auto check = game_cache::to_check(&c, { record(1, 100), record(2, 200), record(3, 300) }, NOW, HOS);
    CHECK(check.empty());
    CHECK((ids(c.games) == std::vector<uint64_t>{ 1, 2, 3 }));

    // A new game, an entry updated, another archived, one gone, a new order.
    c = fresh_cache();
    check = game_cache::to_check(&c, { record(3, 300), record(4, 400), record(2, 250), record(1, 100, 11) }, NOW, HOS);
    CHECK((ids(check) == std::vector<uint64_t>{ 4, 2, 1 }));
    CHECK((ids(c.games) == std::vector<uint64_t>{ 3, 2, 1 }));   // kept meanwhile, in the new order; 4 not yet
    CHECK(check[1].updated == 250 && check[2].event == 11);

    c = fresh_cache();
    check = game_cache::to_check(&c, { record(1, 100) }, NOW, HOS);
    CHECK(check.empty() && (ids(c.games) == std::vector<uint64_t>{ 1 }));   // 2 and 3 removed

    // Expired: every game, the cached ones still kept meanwhile.
    c = fresh_cache();
    check = game_cache::to_check(&c, { record(1, 100), record(2, 200), record(3, 300) }, NOW, MAKEHOSVERSION(23, 0, 0));
    CHECK((ids(check) == std::vector<uint64_t>{ 1, 2, 3 }) && c.games.size() == 3);

    // No cache: every game.
    game_cache::Cache none;
    check = game_cache::to_check(&none, { record(1, 100), record(2, 200) }, NOW, HOS);
    CHECK(check.size() == 2 && none.games.empty());
}

void test_put()
{
    game_cache::Cache c = fresh_cache();
    GameFacts g = game(2, 999);
    g.ticket_missing = true;
    game_cache::put(&c, g);
    CHECK(c.games.size() == 3 && c.games[1].record_updated == 999 && c.games[1].ticket_missing);
    game_cache::put(&c, game(9, 1));
    CHECK((ids(c.games) == std::vector<uint64_t>{ 1, 2, 3, 9 }));
}
}   // namespace

int main()
{
    test_file();
    test_expired();
    test_to_check();
    test_put();
    return CHECK_DONE("game cache assertions passed");
}
