// shared_bell_logic: PlayGuard's bell as the other Boréale homebrews read it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <string>

#include "action/shared_bell_logic.hpp"
#include "check.h"

using shared_bell_logic::Entry;
using shared_bell_logic::text;

static bool has(const std::string& s, const std::string& part)
{
    return s.find(part) != std::string::npos;
}

static void test_written()
{
    const std::vector<Entry> entries = {
        { "pin_missing", true, "No PIN yet", "Anyone can change the limits.", "security" },
        { "week", false, "Your week", "12 h played", "activity" },
        { "odd", true, "Odd", "", "Not A Route" },
    };
    const std::string t = text(entries, "en", "/switch/PlayGuard/PlayGuard.nro", "", 1000);
    CHECK(has(t, "\"schema\":1") && has(t, "\"module\":\"playguard\"") && has(t, "\"nro\":\"switch/PlayGuard/PlayGuard.nro\""));
    CHECK(has(t, "\"id\":\"pin_missing\",\"level\":\"todo\",\"title\":\"No PIN yet\""));
    CHECK(has(t, "\"route\":\"security\"") && has(t, "\"level\":\"info\"") && has(t, "\"lang\":\"en\""));
    CHECK(has(t, "\"id\":\"odd\"") && has(t, "\"route\":\"\""));   // a tab name that is not a route: the home
    CHECK(has(t, "\"updated\":1000"));

    // The same entries later: nothing to write.
    CHECK(text(entries, "en", "/switch/PlayGuard/PlayGuard.nro", t, 2000).empty());
    // One changed: written, with its new time; the others keep theirs.
    std::vector<Entry> moved = entries;
    moved[1].detail = "13 h played";
    const std::string t2 = text(moved, "en", "/switch/PlayGuard/PlayGuard.nro", t, 2000);
    CHECK(has(t2, "13 h played") && has(t2, "\"updated\":2000") && has(t2, "\"updated\":1000"));
    // Another language: every entry written again.
    const std::string t3 = text(entries, "fr", "/switch/PlayGuard/PlayGuard.nro", t, 3000);
    CHECK(has(t3, "\"lang\":\"fr\"") && !has(t3, "\"updated\":1000"));
    // None left: an empty list.
    CHECK(has(text({}, "en", "/switch/PlayGuard/PlayGuard.nro", t, 4000), "\"notices\":[]"));
}

static void test_previous()
{
    // Started from nxlink (no path): the .nro written before is kept, with
    // the members PlayGuard does not know.
    const std::string before =
        "{\"schema\":1,\"module\":\"playguard\",\"nro\":\"switch/PG/PG.nro\",\"future\":true,"
        "\"notices\":[{\"id\":\"week\",\"level\":\"info\",\"title\":\"Your week\",\"detail\":\"12 h played\","
        "\"lang\":\"en\",\"updated\":5,\"expires\":0,\"route\":\"activity\",\"icon\":\"clock\"}]}";
    const std::string t = text({ { "week", false, "Your week", "12 h played", "activity" },
                                 { "update", true, "Update", "1.5.0", "about" } },
                               "en", "", before, 900);
    CHECK(has(t, "\"nro\":\"switch/PG/PG.nro\"") && has(t, ",\"future\":true}"));
    CHECK(has(t, "\"updated\":5,\"expires\":0,\"route\":\"activity\",\"icon\":\"clock\"}"));
    CHECK(has(t, "\"updated\":900"));
    // A newer version's file: never written over.
    const std::string newer = "{\"schema\":2,\"notices\":[]}";
    CHECK(text({ { "week", false, "x", "", "" } }, "en", "", newer, 1).empty());
    // Not a file (damaged): written anew.
    CHECK(has(text({ { "week", false, "x", "", "" } }, "en", "", "{oops", 1), "\"id\":\"week\""));
}

static void test_bounds()
{
    // At most BL_NOTICES_MAX entries, the first ones; a twice-seen id once;
    // a title cut before a character that does not fit whole.
    std::vector<Entry> many;
    for (int i = 0; i < 20; i++) many.push_back({ "n" + std::to_string(i), false, "x", "", "" });
    many.push_back({ "n0", true, "again", "", "" });
    const std::string t = text(many, "en", "/switch/x.nro", "", 1);
    CHECK(has(t, "\"id\":\"n15\"") && !has(t, "\"id\":\"n16\"") && !has(t, "again"));
    std::string title(95, 'a');
    title += "é";   // 2 bytes: 97 with the '\0' fits not
    const std::string t2 = text({ { "long", false, title, "", "" } }, "en", "/switch/x.nro", "", 1);
    CHECK(has(t2, "\"title\":\"" + std::string(95, 'a') + "\""));
}

int main()
{
    CHECK(shared_bell_logic::file("/") == "/config/boreale/notices/playguard.json");
    CHECK(shared_bell_logic::file("./sd") == "./sd/config/boreale/notices/playguard.json");
    test_written();
    test_previous();
    test_bounds();
    return CHECK_DONE("shared_bell_logic: written, kept, bounded assertions passed");
}
