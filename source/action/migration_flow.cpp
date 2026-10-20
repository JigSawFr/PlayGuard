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

// The steps from `first` to `last`, the entries they move added to *moved.
std::vector<migrations::Step> steps(int first, int last, std::shared_ptr<size_t> moved)
{
    const std::vector<migrations::Step> all = {
        // The data folder of the versions that kept it next to the .nro.
        { migrations::DataDir, "data folder",
          []() -> size_t {
#ifdef __SWITCH__
              return data_move::run(data_move::old_dir(), paths::data_dir());
#else
              return 0;
#endif
          } },
        // logs/ before its subfolders (reports/, crashes/, uploads/, play_timer/).
        { migrations::LogsDirs, "logs subfolders", []() { return log_files::migrate(paths::logs_dir()); } },
        // What every system shared, to this one's nand/<key>.
        { migrations::NandDirs, "per-system folder",
          []() { return data_move::to_nand(paths::data_dir(), paths::nand_dir()); } },
    };
    std::vector<migrations::Step> out;
    for (const migrations::Step& s : all) {
        if (s.level < first || s.level > last) continue;
        auto run = s.run;
        out.push_back({ s.level, s.name, [run, moved]() {
                           const size_t n = run();
                           *moved += n;
                           return n;
                       } });
    }
    return out;
}
}   // namespace

void at_start()
{
    migrations::run(paths::migrations_file(),
                    steps(migrations::DataDir, migrations::LogsDirs, std::make_shared<size_t>(0)), app::version(),
                    log_to("start-up"));
}

void for_system()
{
    migrations::run(paths::migrations_file(),
                    steps(migrations::NandDirs, migrations::NandDirs, std::make_shared<size_t>(0)), app::version(),
                    log_to("start-up"));
}

void replay()
{
    ui::confirm("playguard/dev/migrations_body"_i18n, "playguard/dev/migrations_confirm"_i18n, []() {
        ui::write_later("migrations", []() {
            auto moved = std::make_shared<size_t>(0);
            const size_t ran = migrations::run(paths::migrations_file(),
                                               steps(migrations::DataDir, migrations::Latest, moved),
                                               app::version(), log_to("dev tools"), true);
            brls::Logger::info("dev tools: migrations run again: {} step(s), {} entries moved", ran, *moved);
            brls::sync([ran, moved]() {
                ui::info(brls::getStr("playguard/dev/migrations_done", std::to_string(ran), std::to_string(*moved)));
            });
        });
    });
}

}   // namespace migration_flow
