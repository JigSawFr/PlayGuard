// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/migration_flow.hpp"

#include <borealis.hpp>
#include <memory>
#include <string>
#include <vector>

#include "app.hpp"
#include "ui/ui.hpp"
#include "util/data_move.hpp"
#include "util/log_files.hpp"
#include "util/migrations.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace migration_flow
{

namespace
{
// The lines of util/migrations.hpp, in the log, after `prefix`.
migrations::Log log_to(const char* prefix)
{
    return [prefix](bool warning, const std::string& line) {
        if (warning)
            brls::Logger::warning("{}: {}", prefix, line);
        else
            brls::Logger::info("{}: {}", prefix, line);
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
        { { migrations::DataDir, "data folder",
            []() -> size_t {
#ifdef __SWITCH__
                return data_move::run(data_move::old_dir(), paths::data_dir());
#else
                return 0;
#endif
            } },
          "playguard/dev/migration_data_dir", "playguard/dev/migration_data_dir_detail" },
        // logs/ before its subfolders (reports/, crashes/, uploads/, play_timer/).
        { { migrations::LogsDirs, "logs subfolders", []() { return log_files::migrate(paths::logs_dir()); } },
          "playguard/dev/migration_logs", "playguard/dev/migration_logs_detail" },
        // What every system shared, to this one's nand/<key>.
        { { migrations::NandDirs, "per-system folder",
            []() { return data_move::to_nand(paths::data_dir(), paths::nand_dir()); } },
          "playguard/dev/migration_nand", "playguard/dev/migration_nand_detail" },
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
        brls::Logger::info("dev tools: migrations run again: {} step(s), {} entries moved", ran, *moved);
        brls::sync([ran, moved]() {
            ui::info(brls::getStr("playguard/dev/migrations_done", std::to_string(ran), std::to_string(*moved)));
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
        labels.push_back(brls::getStr(known()[i].title));
        details.push_back(brls::getStr(known()[i].detail));
        if (chosen[i]) count++;
    }
    labels.push_back(count ? brls::getStr("playguard/dev/migrations_run", std::to_string(count))
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
        std::string list;
        for (size_t i = 0; i < chosen.size(); i++)
            if (chosen[i]) list += "\n" + brls::getStr("playguard/common/line", labels[i], details[i]);
        ui::confirm(brls::getStr("playguard/dev/migrations_body", list), "playguard/dev/migrations_confirm"_i18n,
                    [chosen]() { run_chosen(chosen); });
    });
}
}   // namespace

void at_start()
{
    migrations::run(paths::migrations_file(),
                    steps({}, std::make_shared<size_t>(0), migrations::DataDir, migrations::LogsDirs), app::version(),
                    log_to("start-up"));
}

void for_system()
{
    migrations::run(paths::migrations_file(),
                    steps({}, std::make_shared<size_t>(0), migrations::NandDirs, migrations::NandDirs), app::version(),
                    log_to("start-up"));
}

void replay()
{
    choose(std::vector<bool>(known().size(), true), 0);
}

}   // namespace migration_flow
