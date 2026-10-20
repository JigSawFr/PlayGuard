// Host tests for source/util/migrations.cpp: a first start runs every step,
// in order, and records the last; the next start runs none; a start after an
// update runs only the new ones; the steps of the start and of the system
// (two calls) add up; a file unreadable runs them all again; one from a later
// build runs none and is not lowered.
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"
#include <string>
#include <vector>

#include "util/migrations.hpp"

namespace mg = migrations;

namespace
{
std::vector<int> g_ran;

mg::Step step(int level, size_t moved = 0)
{
    return { level, "test", [level, moved]() {
                g_ran.push_back(level);
                return moved;
            } };
}

std::vector<mg::Step> start_steps()
{
    return { step(mg::DataDir, 2), step(mg::LogsDirs) };
}
}   // namespace

static void test_first_and_next()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/data/migrations";   // its folder not there yet
    CHECK(mg::done(file) == mg::None);
    g_ran.clear();
    size_t logged = 0, moved = 0;
    CHECK(mg::run(file, start_steps(), [&](const mg::Step&, size_t n) {
        logged++;
        moved += n;
    }) == 2);
    CHECK((g_ran == std::vector<int>{ mg::DataDir, mg::LogsDirs }));
    CHECK(logged == 2 && moved == 2);
    CHECK(mg::done(file) == mg::LogsDirs && fs::read(file) == "2\n");

    // Then the system's step, in a call of its own.
    CHECK(mg::run(file, { step(mg::NandDirs) }) == 1);
    CHECK(mg::done(file) == mg::NandDirs);

    // The next start: nothing.
    g_ran.clear();
    CHECK(mg::run(file, start_steps()) == 0);
    CHECK(mg::run(file, { step(mg::NandDirs) }) == 0);
    CHECK(g_ran.empty() && mg::done(file) == mg::Latest);
}

static void test_update()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations";
    fs::write(file, "1\n");
    g_ran.clear();
    CHECK(mg::run(file, start_steps()) == 1);
    CHECK((g_ran == std::vector<int>{ mg::LogsDirs }));
    CHECK(mg::done(file) == mg::LogsDirs);
}

static void test_unreadable_and_later()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations";
    for (const char* bad : { "", "x", "2x", "-1", "\n" }) {
        fs::write(file, bad);
        CHECK(mg::done(file) == mg::None);
    }
    fs::write(file, "garbage");
    g_ran.clear();
    CHECK(mg::run(file, start_steps()) == 2 && mg::done(file) == mg::LogsDirs);

    // A later build's number: kept, and nothing runs.
    fs::write(file, "7\n");
    g_ran.clear();
    CHECK(mg::run(file, start_steps()) == 0);
    CHECK(mg::run(file, { step(mg::NandDirs) }) == 0);
    CHECK(g_ran.empty() && fs::read(file) == "7\n");

    // An interrupted write: its .tmp is read in its place.
    std::remove(file.c_str());
    fs::write(file + ".tmp", "3");
    CHECK(mg::done(file) == mg::NandDirs);
}

int main()
{
    test_first_and_next();
    test_update();
    test_unreadable_and_later();
    return CHECK_DONE("migrations tests passed");
}
