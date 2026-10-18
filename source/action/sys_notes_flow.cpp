// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sys_notes_flow.hpp"

#include <borealis.hpp>
#include <fmt/format.h>
#include <memory>
#include <utility>

#include "action/sys_update_flow.hpp"
#include "activity/sys_notes_activity.hpp"
#include "app.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/sys_updates.hpp"
#include "util/text.hpp"
#include "util/update.hpp"

using namespace brls::literals;
using sys_notes::Which;

namespace sys_notes_flow
{

namespace
{
// The lists kept on the SD card (a few hundred KB of JSON) are read and
// decoded once, off the UI thread (load()), so the first screen does not
// wait for them. Until they are, nothing is fetched nor kept: what was asked
// meanwhile waits for them (s_asked, s_waiting).
enum class Load { NotStarted, Running, Done };
sys_notes::Cache s_cache;
Load             s_load = Load::NotStarted;
bool             s_fetching[sys_notes::COUNT] = {};
// refresh() asked while the lists were being read, and whether forced.
bool             s_asked[sys_notes::COUNT] = {};
bool             s_asked_force[sys_notes::COUNT] = {};
// keep() asked while the lists were being read: the lists it was given.
sys_notes::Cache s_waiting;
bool             s_waiting_any = false;

std::string cache_file() { return paths::cache_dir() + "/sys_notes.json"; }

int index(Which which) { return (int)which; }

// Off the UI thread (ui::write_later), the encoding too: the write takes a
// copy of the lists.
void save()
{
    auto kept = std::make_shared<const sys_notes::Cache>(s_cache);
    ui::write_later("release notes", [kept]() {
        const std::string text = sys_notes::encode(*kept);
        std::string err;
        // Only saves a fetch on the next opening: a write that fails is just logged.
        if (!paths::ensure_dir(paths::cache_dir()) || !paths::atomic_write(cache_file(), text, &err))
            brls::Logger::warning("release notes: cannot keep them ({})", err);
    }, "sys_notes");
}

// The lists read (UI thread): what was asked meanwhile, then what shows them.
void on_loaded(sys_notes::Cache read)
{
    bool any = false;
    for (Which w : sys_notes::ALL) any = any || !read.of(w).releases.empty();
    s_cache = std::move(read);
    s_load = Load::Done;
    if (s_waiting_any) {
        s_waiting_any = false;
        keep(std::exchange(s_waiting, sys_notes::Cache{}));
    }
    for (Which w : sys_notes::ALL) {
        if (!std::exchange(s_asked[index(w)], false)) continue;
        refresh(w, std::exchange(s_asked_force[index(w)], false));
    }
    SysNotesActivity::loaded();
    // The firmware's entry of the notification bell, with its advice.
    sys_update_flow::refresh_entries();
    // Tools › Console: how far behind each one is, the firmware's advice;
    // About: how far behind PlayGuard is (nothing kept: nothing to say there either).
    if (any) TabBase::refresh_shown();
}
}   // namespace

void load()
{
    if (s_load != Load::NotStarted) return;
    s_load = Load::Running;
    const std::string file = cache_file();
    ui::in_background("release notes cache", [file]() {
        auto read = std::make_shared<sys_notes::Cache>();
        std::string text;
        if (paths::read_file(file, text) && !sys_notes::decode(text, read.get())) *read = {};
        if (ui::quitting()) return;
        brls::sync([read]() { on_loaded(std::move(*read)); });
    });
}

bool ready()
{
    load();
    return s_load == Load::Done;
}

std::string installed(Which which)
{
    if (which == Which::PlayGuard) return app::version();
    if (which == Which::Hekate) return sys_update_flow::installed_hekate();
    SysInfo si;
    sysinfo_get(&si);
    if (which == Which::Atmosphere)
        return si.ams_valid ? fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro) : "";
    if (!si.hos_version) return "";
    return ui::hos_text(si.hos_version);
}

std::string latest(Which which)
{
    // PlayGuard's own update check (action/update_flow) keeps what it found.
    if (which == Which::PlayGuard) return config::get().update_latest;
    const sys_updates::Latest l = sys_updates::from_text(config::get().sys_latest);
    return which == Which::Atmosphere ? l.atmosphere : which == Which::Hekate ? l.hekate : l.firmware;
}

const sys_notes::Cache& cache()
{
    load();
    return s_cache;
}

bool fetching(Which which)
{
    // The lists kept not read yet: the screen waits for them as for a fetch.
    return s_fetching[index(which)] || !ready();
}

void open(Which which)
{
    brls::Logger::info("release notes: {} opened", sys_notes::key(which));
    SysNotesActivity::show(which);
    refresh(which, false);
}

void refresh(Which which, bool force)
{
    // Whether to fetch depends on what was kept: decided once it is read.
    if (!ready()) {
        s_asked[index(which)] = true;
        s_asked_force[index(which)] = s_asked_force[index(which)] || force;
        SysNotesActivity::changed(which);
        return;
    }
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
    // Merged into what was kept once that is read (a newer list replacing one
    // still waiting).
    if (!ready()) {
        for (Which w : { Which::Atmosphere, Which::Hekate, Which::Firmware }) {
            if (fresh.of(w).releases.empty()) continue;
            s_waiting.of(w) = fresh.of(w);
            s_waiting_any = true;
        }
        return;
    }
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
    std::string text;
    if (which == Which::PlayGuard) {
        const std::string newest = latest(which);
        *newer = !newest.empty() && !mine.empty() && update::compare_versions(newest, mine) > 0;
        text = *newer ? brls::getStr("playguard/tools/available", mine, newest) : mine;
    } else {
        text = sys_update_flow::with_latest(sys_notes::key(which), mine, newer);
    }
    if (!*newer) return text;
    // Behind by more than one release: how many (the list kept knows; once
    // it is read, the Console tab is refreshed).
    load();
    const size_t behind = sys_notes::newer_count(s_cache.of(which), mine);
    return behind > 1 ? brls::getStr("playguard/sys_notes/behind", text, (int)behind) : text;
}

std::string firmware_advice()
{
    const std::string fw = latest(Which::Firmware);
    const std::string running = installed(Which::Firmware);
    if (fw.empty() || running.empty() || update::compare_versions(fw, running) <= 0) return "";
    // Nothing said before the notes kept are read (what shows it is refreshed then).
    if (!ready()) return "";
    std::vector<std::string> lines;
    for (Which w : { Which::Atmosphere, Which::Hekate }) {
        const std::string mine = installed(w);
        // hekate off the SD card: not used to boot, nothing to say.
        if (w == Which::Hekate && mine.empty()) continue;
        std::string version;
        const sys_notes::Need need = sys_notes::need_for(s_cache.of(w), mine, fw, &version);
        if (need == sys_notes::Need::Wait) {
            lines.push_back(brls::getStr("playguard/sys_notes/advice_wait", sys_notes::product(w), version, fw));
        } else if (need == sys_notes::Need::Update) {
            lines.push_back(brls::getStr("playguard/sys_notes/advice_update", fw, sys_notes::product(w), version));
        }
    }
    return text::join(lines, "\n");
}

}   // namespace sys_notes_flow
