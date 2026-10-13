// Host tests for source/util/log_files.cpp and source/util/data_move.cpp:
// the names of the reports, crash and upload files, their order, a new one, what the limits
// remove (by count, age and size, any of them; never the newest), the move
// of the logs/ layout before its subfolders, and the move of the data folder
// from sd:/switch/playguard to sd:/config/playguard (the .nro stays, a file
// already there is not replaced, a folder already there is merged), the
// system's key (sysMMC or emuMMC) and the move of one system's files into
// its folder.
#include "check.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "util/data_move.hpp"
#include "util/log_files.hpp"
#include "util/nand.hpp"
#include "util/paths.hpp"

namespace lf = log_files;

namespace
{
std::string g_root;

void write(const std::string& rel, const std::string& content)
{
    const std::string path = g_root + "/" + rel;
    REQUIRE(paths::ensure_dir(path.substr(0, path.find_last_of('/'))));
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE* f = fd >= 0 ? ::fdopen(fd, "wb") : nullptr;
    REQUIRE(f);
    std::fwrite(content.data(), 1, content.size(), f);
    std::fclose(f);
}

void write_size(const std::string& rel, size_t size)
{
    write(rel, std::string(size, 'x'));
}

bool there(const std::string& rel)
{
    struct stat st;
    return stat((g_root + "/" + rel).c_str(), &st) == 0;
}

std::string read(const std::string& rel)
{
    std::string out;
    paths::read_file(g_root + "/" + rel, out);
    return out;
}

void make_root()
{
    char tmpl[] = "/tmp/playguard_logs_XXXXXX";
    REQUIRE(mkdtemp(tmpl));
    g_root = tmpl;
}

void remove_root()
{
    std::system(("rm -rf '" + g_root + "'").c_str());
}

// 2026-10-11 12:00:00 UTC (TZ is UTC in main()).
constexpr std::time_t NOW = 1791720000;
}   // namespace

static void test_names()
{
    CHECK(lf::name(lf::REPORT, "20261011_142530") == "report_20261011_142530.txt");
    CHECK(lf::name(lf::CRASH, "20261011_142530", 2) == "crash_20261011_142530_2.txt");
    CHECK(lf::name(lf::UPLOAD, "20261011_142530") == "upload_20261011_142530.url");
    CHECK(lf::name(lf::SESSION, "20261011_142530") == "session_20261011_142530.txt");
    CHECK(lf::stamp(NOW) == "20261011_120000");
    CHECK(lf::stamp(0) == "19700101_000000");

    std::string st;
    CHECK(lf::parse("report_20261011_142530.txt", lf::REPORT, &st) && st == "20261011_142530");
    CHECK(lf::parse("report_20261011_142530_12.txt", lf::REPORT, &st) && st == "20261011_142530");
    CHECK(!lf::parse("report_20261011_142530.txt", lf::CRASH));
    CHECK(!lf::parse("report_20261011_142530_.txt", lf::REPORT));
    CHECK(!lf::parse("report_20261011_142530_x.txt", lf::REPORT));
    CHECK(!lf::parse("report_2026101_142530.txt", lf::REPORT));
    CHECK(!lf::parse("report_20261011-142530.txt", lf::REPORT));
    CHECK(!lf::parse("report_20261011_142530.csv", lf::REPORT));
    CHECK(!lf::parse("20261011_142530.txt", lf::REPORT));
    CHECK(!lf::parse("reports_20261011_142530.txt", lf::REPORT));
}

static void test_list()
{
    make_root();
    write("r/report_20261009_141203.txt", "a");
    write("r/report_20261009_141203_1.txt", "b");
    write("r/report_20261009_141203_10.txt", "c");
    write("r/report_20261009_141203_2.txt", "d");
    write("r/report_20261011_080000.txt", "e");
    write("r/report_20261001_080000.txt", "f");
    write("r/notes.txt", "not a report");
    write("r/crash_20261012_080000.txt", "not a report");
    const auto names = lf::list(g_root + "/r", lf::REPORT);
    if (CHECK(names.size() == 6)) {
        CHECK(names[0] == "report_20261011_080000.txt");
        // In the same second: _10 after _2, the one without a number last.
        CHECK(names[1] == "report_20261009_141203_10.txt");
        CHECK(names[2] == "report_20261009_141203_2.txt");
        CHECK(names[3] == "report_20261009_141203_1.txt");
        CHECK(names[4] == "report_20261009_141203.txt");
        CHECK(names[5] == "report_20261001_080000.txt");
    }
    CHECK(lf::list(g_root + "/missing", lf::REPORT).empty());
    remove_root();
}

static void test_to_prune()
{
    const std::vector<lf::File> files = {
        { "f5", "20261011_100000", 400 * 1024 },
        { "f4", "20261010_100000", 400 * 1024 },
        { "f3", "20261005_100000", 400 * 1024 },
        { "f2", "20260901_100000", 400 * 1024 },
        { "f1", "20250101_100000", 400 * 1024 },
    };
    const std::string cutoff = "20261004_120000";   // 7 days before NOW

    CHECK(lf::to_prune(files, lf::Limits{}, cutoff).empty());
    CHECK((lf::to_prune(files, lf::Limits{ 3, 0, 0 }, cutoff) == std::vector<std::string>{ "f2", "f1" }));
    CHECK((lf::to_prune(files, lf::Limits{ 0, 7, 0 }, cutoff) == std::vector<std::string>{ "f2", "f1" }));
    // 1 MB: the first two (800 KB) fit, the third goes over.
    CHECK((lf::to_prune(files, lf::Limits{ 0, 0, 1 }, cutoff) == std::vector<std::string>{ "f3", "f2", "f1" }));
    // Any limit removes: the count keeps 4, the size keeps 2.
    CHECK((lf::to_prune(files, lf::Limits{ 4, 0, 1 }, cutoff) == std::vector<std::string>{ "f3", "f2", "f1" }));
    CHECK((lf::to_prune(files, lf::Limits{ 1, 0, 0 }, cutoff) == std::vector<std::string>{ "f4", "f3", "f2", "f1" }));

    // The newest never goes: not when too old, not when too big alone.
    const std::vector<lf::File> old = { { "big", "20200101_000000", 5 * 1024 * 1024 }, { "older", "20190101_000000", 1 } };
    CHECK((lf::to_prune(old, lf::Limits{ 0, 7, 0 }, cutoff) == std::vector<std::string>{ "older" }));
    CHECK((lf::to_prune(old, lf::Limits{ 0, 0, 1 }, cutoff) == std::vector<std::string>{ "older" }));
    CHECK(lf::to_prune({}, lf::Limits{ 1, 1, 1 }, cutoff).empty());
}

static void test_save()
{
    make_root();
    const std::string dir = g_root + "/logs/uploads";
    const std::string a = lf::save(dir, lf::UPLOAD, "a", NOW);
    const std::string b = lf::save(dir, lf::UPLOAD, "b", NOW);   // the same second
    CHECK(a == dir + "/upload_20261011_120000.url" && b == dir + "/upload_20261011_120000_1.url");
    CHECK(read("logs/uploads/upload_20261011_120000_1.url") == "b");
    const auto names = lf::list(dir, lf::UPLOAD);
    CHECK(names.size() == 2 && names[0] == "upload_20261011_120000_1.url");
    // An upload is an Internet shortcut: a .txt beside it is not one of them,
    // and a report never a .url.
    write("logs/uploads/upload_20261011_130000.txt", "x");
    CHECK(lf::list(dir, lf::UPLOAD).size() == 2);
    CHECK(lf::parse("upload_20261011_130000.url", lf::UPLOAD) && !lf::parse("upload_20261011_130000.txt", lf::UPLOAD));
    CHECK(!lf::parse("report_20261011_130000.url", lf::REPORT));
    CHECK(lf::shortcut("https://bpa.st/AAAA", "https://bpa.st/remove/BBBB", "2026-10-11 14:25") ==
          "[InternetShortcut]\r\nURL=https://bpa.st/AAAA\r\n\r\n[PlayGuard]\r\n"
          "Sent=2026-10-11 14:25\r\nRemoval=https://bpa.st/remove/BBBB\r\n");
    CHECK(lf::shortcut("https://gist.github.com/x/1", "", "") ==
          "[InternetShortcut]\r\nURL=https://gist.github.com/x/1\r\n\r\n[PlayGuard]\r\n");
    remove_root();
}

static void test_prune()
{
    make_root();
    const std::string dir = g_root + "/logs/reports";
    write_size("logs/reports/report_20261011_100000.txt", 10);
    write_size("logs/reports/report_20261010_100000.txt", 10);
    write_size("logs/reports/report_20261009_100000.txt", 10);
    write_size("logs/reports/report_20260101_100000.txt", 10);
    write_size("logs/reports/notes.txt", 10);                      // not a report: never removed
    write_size("logs/reports/crash_20200101_000000.txt", 10);      // nor another prefix

    CHECK(lf::prune(dir, lf::REPORT, lf::Limits{}, NOW) == 0);
    CHECK(lf::prune(dir, lf::REPORT, lf::Limits{ 0, 30, 0 }, NOW) == 1);
    CHECK(!there("logs/reports/report_20260101_100000.txt") && there("logs/reports/report_20261009_100000.txt"));
    CHECK(lf::prune(dir, lf::REPORT, lf::Limits{ 2, 0, 0 }, NOW) == 1);
    CHECK(!there("logs/reports/report_20261009_100000.txt"));
    CHECK(there("logs/reports/report_20261011_100000.txt") && there("logs/reports/report_20261010_100000.txt"));
    CHECK(there("logs/reports/notes.txt") && there("logs/reports/crash_20200101_000000.txt"));
    CHECK(lf::prune(g_root + "/missing", lf::REPORT, lf::Limits{ 1, 1, 1 }, NOW) == 0);
    remove_root();
}

static void test_migrate_logs()
{
    make_root();
    write("logs/20261009_141203.txt", "report a");
    write("logs/20261009_141203_1.txt", "report b");
    write("logs/notes.txt", "stays");
    write("logs/20261009_141203_x.txt", "stays: not a report name");
    write("logs/crash.txt", "2026-10-08 09:15:02  PlayGuard 1.0.0 (abc): uncaught x\n2026-10-09 10:00:00  ...\n");
    write("logs/play_timer_log.csv", "csv");
    write("logs/play_timer_log.old.csv", "old csv");
    write("logs/play_timer_block.json", "{}");
    write("logs/uploads.txt", "2026-10-08 09:20  https://bpa.st/AAAA  (remove: https://bpa.st/remove/BBBB)\n"
                              "2026-10-08 09:20  https://gist.github.com/x/1\n"
                              "no date\n");
    // Taken already in the new layout: the old file stays.
    write("logs/reports/report_20261009_141203_1.txt", "newer");

    CHECK(lf::migrate(g_root + "/logs") == 6);   // not the report whose name is taken
    CHECK(read("logs/reports/report_20261009_141203.txt") == "report a");
    CHECK(read("logs/reports/report_20261009_141203_1.txt") == "newer" && read("logs/20261009_141203_1.txt") == "report b");
    CHECK(there("logs/notes.txt") && there("logs/20261009_141203_x.txt"));
    CHECK(!there("logs/crash.txt") && read("logs/crashes/crash_20261008_091502.txt").find("uncaught x") != std::string::npos);
    CHECK(read("logs/play_timer/log.csv") == "csv" && read("logs/play_timer/log.old.csv") == "old csv");
    CHECK(read("logs/play_timer/block.json") == "{}" && !there("logs/play_timer_block.json"));
    // A shortcut per link, named after its line's date; a line without a link dropped.
    CHECK(!there("logs/uploads.txt"));
    CHECK(read("logs/uploads/upload_20261008_092000.url") ==
          lf::shortcut("https://bpa.st/AAAA", "https://bpa.st/remove/BBBB", "2026-10-08 09:20"));
    CHECK(read("logs/uploads/upload_20261008_092000_1.url") ==
          lf::shortcut("https://gist.github.com/x/1", "", "2026-10-08 09:20"));
    CHECK(read("logs/uploads/upload_20261008_092000_1.url").find("Removal=") == std::string::npos);
    CHECK(lf::list(g_root + "/logs/uploads", lf::UPLOAD).size() == 2);
    // Done once: nothing left to move.
    CHECK(lf::migrate(g_root + "/logs") == 0);

    // The upload files of the version before the shortcuts, a .txt per link:
    // a shortcut each; a name already taken, the next one.
    write("logs/uploads/upload_20261009_100000.txt", "2026-10-09 10:00  https://bpa.st/CCCC  (remove: https://bpa.st/remove/DDDD)\n");
    write("logs/uploads/upload_20261008_092000.txt", "2026-10-08 09:20  https://bpa.st/EEEE\n");
    write("logs/uploads/notes.txt", "not an upload");
    CHECK(lf::migrate(g_root + "/logs") == 2);
    CHECK(!there("logs/uploads/upload_20261009_100000.txt") && !there("logs/uploads/upload_20261008_092000.txt"));
    CHECK(read("logs/uploads/upload_20261009_100000.url") ==
          lf::shortcut("https://bpa.st/CCCC", "https://bpa.st/remove/DDDD", "2026-10-09 10:00"));
    CHECK(read("logs/uploads/upload_20261008_092000_2.url").find("URL=https://bpa.st/EEEE\r\n") != std::string::npos);
    CHECK(there("logs/uploads/notes.txt") && lf::list(g_root + "/logs/uploads", lf::UPLOAD).size() == 4);
    CHECK(lf::migrate(g_root + "/logs") == 0);

    // A crash.txt without a date: its name says so, and a second one does not replace it.
    write("logs/crash.txt", "garbage");
    CHECK(lf::migrate(g_root + "/logs") == 1 && read("logs/crashes/crash_19700101_000000.txt") == "garbage");
    write("logs/crash.txt", "garbage 2");
    CHECK(lf::migrate(g_root + "/logs") == 1 && read("logs/crashes/crash_19700101_000000_1.txt") == "garbage 2");
    CHECK(lf::migrate(g_root + "/missing") == 0);
    remove_root();
}

static void test_data_move()
{
    make_root();
    const std::string from = g_root + "/switch/playguard", to = g_root + "/config/playguard";
    write("switch/playguard/playguard.nro", "NRO0");
    write("switch/playguard/LICENSE.txt", "GPL");
    write("switch/playguard/notes.txt", "not PlayGuard's");
    write("switch/playguard/config.json", "{\"a\": 1}");
    write("switch/playguard/config.json.bad", "bad");
    write("switch/playguard/history.json", "[]");
    write("switch/playguard/watch.json", "{}");
    write("switch/playguard/github_token", "ghu");
    write("switch/playguard/RESCUE", "");
    write("switch/playguard/profiles/school.json", "p");
    write("switch/playguard/backups/20261010_100000.json", "b");
    write("switch/playguard/cache/icons/0100.jpg", "i");
    write("switch/playguard/logs/crash.txt", "c");
    write("switch/playguard/sync.conf", "enabled=1");
    write("switch/playguard/sync/profiles.txt", "60=P");
    // Already in the new folder: the file is kept, the folder merged.
    write("config/playguard/watch.json", "newer");
    write("config/playguard/logs/reports/report_20261011_100000.txt", "r");

    CHECK(data_move::run(from, to) == 11);   // not watch.json; logs/ merged: its one file
    CHECK(read("config/playguard/config.json") == "{\"a\": 1}" && read("config/playguard/config.json.bad") == "bad");
    CHECK(read("config/playguard/history.json") == "[]" && read("config/playguard/github_token") == "ghu");
    CHECK(there("config/playguard/RESCUE") && read("config/playguard/profiles/school.json") == "p");
    CHECK(read("config/playguard/backups/20261010_100000.json") == "b");
    CHECK(read("config/playguard/cache/icons/0100.jpg") == "i");
    CHECK(read("config/playguard/logs/crash.txt") == "c" && there("config/playguard/logs/reports/report_20261011_100000.txt"));
    CHECK(read("config/playguard/sync.conf") == "enabled=1" && read("config/playguard/sync/profiles.txt") == "60=P");
    CHECK(!there("switch/playguard/logs") && !there("switch/playguard/profiles"));
    CHECK(read("config/playguard/watch.json") == "newer" && read("switch/playguard/watch.json") == "{}");
    // The app and what is not PlayGuard's data stay.
    CHECK(there("switch/playguard/playguard.nro") && there("switch/playguard/LICENSE.txt") && there("switch/playguard/notes.txt"));
    CHECK(!there("config/playguard/playguard.nro"));
    // Again: only the file that could not be moved is still there.
    CHECK(data_move::run(from, to) == 0);
    CHECK(data_move::run(g_root + "/missing", to) == 0);
    CHECK(data_move::run(to, to) == 0);
    CHECK(data_move::old_dir() == "/switch/playguard");
    remove_root();
}

static void test_nand()
{
    CHECK(nand::key(nand::Storage::SysMMC) == "sysmmc");
    CHECK(nand::key(nand::Storage::EmuMMC) == "emummc");
    CHECK(nand::key(nand::Storage::Unknown) == "unknown");
}

static void test_to_nand()
{
    make_root();
    const std::string data = g_root + "/config/playguard", sys = data + "/nand/sysmmc";
    write("config/playguard/config.json", "{}");
    write("config/playguard/history.json", "[]");
    write("config/playguard/history.json.bad", "bad");
    write("config/playguard/watch.json", "{}");
    write("config/playguard/backups/20261010_100000.json", "b");
    write("config/playguard/cache/activity_all.bin", "a");
    write("config/playguard/cache/activity_0011223344556677.bin", "u");
    write("config/playguard/cache/dev_builds.json", "shared");
    write("config/playguard/cache/icons/0100.jpg", "shared");
    write("config/playguard/profiles/school.json", "shared");
    write("config/playguard/sync/nro_state.txt", "read_only=0");
    write("config/playguard/sync/agent_events.log", "e");
    write("config/playguard/sync/profiles.txt", "shared");

    CHECK(data_move::to_nand(data, sys) == 8);
    CHECK(read("config/playguard/nand/sysmmc/history.json") == "[]" && read("config/playguard/nand/sysmmc/history.json.bad") == "bad");
    CHECK(read("config/playguard/nand/sysmmc/watch.json") == "{}");
    CHECK(read("config/playguard/nand/sysmmc/backups/20261010_100000.json") == "b");
    CHECK(read("config/playguard/nand/sysmmc/cache/activity_all.bin") == "a");
    CHECK(there("config/playguard/nand/sysmmc/cache/activity_0011223344556677.bin"));
    // The remote link's: this system's records and agent files; the profiles shared.
    CHECK(read("config/playguard/nand/sysmmc/sync/nro_state.txt") == "read_only=0");
    CHECK(read("config/playguard/nand/sysmmc/sync/agent_events.log") == "e");
    CHECK(read("config/playguard/sync/profiles.txt") == "shared");
    // Shared: the preferences, the profiles, the icons, the build list.
    CHECK(there("config/playguard/config.json") && there("config/playguard/profiles/school.json"));
    CHECK(there("config/playguard/cache/dev_builds.json") && there("config/playguard/cache/icons/0100.jpg"));
    CHECK(!there("config/playguard/history.json") && !there("config/playguard/backups"));
    // The next system finds nothing left to take.
    CHECK(data_move::to_nand(data, data + "/nand/emummc") == 0);
    CHECK(!there("config/playguard/nand/emummc"));
    remove_root();
}

int main()
{
    setenv("TZ", "UTC", 1);
    tzset();
    test_names();
    test_list();
    test_to_prune();
    test_save();
    test_prune();
    test_migrate_logs();
    test_data_move();
    test_nand();
    test_to_nand();
    return CHECK_DONE("log_files names, order, limits, migration, data_move and nand tests passed");
}
