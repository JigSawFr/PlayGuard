// Host tests for source/action/sync_icons_logic.cpp: which games' icons the
// remote link publishes, what is left to send or clear, and icons.txt.
#include "check.h"

#include "action/sync_icons_logic.hpp"

using namespace sync_icons_logic;

static void test_wanted()
{
    const uint64_t A = 0x0100000000010000ULL, B = 0x01000A10041EA000ULL, C = 0x0100152000022000ULL,
                   D = 0x01006A800016E000ULL, E = 0x0100ABF008968000ULL;
    std::vector<Played> games = {
        { A, 1000, 0 },      // played, not this week
        { B, 5000, 600 },    // this week
        { C, 0, 0 },         // never: left out
        { D, 0, 1200 },      // this week, its all-time statistics not read
        { E, 3000, 0 },
        { 0, 9000, 60 },     // no id
        { B, 5000, 600 },    // twice: once
    };
    // This week's first, the most recent of them first (one unknown: last of them).
    CHECK(wanted(games) == std::vector<uint64_t>({ B, D, E, A }));
    CHECK(wanted(games, 2) == std::vector<uint64_t>({ B, D }));
    CHECK(wanted(games, 0).empty());
    CHECK(wanted({}).empty());
    // As recent: the one played more this week first.
    CHECK(wanted({ { A, 100, 60 }, { B, 100, 120 } }) == std::vector<uint64_t>({ B, A }));
    // A large library: ICONS_MAX at most, the most recent kept.
    std::vector<Played> many;
    for (uint64_t i = 1; i <= 100; i++) many.push_back({ 0x0100000000000000ULL + i, i, 0 });
    const auto kept = wanted(many);
    CHECK(kept.size() == ICONS_MAX);
    CHECK(kept.front() == 0x0100000000000000ULL + 100 && kept.back() == 0x0100000000000000ULL + 100 - ICONS_MAX + 1);
}

static void test_plan()
{
    const std::string t = target_of("broker.lan", 1883, "playguard/a1b2c3d4");
    CHECK(t.size() == 8 && t.find("broker") == std::string::npos);
    CHECK(t == target_of("broker.lan", 1883, "playguard/a1b2c3d4"));
    CHECK(t != target_of("broker.lan", 8883, "playguard/a1b2c3d4"));
    CHECK(t != target_of("broker.lan", 1883, "playguard/21b2c3d4"));   // the emuMMC's device
    CHECK(t != target_of("other.lan", 1883, "playguard/a1b2c3d4"));

    // Nothing sent yet: every one.
    Sent sent;
    Plan p = plan({ 1, 2, 3 }, sent, t);
    CHECK(p.send == std::vector<uint64_t>({ 1, 2, 3 }) && p.clear.empty() && !p.empty());
    record(sent, t, 1, 40000, false);
    record(sent, t, 2, 0, false);   // none to publish: not tried again
    record(sent, t, 3, 12000, false);
    CHECK(sent.target == t && sent.icons.size() == 3);
    CHECK(plan({ 1, 2, 3 }, sent, t).empty());
    CHECK(plan({ 3, 1, 2 }, sent, t).empty());   // the order does not matter
    // A game played lately comes in, one goes out of the set.
    p = plan({ 4, 1, 2 }, sent, t);
    CHECK(p.send == std::vector<uint64_t>({ 4 }) && p.clear == std::vector<uint64_t>({ 3 }));
    record(sent, t, 4, 30000, false);
    record(sent, t, 3, 0, true);
    CHECK(sent.icons.size() == 3 && !sent.icons.count(3));
    // publish_icons turned off: every one cleared.
    p = plan({}, sent, t);
    CHECK(p.send.empty() && p.clear == std::vector<uint64_t>({ 1, 2, 4 }));
    // Another broker: every wanted one sent there, none cleared (the old ones
    // are on the other broker); the first one recorded starts its list anew.
    const std::string other = target_of("other.lan", 1883, "playguard/a1b2c3d4");
    p = plan({ 1, 5 }, sent, other);
    CHECK(p.send == std::vector<uint64_t>({ 1, 5 }) && p.clear.empty());
    record(sent, other, 5, 100, false);
    CHECK(sent.target == other && sent.icons.size() == 1 && sent.icons.count(5));
    CHECK(plan({ 1, 5 }, sent, other).send == std::vector<uint64_t>({ 1 }));
}

static void test_file()
{
    Sent sent;
    record(sent, "0a1b2c3d", 0x0100000000010000ULL, 40123, false);
    record(sent, "0a1b2c3d", 0x01000A10041EA000ULL, 0, false);
    const std::string t = text(sent);
    CHECK(t.find("target=0a1b2c3d\n") != std::string::npos);
    CHECK(t.find("0100000000010000=40123\n") != std::string::npos);
    CHECK(t.find("01000A10041EA000=0\n") != std::string::npos);
    const Sent back = parse(t);
    CHECK(back.target == sent.target && back.icons == sent.icons);
    // Windows line ends, lower case, and lines it does not understand.
    const Sent odd = parse("target=ffffffff\r\n0100000000010000=12\r\n01000a10041ea000=7\nnoequals\n"
                           "0100=5\n0100000000010000X=5\n0100000000020000=-1\n0100000000030000=\n# a comment=1\n");
    CHECK(odd.target == "ffffffff" && odd.icons.size() == 2);
    CHECK(odd.icons.at(0x0100000000010000ULL) == 12 && odd.icons.at(0x01000A10041EA000ULL) == 7);
    CHECK(parse("").target.empty() && parse("").icons.empty());
    // An empty file: everything is sent (no target matches).
    CHECK(plan({ 1 }, parse(""), "0a1b2c3d").send.size() == 1);
    CHECK(topic(0x01000A10041EA000ULL) == "icon/01000A10041EA000");
}

int main()
{
    test_wanted();
    test_plan();
    test_file();
    return CHECK_DONE("sync_icons_logic: all tests passed");
}
