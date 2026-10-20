// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/ha_update_flow.hpp"

#include <borealis.hpp>

#include "action/notices.hpp"
#include "activity/ha_update_activity.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/sync_files.hpp"
#include "util/sys_updates.hpp"
#include "util/update.hpp"

using namespace brls::literals;

namespace ha_update_flow
{

namespace
{
constexpr const char* NOTICE = "ha_integration";

bool s_loaded = false, s_checking = false;
std::string s_doc;   // as on the SD card
sync_files::Integration s_info;

std::string changelog_file()
{
    return paths::cache_dir() + "/ha_changelog.md";
}

void load()
{
    if (s_loaded) return;
    s_loaded = true;
    if (!paths::read_file(sync_files::integration_file(), s_doc)) s_doc.clear();
    s_info = sync_files::parse_integration(s_doc);
}

bool due()
{
    const auto& cfg = config::get();
    return cfg.update_daily && cfg.ha_checked != ui::today_date();
}

void note()
{
    const std::string latest = newer();
    if (latest.empty()) {
        notices::clear(NOTICE);
        return;
    }
    notices::set(NOTICE, notices::Level::Info, brls::getStr("playguard/notices/ha_integration", latest),
                 brls::getStr("playguard/notices/ha_integration_detail", s_info.version), "playguard/notices/whats_new"_i18n,
                 []() { open(); });
}

void check()
{
    if (s_checking || s_info.version.empty()) return;
    s_checking = true;
    const std::string installed = s_info.version;
    std::string kept;
    if (!paths::read_file(changelog_file(), kept)) kept.clear();
    ui::in_background("integration update check", [installed, kept]() {
        std::string error;
        const std::string latest = sys_updates::fetch_integration(&error);
        // What changed since: the newer release's CHANGELOG.md, unless kept already.
        std::string md;
        bool fetched = false;
        if (!latest.empty() && update::compare_versions(latest, installed) > 0 &&
            changelog::release_notes(kept, latest).empty())
            fetched = sys_updates::fetch_integration_changelog(latest, &md, &error);
        if (ui::quitting()) return;
        brls::sync([installed, latest, md, fetched, error]() {
            s_checking = false;
            if (!error.empty()) brls::Logger::info("ha integration: update check: {}", error);
            if (latest.empty()) return;   // what the last check found stays
            auto& cfg = config::get();
            cfg.ha_latest = latest;
            cfg.ha_checked = ui::today_date();
            ui::save_config();
            if (fetched) {
                const std::string file = changelog_file();
                ui::write_later("integration changelog", [file, md]() {
                    paths::ensure_dir(paths::cache_dir());
                    std::string err;
                    if (!paths::atomic_write(file, md, &err))
                        brls::Logger::warning("ha integration: changelog not kept: {}", err);
                }, file);
            }
            if (update::compare_versions(latest, installed) > 0)
                brls::Logger::info("ha integration: {} installed, {} available", installed, latest);
            else
                brls::Logger::info("ha integration: {} installed, up to date", installed);
            note();
        });
    });
}

// A new document: the versions, the bell, a check when the version is new.
void apply(const std::string& doc)
{
    const std::string before = s_info.version;
    s_doc = doc;
    s_info = sync_files::parse_integration(doc);
    if (s_info.version.empty()) brls::Logger::info("ha integration: none");
    else brls::Logger::info("ha integration: {} (Home Assistant {})", s_info.version, s_info.homeassistant);
    note();
    if (!s_info.version.empty() && s_info.version != before && due()) check();
}
}   // namespace

void at_start()
{
    load();
    note();
    if (!s_info.version.empty() && due()) check();
}

void check_now()
{
    load();
    check();
}

void received(const std::string& doc)
{
    load();
    if (doc == s_doc) return;   // again at each connection
    sync_files::write_integration(doc);
    apply(doc);
}

void reload()
{
    std::string doc;
    if (!paths::read_file(sync_files::integration_file(), doc)) doc.clear();
    s_loaded = true;
    if (doc != s_doc) apply(doc);
}

std::string installed()
{
    load();
    return s_info.version;
}

std::string homeassistant()
{
    load();
    return s_info.homeassistant;
}

std::string newer()
{
    load();
    const std::string& latest = config::get().ha_latest;
    if (latest.empty() || s_info.version.empty() || update::compare_versions(latest, s_info.version) <= 0) return "";
    return latest;
}

void open()
{
    brls::Application::pushActivity(new HaUpdateActivity());
}

std::vector<changelog::Line> changes()
{
    ui::settle_writes(changelog_file());   // the changelog the check just kept
    std::string md;
    const std::string latest = newer();
    if (latest.empty() || !paths::read_file(changelog_file(), md) || changelog::release_notes(md, latest).empty())
        return {};
    return changelog::newer_than(md, s_info.version);
}

}   // namespace ha_update_flow
