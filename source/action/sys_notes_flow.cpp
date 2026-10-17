// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sys_notes_flow.hpp"

#include <borealis.hpp>
#include <fmt/format.h>

#include "action/sys_update_flow.hpp"
#include "activity/sys_notes_activity.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/sys_updates.hpp"
#include "util/update.hpp"

using namespace brls::literals;
using sys_notes::Which;

namespace sys_notes_flow
{

namespace
{
sys_notes::Cache s_cache;
bool             s_cache_read = false;
bool             s_fetching[3] = { false, false, false };

std::string cache_file() { return paths::data_dir() + "/cache/sys_notes.json"; }

void read_cache()
{
    if (s_cache_read) return;
    s_cache_read = true;
    std::string text;
    if (paths::read_file(cache_file(), text) && !sys_notes::decode(text, &s_cache)) s_cache = {};
}

// Off the UI thread (ui::write_later), as they are now.
void save()
{
    ui::write_later("release notes", [text = sys_notes::encode(s_cache)]() {
        std::string err;
        // Only saves a fetch on the next opening: a write that fails is just logged.
        if (!paths::ensure_dir(paths::data_dir() + "/cache") || !paths::atomic_write(cache_file(), text, &err))
            brls::Logger::warning("release notes: cannot keep them ({})", err);
    }, "sys_notes");
}

int index(Which which) { return (int)which; }
}   // namespace

std::string installed(Which which)
{
    if (which == Which::Hekate) return sys_update_flow::installed_hekate();
    SysInfo si;
    sysinfo_get(&si);
    if (which == Which::Atmosphere)
        return si.ams_valid ? fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro) : "";
    if (!si.hos_version) return "";
    char fw[16];
    sysinfo_version_string(si.hos_version, fw, sizeof(fw));
    return fw;
}

std::string latest(Which which)
{
    const sys_updates::Latest l = sys_updates::from_text(config::get().sys_latest);
    return which == Which::Atmosphere ? l.atmosphere : which == Which::Hekate ? l.hekate : l.firmware;
}

const sys_notes::Cache& cache()
{
    read_cache();
    return s_cache;
}

bool fetching(Which which)
{
    return s_fetching[index(which)];
}

void open(Which which)
{
    brls::Logger::info("release notes: {} opened", sys_notes::key(which));
    read_cache();
    SysNotesActivity::show(which);
    refresh(which, false);
}

void refresh(Which which, bool force)
{
    read_cache();
    const sys_notes::List& kept = s_cache.of(which);
    // Fetched today, with the notes of what a screen shows: nothing to do.
    bool complete = !kept.releases.empty();
    if (which == Which::Firmware) {
        size_t hidden = 0;
        int at = -1;
        for (size_t i : sys_notes::tabs(kept, installed(which), &hidden, &at))
            complete = complete && !kept.releases[i].notes.empty();
    }
    if (s_fetching[index(which)] || (!force && complete && kept.fetched == ui::today_date())) return;
    s_fetching[index(which)] = true;
    SysNotesActivity::changed(which);
    const std::string mine = installed(which);
    ui::in_background("release notes", [which, mine]() {
        sys_notes::List fresh;
        std::string error;
        const bool ok = sys_notes::fetch(which, mine, true, &fresh, &error);
        if (ui::quitting()) return;
        brls::sync([which, ok, fresh, error]() {
            s_fetching[index(which)] = false;
            if (!error.empty()) brls::Logger::info("release notes: {}", error);
            brls::Logger::info("release notes: {} {}", sys_notes::key(which), ok ? "fetched" : "not fetched");
            if (ok) {
                sys_notes::List dated = fresh;
                dated.fetched = ui::today_date();
                sys_notes::merge(&s_cache.of(which), dated);
                save();
            }
            SysNotesActivity::fetched(which, ok);
            sys_update_flow::refresh_entries();
        });
    });
}

sys_notes::Cache fetch_lists()
{
    sys_notes::Cache fresh;
    for (Which w : { Which::Atmosphere, Which::Hekate, Which::Firmware }) {
        std::string error;
        sys_notes::List l;
        if (sys_notes::fetch(w, "", false, &l, &error)) fresh.of(w) = l;
        else if (!error.empty()) brls::Logger::info("release notes: {}", error);
    }
    return fresh;
}

void keep(const sys_notes::Cache& fresh)
{
    read_cache();
    bool any = false;
    for (Which w : { Which::Atmosphere, Which::Hekate, Which::Firmware }) {
        if (fresh.of(w).releases.empty()) continue;
        sys_notes::List dated = fresh.of(w);
        // A list without the firmware's pages: not "fetched" for the screen.
        dated.fetched = w == Which::Firmware ? s_cache.of(w).fetched : ui::today_date();
        sys_notes::merge(&s_cache.of(w), dated);
        any = true;
    }
    if (!any) return;
    save();
    brls::Logger::info("release notes: lists kept");
}

std::string status_text(Which which, bool* newer)
{
    const std::string mine = installed(which);
    const std::string text = sys_update_flow::with_latest(sys_notes::key(which), mine, newer);
    if (!*newer) return text;
    // Behind by more than one release: how many (the list kept knows).
    read_cache();
    const size_t behind = sys_notes::newer_count(s_cache.of(which), mine);
    return behind > 1 ? brls::getStr("playguard/sys_notes/behind", text, (int)behind) : text;
}

std::string firmware_advice()
{
    const std::string fw = latest(Which::Firmware);
    const std::string running = installed(Which::Firmware);
    if (fw.empty() || running.empty() || update::compare_versions(fw, running) <= 0) return "";
    read_cache();
    std::string text;
    for (Which w : { Which::Atmosphere, Which::Hekate }) {
        const std::string mine = installed(w);
        // hekate off the SD card: not used to boot, nothing to say.
        if (w == Which::Hekate && mine.empty()) continue;
        std::string version;
        const sys_notes::Need need = sys_notes::need_for(s_cache.of(w), mine, fw, &version);
        std::string line;
        if (need == sys_notes::Need::Wait) {
            line = brls::getStr("playguard/sys_notes/advice_wait", sys_notes::product(w), version, fw);
        } else if (need == sys_notes::Need::Update) {
            line = brls::getStr("playguard/sys_notes/advice_update", fw, sys_notes::product(w), version);
        }
        if (!line.empty()) text += (text.empty() ? "" : "\n") + line;
    }
    return text;
}

}   // namespace sys_notes_flow
