// Host tests for source/util/rollback.cpp: when the previous build is
// offered at start-up, its label and the start check on disk (damaged files
// included), and the swap of <nro> and <nro>.previous (back and forth, with
// no previous build, with no current one).
#include "check.h"
#include <cstdio>
#include <string>
#include <unistd.h>

#include "util/paths.hpp"
#include "util/rollback.hpp"

using rollback::Offer;

static std::string dir()
{
    static std::string d;
    if (d.empty()) {
        char tmpl[] = "/tmp/pg_rollback_XXXXXX";
        REQUIRE(mkdtemp(tmpl));
        d = tmpl;
    }
    return d;
}

static void put(const std::string& path, const std::string& text)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    REQUIRE(f);
    std::fwrite(text.data(), 1, text.size(), f);
    std::fclose(f);
}

static std::string get(const std::string& path)
{
    std::string text;
    return paths::read_file(path, text) ? text : "<none>";
}

static void test_offer()
{
    // Nothing without a previous build, whatever happened.
    CHECK(rollback::offer_at_start(true, 3, true, false) == Offer::None);
    // The build installed started once without reaching its first screen.
    CHECK(rollback::offer_at_start(true, 1, false, true) == Offer::Crashed);
    CHECK(rollback::offer_at_start(true, 1, true, true) == Offer::Crashed);
    // Its first start (nothing failed yet), or no check at all: only when asked.
    CHECK(rollback::offer_at_start(true, 0, false, true) == Offer::None);
    CHECK(rollback::offer_at_start(true, 0, true, true) == Offer::Asked);
    CHECK(rollback::offer_at_start(false, 0, true, true) == Offer::Asked);
    CHECK(rollback::offer_at_start(false, 0, false, true) == Offer::None);
}

static void test_files()
{
    const std::string label = dir() + "/previous_build.json", check = dir() + "/boot_check.json";
    std::string l;
    CHECK(!rollback::read_label(label, &l));
    CHECK(rollback::write_label(label, "1.2.0 (abc1234)") && rollback::read_label(label, &l) && l == "1.2.0 (abc1234)");
    put(label, "{\"label\": 3}");
    CHECK(!rollback::read_label(label, &l));
    put(label, "not json");
    CHECK(!rollback::read_label(label, &l));

    rollback::Check c;
    CHECK(!rollback::read_check(check, &c));
    c.label = "PR #38";
    c.starts = 1;
    CHECK(rollback::write_check(check, c));
    rollback::Check back;
    CHECK(rollback::read_check(check, &back) && back.label == "PR #38" && back.starts == 1);
    put(check, "{\"starts\": -4}");
    CHECK(rollback::read_check(check, &back) && back.starts == 0 && back.label.empty());
    put(check, "[]");
    CHECK(!rollback::read_check(check, &back));
}

static void test_swap()
{
    const std::string nro = dir() + "/playguard.nro", prev = rollback::previous_file(nro);
    CHECK(prev == nro + ".previous");
    std::string err;
    put(nro, "new");
    CHECK(!rollback::swap(nro, &err) && !err.empty() && get(nro) == "new");   // no previous build
    put(prev, "old");
    CHECK(rollback::swap(nro, &err) && get(nro) == "old" && get(prev) == "new");
    CHECK(rollback::swap(nro, &err) && get(nro) == "new" && get(prev) == "old");   // and back
    CHECK(get(nro + ".swap") == "<none>");
    // No current build (deleted by hand): the previous one takes its place.
    std::remove(nro.c_str());
    CHECK(rollback::swap(nro, &err) && get(nro) == "old" && get(prev) == "<none>");
}

int main()
{
    test_offer();
    test_files();
    test_swap();
    return CHECK_DONE("rollback offer, label and start check files, and swap assertions passed");
}
