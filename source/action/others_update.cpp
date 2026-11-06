// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/others_update.hpp"

#include <aurore.hpp>
#include <cstdint>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include "action/boreale_apps_logic.hpp"
#include "sync/sync_discovery.h"
#include "util/paths.hpp"

namespace others_update
{

// The update check made for all the Boréale homebrews installed (their
// registry's repositories, config/boreale/updates.conf: boreale-link's
// bl_updates), at most once a day whoever made the last one: the bells show
// another homebrew's update without the network. PlayGuard's own newest
// release, from the check just made. On the update check's thread.
void check(const update::Result& own)
{
    const std::string file = boreale_apps_logic::updates_file(paths::sd_root());
    std::string text;
    paths::read_file(file, text);
    const int64_t now = (int64_t)std::time(nullptr);
    if (!boreale_apps_logic::updates_due(text, now)) return;
    std::vector<std::pair<std::string, std::string>> latest;
    if (own.verdict != update::Verdict::Unknown && own.verdict != update::Verdict::NoRelease)
        latest.emplace_back(SYNC_MODULE, own.latest.version);
    for (const boreale_apps_logic::App& app : boreale_apps_logic::installed(paths::sd_root())) {
        if (app.self || app.repo.empty()) continue;
        const std::string version = update::latest_of(app.repo);
        if (!version.empty()) latest.emplace_back(app.module, version);
    }
    const std::string out = boreale_apps_logic::updates_text(text, latest, now);
    std::string why;
    if (out.empty()) return;
    if (!paths::ensure_dir(file.substr(0, file.rfind('/'))) || !paths::atomic_write(file, out, &why))
        aurore::Logger::warning("update: {} not written: {}", file, why);
    else
        aurore::Logger::info("update: newest releases of {} homebrew(s) kept for all", latest.size());
}

}   // namespace others_update
