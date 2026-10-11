// Host tests for source/action/icon_logic.cpp: the icons' bytes kept for at
// most a fixed number of games, and the decoded icon shrunk to its drawn size.
#include "check.h"

#include "action/icon_logic.hpp"

using namespace icon_logic;

static std::vector<unsigned char> bytes(unsigned char v, size_t n = 3)
{
    return std::vector<unsigned char>(n, v);
}

static void test_lru()
{
    Lru lru(3);
    CHECK(lru.get(1) == nullptr);
    lru.put(1, bytes(1));
    lru.put(2, bytes(2));
    lru.put(3, bytes(3));
    CHECK(lru.size() == 3);
    // 1 used again: 2 is now the least recently used, and goes first.
    REQUIRE(lru.get(1) != nullptr);
    CHECK((*lru.get(1))[0] == 1);
    lru.put(4, bytes(4));
    CHECK(lru.size() == 3);
    CHECK(!lru.contains(2));
    CHECK(lru.contains(1) && lru.contains(3) && lru.contains(4));
    // contains() does not count as a use: 3 goes next.
    CHECK(lru.contains(3));
    lru.put(5, bytes(5));
    CHECK(!lru.contains(3));
    // Putting a kept game again replaces its bytes, without growing.
    lru.put(4, bytes(9, 5));
    CHECK(lru.size() == 3);
    REQUIRE(lru.get(4) != nullptr);
    CHECK(lru.get(4)->size() == 5 && (*lru.get(4))[0] == 9);
    // And made it the most recently used: 1 then 5 go before it.
    lru.put(6, bytes(6));
    lru.put(7, bytes(7));
    CHECK(lru.contains(4) && !lru.contains(1) && !lru.contains(5));

    // Many more than it keeps: never more than its capacity.
    Lru big(64);
    for (uint64_t id = 0; id < 1000; id++) big.put(id, bytes((unsigned char)id));
    CHECK(big.size() == 64);
    CHECK(big.contains(999) && big.contains(936) && !big.contains(935));
}

static void test_fit()
{
    CHECK(fit(256, 256, 88) == std::make_pair(88, 88));
    CHECK(fit(64, 64, 88) == std::make_pair(64, 64));     // smaller: its own size
    CHECK(fit(88, 88, 88) == std::make_pair(88, 88));
    CHECK(fit(256, 128, 88) == std::make_pair(88, 44));   // the shape kept
    CHECK(fit(128, 256, 88) == std::make_pair(44, 88));
    CHECK(fit(1000, 1, 88) == std::make_pair(88, 1));     // never 0 wide
    CHECK(fit(0, 10, 88) == std::make_pair(0, 10));       // a bad size is left alone
}

static void test_shrink()
{
    // 4 x 2 to 2 x 1: each pixel the mean of a 2 x 2 block, alpha too.
    const unsigned char src[4 * 2 * 4] = {
        0, 0, 0, 255,       100, 0, 0, 255,     10, 10, 10, 0,     10, 10, 10, 0,
        0, 100, 0, 255,     0, 0, 100, 255,     30, 30, 30, 0,     30, 30, 30, 100,
    };
    const std::vector<unsigned char> out = shrink(src, 4, 2, 2, 1);
    REQUIRE(out.size() == 2 * 1 * 4);
    CHECK(out[0] == 25 && out[1] == 25 && out[2] == 25 && out[3] == 255);
    CHECK(out[4] == 20 && out[5] == 20 && out[6] == 20 && out[7] == 25);

    // 256 to 88 (not a whole ratio): a flat colour stays the same everywhere.
    std::vector<unsigned char> flat(256 * 256 * 4);
    for (size_t i = 0; i < flat.size(); i += 4) {
        flat[i] = 200;
        flat[i + 1] = 100;
        flat[i + 2] = 50;
        flat[i + 3] = 255;
    }
    const std::vector<unsigned char> small = shrink(flat.data(), 256, 256, 88, 88);
    REQUIRE(small.size() == 88 * 88 * 4);
    bool same = true;
    for (size_t i = 0; i < small.size(); i += 4)
        same = same && small[i] == 200 && small[i + 1] == 100 && small[i + 2] == 50 && small[i + 3] == 255;
    CHECK(same);

    // The left half black, the right half white: still split down the middle.
    std::vector<unsigned char> halves(256 * 256 * 4, 0);
    for (int y = 0; y < 256; y++)
        for (int x = 128; x < 256; x++)
            for (int c = 0; c < 4; c++) halves[((size_t)y * 256 + x) * 4 + c] = 255;
    const std::vector<unsigned char> split = shrink(halves.data(), 256, 256, 88, 88);
    REQUIRE(split.size() == 88 * 88 * 4);
    CHECK(split[0] == 0 && split[(43) * 4] == 0);
    CHECK(split[(44) * 4] == 255 && split[(87) * 4] == 255);

    // The same size, or bigger, or a bad one: nothing.
    CHECK(shrink(src, 4, 2, 4, 2).size() == 4 * 2 * 4);
    CHECK(shrink(src, 4, 2, 8, 2).empty());
    CHECK(shrink(src, 4, 2, 0, 1).empty());
    CHECK(shrink(nullptr, 4, 2, 2, 1).empty());
}

int main()
{
    test_lru();
    test_fit();
    test_shrink();
    return CHECK_DONE("icon_logic tests passed");
}
