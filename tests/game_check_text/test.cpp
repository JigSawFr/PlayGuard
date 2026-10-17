// Host tests for action/game_check_text.hpp: the rules behind what Console ›
// Check the installed games says of a game (each problem and section its own
// key, the warnings, the firmware asked for, the fix text chosen) and the IDs
// and console codes as the screen writes them.
#include "check.h"
#include <cstring>
#include <set>
#include <string>

#include "action/game_check_text.hpp"

namespace
{
GameFacts game()
{
    GameFacts g;
    std::memset(&g, 0, sizeof g);
    return g;
}
}   // namespace

static void test_keys()
{
    using namespace game_check_text;
    // Every problem but GC_OK has its own key; no two the same.
    CHECK(std::string(issue_key(GC_OK)).empty() && std::string(issue_key(GC_ISSUE_COUNT)).empty());
    std::set<std::string> seen;
    for (int i = GC_OK + 1; i < GC_ISSUE_COUNT; i++) {
        const std::string k = issue_key((GameIssue)i);
        CHECK(!k.empty() && seen.insert(k).second);
    }
    CHECK(std::string(issue_key(GC_FIRMWARE_TOO_OLD)) == "firmware" && std::string(issue_key(GC_NO_BASE)) == "no_base");
    CHECK(issue_text_key(GC_CORRUPT, "short") == "playguard/games/issues/corrupt/short");
    CHECK(issue_text_key(GC_LAUNCH_VERSION, "cause") == "playguard/games/issues/launch_version/cause");

    // The sections: each once, in the screen's order, each its own key.
    CHECK(GROUPS[0] == GC_CAT_FIX && GROUPS[1] == GC_CAT_VERIFY && GROUPS[2] == GC_CAT_REINSTALL && GROUPS[3] == GC_CAT_SYSTEM);
    CHECK(std::string(category_key(GC_CAT_FIX)) == "fix" && std::string(category_key(GC_CAT_VERIFY)) == "verify");
    CHECK(std::string(category_key(GC_CAT_REINSTALL)) == "reinstall" && std::string(category_key(GC_CAT_SYSTEM)) == "system");
    CHECK(MANY == 3);
}

static void test_warnings()
{
    using namespace game_check_text;
    // The game is there, something around it is not: a warning.
    for (GameIssue i : { GC_STORAGE_UNAVAILABLE, GC_FIRMWARE_TOO_OLD, GC_KEY_TOO_NEW, GC_LAUNCH_VERSION })
        CHECK(warn_only(i));
    for (GameIssue i : { GC_OK, GC_ARCHIVED, GC_NO_BASE, GC_FILES_MISSING, GC_CORRUPT, GC_NEEDS_VERIFY,
                         GC_TICKET_MISSING, GC_LAUNCH_REFUSED, GC_NO_CONTROL })
        CHECK(!warn_only(i));
}

static void test_firmware()
{
    using namespace game_check_text;
    const u32 v20 = MAKEHOSVERSION(20, 0, 0), v21 = MAKEHOSVERSION(21, 0, 0), v22 = MAKEHOSVERSION(22, 1, 0);
    GameFacts g = game();
    CHECK(required_hos(g) == 0);
    g.base_required_hos = v21;
    CHECK(required_hos(g) == v21);
    g.patch_required_hos = v22;
    CHECK(required_hos(g) == v22);
    g.patch_required_hos = v20;   // an update asking for less than the game
    CHECK(required_hos(g) == v21);

    // Only the update too new: playing without it is the other way out.
    g.base_required_hos = v20;
    g.patch_required_hos = v22;
    CHECK(fix_key(g, GC_FIRMWARE_TOO_OLD, v21) == "playguard/games/issues/firmware/fix_update");
    // The game itself too new: the firmware's own fix.
    g.base_required_hos = v22;
    CHECK(fix_key(g, GC_FIRMWARE_TOO_OLD, v21) == "playguard/games/issues/firmware/fix");
    // A key too new reads as a firmware too old.
    CHECK(fix_key(g, GC_KEY_TOO_NEW, v21) == "playguard/games/issues/firmware/fix");
    CHECK(fix_key(g, GC_TICKET_MISSING, v21) == "playguard/games/issues/ticket/fix");
}

static void test_ids()
{
    using namespace game_check_text;
    CHECK(id_text(0x0100ABCD12345000ULL) == "0100ABCD12345000");
    CHECK(id_text(0x1A) == "000000000000001A" && id_text(0xFFFFFFFFFFFFFFFFULL) == "FFFFFFFFFFFFFFFF");

    GameFacts g = game();
    g.app_id = 0x0100000000010000ULL;
    CHECK(history_label(g) == "0100000000010000");
    std::strcpy(g.name, "Super Game");
    CHECK(history_label(g) == "Super Game (0100000000010000)");

    // Module in bits 0..8 (2000 + it), description in bits 9..21.
    CHECK(console_code(0) == "2000-0000");
    CHECK(console_code(16 | (601u << 9)) == "2016-0601");
    CHECK(console_code(0x1FF | (0x1FFFu << 9)) == "2511-8191");
    CHECK(console_code(0xFFFFFFFFu) == "2511-8191");   // the bits above the description ignored
}

int main()
{
    test_keys();
    test_warnings();
    test_firmware();
    test_ids();
    CHECK_DONE("game_check_text assertions passed");
}
