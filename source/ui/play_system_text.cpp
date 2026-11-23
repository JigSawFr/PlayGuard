// ui — compatibility, serial and game-patch lines as text (PlayGuard only: the
// kit's ui/system_text.cpp has the firmware and storage ones).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "core/fw_policy.h"
#include "ui/play_ui.hpp"

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "util/paths.hpp"
#include "util/prodinfo.hpp"

#include <format>

using namespace aurore::literals;

namespace ui
{

std::string compat_text(const SysInfo& info, const char** color)
{
    const std::string tested = hos_text(PCTL_FW_TESTED_MAX);
    switch (sysinfo_compat(&info)) {
        case SysCompat_Ok:
            if (color) *color = "cadran/teal";
            return "playguard/compat/ok"_i18n;
        case SysCompat_UntestedNewer:
            if (color) *color = "cadran/amber";
            return aurore::getStr("playguard/compat/untested", std::string(tested));
        case SysCompat_PlayTimerUnsupported:
            if (color) *color = "cadran/amber";
            return "playguard/compat/too_old"_i18n;
        default:
            if (color) *color = "cadran/coral";
            return "playguard/compat/not_ams"_i18n;
    }
}

std::string other_storage_note(const SysInfo& info)
{
    switch (sysinfo_storage(&info)) {
        case SysStorage_EmuMMC: return "playguard/tools/storage_note_emummc"_i18n;
        case SysStorage_SysMMC: {
            // Read once: emummc.ini only changes with the SD card out.
            static const bool emummc = patches::emummc_configured(paths::sd_root());
            return emummc ? "playguard/tools/storage_note_sysmmc"_i18n : "";
        }
        default: return "";
    }
}

bool serial_warning(const SysInfo& info)
{
    return sysinfo_storage(&info) == SysStorage_EmuMMC && info.blank_valid && !info.blank;
}

std::string blank_text(const SysInfo& info, const char** color)
{
    if (!info.blank_valid) {
        if (color) *color = "cadran/text2";
        return "cadran/common/unavailable"_i18n;
    }
    if (color) *color = info.blank ? "cadran/teal" : (serial_warning(info) ? "cadran/amber" : "cadran/text2");
    if (!info.blank) return "cadran/common/no"_i18n;
    // With what the system sees instead.
    return info.serial_valid && info.serial[0] ? aurore::getStr("playguard/tools/blank_yes", std::string(info.serial))
                                               : "cadran/common/yes"_i18n;
}

const RealSerial& real_serial(const SysInfo& info)
{
    // Read once: the backups only change with a reboot (or the SD card out).
    static bool       read = false;
    static RealSerial r;
    if (read) return r;
    read = true;
    if (!(info.blank_valid && info.blank)) {
        // Not blanked: the system gives the real one.
        if (info.serial_valid && info.serial[0]) {
            r.serial = info.serial;
            r.note   = aurore::getStr("playguard/tools/serial_how_system", r.serial);
        } else {
            r.note = "playguard/tools/serial_how_unread"_i18n;
        }
        return r;
    }
    const prodinfo::Found f = prodinfo::find(paths::sd_root(), info.device_id_valid, info.device_id);
    r.source = f.source;
    r.serial = f.serial;
    switch (f.source) {
        case prodinfo::Source::Matched:
            r.note = aurore::getStr("playguard/tools/serial_how_backup", r.serial, f.file);
            break;
        case prodinfo::Source::Single:
            r.note = aurore::getStr("playguard/tools/serial_how_backup_single", r.serial, f.file);
            break;
        case prodinfo::Source::Ambiguous: r.note = "playguard/tools/serial_how_ambiguous"_i18n; break;
        case prodinfo::Source::Others: r.note = "playguard/tools/serial_how_others"_i18n; break;
        case prodinfo::Source::None: r.note = "playguard/tools/serial_how_none"_i18n; break;
    }
    return r;
}

std::string serial_text(const SysInfo& info)
{
    const std::string& s = real_serial(info).serial;
    if (s.empty()) return "cadran/common/unavailable"_i18n;
    if (s.size() < 8) return s;
    // Keep the prefix (model / region) and the last two digits; the whole
    // number is in the note (Ⓐ).
    std::string masked = s.substr(0, 7);
    for (size_t i = 7; i + 2 < s.size(); i++) masked += "•";
    return masked + s.substr(s.size() - 2);
}

bool patches_warning(const patches::Report& report)
{
    const auto st = patches::status(report);
    return st == patches::Status::FilesOnly || st == patches::Status::SysPatchIncomplete || patches::mixed(report);
}

static std::string join_list(const std::vector<std::string>& items)
{
    std::string out;
    for (const auto& i : items) out += (out.empty() ? "" : ", ") + i;
    return out;
}

std::string patches_text(const patches::Report& report, const char** color)
{
    switch (patches::status(report)) {
        case patches::Status::SysPatch:
            if (color) *color = "cadran/teal";
            return "playguard/tools/patches_syspatch"_i18n;
        case patches::Status::SysPatchIncomplete:
            if (color) *color = "cadran/amber";
            return "playguard/tools/patches_incomplete"_i18n;
        case patches::Status::FilesOnly:
            if (color) *color = "cadran/amber";
            return aurore::getStr("playguard/tools/patches_files", join_list(report.files));
        default:
            if (color) *color = "cadran/text2";
            return "playguard/tools/patches_none"_i18n;
    }
}

std::string patches_note(const patches::Report& report, const SysInfo& info, bool* warn)
{
    *warn = true;
    switch (patches::status(report)) {
        case patches::Status::FilesOnly:
            return "playguard/tools/patches_note_files"_i18n;
        case patches::Status::SysPatch:
            // sigpatch files beside it: two tools on the same checks, a
            // cause of forwarders that do not start (patches::mixed).
            if (patches::mixed(report)) return aurore::getStr("playguard/tools/patches_note_mixed", join_list(report.files));
            *warn = false;
            return report.files.empty() ? std::string()
                                        : aurore::getStr("playguard/tools/patches_note_redundant", join_list(report.files));
        case patches::Status::SysPatchIncomplete:
            switch (patches::issue(report)) {
                case patches::Issue::NotAtBoot: return "playguard/tools/patches_note_not_at_boot"_i18n;
                case patches::Issue::NoLog:     return "playguard/tools/patches_note_no_log"_i18n;
                case patches::Issue::StaleLog:
                    return aurore::getStr("playguard/tools/patches_note_stale",
                                        report.log_fw.empty() ? std::string("?") : report.log_fw,
                                        std::string(report.log_emummc ? "emuMMC" : "sysMMC"));
                case patches::Issue::Skipped:
                    return aurore::getStr("playguard/tools/patches_note_skipped",
                                        std::string(info.emummc ? "patch_emummc" : "patch_sysmmc"));
                case patches::Issue::NotPatched:
                    return aurore::getStr("playguard/tools/patches_note_unpatched", join_list(patches::unpatched(report))) +
                           (patches::mixed(report)
                                ? "\n" + aurore::getStr("playguard/tools/patches_note_mixed", join_list(report.files))
                                : std::string());
                default: break;
            }
            break;
        default: break;
    }
    *warn = false;
    return "";
}

}   // namespace ui
