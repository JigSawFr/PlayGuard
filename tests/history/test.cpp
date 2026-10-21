// Host tests for source/util/history.cpp: the change history on the SD card,
// newest first, trimmed to the newest 500, a damaged file put aside (the
// previous one kept), and what can be undone.
#include "check.h"
#include "temp_dir.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "util/history.hpp"
#include "util/paths.hpp"

static history::Entry limits(int from, int to)
{
    history::Entry e;
    e.when = "2026-10-08 18:30";
    e.kind = "limits";
    e.source = "uniform";
    e.before.assign(7, from);
    e.after.assign(7, to);
    return e;
}

static bool undoable_limits(int from, int to)
{
    return history::undoable(limits(from, to));
}

static void test_round_trip()
{
    CHECK(history::load().empty());   // no file yet

    history::Entry level;
    level.when = "2026-10-08 18:31";
    level.kind = "level";
    level.before = { 3 };
    level.after = { 4 };
    history::Entry pin;
    pin.when = "2026-10-08 18:32";
    pin.kind = "pin";
    pin.source = "first_steps";

    CHECK(history::append(limits(120, 180)));
    CHECK(history::append(level));
    CHECK(history::append(pin));
    auto all = history::load();
    CHECK(all.size() == 3);
    CHECK(all[0].kind == "pin" && all[0].source == "first_steps" && all[0].before.empty());
    CHECK(all[1].kind == "level" && all[1].before == std::vector<int>{ 3 } && all[1].after == std::vector<int>{ 4 });
    CHECK(all[2].kind == "limits" && all[2].after.size() == 7 && all[2].after[3] == 180);

    CHECK(history::undoable(all[1]) && history::undoable(all[2]));
    CHECK(!history::undoable(all[0]));                 // an event, not a value
    history::Entry shown;
    shown.when = "2026-10-08 18:33";
    shown.kind = "pin_shown";
    CHECK(history::append(shown));
    const auto with_shown = history::load();
    CHECK(with_shown.size() == 4 && with_shown[0].kind == "pin_shown" && with_shown[0].before.empty() &&
          !history::undoable(with_shown[0]));
    history::Entry odd = all[2];
    odd.before.pop_back();
    CHECK(!history::undoable(odd));                    // 6 days is not a week
    odd = all[1];
    odd.kind = "something";
    CHECK(!history::undoable(odd));
    // Limits changed outside PlayGuard (outside_watch): recorded with the
    // week before and after, never put back from the history.
    odd = all[2];
    odd.kind = "outside_limits";
    odd.before = odd.after;
    odd.before[0] = 60;
    CHECK(!history::undoable(odd));
}

static void test_value_ranges()
{
    // Each kind takes only what the console takes (the ranges of a restore).
    auto one = [](const char* kind, std::vector<int> before, std::vector<int> after) {
        history::Entry e;
        e.kind = kind;
        e.before = before;
        e.after = after;
        return history::undoable(e);
    };
    CHECK(one("level", { 0 }, { 4 }) && !one("level", { 5 }, { 4 }) && !one("level", { -1 }, { 4 }));
    CHECK(one("org", { 12 }, { 0 }) && !one("org", { 13 }, { 0 }));
    CHECK(one("vr", { 1 }, { 0 }) && !one("vr", { 2 }, { 0 }));
    CHECK(one("alarm", { 0 }, { 1 }) && !one("alarm", { 0 }, { -1 }));
    CHECK(one("time_up", { 0 }, { 1 }) && !one("time_up", { 2 }, { 1 }) && !one("time_up", { 0 }, { 0, 1 }));
    CHECK(one("custom", { 21, 1, 0 }, { 0, 0, 1 }) && !one("custom", { 22, 0, 0 }, { 0, 0, 0 }));
    CHECK(!one("custom", { 12, 2, 0 }, { 0, 0, 0 }));
    CHECK(undoable_limits(1440, 0xFFFF) && !undoable_limits(1441, 60) && !undoable_limits(60, -1));
    CHECK(!undoable_limits(0x10000, 60));

    // 64-bit values in the file are refused, not narrowed: 2^32 + 3 is not 3.
    CHECK(paths::atomic_write(paths::history_file(),
        R"({"entries": [{"kind": "level", "before": [4294967299], "after": [2]},
                        {"kind": "level", "before": [18446744073709551615], "after": [2]},
                        {"kind": "level", "before": [-4294967295], "after": [2]},
                        {"kind": "org", "before": [3], "after": [2]}]})"));
    auto all = history::load();
    CHECK(all.size() == 1 && all[0].kind == "org" && history::undoable(all[0]));
}

// A change the console refused, and the version that recorded each entry.
static void test_refused_and_version()
{
    CHECK(paths::atomic_write(paths::history_file(), "{\"entries\": []}"));
    history::Entry refused = limits(60, 90);
    refused.result = "0x0001088E";
    refused.version = "1.2.0";
    history::Entry done = limits(90, 120);
    done.version = "1.2.0";
    CHECK(history::append(refused) && history::append(done));
    auto all = history::load();
    REQUIRE(all.size() == 2);
    CHECK(all[0].result.empty() && all[0].version == "1.2.0" && history::undoable(all[0]));
    CHECK(all[1].result == "0x0001088E" && all[1].version == "1.2.0");
    // Nothing changed: nothing to put back.
    CHECK(!history::undoable(all[1]));
    // Written only when there is something to say.
    std::string text;
    CHECK(paths::read_file(paths::history_file(), text));
    CHECK(text.find("\"result\":\"0x0001088E\"") != std::string::npos);
    size_t results = 0;
    for (size_t at = text.find("\"result\""); at != std::string::npos; at = text.find("\"result\"", at + 1)) results++;
    CHECK(results == 1);
    // An entry of an older version, without them: read as before.
    CHECK(paths::atomic_write(paths::history_file(),
                              R"({"entries": [{"when": "2026-10-08 18:30", "kind": "level", "before": [1], "after": [2]}]})"));
    all = history::load();
    CHECK(all.size() == 1 && all[0].result.empty() && all[0].version.empty() && history::undoable(all[0]));
    CHECK(!all[0].dev && all[0].commit.empty() && all[0].pr == 0);
    CHECK(std::remove(paths::history_file().c_str()) == 0);
}

// A development build says which one it is: its commit and pull request.
static void test_dev_build()
{
    CHECK(paths::atomic_write(paths::history_file(), "{\"entries\": []}"));
    history::Entry release = limits(60, 90);
    release.version = "1.3.0";
    history::Entry dev = limits(90, 120);
    dev.version = "1.3.0";
    dev.dev     = true;
    dev.commit  = "066b7e5";
    dev.pr      = 306;
    CHECK(history::append(release) && history::append(dev));
    auto all = history::load();
    REQUIRE(all.size() == 2);
    CHECK(all[0].dev && all[0].commit == "066b7e5" && all[0].pr == 306);
    CHECK(!all[1].dev && all[1].commit.empty() && all[1].pr == 0);
    // A release's entry carries none of it.
    std::string text;
    CHECK(paths::read_file(paths::history_file(), text));
    CHECK(text.find("\"dev\":true") != std::string::npos && text.find("\"pr\":306") != std::string::npos);
    size_t devs = 0;
    for (size_t at = text.find("\"dev\""); at != std::string::npos; at = text.find("\"dev\"", at + 1)) devs++;
    CHECK(devs == 1);
    CHECK(std::remove(paths::history_file().c_str()) == 0);
}

static void test_trim_and_damage()
{
    // A full history (as 500 appends would leave it, written at once: each
    // append reads and writes the whole file, slow under the sanitizers),
    // then 10 more changes, each dropping the oldest.
    std::string full_text = "{\n \"schema\": 1,\n \"entries\": [";
    for (int i = 0; i < 500; i++) {
        const std::string from = std::to_string(i), to = std::to_string(i + 1);
        std::string before = from, after = to;
        for (int d = 1; d < 7; d++) before += "," + from, after += "," + to;
        full_text += std::string(i ? ",\n  " : "\n  ") + "{\"after\":[" + after + "],\"before\":[" + before +
                     "],\"detail\":\"\",\"kind\":\"limits\",\"source\":\"uniform\",\"when\":\"2026-10-08 18:30\"}";
    }
    full_text += "\n ]\n}\n";
    CHECK(paths::atomic_write(paths::history_file(), full_text));
    CHECK(history::load().size() == 500);
    for (int i = 500; i < 510; i++) CHECK(history::append(limits(i, i + 1)));
    auto all = history::load();
    CHECK(all.size() == history::MAX_ENTRIES && history::MAX_ENTRIES == 500);
    CHECK(all.front().after[0] == 510);                // the newest kept
    CHECK(all.back().after[0] == 510 - 499);           // the oldest kept
    size_t full = 0;
    CHECK(history::check(&full) == history::Damage::None && full == history::MAX_ENTRIES);

    // A damaged entry is skipped, a damaged file is an empty history.
    CHECK(paths::atomic_write(paths::history_file(),
        R"({"entries": [{"kind": "level", "before": [1], "after": [2]}, {"kind": 5}, "x",
                        {"kind": "limits", "before": "no"}]})"));
    all = history::load();
    CHECK(all.size() == 1 && all[0].kind == "level");
    // check() counts what load() gives, damaged entries left out, from its
    // own reading (the History tab's count at start-up).
    size_t counted = 99;
    CHECK(history::check(&counted) == history::Damage::None && counted == 1);
    CHECK(paths::atomic_write(paths::history_file(), "not json"));
    CHECK(history::load().empty());
    // The next change does not overwrite it: it is kept as history.json.bad.
    CHECK(history::append(limits(60, 90)) && history::load().size() == 1);
    std::string kept;
    CHECK(paths::read_file(paths::history_file() + ".bad", kept) && kept == "not json");
    // A good file is appended to, and leaves the kept one alone.
    bool put_aside = true;
    CHECK(history::append(limits(90, 120), nullptr, &put_aside) && history::load().size() == 2 && !put_aside);
    CHECK(paths::read_file(paths::history_file() + ".bad", kept) && kept == "not json");
    CHECK(history::check() == history::Damage::None);

    // Damaged again: the first .bad is kept as .bad.1, and the caller is told.
    CHECK(paths::atomic_write(paths::history_file(), "{\"entries\": 3}"));
    CHECK(history::append(limits(5, 10), nullptr, &put_aside) && put_aside && history::load().size() == 1);
    CHECK(paths::read_file(paths::history_file() + ".bad", kept) && kept == "{\"entries\": 3}");
    CHECK(paths::read_file(paths::history_file() + ".bad.1", kept) && kept == "not json");

    // check() at start-up: puts it aside now (a third drops the oldest), and
    // the history starts again; missing is not damaged.
    CHECK(paths::atomic_write(paths::history_file(), "third"));
    counted = 99;
    CHECK(history::check(&counted) == history::Damage::PutAside && counted == 0 && history::load().empty());
    CHECK(paths::read_file(paths::history_file() + ".bad", kept) && kept == "third");
    CHECK(paths::read_file(paths::history_file() + ".bad.1", kept) && kept == "{\"entries\": 3}");
    CHECK(history::check() == history::Damage::None);

    // Cannot be put aside (the .bad cannot be renamed): nothing is written over it.
    const std::string older = paths::history_file() + ".bad.1";
    CHECK(std::remove(older.c_str()) == 0 && mkdir(older.c_str(), 0700) == 0);
    CHECK(paths::atomic_write(older + "/x", "keeps the folder non-empty"));
    CHECK(paths::atomic_write(paths::history_file(), "fourth"));
    counted = 99;
    CHECK(history::check(&counted) == history::Damage::Stuck && counted == 0 && history::load().empty());
    std::string error;
    CHECK(!history::append(limits(1, 2), &error) && !error.empty());
    CHECK(paths::read_file(paths::history_file(), kept) && kept == "fourth");
    CHECK(paths::read_file(paths::history_file() + ".bad", kept) && kept == "third");
    CHECK(std::remove((older + "/x").c_str()) == 0 && rmdir(older.c_str()) == 0);
}

int main()
{
    TempDir tmp("history", TempDir::Enter);   // paths::data_dir() is ./playguard_data on the host

    test_round_trip();
    test_refused_and_version();
    test_dev_build();
    test_trim_and_damage();
    test_value_ranges();

    CHECK(tmp.remove());
    return CHECK_DONE("history round trip, order, trimming, damaged-file, undo and value-range assertions passed");
}
