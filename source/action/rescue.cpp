// rescue — app side (see rescue.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rescue.hpp"

#include <borealis.hpp>
#include <cstdio>
#include <sys/stat.h>

#include "action/data_notice.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace rescue
{

namespace
{
std::string report_path()
{
    return paths::data_dir() + "/" + RESCUE_REPORT_NAME;
}

// atmosphere/contents/<id>/ on the SD card, with its trailing slash.
std::string module_dir()
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';   // "/" on the console
    path += RESCUE_SYSMODULE_PATH;
    return path.substr(0, path.rfind('/') + 1);
}
}   // namespace

const char* revision()
{
#ifdef APP_RESCUE_REV
    return APP_RESCUE_REV;
#else
    return "";
#endif
}

void note_version()
{
    if (!installed() || !*revision()) {
        notices::clear("rescue_old");
        return;
    }
    // "1.2.0 84d0eaa86878": the version it came with, its revision.
    std::string text, version, rev;
    if (paths::read_file(module_dir() + "playguard-rescue.txt", text)) {
        const size_t space = text.find(' ');
        if (space != std::string::npos) {
            version = text.substr(0, space);
            rev = text.substr(space + 1);
            rev = rev.substr(0, rev.find_first_of("\r\n "));
        }
    }
    if (rev == revision()) {
        notices::clear("rescue_old");
        return;
    }
    if (!bundled()) {
        const std::string detail = version.empty()
            ? brls::getStr("playguard/notices/rescue_unknown", app::version())
            : brls::getStr("playguard/notices/rescue_old_detail", version, app::version());
        notices::set("rescue_old", notices::Level::Action, "playguard/notices/rescue_old"_i18n, detail);
        return;
    }
    const std::string detail = version.empty()
        ? brls::getStr("playguard/notices/rescue_unknown_update", app::version())
        : brls::getStr("playguard/notices/rescue_old_update", version, app::version());
    notices::set("rescue_old", notices::Level::Action, "playguard/notices/rescue_old"_i18n, detail,
                 "playguard/notices/rescue_update"_i18n, []() {
                     std::string error;
                     if (!update(&error)) {
                         ui::error(brls::getStr("playguard/notices/rescue_update_failed", error));
                         return;
                     }
                     notices::clear("rescue_old");
                     ui::info("playguard/notices/rescue_updated"_i18n);
                 });
}

bool bundled()
{
    struct stat st;
    return stat(BRLS_ASSET("rescue/exefs.nsp"), &st) == 0;
}

bool update(std::string* error)
{
    std::string nsp;
    if (!paths::read_file(BRLS_ASSET("rescue/exefs.nsp"), nsp) || nsp.empty()) {
        if (error) *error = "This build does not carry the module";
        return false;
    }
    const std::string dir = module_dir();
    // The module first, then the file naming it: should the module not be
    // written, the next start still reads the old revision and offers this again.
    if (!paths::atomic_write(dir + "exefs.nsp", nsp, error)) {
        brls::Logger::error("rescue: the module could not be updated: {}", error ? *error : "");
        return false;
    }
    if (!paths::atomic_write(dir + "playguard-rescue.txt", app::version() + " " + revision() + "\n", error)) {
        brls::Logger::error("rescue: playguard-rescue.txt could not be written: {}", error ? *error : "");
        return false;
    }
    brls::Logger::info("rescue: module updated to revision {} ({} bytes)", revision(), nsp.size());
    return true;
}

bool pending()
{
    std::string ignore;
    return paths::read_file(report_path(), ignore);
}

bool installed()
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';   // "/" on the console
    path += RESCUE_SYSMODULE_PATH;
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::optional<RescueReport> take()
{
    const std::string path = report_path();
    std::string text;
    if (!paths::read_file(path, text)) return std::nullopt;
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());

    RescueReport r;
    if (!rescue_report_parse(text.c_str(), text.size(), &r)) {
        // The sysmodule may or may not have acted: the parent is told (once,
        // on the main screen) rather than left guessing.
        brls::Logger::warning("rescue report present but unreadable");
        data_notice::rescue_unreadable();
        return std::nullopt;
    }
    brls::Logger::info("rescue report: mode={} result={} rc=0x{:08X}",
                       rescue_mode_name(r.mode), rescue_result_name(r.result), (unsigned)r.rc);
    // A plain event so a parent sees recovery was used; the specifics are on
    // the recovery screen and in the log. The delete / PIN actions the parent
    // then takes record their own entries.
    history_flow::record_event("rescue");
    return r;
}

bool confirmed(const RescueReport& report)
{
    const bool present = installed();
    u32 pin_length = 0;
    bool unlocked = false;
    const Result rc = pctl_lock_state(&pin_length, &unlocked);
    const bool ok = present && R_SUCCEEDED(rc) && rescue_report_confirmed(&report, pin_length, unlocked);
    brls::Logger::info("rescue report {} by the console (sysmodule {}, rc=0x{:08X}, PIN {}, unlocked={})",
                       ok ? "confirmed" : "not confirmed", present ? "installed" : "missing", (unsigned)rc,
                       pin_length ? "set" : "none", unlocked);
    return ok;
}

}   // namespace rescue
