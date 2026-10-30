// hbmenu_lock's check: what locking needs and leaves open, what Atmosphère
// reads, the diagnostic's report.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/hbmenu_lock.hpp"

#include <aurore.hpp>
#include <cerrno>
#include <cstdio>
#include <format>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/cadran_modal.hpp>
#include <aurore/cadran/view/qr_view.hpp>

#include "action/forwarder_flow.hpp"
#include "action/forwarders_logic.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "action/pin_lock.hpp"
#include "action/rescue.hpp"
#include "app.hpp"
#include "core/gate.h"
#include "core/hbl_lock.h"
#include "core/rescue.h"
#include "ui/ui.hpp"
#include "util/diagnostics.hpp"
#include "util/forwarders.hpp"
#include "util/paths.hpp"
#include "util/rescue_page.hpp"
#include "util/pctl_ops_c.hpp"

#include "action/hbmenu_lock_shared.hpp"

using namespace aurore::literals;

namespace hbmenu_lock
{

using namespace detail;

namespace
{

// uLaunch, which replaces the HOME menu and starts homebrew its own way:
// the lock does not reach it.
bool ulaunch_present()
{
    struct stat st;
    return stat(on_sd("atmosphere/contents/0100000000001000/exefs.nsp").c_str(), &st) == 0;
}

// Locking on emuMMC, or on a sysMMC with an emuMMC set up: the lock is on
// the SD card, both systems read it; the forwarders are each system's. What
// to say, from the system it runs on ("" when there is no other one).
std::string other_system()
{
    SysInfo si;
    sysinfo_get(&si);
    if (ui::other_storage_note(si).empty()) return "";
    return sysinfo_storage(&si) == SysStorage_EmuMMC ? "playguard/hbmenu/other_system_emummc"_i18n
                                                     : "playguard/hbmenu/other_system_sysmmc"_i18n;
}

// Why a write under `dir` failed, read now: the SD card full, or written to
// by nothing (forwarders_logic::write_hint); "" when neither shows.
std::string hint_for(const std::string& dir)
{
    uint64_t room = UINT64_MAX;
    struct statvfs v;
    if (statvfs(dir.c_str(), &v) == 0) room = (uint64_t)v.f_bavail * (v.f_frsize ? v.f_frsize : v.f_bsize);
    int err = 0;
    const std::string probe = dir + "/.playguard_write_test";
    const int fd = open(probe.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) err = errno;
    else {
        close(fd);
        unlink(probe.c_str());
    }
    switch (forwarders_logic::write_hint(err, room)) {
        case forwarders_logic::WriteHint::Full: return "playguard/hbmenu/hint_full"_i18n;
        case forwarders_logic::WriteHint::ReadOnly: return "playguard/hbmenu/hint_read_only"_i18n;
        default: return "";
    }
}

}   // namespace

namespace detail
{

// The checks, read now (with `take_log`, the gate's log first: a menu it
// opened is a way back seen working; UI thread only then).
forwarders_logic::LockCheck check(bool take_log)
{
    if (take_log) tell_refusals(forwarder_flow::take_log());
    u32 pin_length = 0;
    const bool pin = R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length > 0;
    return forwarders_logic::lock_check(forwarder_flow::rows(), pin,
                                        forwarders_logic::patches_state(ui::patch_report()));
}

// The checks as a dialog's lines (the lock's confirmation, the check).
std::vector<ui::CheckLine> check_lines(const forwarders_logic::LockCheck& c, bool open_now)
{
    using ui::Check;
    std::vector<ui::CheckLine> out;
    out.push_back({ Check::Header, "playguard/hbmenu/sec_ready"_i18n });
    out.push_back({ c.pin ? Check::Ok : Check::Bad, c.pin ? "playguard/hbmenu/line_pin"_i18n : "playguard/hbmenu/need_pin"_i18n });
    out.push_back({ c.patches ? Check::Ok : Check::Bad,
                    c.patches ? "playguard/hbmenu/line_patches"_i18n : "playguard/hbmenu/need_patches"_i18n });
    out.push_back({ c.self ? Check::Ok : Check::Bad, c.self ? "playguard/hbmenu/line_self"_i18n : "playguard/hbmenu/need_self"_i18n });
    if (!c.verified.empty())
        out.push_back({ Check::Ok, aurore::getStr("playguard/hbmenu/line_menu", joined(c.verified)) });
    else if (!c.menus.empty())
        out.push_back({ Check::Bad, aurore::getStr("playguard/hbmenu/need_verified", joined(c.menus)) });
    else
        out.push_back({ Check::Bad, "playguard/hbmenu/need_menu"_i18n });
    out.push_back({ Check::Header, "playguard/hbmenu/sec_open"_i18n });
    if (open_now) out.push_back({ Check::Warn, "playguard/hbmenu/line_open_now"_i18n });
    if (c.open_doors.empty()) out.push_back({ Check::Ok, "playguard/hbmenu/line_no_doors"_i18n });
    else out.push_back({ Check::Warn, aurore::getStr("playguard/hbmenu/open_doors", joined(c.open_doors)) });
    if (!c.sd_writers.empty()) out.push_back({ Check::Warn, aurore::getStr("playguard/hbmenu/sd_writers", joined(c.sd_writers)) });
    if (ulaunch_present()) out.push_back({ Check::Warn, "playguard/hbmenu/ulaunch"_i18n });
    const std::string other = other_system();
    if (!other.empty()) out.push_back({ Check::Warn, other });
    return out;
}

}   // namespace detail

std::string write_hint(const std::string& dir)
{
    struct stat st;
    // The folder itself missing: its parent says it.
    std::string at = dir;
    while (at.size() > 1 && stat(at.c_str(), &st) != 0) {
        const size_t slash = at.find_last_of('/');
        if (slash == std::string::npos || slash == 0) break;
        at.resize(slash);
    }
    return hint_for(at);
}

namespace
{
// What the check reads, once for both the dialog and the report.
struct State
{
    bool file = false, lock_block = false, defaults = false, album = true, any_app = true, state = false;
    std::string locked_on, ams, patches, module, module_version, gate;
    forwarders_logic::LockCheck c;
};

State read_state(bool take_log)
{
    State st;
    std::string text;
    st.file = paths::read_file(config_path(), text);
    st.lock_block = hbl_lock_locked(text.data(), text.size());
    st.defaults = hbl_lock_has_defaults(text.data(), text.size());
    hbl_lock_effective(text.data(), text.size(), &st.album, &st.any_app);
    std::string s;
    st.state = paths::read_file(in_data(HBL_LOCK_STATE_NAME), s);
    uint8_t a, b, c;
    if (st.state && hbl_lock_state_parse(s.data(), s.size(), &a, &b, &c)) st.locked_on = std::format("{}.{}.{}", a, b, c);
    SysInfo si;
    sysinfo_get(&si);
    if (si.ams_valid) st.ams = std::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro);
    const auto p = forwarders_logic::patches_state(ui::patch_report());
    st.patches = p == forwarders_logic::Patches::Ready        ? "ready"
                 : p == forwarders_logic::Patches::Unknown    ? "unknown"
                 : p == forwarders_logic::Patches::Incomplete ? "incomplete"
                                                              : "missing";
    st.module = !rescue::installed() ? "none" : rescue::outdated() ? "outdated" : "current";
    st.module_version = rescue::installed_version();
    st.gate = paths::file_exists(in_data(GATE_NRO_NAME)) ? "present" : "none";
    st.c = check(take_log);
    return st;
}

std::string yes_no(bool v)
{
    return v ? "playguard/hbmenu/yes"_i18n : "playguard/hbmenu/no"_i18n;
}

// The check's third button: Lock while it is not, else Update the module.
bool lock_offered(const State& st)
{
    return !st.lock_block && !app::read_only();
}
bool update_offered(const State& st)
{
    return !lock_offered(st) && st.module == "outdated" && rescue::bundled() && !app::read_only();
}

// The rescue module's line: whether it is installed and, when it is not
// the one this PlayGuard comes with, which version it came with and which
// one replaces it.
std::string module_text(const State& st)
{
    if (st.module == "none") return "playguard/hbmenu/check_none"_i18n;
    if (st.module == "current") return "playguard/hbmenu/check_current"_i18n;
    const std::string how = update_offered(st) ? "playguard/hbmenu/module_how_update"_i18n
                                               : aurore::getStr("playguard/hbmenu/module_how_zip", app::version());
    return st.module_version.empty()
        ? aurore::getStr("playguard/hbmenu/check_outdated_unknown", app::version(), how)
        : aurore::getStr("playguard/hbmenu/check_outdated", st.module_version, app::version(), how);
}

}   // namespace

void show_check(std::function<void(bool)> done)
{
    using ui::Check;
    const State st = read_state(true);
    const std::string unknown = "playguard/forwarders/unknown"_i18n;
    std::vector<ui::CheckLine> lines;
    // Not locked: said first, before what Atmosphère reads.
    if (!st.lock_block) lines.push_back({ Check::Warn, "playguard/hbmenu/check_not_locked"_i18n });
    // What Atmosphère will read, then the checks, then the system's.
    lines.push_back({ Check::Header, "playguard/hbmenu/sec_reads"_i18n });
    lines.push_back({ st.lock_block ? Check::Ok : Check::Info,
                      aurore::getStr("playguard/hbmenu/check_file", HBL_LOCK_CONFIG_PATH,
                                   !st.file ? "playguard/hbmenu/check_none"_i18n
                                   : st.lock_block ? "playguard/hbmenu/state_locked"_i18n
                                                   : "playguard/hbmenu/state_open"_i18n) });
    lines.push_back({ st.album ? Check::Info : Check::Ok, aurore::getStr("playguard/hbmenu/check_album", yes_no(st.album)) });
    lines.push_back({ st.any_app ? Check::Info : Check::Ok, aurore::getStr("playguard/hbmenu/check_r", yes_no(st.any_app)) });
    for (const ui::CheckLine& l : check_lines(st.c, !st.lock_block)) lines.push_back(l);
    lines.push_back({ Check::Header, "playguard/hbmenu/sec_system"_i18n });
    const std::string ams = st.ams.empty() ? unknown : st.ams;
    // The version it was locked under, only while it is.
    lines.push_back({ Check::Info, st.lock_block && !st.locked_on.empty()
                                       ? aurore::getStr("playguard/hbmenu/check_ams", ams, st.locked_on)
                                       : aurore::getStr("playguard/hbmenu/check_ams_plain", ams) });
    lines.push_back({ st.module == "outdated" ? Check::Warn : Check::Info,
                      aurore::getStr("playguard/hbmenu/check_module", module_text(st)) });
    const auto finish = [done](bool changed) {
        if (done) done(changed);
    };
    auto* dialog = ui::box_dialog(ui::checklist_body("playguard/hbmenu/check"_i18n, lines));
    // The rescue sheet and OK; alone above them, the way to the lock while it
    // is not locked, else an outdated module's update.
    dialog->addButton("playguard/hbmenu/rescue"_i18n, []() { aurore::sync([]() { show_rescue(); }); });
    dialog->addButton("hints/ok"_i18n, []() {});
    if (lock_offered(st))
        dialog->addButton("playguard/hbmenu/lock"_i18n, [finish]() { aurore::sync([finish]() { lock(finish); }); });
    else if (update_offered(st))
        dialog->addButton("playguard/hbmenu/module_update"_i18n,
                          [finish]() { aurore::sync([finish]() { rescue::update_now(finish); }); });
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    if (aurore::View* ok = dialog->getView("aurore/dialog/button2")) dialog->setLastFocusedView(ok);
    dialog->open();
}

Summary summary()
{
    Summary out;
    out.locked = locked();
    out.check = check();
    out.module = rescue::installed();
    return out;
}

std::string report()
{
    // What is on the card as it is: the report may be built off the UI thread.
    const State st = read_state(false);
    return std::format(
        "=== homebrew menu ===\n"
        "override_config: {} (PlayGuard lock={} defaults={})\n"
        "as read        : album opens hbmenu={} R on a game={}\n"
        "state file     : {} (locked on {})\n"
        "atmosphere     : {}\n"
        "ways back      : playguard forwarder={} protected menus={} (seen working {})\n"
        "open doors     : {} (unprotected {}, writing the SD card {})\n"
        "patches={} rescue module={} gate={}\n\n",
        st.file ? "present" : "none", st.lock_block ? 1 : 0, st.defaults ? 1 : 0, st.album ? 1 : 0, st.any_app ? 1 : 0,
        st.state ? "present" : "none", st.locked_on.empty() ? "unknown" : st.locked_on,
        st.ams.empty() ? "unknown" : st.ams, st.c.self ? 1 : 0, st.c.menus.size(), st.c.verified.size(), st.c.open_doors.size(),
        st.c.unprotected.size(), st.c.sd_writers.size(), st.patches, st.module, st.gate);
}

}   // namespace hbmenu_lock
