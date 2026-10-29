// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/self_test.hpp"

#include <aurore.hpp>

#include <aurore/cadran/ui/cadran_modal.hpp>

#include "action/forwarder_flow.hpp"
#include "action/forwarders_logic.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/rescue.hpp"
#include "app.hpp"
#include "core/gate.h"
#include "core/hbl_lock.h"
#include "core/pctl_ops.h"
#include "core/sysinfo.h"
#include "ui/ui.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"
#include "util/paths.hpp"

using namespace aurore::literals;
using self_test_logic::Item;
using Res = self_test_logic::Result;   // not libnx's Result

namespace self_test
{

namespace
{
constexpr const char* REPORT_NAME = "self-test.md";

std::string on_sd(const char* rel)
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';
    return path + rel;
}

std::string joined(const std::vector<std::string>& names)
{
    std::string out;
    for (const std::string& n : names) out += (out.empty() ? "" : ", ") + n;
    return out;
}

void console(std::vector<Item>& out)
{
    const char* S = "Console";
    u32 pin_length = 0;
    const bool read = R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr));
    out.push_back({ S, !read ? Res::Warn : pin_length ? Res::Pass : Res::Fail,
                    !read ? "The PIN could not be read" : pin_length ? "A PIN on the console" : "No PIN on the console: the gate has nothing to ask" });
    switch (forwarders_logic::patches_state(ui::patch_report())) {
        case forwarders_logic::Patches::Ready: out.push_back({ S, Res::Pass, "Signature patches: ready for forwarders" }); break;
        case forwarders_logic::Patches::Unknown:
            out.push_back({ S, Res::Warn, "Signature patches: sys-patch at boot, its log not read" });
            break;
        case forwarders_logic::Patches::Incomplete:
            out.push_back({ S, Res::Fail,
                            "Signature patches: incomplete, a forwarder would not start (not patched: " +
                                forwarders_logic::patch_gaps(ui::patch_report()) + ")" });
            break;
        default: out.push_back({ S, Res::Fail, "Signature patches: missing (a forwarder would not start)" });
    }
    const std::string self = app::self_path();
    out.push_back({ S, self.empty() ? Res::Warn : Res::Pass,
                    self.empty() ? "PlayGuard's own .nro: unknown" : "Started from " + self });
    const std::string hint = hbmenu_lock::write_hint(paths::data_dir());
    out.push_back({ S, hint.empty() ? Res::Pass : Res::Fail,
                    hint.empty() ? "config/playguard/ can be written" : "config/playguard/ cannot be written: " + hint });
}

void forwarders_part(std::vector<Item>& out)
{
    const char* S = "Forwarders";
    const bool loader = forwarder_flow::loader_present();
    out.push_back({ S, loader ? Res::Pass : Res::Fail,
                    loader ? "This build carries the forwarders' program" : "This build has no forwarders' program" });
    if (!loader) return;
    using forwarders_logic::State;
    int made = 0;
    for (const forwarders_logic::Row& r : forwarder_flow::rows()) {
        if (r.state == State::Elsewhere) {
            out.push_back({ S, Res::Warn, r.name + ": made by another app (what it starts is not known)" });
            continue;
        }
        made++;
        std::string what = r.name + (r.self ? " (PlayGuard's own)" : "") + ": ";
        switch (r.state) {
            case State::Ready:
                what += "on the HOME menu";
                if (r.pin) what += r.verified ? ", protected, seen working through its gate" : ", protected, not started through its gate yet";
                // Made before it carried its rules: its free hours wait for it to be made again.
                if (r.pin && !r.carried) what += ", made before it carried its rules (Make again)";
                out.push_back({ S, r.pin && (!r.verified || !r.carried) ? Res::Warn : Res::Pass, what });
                break;
            case State::NotInstalled: out.push_back({ S, Res::Fail, what + "no longer on the HOME menu" }); break;
            case State::NroMissing: out.push_back({ S, Res::Fail, what + "its .nro is missing (" + nro::sd_path(r.nro) + ")" }); break;
            case State::GateMissing: out.push_back({ S, Res::Fail, what + "its gate is missing on the SD card" }); break;
            default: break;
        }
    }
    if (!made) out.push_back({ S, Res::Skip, "No forwarder made by PlayGuard" });
    // The gate on the card: the one this PlayGuard carries.
    std::string bundled, on_card;
    if (!paths::read_file(AURORE_ASSET("gate/gate.nro"), bundled) || bundled.empty())
        out.push_back({ S, Res::Skip, "This build carries no gate (PlayGuard is the gate)" });
    else if (!paths::read_file(paths::data_dir() + "/" + GATE_NRO_NAME, on_card))
        out.push_back({ S, Res::Skip, "No gate on the SD card yet (written once a forwarder is protected)" });
    else
        out.push_back({ S, on_card == bundled ? Res::Pass : Res::Warn,
                        on_card == bundled ? "The gate on the SD card is this PlayGuard's"
                                           : "The gate on the SD card differs from this PlayGuard's (written again at the next start)" });
}

void homebrew_menu(std::vector<Item>& out)
{
    const char* S = "Homebrew menu";
    const hbmenu_lock::Summary s = hbmenu_lock::summary();
    std::string ini;
    const bool file = paths::read_file(on_sd(HBL_LOCK_CONFIG_PATH), ini);
    bool album = true, any_app = true;
    hbl_lock_effective(ini.data(), ini.size(), &album, &any_app);
    // A reading while unlocked; a pass once locked and read so.
    out.push_back({ S, s.locked && !album && !any_app ? Res::Pass : Res::Skip,
                    std::string(HBL_LOCK_CONFIG_PATH) + ": " + (!file ? "none" : s.locked ? "locked by PlayGuard" : "not locked") +
                        "; the album opens it: " + (album ? "yes" : "no") + "; R on a game: " + (any_app ? "yes" : "no") });
    if (s.locked && (album || any_app))
        out.push_back({ S, Res::Fail, "Locked, yet Atmosphère would still open the homebrew menu" });
    const forwarders_logic::LockCheck& c = s.check;
    out.push_back({ S, c.self ? Res::Pass : (s.locked ? Res::Fail : Res::Skip),
                    c.self ? "Way back: PlayGuard's own forwarder" : "No PlayGuard forwarder on the HOME menu" });
    if (!c.verified.empty()) out.push_back({ S, Res::Pass, "Protected menus seen working: " + joined(c.verified) });
    else if (!c.menus.empty()) out.push_back({ S, Res::Warn, "Protected menus not started through their gate yet: " + joined(c.menus) });
    else out.push_back({ S, s.locked ? Res::Fail : Res::Skip, "No protected homebrew menu (sphaira or hbmenu)" });
    if (!c.open_doors.empty()) out.push_back({ S, Res::Warn, "Would open homebrew without the PIN: " + joined(c.open_doors) });
    out.push_back({ S, !s.module ? Res::Skip : rescue::outdated() ? Res::Warn : Res::Pass,
                    !s.module ? "Rescue module: not installed" : rescue::outdated() ? "Rescue module: an older one" : "Rescue module: installed, this PlayGuard's" });
}
}   // namespace

std::vector<Item> run()
{
    std::vector<Item> out;
    console(out);
    forwarders_part(out);
    homebrew_menu(out);
    return out;
}

void open()
{
    const std::vector<Item> items = run();
    SysInfo si;
    sysinfo_get(&si);
    const std::string storage = sysinfo_storage(&si) == SysStorage_EmuMMC ? "emuMMC"
                                : sysinfo_storage(&si) == SysStorage_SysMMC ? "sysMMC"
                                                                             : "storage unknown";
    const self_test_logic::Header h = { app::version(), ui::fw_text(si), storage, ui::now_stamp() };
    const std::string md = self_test_logic::markdown(h, items);
    std::string err;
    const std::string path = paths::data_dir() + "/" + REPORT_NAME;
    const bool written = paths::ensure_dir(paths::data_dir()) && paths::atomic_write(path, md, &err);
    if (!written) aurore::Logger::warning("self-test: {} not written ({})", REPORT_NAME, err);
    std::vector<ui::CheckLine> lines;
    std::string section;
    for (const Item& i : items) {
        if (i.section != section) lines.push_back({ ui::Check::Header, section = i.section });
        lines.push_back({ i.result == Res::Pass   ? ui::Check::Ok
                          : i.result == Res::Warn ? ui::Check::Warn
                          : i.result == Res::Fail ? ui::Check::Bad
                                                     : ui::Check::Info,
                          i.text });
    }
    const self_test_logic::Tally t = self_test_logic::tally(items);
    const std::string note = written ? aurore::getStr("playguard/dev/self_test_written", "config/playguard/" + std::string(REPORT_NAME))
                                     : aurore::getStr("playguard/dev/self_test_not_written", err);
    auto* dialog = ui::box_dialog(ui::checklist_body("playguard/dev/self_test"_i18n, lines, note,
                                                       aurore::getStr("playguard/dev/self_test_tally", t.pass, t.warn, t.fail, t.skip)));
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    dialog->open();
    aurore::Logger::info("self-test: {} passed, {} to know, {} failed, {} not checked", t.pass, t.warn, t.fail, t.skip);
}

}   // namespace self_test
