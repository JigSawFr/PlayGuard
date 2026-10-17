// Host tests for action/game_check_text.hpp: the rules behind what Console ›
// Check the installed games says of a game (each problem and section its own
// key, the warnings, the firmware asked for, the fix text chosen), the IDs
// and console codes as the screen writes them, and the list (the sections
// sorted, two games of the same name told apart, the selection kept, the
// line above the list, the list as text, the names a question lists, a
// batch of checks counted), with words of the test's own.
#include "check.h"
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "action/game_check_text.hpp"

namespace
{
GameFacts game()
{
    GameFacts g;
    std::memset(&g, 0, sizeof g);
    return g;
}

// A game installed and readable (GC_OK), with a name and a version.
GameFacts fine(u64 id, const char* name, const char* version = "")
{
    GameFacts g = game();
    g.app_id = id;
    std::strcpy(g.name, name);
    std::strcpy(g.display_version, version);
    g.meta_count = 1;
    g.has_base = true;
    g.control_ok = true;
    return g;
}

game_check_text::Words words()
{
    game_check_text::Words w;
    w.unnamed = [](const std::string& id) { return "Game " + id; };
    w.version = [](const std::string& v) { return "v" + v; };
    w.problem = [](const GameFacts&, GameIssue issue) { return std::string(game_check_text::issue_key(issue)); };
    w.list_title = "To install again";
    w.checked_now = "Just now.";
    w.checked_min = [](int n) { return std::to_string(n) + " min ago."; };
    w.checked_h = [](int n) { return std::to_string(n) + " h ago."; };
    return w;
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

static void test_list()
{
    using namespace game_check_text;
    const Words w = words();
    const u32 hos = MAKEHOSVERSION(20, 0, 0);
    // An unnamed game: its ID.
    GameFacts unnamed = fine(0x1A, "");
    CHECK(game_name(unnamed, w.unnamed) == "Game 000000000000001A" && game_name(fine(1, "Zelda"), w.unnamed) == "Zelda");

    // Fine, archived (to install again), its name and icon unread (to check),
    // corrupt (to install again).
    GameFacts ok = fine(1, "Fine");
    GameFacts archived = fine(2, "beta");
    archived.meta_count = 0;
    GameFacts no_control = fine(3, "Gamma");
    no_control.control_ok = false;
    GameFacts corrupt = fine(4, "Alpha");
    corrupt.verified = GC_VERIFIED_CORRUPT;
    GameFacts corrupt2 = fine(5, "alpha2");
    corrupt2.verified = GC_VERIFIED_CORRUPT;
    const Sorted sorted = sort_broken({ ok, archived, no_control, corrupt2, corrupt }, hos, w);
    // Every one with a problem, in the cache's order.
    CHECK(sorted.all.size() == 4 && sorted.all[0].game.app_id == 2 && sorted.all[3].game.app_id == 4);
    CHECK(sorted.by_group[GC_CAT_FIX].empty() && sorted.by_group[GC_CAT_SYSTEM].empty());
    CHECK(sorted.by_group[GC_CAT_VERIFY].size() == 1 && sorted.by_group[GC_CAT_VERIFY][0].issue == GC_NO_CONTROL);
    // In a section, by problem (GC_ARCHIVED first), then by name, case aside.
    const auto& reinstall = sorted.by_group[GC_CAT_REINSTALL];
    CHECK(reinstall.size() == 3 && reinstall[0].game.app_id == 2 && reinstall[1].game.app_id == 4 &&
          reinstall[2].game.app_id == 5);

    // Two games of the same name: by version, else by ID; an unnamed one as it is.
    GameFacts eu = fine(0x10, "Same", "1.0.0"), us = fine(0x20, "Same", "1.2.0"), jp = fine(0x30, "Same", "1.2.0");
    for (GameFacts* g : { &eu, &us, &jp }) g->meta_count = 0;
    std::map<u64, std::string> names = line_names({ { eu, GC_ARCHIVED }, { us, GC_ARCHIVED } }, w);
    CHECK(names[0x10] == "Same · 1.0.0" && names[0x20] == "Same · 1.2.0");
    names = line_names({ { eu, GC_ARCHIVED }, { us, GC_ARCHIVED }, { jp, GC_ARCHIVED } }, w);
    CHECK(names[0x10] == "Same · 1.0.0" && names[0x20] == "Same · 0000000000000020" && names[0x30] == "Same · 0000000000000030");
    GameFacts eu2 = eu;
    eu2.app_id = 0x40;
    eu2.display_version[0] = '\0';   // no version: the ID
    names = line_names({ { eu, GC_ARCHIVED }, { eu2, GC_ARCHIVED }, { unnamed, GC_ARCHIVED } }, w);
    CHECK(names[0x10] == "Same · 1.0.0" && names[0x40] == "Same · 0000000000000040" && names[0x1A] == "Game 000000000000001A");
    names = line_names({ { eu, GC_ARCHIVED } }, w);
    CHECK(names[0x10] == "Same");

    // The games picked that left the list are no longer picked.
    CHECK(still_selected({ 1, 2, 4, 9 }, sorted.all) == std::set<u64>({ 2, 4 }));
    CHECK(still_selected({}, sorted.all).empty());

    // How long ago.
    CHECK(age_text(0, 1000, w).empty() && age_text(1000, 1059, w) == "Just now.");
    CHECK(age_text(1000, 1060, w) == "1 min ago." && age_text(1000, 1000 + 3599, w) == "59 min ago.");
    CHECK(age_text(1000, 1000 + 3 * 3600 + 5, w) == "3 h ago.");

    // The line above the list.
    CHECK(result_line("None of 12.", true, "Just now.", {}) == "None of 12. Just now.");
    CHECK(result_line("3 of 12.", false, "Just now.", { "", "timing" }) == "3 of 12. · Just now.\ntiming");
    CHECK(result_line("3 of 12.", false, "", { "More than 1000.", "timing" }) == "3 of 12.\nMore than 1000.\ntiming");
    CHECK(result_line("", false, "Just now.", { "x" }) == "Just now.\nx");

    // The list as text: ID, name, version, problem.
    CHECK(list_text({ archived, eu }, hos, w) ==
          "To install again\n\n0000000000000002  beta · archived\n0000000000000010  Same · v1.0.0 · archived\n");
    CHECK(list_text({}, hos, w) == "To install again\n\n");

    // Six names at most, then "…".
    std::vector<GameFacts> many;
    CHECK(name_lines(many, w).empty());
    for (int i = 0; i < 6; i++) many.push_back(fine(0x100 + i, ("G" + std::to_string(i)).c_str()));
    CHECK(name_lines(many, w) == "\nG0\nG1\nG2\nG3\nG4\nG5");
    many.push_back(fine(0x200, "G6"));
    CHECK(name_lines(many, w) == "\nG0\nG1\nG2\nG3\nG4\nG5\n…");

    // A batch counted.
    Verified bad, good, failed;
    bad.game = corrupt;
    bad.corrupt = true;
    good.game = ok;
    failed.game = archived;
    failed.rc = 0x1234;
    failed.corrupt = true;   // a check that failed found nothing
    const Tally t = tally({ bad, good, failed, good });
    CHECK(t.corrupt.size() == 1 && t.corrupt[0].app_id == 4 && t.intact == 2 && t.failed == 1);
    CHECK(tally({}).corrupt.empty() && tally({}).intact == 0 && tally({}).failed == 0);
}

int main()
{
    test_keys();
    test_warnings();
    test_firmware();
    test_ids();
    test_list();
    CHECK_DONE("game_check_text assertions passed");
}
