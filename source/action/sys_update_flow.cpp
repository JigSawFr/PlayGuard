// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sys_update_flow.hpp"

#include <borealis.hpp>
#include <fmt/format.h>
#include <mutex>
#include <string>

#include "action/notices.hpp"
#include "action/sys_notes_flow.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/sys_updates.hpp"
#include "util/update.hpp"

using namespace brls::literals;

namespace sys_update_flow
{

namespace
{
bool s_checking = false;

// One entry: `latest` newer than `running`, else none.
void entry(const char* id, const std::string& latest, const std::string& running, const std::string& title,
           const std::string& detail)
{
    if (latest.empty() || running.empty() || update::compare_versions(latest, running) <= 0) notices::clear(id);
    else notices::set(id, notices::Level::Info, title, detail);
}

// The entries from what the last check found, against what runs now.
void note()
{
    const sys_updates::Latest latest = sys_updates::from_text(config::get().sys_latest);
    SysInfo si;
    sysinfo_get(&si);
    const std::string ams = si.ams_valid ? fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro) : "";
    char fw[16];
    sysinfo_version_string(si.hos_version, fw, sizeof(fw));
    const std::string hekate = installed_hekate();
    entry("sys_atmosphere", latest.atmosphere, ams, brls::getStr("playguard/notices/sys_atmosphere", latest.atmosphere),
          brls::getStr("playguard/notices/sys_atmosphere_detail", ams));
    entry("sys_hekate", latest.hekate, hekate, brls::getStr("playguard/notices/sys_hekate", latest.hekate),
          brls::getStr("playguard/notices/sys_hekate_detail", hekate));
    // What the notes of Atmosphère and hekate say of that firmware, when they say something.
    const std::string advice = sys_notes_flow::firmware_advice();
    entry("sys_firmware", latest.firmware, si.hos_version ? std::string(fw) : "",
          brls::getStr("playguard/notices/sys_firmware", latest.firmware),
          brls::getStr("playguard/notices/sys_firmware_detail", std::string(fw)) + (advice.empty() ? "" : "\n" + advice));
}

void check()
{
    if (s_checking) return;
    s_checking = true;
    ui::in_background("system update check", []() {
        std::string error;
        const sys_updates::Latest found = sys_updates::fetch(&error);
        // The release lists too (one request each): how far behind, and
        // whether the newest firmware is supported yet.
        const sys_notes::Cache lists = ui::quitting() ? sys_notes::Cache{} : sys_notes_flow::fetch_lists();
        if (ui::quitting()) return;
        brls::sync([found, error, lists]() {
            sys_notes_flow::keep(lists);
            s_checking = false;
            if (!error.empty()) brls::Logger::info("system update check: {}", error);
            // What could not be read keeps what the last check found.
            auto& cfg = config::get();
            sys_updates::Latest kept = sys_updates::from_text(cfg.sys_latest);
            if (!found.atmosphere.empty()) kept.atmosphere = found.atmosphere;
            if (!found.hekate.empty()) kept.hekate = found.hekate;
            if (!found.firmware.empty()) kept.firmware = found.firmware;
            const bool any = !found.atmosphere.empty() || !found.hekate.empty() || !found.firmware.empty();
            if (any) {
                cfg.sys_latest  = sys_updates::to_text(kept);
                cfg.sys_checked = ui::today_date();
                ui::save_config();
            }
            note();
        });
    });
}
}   // namespace

// bootloader/update.bin is what hekate loads when it is newer than the
// payload that started it: the version on the SD card. The version sits in
// the first KB of a file of about 200 KB: only that is read, once (Tools ›
// Console asks for it each time it is shown).
std::string installed_hekate()
{
    static std::mutex lock;
    static bool read = false;
    static std::string version;
    std::lock_guard<std::mutex> guard(lock);
    if (read) return version;
    read = true;
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';
    std::string bytes;
    if (paths::read_ends(path + "bootloader/update.bin", 1024, 0, bytes)) version = sys_updates::hekate_version(bytes);
    return version;
}

std::string with_latest(const char* which, const std::string& running, bool* newer)
{
    const sys_updates::Latest l = sys_updates::from_text(config::get().sys_latest);
    const std::string w = which;
    const std::string& latest = w == "atmosphere" ? l.atmosphere : w == "hekate" ? l.hekate : l.firmware;
    *newer = !latest.empty() && !running.empty() && update::compare_versions(latest, running) > 0;
    return *newer ? brls::getStr("playguard/tools/available", running, latest) : running;
}

void at_start()
{
    note();
    const auto& cfg = config::get();
    if (cfg.update_daily && cfg.sys_checked != ui::today_date()) check();
}

void refresh_entries()
{
    note();
}

void check_now()
{
    check();
}

}   // namespace sys_update_flow
