// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "util/diagnostics.hpp"

#include <ctime>
#include <fmt/format.h>
#include <sys/statvfs.h>
#include <vector>

#include "app.hpp"
#include "util/log_files.hpp"
#include "util/patches.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/perf.hpp"
#include "util/session_log.hpp"
#include "util/startup.h"
#include "core/platform.h"

namespace diagnostic
{

namespace
{
std::string (*s_firmware_choice)() = nullptr;

// "1.5 GiB", "312.4 MiB", "18 KiB".
std::string bytes(uint64_t n)
{
    if (n >= (1ULL << 30)) return fmt::format("{:.1f} GiB", (double)n / (1ULL << 30));
    if (n >= (1ULL << 20)) return fmt::format("{:.1f} MiB", (double)n / (1ULL << 20));
    return fmt::format("{} KiB", (n + 1023) / 1024);
}

std::string first_line(const std::string& text)
{
    return text.substr(0, text.find('\n'));
}

// This run, the one before, the memory, the SD card and the crash files kept.
std::string logs_section()
{
    std::string out = "=== logs ===\n";
    const std::string current = session_log::current_name();
    out += fmt::format("session     : {}, {} s since the start\n",
                       current.empty() ? std::string("no session file") : "logs/sessions/" + current,
                       session_log::uptime_s());

    // The run before this one, and how it ended.
    std::string before = "none kept";
    for (const std::string& name : log_files::list(paths::sessions_dir(), log_files::SESSION)) {
        if (name == current) continue;
        std::string text;   // its end line: the end of the file is enough
        if (!session_log::tail_of(paths::sessions_dir() + "/" + name, &text)) {
            before = name + ": cannot be read";
            break;
        }
        const std::string reason = session_log::end_reason(text);
        before = name + ": " + (reason.empty() ? std::string("did not end (crash, hang or power off)") : reason);
        break;
    }
    out += "last run    : " + before + "\n";

    uint64_t used = 0, total = 0;
    out += "memory      : " +
           (platform_memory(&used, &total) ? fmt::format("{} allocated, of {} for the process", bytes(used), bytes(total)) : std::string("unknown")) +
           "\n";
    struct statvfs fs;
    if (statvfs(paths::sd_root().c_str(), &fs) == 0 && fs.f_blocks > 0)
        out += fmt::format("sd card     : {} free of {}\n", bytes((uint64_t)fs.f_bavail * fs.f_frsize),
                           bytes((uint64_t)fs.f_blocks * fs.f_frsize));
    else
        out += "sd card     : unknown\n";

    // The newest crash files, with their first line (what happened).
    const std::vector<std::string> crashes = log_files::list(paths::crashes_dir(), log_files::CRASH);
    out += fmt::format("crash files : {}{}\n", crashes.size(), crashes.empty() ? "" : " (newest first)");
    for (size_t i = 0; i < crashes.size() && i < 5; i++) {
        std::string text;
        paths::read_file(paths::crashes_dir() + "/" + crashes[i], text);
        out += "  " + crashes[i] + "\n    " + first_line(text) + "\n";
    }
    return out + "\n";
}
}   // namespace

void set_firmware_choice(std::string (*summary)()) { s_firmware_choice = summary; }

const char* storage_name(SysStorage storage)
{
    switch (storage) {
        case SysStorage_EmuMMC: return "emuMMC";
        case SysStorage_SysMMC: return "sysMMC";
        default:                return nullptr;
    }
}

std::string current_report()
{
    SysInfo si;
    sysinfo_get(&si);
    char fw[16];
    sysinfo_version_string(si.hos_version, fw, sizeof(fw));

    // Game patches: sys-patch state and sigpatch files (no personal data).
    const patches::Report pr = patches::detect(paths::sd_root(), fw, si.emummc);
    std::string files;
    for (const auto& f : pr.files) files += (files.empty() ? "" : ",") + f;
    static const char* const STATUS[] = { "none", "sys-patch", "sys-patch incomplete", "sigpatch files only" };
    static const char* const ISSUE[]  = { "none", "not at boot", "no log", "stale log", "skipped", "not patched" };
    const std::string patch_line = fmt::format(
        "{} (issue: {}; sys-patch installed={} boot2={} log={}{}; fs={} ldr={} es={}; files={})",
        STATUS[(int)patches::status(pr)], ISSUE[(int)patches::issue(pr)],
        pr.syspatch_installed ? 1 : 0, pr.syspatch_at_boot ? 1 : 0,
        !pr.log_present ? "none" : (pr.log_current ? "current" : "stale"),
        pr.log_present ? fmt::format(" fw={} emummc={}", pr.log_fw, pr.log_emummc ? 1 : 0) : std::string(),
        patches::module_name(pr.fs), patches::module_name(pr.ldr), patches::module_name(pr.es),
        files.empty() ? std::string("none") : files);

    // The serial number itself is never written: reports are attached to
    // public bug reports.
    const char* storage = storage_name(sysinfo_storage(&si));
    std::string out = fmt::format(
        "=== PlayGuard diagnostic ===\n"
        "app version : {}{}\n"
        "modes       : read_only={} dev={} firmware_choice={} platform={}\n"
        "firmware    : {}\n"
        "atmosphere  : {} ({})\n"
        "launch mode : {}\n"
        "storage     : {}\n"
        "prodinfo    : {}\n"
        "game patches: {}\n"
        "compat      : {}\n\n",
        app::version(), app::commit().empty() ? std::string() : " (commit " + app::commit() + ")",
        app::read_only() ? 1 : 0, app::dev_mode() ? 1 : 0, s_firmware_choice ? s_firmware_choice() : "unknown",
#ifdef __SWITCH__
        "switch",
#else
        "desktop-simulation",
#endif
        fw,
        si.ams_valid ? fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro) : std::string("unknown"),
        si.is_atmosphere ? "detected" : "not detected",
        si.applet_mode ? "applet (album)" : "application",
        fmt::format("{} (data: nand/{})",
                    storage ? storage : "unknown",
                    paths::nand_key()),
        !si.blank_valid ? "unknown" : (si.blank ? "blanked" : "not blanked"),
        patch_line,
        (int)sysinfo_compat(&si));

    // How long this run took to start, step by step (util/startup.h).
    std::vector<char> buf(4096);
    startup_report(buf.data(), buf.size());
    out += buf.data();
    out += "\n";
    // How long the slow work of the screens took in this run (util/perf.hpp).
    out += perf::report() + "\n";
    out += logs_section();

    time_clock_dump(buf.data(), buf.size());
    out += buf.data();

    buf.assign(24576, '\0');
    pctl_dump(buf.data(), buf.size());
    out += buf.data();
    if (std::string(buf.data()).size() >= buf.size() - 1) out += "\n(report truncated)\n";

    // Last: what led here, which can be long.
    const std::string recent = session_log::recent();
    out += "\n=== last lines logged (oldest first) ===\n" + (recent.empty() ? std::string("(none)\n") : recent);
    return out;
}

std::string save(const std::string& report, std::string* error)
{
    return log_files::save(paths::reports_dir(), log_files::REPORT, report, std::time(nullptr), error);
}

}   // namespace diagnostic
