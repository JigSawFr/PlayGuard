// Host tests for source/util/config.cpp and the ".tmp" recovery of
// source/util/paths.cpp: every field read on its own, out-of-range values put
// back to their defaults, records dropped as a whole, round trip, recovery
// of a save that stopped between its remove and its rename, a damaged file
// put aside (config.json.bad) before anything saves over it.
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util/config.hpp"
#include "util/paths.hpp"

// config.json alone, as the versions before state.json left it: its
// per-system fields are read from it.
// A save as the app makes one (ui::save_config_now): the content, then the
// atomic writes; false when either refuses.
static bool save_config()
{
    config::Snapshot snap;
    return config::snapshot(snap) && config::write(snap);
}

static void write_config(const std::string& text)
{
    CHECK(paths::atomic_write(paths::config_file(), text));
    std::remove(paths::state_file().c_str());
}

static void test_defaults()
{
    std::remove(paths::config_file().c_str());
    config::load();
    const auto& c = config::get();
    CHECK(c.language == "system" && c.theme == "system" && c.auto_relock && !c.dev_mode && !c.pt_log);
    CHECK(!c.extra_auto_restore);
    CHECK(c.extra_weekday == -1 && !c.relock_pending && c.fw_gate_choice.empty());
    CHECK(c.update_via.empty() && c.update_latest.empty());   // the update method: asked the first time

    CHECK(config::loaded() == config::Loaded::Missing);   // a first run: nothing to tell
    for (const char* bad : { "", "not json", "[1, 2]", "42", "{\"language\": " }) {
        write_config(bad);
        config::get().dev_mode = true;
        config::load();
        CHECK(!config::get().dev_mode && config::get().language == "system");
        CHECK(config::loaded() == config::Loaded::PutAside);
    }
    std::remove(paths::config_file().c_str());
    std::remove((paths::config_file() + ".bad").c_str());
    std::remove((paths::config_file() + ".bad.1").c_str());
}

using fs::exists;

// A file that is there but cannot be read as settings is put aside before
// any save, the previous damaged copy kept; a missing one is a silent first run.
static void test_damaged()
{
    const std::string file = paths::config_file(), bad = file + ".bad", older = bad + ".1";
    const std::string first = R"({"relock_pending": true, "console_lock": tr)";
    std::string text;
    write_config(first);
    config::load();
    CHECK(config::loaded() == config::Loaded::PutAside && !config::get().relock_pending);
    CHECK(!exists(file) && paths::read_file(bad, text) && text == first);
    // The automatic saves at start-up then write a new file; the damaged one stays.
    CHECK(save_config() && exists(file));
    CHECK(paths::read_file(bad, text) && text == first);
    config::load();
    CHECK(config::loaded() == config::Loaded::Read);

    // Damaged again: the first .bad becomes .bad.1; a third drops the oldest.
    write_config("second");
    config::load();
    CHECK(config::loaded() == config::Loaded::PutAside);
    CHECK(paths::read_file(bad, text) && text == "second");
    CHECK(paths::read_file(older, text) && text == first);
    write_config("third");
    config::load();
    CHECK(paths::read_file(bad, text) && text == "third" && paths::read_file(older, text) && text == "second");

    // Only a damaged .tmp left (a save stopped before its rename): that one is put aside.
    CHECK(paths::atomic_write(file + ".tmp", "{ half"));
    std::remove(file.c_str());
    CHECK(exists(file + ".tmp"));
    config::load();
    CHECK(config::loaded() == config::Loaded::PutAside && !exists(file + ".tmp"));
    CHECK(paths::read_file(bad, text) && text == "{ half");

    // There but unreadable (a folder in its place) and it cannot be moved
    // (the .bad cannot be renamed either): saves refuse, nothing is replaced.
    CHECK(mkdir(file.c_str(), 0700) == 0);
    std::remove(older.c_str());
    CHECK(mkdir(older.c_str(), 0700) == 0);
    CHECK(paths::atomic_write(older + "/x", "keeps the folder non-empty"));
    config::load();
    CHECK(config::loaded() == config::Loaded::Stuck && exists(file));
    CHECK(!save_config());
    CHECK(paths::read_file(bad, text) && text == "{ half");
    std::remove((older + "/x").c_str());
    CHECK(rmdir(older.c_str()) == 0 && rmdir(file.c_str()) == 0);

    // Missing again: a first run, silent, and saves work.
    std::remove(bad.c_str());
    config::load();
    CHECK(config::loaded() == config::Loaded::Missing && save_config());
}

static void test_fields()
{
    // A wrong field keeps its default; the others are read.
    write_config(R"({"language": 5, "theme": "dark", "dev_mode": 1, "auto_relock": false, "pt_log": "yes",
                    "fw_gate_fw": "24.0.0", "fw_gate_app": "1.0.0", "fw_gate_choice": "risk",
                    "extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 60, "extra_value": 90,
                    "relock_pending": true, "extra_auto_restore": true,
                    "custom_servers": ["a.example", 7, "b.example"]})");
    config::load();
    auto c = config::get();
    CHECK(c.language == "system" && c.theme == "dark" && !c.dev_mode && !c.auto_relock && !c.pt_log);
    CHECK(c.fw_gate_fw == "24.0.0" && c.fw_gate_app == "1.0.0" && c.fw_gate_choice == "risk");
    CHECK(c.extra_weekday == 3 && c.extra_date == "2026-10-06" && c.extra_base == 60 && c.extra_value == 90);
    CHECK(c.relock_pending && c.extra_auto_restore);
    CHECK(c.custom_servers.size() == 2 && c.custom_servers[1] == "b.example");

    // Unknown values go back to their defaults.
    write_config(R"({"language": "xx", "theme": "pink", "update_via": "ftp", "fw_gate_fw": "24.0.0",
                    "fw_gate_app": "1.0.0", "fw_gate_choice": "maybe"})");
    config::load();
    c = config::get();
    CHECK(c.language == "system" && c.theme == "system" && c.update_via.empty());
    CHECK(c.fw_gate_choice.empty() && c.fw_gate_fw.empty() && c.fw_gate_app.empty());

    // The extra-time record is all or nothing: a base of -1 would otherwise
    // put back 0xFFFF ("no limit"), a mistyped one 0 min.
    const char* records[] = {
        R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": -1, "extra_value": 90})",
        R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 90.5, "extra_value": 90})",
        R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": "60", "extra_value": 90})",
        R"({"extra_weekday": 9, "extra_date": "2026-10-06", "extra_base": 60, "extra_value": 90})",
        R"({"extra_weekday": 3, "extra_date": "yesterday", "extra_base": 60, "extra_value": 90})",
        R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 60, "extra_value": 2000})",
        R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 60, "extra_value": 99999999999})",
    };
    for (const char* r : records) {
        write_config(r);
        config::load();
        c = config::get();
        CHECK(c.extra_weekday == -1 && c.extra_date.empty() && c.extra_base == 0 && c.extra_value == 0);
    }

    // No more play today on a day without a limit: "no limit" (0xFFFF) is
    // put back the next day, so that one base outside 0..1440 is kept.
    write_config(R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 65535, "extra_value": 0})");
    config::load();
    c = config::get();
    CHECK(c.extra_weekday == 3 && c.extra_base == 65535 && c.extra_value == 0);
    write_config(R"({"extra_weekday": 3, "extra_date": "2026-10-06", "extra_base": 65534, "extra_value": 0})");
    config::load();
    CHECK(config::get().extra_weekday == -1);

    // Custom servers: no empty name, nothing longer than a DNS name, at most 10.
    std::string servers = "[\"\", \"" + std::string(300, 'x') + "\"";
    for (int i = 0; i < 12; i++) servers += ", \"s" + std::to_string(i) + ".example\"";
    write_config("{\"custom_servers\": " + servers + "], \"ntp_server\": \"" + std::string(300, 'y') + "\"}");
    config::load();
    c = config::get();
    CHECK(c.custom_servers.size() == config::MAX_CUSTOM_SERVERS && c.custom_servers[0] == "s0.example");
    CHECK(c.ntp_server.empty());
}

static void test_round_trip()
{
    config::Config& c = config::get();
    c = config::Config{};
    c.language = "fr";
    c.theme = "light";
    c.ntp_server = "fr.pool.ntp.org";
    c.custom_servers = { "time.example" };
    c.dev_mode = true;
    c.pt_log = true;
    c.relock_pending = true;
    c.extra_weekday = 0;
    c.extra_date = "2026-10-05";
    c.extra_base = 120;
    c.extra_value = 150;
    c.extra_auto_restore = true;
    c.intro = false;
    CHECK(save_config());

    std::string text;
    CHECK(paths::read_file(paths::config_file(), text) && text.find("\"schema\": 1") != std::string::npos);
    c = config::Config{};
    config::load();
    CHECK(c.language == "fr" && c.theme == "light" && c.ntp_server == "fr.pool.ntp.org");
    CHECK(c.custom_servers.size() == 1 && c.dev_mode && c.pt_log && c.relock_pending);
    CHECK(c.extra_weekday == 0 && c.extra_base == 120 && c.extra_value == 150 && c.extra_auto_restore);
    CHECK(!c.intro);

    // Console lock: the flag and the seven saved limits round-trip.
    c = config::Config{};
    c.console_lock = true;
    c.console_lock_prev = { 180, 120, 120, 120, 120, 120, 0xFFFF };
    CHECK(save_config());
    c = config::Config{};
    config::load();
    CHECK(c.console_lock && c.console_lock_prev == std::vector<int>({ 180, 120, 120, 120, 120, 120, 0xFFFF }));
}

static void test_console_lock()
{
    // Saved limits must be exactly seven, each a real limit or 0xFFFF.
    const char* bad[] = {
        R"({"console_lock": true, "console_lock_prev": [0, 0, 0, 0, 0, 0]})",       // six
        R"({"console_lock": true, "console_lock_prev": [0, 0, 0, 0, 0, 0, 0, 0]})", // eight
        R"({"console_lock": true, "console_lock_prev": [0, 0, 0, 0, 0, 0, 1441]})", // out of range
        R"({"console_lock": true, "console_lock_prev": [0, 0, 0, 0, 0, 0, -1]})",
        R"({"console_lock": true, "console_lock_prev": "all"})",
    };
    for (const char* b : bad) {
        write_config(b);
        config::load();
        // The flag is kept (it is a plain bool), but the nonsense limits are dropped,
        // so turning the lock off clears the limit rather than restoring garbage.
        CHECK(config::get().console_lock_prev.empty());
    }
    // 0 every day (a real "locked" record) is kept.
    write_config(R"({"console_lock": true, "console_lock_prev": [0, 0, 0, 0, 0, 0, 0]})");
    config::load();
    CHECK(config::get().console_lock && config::get().console_lock_prev.size() == 7);
    // Default: off, nothing saved.
    config::Config d;
    CHECK(!d.console_lock && d.console_lock_prev.empty());
}

static void test_preferences()
{
    // Defaults.
    config::Config d;
    CHECK(d.start_tab == "dashboard" && d.extra_amounts == std::vector<int>({ 15, 30, 60 }));
    CHECK(d.activity_period == 1 && d.export_format == 0 && d.backup_keep == 0);
    CHECK(d.activity_account == "last" && d.activity_account_last.empty());
    CHECK(d.logs_keep == 20 && d.logs_keep_days == 0 && d.logs_keep_mb == 0);
    CHECK(!d.update_daily && d.update_checked.empty() && !d.clock_check_at_start && d.pin_lock == "changes" && !d.pin_ask_again);
    CHECK(d.onboarding_at_start && d.intro && d.dev_builds_order == "newest");
    CHECK(d.support_reminder && d.support_reminded.empty() && d.seen_version.empty());
    write_config(R"({"support_reminder": false, "support_reminded": "2026-10-09", "seen_version": "1.1.0"})");
    config::load();
    CHECK(!config::get().support_reminder && config::get().support_reminded == "2026-10-09");
    CHECK(config::get().seen_version == "1.1.0");
    write_config(R"({"support_reminded": "last month"})");
    config::load();
    CHECK(config::get().support_reminder && config::get().support_reminded.empty());
    write_config(R"({"onboarding_at_start": false})");
    config::load();
    CHECK(!config::get().onboarding_at_start);
    write_config(R"({"pin_lock": "open", "pin_ask_again": true})");
    config::load();
    CHECK(config::get().pin_lock == "open" && config::get().pin_ask_again);
    write_config(R"({"pin_lock": "off"})");
    config::load();
    CHECK(config::get().pin_lock == "off");   // chosen on purpose: kept
    // Unknown, of the wrong type, or damaged: the prompt stays on.
    write_config(R"({"pin_lock": "always"})");
    config::load();
    CHECK(config::get().pin_lock == "changes");
    write_config(R"({"pin_lock": 0})");
    config::load();
    CHECK(config::get().pin_lock == "changes");
    write_config(R"({"pin_lock": "off")");
    config::load();
    CHECK(config::get().pin_lock == "changes");

    // Values from the lists are kept.
    write_config(R"({"start_tab": "activity", "extra_amounts": [30, 60, 90], "activity_period": 2,
                    "activity_account": "0123456789ABCDEF0123456789ABCDEF", "activity_account_last": "",
                    "export_format": 3, "backup_keep": 10, "update_daily": true,
                    "update_checked": "2026-10-07", "clock_check_at_start": true, "intro": false,
                    "logs_keep": 0, "logs_keep_days": 30, "logs_keep_mb": 5, "dev_builds_order": "number",
                    "update_via": "inplace", "update_latest": "1.2.0", "update_snooze_version": "1.2.0",
                    "update_snooze_until": "2026-11-01"})");
    config::load();
    auto c = config::get();
    CHECK(c.update_via == "inplace" && c.update_latest == "1.2.0");
    CHECK(c.update_snooze_version == "1.2.0" && c.update_snooze_until == "2026-11-01");
    CHECK(!c.intro && c.dev_builds_order == "number");
    CHECK(c.logs_keep == 0 && c.logs_keep_days == 30 && c.logs_keep_mb == 5);
    CHECK(c.start_tab == "activity" && c.extra_amounts == std::vector<int>({ 30, 60, 90 }));
    CHECK(c.activity_period == 2 && c.export_format == 3 && c.backup_keep == 10);
    CHECK(c.activity_account == "0123456789ABCDEF0123456789ABCDEF" && c.activity_account_last.empty());
    CHECK(c.update_daily && c.update_checked == "2026-10-07" && c.clock_check_at_start);

    // Anything else goes back to the default: no 7-minute keep, no 999-minute
    // extra time, no tab that does not exist.
    write_config(R"({"start_tab": "hidden", "extra_amounts": [15, 30, 999], "activity_period": 5,
                    "activity_account": "first", "activity_account_last": "0123456789abcdef0123456789abcdef",
                    "export_format": -1, "backup_keep": 7, "update_checked": "yesterday", "dev_builds_order": "size",
                    "logs_keep": 3, "logs_keep_days": -7, "logs_keep_mb": "5",
                    "update_snooze_version": "1.2.0", "update_snooze_until": "soon"})");
    config::load();
    c = config::get();
    CHECK(c.update_snooze_version.empty() && c.update_snooze_until.empty());   // a postponement needs its day
    CHECK(c.dev_builds_order == "newest");
    CHECK(c.logs_keep == 20 && c.logs_keep_days == 0 && c.logs_keep_mb == 0);
    CHECK(c.start_tab == "dashboard" && c.extra_amounts == std::vector<int>({ 15, 30, 60 }));
    CHECK(c.activity_period == 1 && c.export_format == 0 && c.backup_keep == 0 && c.update_checked.empty());
    CHECK(c.activity_account == "last" && c.activity_account_last.empty());
    // The tabs that became Console: the network clock and the console part of Tools.
    for (const char* old_tab : { "clock", "tools" }) {
        write_config(std::string(R"({"start_tab": ")") + old_tab + "\"}");
        config::load();
        CHECK(config::get().start_tab == "console");
    }
    write_config(R"({"extra_amounts": "15,30,60", "backup_keep": "all"})");
    config::load();
    CHECK(config::get().extra_amounts == std::vector<int>({ 15, 30, 60 }) && config::get().backup_keep == 0);
    // One entry of the wrong type refuses the whole list, rather than being
    // skipped: [10, 20.5, 20, 30] is not [10, 20, 30], and seven limits plus
    // a "60" are not seven limits.
    write_config(R"({"extra_amounts": [10, 20.5, 20, 30],
                    "console_lock_prev": [60, 60, "60", 60, 60, 60, 60, 60]})");
    config::load();
    CHECK(config::get().extra_amounts == std::vector<int>({ 15, 30, 60 }));
    CHECK(config::get().console_lock_prev.empty());
    write_config(R"({"extra_amounts": [10, null, 20, 30]})");
    config::load();
    CHECK(config::get().extra_amounts == std::vector<int>({ 15, 30, 60 }));
    // update_checked: a real "YYYY-MM-DD", not any 10 characters.
    write_config(R"({"update_checked": "yesterday!"})");
    config::load();
    CHECK(config::get().update_checked.empty());
    write_config(R"({"update_checked": "2026/10/07"})");
    config::load();
    CHECK(config::get().update_checked.empty());

    // 64-bit values are not narrowed into a valid one: 2^32 + 15 is not 15,
    // 2^32 is not 0 (a "locked" day), 2^64 - 1 is not -1.
    write_config(R"({"extra_amounts": [4294967311, 30, 60], "backup_keep": 4294967306,
                    "console_lock_prev": [4294967296, 0, 0, 0, 0, 0, 0],
                    "extra_weekday": 18446744073709551615, "extra_date": "2026-10-07",
                    "extra_base": 60, "extra_value": 90})");
    config::load();
    c = config::get();
    CHECK(c.extra_amounts == std::vector<int>({ 15, 30, 60 }) && c.backup_keep == 0);
    CHECK(c.console_lock_prev.empty() && c.extra_weekday == -1 && c.extra_date.empty());

    // Round trip.
    config::get() = config::Config{};
    config::get().start_tab = "history";
    config::get().extra_amounts = { 5, 10, 15 };
    config::get().backup_keep = 20;
    config::get().logs_keep = 50;
    config::get().logs_keep_days = 365;
    config::get().logs_keep_mb = 20;
    config::get().support_reminder = false;
    config::get().seen_version = "1.2.0";
    CHECK(save_config());
    config::get() = config::Config{};
    config::load();
    CHECK(config::get().start_tab == "history" && config::get().extra_amounts == std::vector<int>({ 5, 10, 15 }));
    CHECK(config::get().backup_keep == 20);
    CHECK(config::get().logs_keep == 50 && config::get().logs_keep_days == 365 && config::get().logs_keep_mb == 20);
    CHECK(!config::get().support_reminder && config::get().seen_version == "1.2.0");
}

static void test_tmp_recovery()
{
    // Stopped after removing config.json, before renaming config.json.tmp.
    config::get() = config::Config{};
    config::get().theme = "dark";
    CHECK(save_config());
    const std::string file = paths::config_file();
    CHECK(std::rename(file.c_str(), (file + ".tmp").c_str()) == 0);
    config::get() = config::Config{};
    config::load();
    CHECK(config::get().theme == "dark");

    // The next save must not truncate that .tmp, the only good copy: it is
    // promoted first, and a save that fails afterwards still leaves it.
    std::string text;
    CHECK(paths::atomic_write(file, R"({"theme": "light"})"));
    CHECK(paths::read_file(file, text) && text == R"({"theme": "light"})");
    CHECK(std::rename(file.c_str(), (file + ".tmp").c_str()) == 0);
    CHECK(paths::atomic_write(file, R"({"theme": "dark"})"));
    CHECK(paths::read_file(file, text) && text == R"({"theme": "dark"})");
    struct stat st;
    CHECK(stat((file + ".tmp").c_str(), &st) != 0);
    // Same, but the write fails (file size limit): the old content survives.
    CHECK(std::rename(file.c_str(), (file + ".tmp").c_str()) == 0);
    std::signal(SIGXFSZ, SIG_IGN);
    rlimit saved;
    CHECK(getrlimit(RLIMIT_FSIZE, &saved) == 0);
    rlimit small = saved;
    small.rlim_cur = 64;
    CHECK(setrlimit(RLIMIT_FSIZE, &small) == 0);
    const bool wrote = paths::atomic_write(file, std::string(4096, ' '));
    CHECK(setrlimit(RLIMIT_FSIZE, &saved) == 0);
    CHECK(!wrote);
    CHECK(paths::read_file(file, text) && text == R"({"theme": "dark"})");

    // A leftover .tmp never wins over the file itself.
    write_config(R"({"theme": "light"})");
    CHECK(paths::atomic_write(file + ".tmp", R"({"theme": "dark"})"));
    config::load();
    CHECK(config::get().theme == "light");

    // Both missing: nothing to read.
    std::remove(file.c_str());
    std::remove((file + ".tmp").c_str());
    CHECK(!paths::read_file(file, text));

    // A first write cut short (no file before it) leaves a ".part", never a
    // ".tmp": nothing is read in its place, it is not taken for a damaged
    // file, and the next write drops it.
    {
        FILE* f = std::fopen((file + ".part").c_str(), "wb");
        CHECK(f && std::fputs("{\"theme\": \"da", f) >= 0 && std::fclose(f) == 0);
    }
    CHECK(!paths::read_file(file, text) && !paths::exists_or_pending(file) && !paths::file_exists(file));
    config::get() = config::Config{};
    config::load();
    CHECK(config::loaded() == config::Loaded::Missing && config::get().theme == "system");
    CHECK(paths::atomic_write(file, R"({"theme": "dark"})"));
    CHECK(paths::read_file(file, text) && text == R"({"theme": "dark"})");
    CHECK(!exists(file + ".part") && !exists(file + ".tmp"));
    // The same write failing (file size limit) leaves no ".tmp" either.
    std::remove(file.c_str());
    CHECK(setrlimit(RLIMIT_FSIZE, &small) == 0);
    const bool first = paths::atomic_write(file, std::string(4096, ' '));
    CHECK(setrlimit(RLIMIT_FSIZE, &saved) == 0);
    CHECK(!first && !exists(file) && !exists(file + ".tmp") && !exists(file + ".part"));
    CHECK(!paths::exists_or_pending(file));
    // A file and an older ".tmp" next to it (a write stopped before its last
    // step): the next write replaces both with its own content.
    write_config(R"({"theme": "light"})");
    {
        FILE* f = std::fopen((file + ".tmp").c_str(), "wb");
        CHECK(f && std::fputs("{\"theme\": \"dark\"}", f) >= 0 && std::fclose(f) == 0);
    }
    CHECK(paths::atomic_write(file, R"({"theme": "system"})"));
    CHECK(paths::read_file(file, text) && text == R"({"theme": "system"})" && !exists(file + ".tmp"));
}

// Each system (util/nand.hpp) has its own state.json: the console lock, the
// extra-time record, a pending relock and the firmware choice of one never
// act on another; the preferences are shared.
static void test_systems()
{
    paths::set_nand("sysmmc");
    // From a version before state.json: config.json holds this system's state.
    write_config(R"({"theme": "dark", "console_lock": true, "relock_pending": true,
                    "console_lock_prev": [60, 60, 60, 60, 60, 60, 60]})");
    config::load();
    CHECK(config::get().console_lock && config::get().relock_pending && config::get().theme == "dark");
    CHECK(save_config());
    std::string text;
    CHECK(paths::read_file(paths::config_file(), text));
    CHECK(text.find("console_lock") == std::string::npos && text.find("relock_pending") == std::string::npos);
    CHECK(text.find("\"theme\": \"dark\"") != std::string::npos);
    CHECK(paths::read_file(paths::state_file(), text) && text.find("\"console_lock\": true") != std::string::npos);
    CHECK(paths::state_file() == paths::data_dir() + "/nand/sysmmc/state.json");

    // Another system: the shared preferences, none of the first one's state.
    paths::set_nand("emummc");
    config::load();
    CHECK(config::get().theme == "dark" && !config::get().console_lock && !config::get().relock_pending);
    CHECK(config::get().console_lock_prev.empty());
    config::get().fw_gate_fw = "24.0.0", config::get().fw_gate_app = "1.0.0", config::get().fw_gate_choice = "probe";
    config::get().theme = "light";
    CHECK(save_config());

    // Back on the first: its own state, the preference changed on the other.
    paths::set_nand("sysmmc");
    config::load();
    CHECK(config::get().console_lock && config::get().relock_pending && config::get().console_lock_prev.size() == 7);
    CHECK(config::get().fw_gate_choice.empty() && config::get().theme == "light");

    // A damaged state.json: put aside, said like config.json, this system's
    // state from its defaults, the preferences kept.
    CHECK(paths::atomic_write(paths::state_file(), "{ half"));
    config::load();
    CHECK(config::loaded() == config::Loaded::PutAside);
    CHECK(!config::get().console_lock && config::get().theme == "light");
    CHECK(paths::read_file(paths::state_file() + ".bad", text) && text == "{ half");
    std::remove((paths::state_file() + ".bad").c_str());
    std::system(("rm -rf '" + paths::data_dir() + "/nand'").c_str());
    paths::set_nand("unknown");
}

int main()
{
    TempDir tmp("config", TempDir::Enter);   // paths::data_dir() is ./playguard_data on the host

    test_defaults();
    test_damaged();
    test_fields();
    test_round_trip();
    test_console_lock();
    test_preferences();
    test_tmp_recovery();
    test_systems();

    CHECK(tmp.remove());
    return CHECK_DONE("config fields, ranges, round trip, .tmp recovery, damaged-file and per-system state assertions passed");
}
