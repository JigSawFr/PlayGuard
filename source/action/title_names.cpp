// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/title_names.hpp"

#include <borealis.hpp>

#include "action/forwarder_flow.hpp"
#include "action/game_scan.hpp"
#include "ui/ui.hpp"
#include "util/forwarders.hpp"
#include "util/paths.hpp"
#include "util/sync_files.hpp"
#include "util/titles.hpp"

namespace title_names
{

namespace
{
titles::Record s_record;
bool s_loaded = false;

std::string record_file()
{
    return paths::nand_dir() + "/titles.txt";
}

titles::Record& record()
{
    if (!s_loaded) {
        s_loaded = true;
        std::string text;
        if (paths::read_file(record_file(), text)) s_record = titles::parse(text);
    }
    return s_record;
}

void save()
{
    const std::string text = titles::text(s_record), agent = titles::agent_text(s_record);
    const std::string file = record_file(), dir = sync_files::system_dir();
    ui::write_later("titles.txt", [file, text, dir, agent]() {
        std::string err;
        if (!paths::atomic_write(file, text, &err)) brls::Logger::warning("titles: not saved ({})", err);
        paths::ensure_dir(dir);
        if (!paths::atomic_write(dir + "/titles.txt", agent, &err)) brls::Logger::warning("titles: agent's copy not saved ({})", err);
    }, "titles.txt");
}
}   // namespace

SyncTitleKind kind(uint64_t id)
{
    return titles::kind(record(), id);
}

std::string recorded(uint64_t id)
{
    return titles::name(record(), id);
}

void learn(const PlayStats& stats)
{
    titles::Record& r = record();
    bool changed = false;
    // Made here: homebrews whatever their ID, by the name they were given;
    // PlayGuard's own told apart.
    const std::string self = forwarder_flow::self_nro();
    for (const forwarders::Made& m : forwarders::list())
        changed |= titles::note(r, m.id, m.nro == self ? SyncTitle_PlayGuard : SyncTitle_Homebrew, m.name);
    for (uint32_t i = 0; i < stats.count; i++) {
        const GameStat& g = stats.games[i];
        changed |= titles::note(r, g.app_id, titles::kind(r, g.app_id), g.name);
    }
    if (const auto checked = game_scan::latest())
        for (const GameFacts& f : checked->games) changed |= titles::note(r, f.app_id, titles::kind(r, f.app_id), f.name);
    // The agent's copy, once a run even when nothing changed (made by an
    // older PlayGuard: none yet).
    static bool once = false;
    if (changed || !once) save();
    once = true;
}

void learn_forwarder(uint64_t id, const std::string& name, bool self)
{
    if (titles::note(record(), id, self ? SyncTitle_PlayGuard : SyncTitle_Homebrew, name)) save();
}

}   // namespace title_names
