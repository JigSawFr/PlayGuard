// Host tests for source/util/migrations.cpp: a first start runs every step,
// in order, and records the last and the build in migrations.json; the next
// start runs none; a start after an update runs only the new ones; the steps
// of the start and of the system (two calls) add up; a file unreadable runs
// them all again; an earlier build lowers a later one's level to its own,
// and the later one runs the steps above it again; going back to an earlier
// build sets the level to 0; which installs count as earlier.
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"
#include <cstdio>
#include <string>
#include <vector>

#include <borealis/extern/nlohmann/json.hpp>

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

// Both calls of a start (main.cpp: before and after the system's folder).
size_t start(const std::string& file, const std::string& version)
{
    return mg::run(file, start_steps(), version) + mg::run(file, { step(mg::NandDirs) }, version);
}

mg::State state(const std::string& file)
{
    mg::State s;
    mg::read(file, &s);
    return s;
}
}   // namespace

static void test_first_and_next()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/data/migrations.json";   // its folder not there yet
    mg::State s;
    CHECK(!mg::read(file, &s) && s.level == mg::None && s.version.empty());
    g_ran.clear();
    std::vector<std::string> lines;
    CHECK(mg::run(file, start_steps(), "1.2.0", [&](const std::string& l) { lines.push_back(l); }) == 2);
    CHECK((g_ran == std::vector<int>{ mg::DataDir, mg::LogsDirs }));
    CHECK(lines.size() == 2 && lines[0] == "migration 1 (test): 2 entries moved");
    const nlohmann::json j = nlohmann::json::parse(fs::read(file));
    CHECK(j["schema"] == mg::SCHEMA && j["level"] == mg::LogsDirs && j["version"] == "1.2.0");

    // Then the system's step, in a call of its own.
    CHECK(mg::run(file, { step(mg::NandDirs) }, "1.2.0") == 1);
    CHECK(state(file).level == mg::NandDirs);

    // The next start: nothing, and nothing written.
    g_ran.clear();
    CHECK(mg::write(file, { mg::Latest, "1.2.0" }));
    const std::string before = fs::read(file);
    CHECK(start(file, "1.2.0") == 0);
    CHECK(g_ran.empty() && fs::read(file) == before);

    // An update: nothing to run, the build recorded.
    CHECK(start(file, "1.3.0") == 0 && state(file).version == "1.3.0");
}

static void test_update()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations.json";
    fs::write(file, "{\"schema\": 1, \"level\": 1, \"version\": \"1.1.0\"}");
    g_ran.clear();
    CHECK(start(file, "1.2.0") == 2);
    CHECK((g_ran == std::vector<int>{ mg::LogsDirs, mg::NandDirs }));
    CHECK(state(file).level == mg::Latest && state(file).version == "1.2.0");
}

static void test_unreadable()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations.json";
    for (const char* bad : { "", "x", "3", "[]", "{}", "{\"level\": -1}", "{\"level\": \"3\"}", "{\"level\": 1.5}" }) {
        fs::write(file, bad);
        mg::State s;
        CHECK(!mg::read(file, &s) && s.level == mg::None);
    }
    g_ran.clear();
    CHECK(start(file, "1.2.0") == 3 && state(file).level == mg::Latest);

    // No version: still read.
    fs::write(file, "{\"level\": 2}");
    CHECK(state(file).level == 2 && state(file).version.empty());

    // An interrupted write: its .tmp is read in its place.
    std::remove(file.c_str());
    fs::write(file + ".tmp", "{\"level\": 3, \"version\": \"1.2.0\"}");
    CHECK(state(file).level == mg::NandDirs);
}

static void test_later_then_earlier()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations.json";
    // A later build, with a step 5 this one does not know, then this one.
    CHECK(mg::write(file, { 5, "1.4.0" }));
    g_ran.clear();
    std::vector<std::string> lines;
    CHECK(mg::run(file, start_steps(), "1.2.0", [&](const std::string& l) { lines.push_back(l); }) == 0);
    CHECK(mg::run(file, { step(mg::NandDirs) }, "1.2.0") == 0);
    CHECK(g_ran.empty());
    CHECK(lines.size() == 1 && lines[0] == "data last used by PlayGuard 1.4.0 (migration 5), level back to 3");
    // Its own level and version: the later build runs 4 and 5 again.
    CHECK(state(file).level == mg::Latest && state(file).version == "1.2.0");
}

static void test_earlier_build_next()
{
    TempDir tmp("migrations");
    const std::string file = tmp.path() + "/migrations.json";
    // No file: nothing written (every step runs anyway).
    CHECK(mg::earlier_build_next(file) && !fs::exists(file));

    CHECK(mg::write(file, { mg::Latest, "1.2.0" }));
    CHECK(mg::earlier_build_next(file));
    CHECK(state(file).level == mg::None && state(file).version == "1.2.0");
    // The earlier build (one from before this file) leaves it so; this one
    // is back: every step again, once.
    g_ran.clear();
    CHECK(start(file, "1.2.0") == 3 && start(file, "1.2.0") == 0);

    // Damaged: written as level 0.
    fs::write(file, "garbage");
    CHECK(mg::earlier_build_next(file) && state(file).level == mg::None);
}

static void test_is_earlier()
{
    CHECK(!mg::is_earlier("1.3.0", "1.2.0"));
    CHECK(mg::is_earlier("1.1.0", "1.2.0"));
    CHECK(mg::is_earlier("1.2.0", "1.2.0"));   // the same release, maybe another commit
    CHECK(mg::is_earlier("1.2.0-rc1", "1.2.0"));
    CHECK(mg::is_earlier("", "1.2.0"));         // a branch, a pull request, a commit
    CHECK(mg::is_earlier("garbage", "1.2.0"));
}

int main()
{
    test_first_and_next();
    test_update();
    test_unreadable();
    test_later_then_earlier();
    test_earlier_build_next();
    test_is_earlier();
    return CHECK_DONE("migrations tests passed");
}
