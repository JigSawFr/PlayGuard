// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/migration_flow.hpp"

#include <aurore.hpp>
#include <memory>
#include <string>
#include <vector>

#include <aurore/cadran/ui/cadran_text.hpp>

#include "app.hpp"
#include "ui/ui.hpp"
#include "util/data_cleanup.hpp"
#include "util/data_move.hpp"
#include "util/log_files.hpp"
#include "util/migrations.hpp"
#include "util/paths.hpp"

using namespace aurore::literals;

namespace migration_flow
{

namespace
{
// PlayGuard's steps (util/migrations.hpp), in the order they run. Their
// numbers never change; a step whose shim goes (data_move's and
// data_move::logs's, after April 2027) leaves its number unused.
enum : int
{
    DataDir  = 1,   // sd:/switch/playguard to sd:/config/playguard (data_move::run, the console only)
    LogsDirs = 2,   // logs/ before its subfolders (data_move::logs)
    NandDirs = 3,   // what every system shared, to the first one's nand/<key> (data_move::to_nand)
    SharedIcons = 4, // cache/icons/ gone: the icons are the Boréale homebrews' shared cache (config/boreale/cache/)
};

// The lines of util/migrations.hpp, in the log, after `prefix`.
migrations::Log log_to(const char* prefix)
{
    return [prefix](bool warning, const std::string& line) {
        if (warning)
            aurore::Logger::warning("{}: {}", prefix, line);
        else
            aurore::Logger::info("{}: {}", prefix, line);
    };
}

// Each step, with its name in the log and, for the developer tools'
// choice, its title and what it moves (translation keys).
struct Known
{
    migrations::Step step;
    const char*      title;
    const char*      detail;
};

const std::vector<Known>& known()
{
    static const std::vector<Known> all = {
        // The data folder of the versions that kept it next to the .nro.
        { { DataDir, "data folder",
            []() -> size_t {
#ifdef __SWITCH__
                return data_move::run(data_move::old_dir(), paths::data_dir());
#else
                return 0;
#endif
            } },
          "playguard/dev/migration_data_dir", "playguard/dev/migration_data_dir_detail" },
        // logs/ before its subfolders (reports/, crashes/, uploads/, play_timer/).
        { { LogsDirs, "logs subfolders", []() { return data_move::logs(paths::logs_dir()); } },
          "playguard/dev/migration_logs", "playguard/dev/migration_logs_detail" },
        // What every system shared, to this one's nand/<key>.
        { { NandDirs, "per-system folder",
            []() { return data_move::to_nand(paths::data_dir(), paths::nand_dir()); } },
          "playguard/dev/migration_nand", "playguard/dev/migration_nand_detail" },
        // The icons PlayGuard kept for itself before the Boréale homebrews
        // shared theirs (boreale-link's bl_cache, config/boreale/cache/): read
        // again from the console as they are needed. Goes with the releases
        // that still had cache/icons/ (after April 2027).
        { { SharedIcons, "own icon cache",
            []() { return data_cleanup::remove_folder(paths::cache_dir() + "/icons").removed; },
            // Nothing to put back: 1.3.0 reads the icons again from the console as it needs them.
            []() -> size_t { return 0; } },
          "playguard/dev/migration_icons", "playguard/dev/migration_icons_detail" },
    };
    return all;
}

// The steps `chosen` names (by their place in known(); all when empty), the
// entries they move added to *moved.
std::vector<migrations::Step> steps(const std::vector<bool>& chosen, std::shared_ptr<size_t> moved, int first = 0,
                                    int last = 1 << 30)
{
    std::vector<migrations::Step> out;
    for (size_t i = 0; i < known().size(); i++) {
        const migrations::Step& s = known()[i].step;
        if (s.level < first || s.level > last || (!chosen.empty() && !chosen[i])) continue;
        auto run = s.run;
        out.push_back({ s.level, s.name, [run, moved]() {
                           const size_t n = run();
                           *moved += n;
                           return n;
                       } });
    }
    return out;
}

// The steps `chosen` names, on the write queue, whatever migrations.json
// records; then a dialog with how many ran and moved.
void run_chosen(const std::vector<bool>& chosen)
{
    ui::write_later("migrations", [chosen]() {
        auto moved = std::make_shared<size_t>(0);
        const size_t ran = migrations::run(paths::migrations_file(), steps(chosen, moved), app::version(),
                                           log_to("dev tools"), true);
        aurore::Logger::info("dev tools: migrations run again: {} step(s), {} entries moved", ran, *moved);
        aurore::sync([ran, moved]() {
            ui::info(aurore::getStr("playguard/dev/migrations_done", std::to_string(ran), std::to_string(*moved)));
        });
    });
}

// The choice: a line per step (checked: chosen; A on it checks or unchecks
// it, the list opening again on it), then the line that runs those chosen.
void choose(std::vector<bool> chosen, int focus)
{
    std::vector<std::string> labels, details;
    size_t count = 0;
    for (size_t i = 0; i < known().size(); i++) {
        labels.push_back(aurore::getStr(known()[i].title));
        details.push_back(aurore::getStr(known()[i].detail));
        if (chosen[i]) count++;
    }
    labels.push_back(count ? aurore::getStr("playguard/dev/migrations_run", std::to_string(count))
                           : "playguard/dev/migrations_run_none"_i18n);
    std::vector<bool> marks = chosen;
    marks.push_back(false);
    ui::pick_marked("playguard/dev/migrations"_i18n, labels, marks, focus,
                    [chosen, labels, details, count](int index) mutable {
        if (index < (int)chosen.size()) {
            chosen[index] = !chosen[index];
            choose(chosen, index);
            return;
        }
        if (!count) {
            choose(chosen, 0);
            return;
        }
        // What each one moves, as a table.
        cadran_text::Rows rows;
        for (size_t i = 0; i < chosen.size(); i++)
            if (chosen[i]) rows.push_back({ labels[i], details[i] });
        ui::confirm(aurore::getStr("playguard/dev/migrations_body", cadran_text::table(rows)), "playguard/dev/migrations_confirm"_i18n,
                    [chosen]() { run_chosen(chosen); });
    });
}
}   // namespace

void at_start()
{
    migrations::set_latest(SharedIcons);
    migrations::run(paths::migrations_file(),
                    steps({}, std::make_shared<size_t>(0), DataDir, LogsDirs), app::version(),
                    log_to("start-up"));
}

void for_system()
{
    migrations::run(paths::migrations_file(),
                    steps({}, std::make_shared<size_t>(0), NandDirs, SharedIcons), app::version(),
                    log_to("start-up"));
}

void going_back(int level, const migrations::Log& log)
{
    if (level >= 0) {
        std::vector<migrations::Step> all;
        for (const Known& k : known()) all.push_back(k.step);
        migrations::undo_to(paths::migrations_file(), all, level, log);
    }
    migrations::earlier_build_next(paths::migrations_file(), log);
}

int release_level(const std::string& label)
{
    // The builds that did not write their level in previous_build.json
    // (1.4.0 does): the steps each knew. Only releases that had migrations.
    if (label == "1.3.0") return NandDirs;
    return -1;
}

bool undoable(int from, int to)
{
    for (const Known& k : known())
        if (k.step.level > to && k.step.level <= from && !k.step.undo) return false;
    return true;
}

void verify_all()
{
    ui::write_later("migrations", []() {
        auto moved = std::make_shared<size_t>(0);
        const size_t ran = migrations::run(paths::migrations_file(), steps({}, moved), app::version(),
                                           log_to("verify"), true);
        if (*moved)
            aurore::Logger::warning("verify: {} entries still to move: the migration of the build that crashed was not complete", *moved);
        else
            aurore::Logger::info("verify: {} step(s) checked, nothing left to move: the migration was complete", ran);
    });
}

void replay()
{
    choose(std::vector<bool>(known().size(), true), 0);
}

}   // namespace migration_flow
