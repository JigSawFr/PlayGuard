// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_flow_shared.hpp"

#include <algorithm>
#include <cstdio>
#include <borealis.hpp>

#include "action/forwarder_flow.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "app.hpp"
#include "core/forwarder.h"
#include "core/gate.h"
#include "ui/ui.hpp"
#include "util/forwarder_gate.hpp"
#include "util/forwarder_icon.hpp"
#include "util/paths.hpp"
#include "util/sha256.hpp"

using namespace brls::literals;

namespace forwarder_flow::detail
{

// "/switch/…" under the SD card's root as the app reads it.
std::string on_sd(const std::string& sd_path)
{
    const std::string root = paths::sd_root();
    return root == "/" ? sd_path : root + sd_path;
}

bool nro_exists(const std::string& sdmc)
{
    const std::string p = nro::sd_path(sdmc);
    return !p.empty() && paths::file_exists(on_sd(p));
}

// The argv a forwarder hands its .nro: its path, quoted when it holds a
// space (libnx splits the argv there), as sphaira writes it.
std::string argv_of(const std::string& sdmc)
{
    return sdmc.find(' ') == std::string::npos ? sdmc : "\"" + sdmc + "\"";
}

// The detail the history keeps: the name, and the program ID.
std::string history_label(const std::string& name, uint64_t id)
{
    return name + " (" + forwarders::hex_id(id) + ")";
}

// What a forwarder is built from: the .nro's icon and NACP. PlayGuard's own
// falls back to the icon it ships with when its .nro cannot be read (the
// desktop has none).
bool assets_of(const std::string& sdmc, bool self, nro::Info& info, std::string& error)
{
    const std::string path = nro::sd_path(sdmc);
    const bool read = !path.empty() && nro::read(on_sd(path), info);
    if (self) {
        // Its .nro's icon, else the app's own (a build without one).
        info.icon = forwarder_icon::normalize(info.icon);
        if (info.icon.empty()) {
            std::string icon;
            if (paths::read_file(BRLS_ASSET("img/app_icon.jpg"), icon))
                info.icon = forwarder_icon::normalize(std::vector<uint8_t>(icon.begin(), icon.end()));
        }
        // Its version, as the HOME menu's data management shows it.
        if (info.version.empty()) {
            if (info.nacp.size() < nro::NACP_SIZE) info.nacp.assign(nro::NACP_SIZE, 0);
            const std::string v = app::version();
            std::copy(v.begin(), v.begin() + (long)std::min<size_t>(v.size(), 0xF), info.nacp.begin() + 0x3060);
        }
        return nro::icon_usable(info.icon);
    }
    if (!read) {
        error = brls::getStr("playguard/forwarders/nro_unreadable", path.empty() ? sdmc : path);
        return false;
    }
    // As sphaira does: another size or a PNG made a 256 x 256 JPEG, and so
    // is one with EXIF data or progressive coding; none at all, a stand-in
    // (the controller of img/emoji).
    info.icon = forwarder_icon::normalize(info.icon);
    if (info.icon.empty()) {
        std::string png;
        if (paths::read_file(BRLS_ASSET("img/emoji/1f3ae.png"), png))
            info.icon = forwarder_icon::placeholder(std::vector<uint8_t>(png.begin(), png.end()));
    }
    if (!nro::icon_usable(info.icon)) {
        error = brls::getStr("playguard/forwarders/no_icon", path);
        return false;
    }
    return true;
}


Gate gate_nro()
{
    Gate g;
    std::string bundled;
    if (paths::read_file(BRLS_ASSET("gate/gate.nro"), bundled) && !bundled.empty()) {
        Sha256 h;
        h.update(bundled.data(), bundled.size());
        g.sha256 = h.digest();
        std::string hex;
        for (size_t i = 0; i < 8; i++) hex += fmt::format("{:02x}", g.sha256[i]);
        const std::string pinned = paths::data_dir() + "/gate-" + hex + ".nro";
        const std::string plain = paths::data_dir() + "/" + GATE_NRO_NAME;
        // Written when missing or not this one (a file changed on the card is
        // put back: the loader would refuse it).
        const auto put = [&bundled](const std::string& path, std::string* err) {
            std::string on_card;
            return (paths::read_file(path, on_card) && on_card == bundled) ||
                   (paths::ensure_dir(paths::data_dir()) && paths::atomic_write(path, bundled, err));
        };
        std::string err;
        if (put(pinned, &err)) {
            g.path = nro::sdmc_path(pinned);
            g.pinned = true;
            if (!put(plain, &err)) brls::Logger::warning("forwarders: {} not written ({})", GATE_NRO_NAME, err);
            return g;
        }
        brls::Logger::warning("forwarders: the gate not written ({}): PlayGuard is the gate", err);
    }
    g.path = self_nro();
    return g;
}

void tidy_gates()
{
    const std::string current = gate_nro().path;
    std::vector<std::string> used = { current };
    for (const forwarders::Made& m : forwarders::list())
        if (m.pin) used.push_back(m.gate);
    for (const std::string& f : paths::list_files(paths::data_dir(), ".nro")) {
        if (f.rfind("gate-", 0) != 0) continue;
        const std::string sdmc = nro::sdmc_path(paths::data_dir() + "/" + f);
        if (std::find(used.begin(), used.end(), sdmc) != used.end()) continue;
        if (std::remove((paths::data_dir() + "/" + f).c_str()) == 0) brls::Logger::info("forwarders: {} no longer used", f);
    }
}

// Builds and installs `m` from its .nro; says how it went (`done_text`, the
// change history's `kind`), only when it fails with `quiet`. Protected with
// the PIN, it goes through the gate as it is now (gate_nro).
bool install(const forwarders::Made& m, bool self, const char* kind, const std::string& done_text, bool quiet)
{
    nro::Info info;
    std::string error;
    if (!assets_of(m.nro, self, info, error)) {
        ui::error("playguard/forwarders/add_err"_i18n, error);
        return false;
    }
    // Its icon from the album, when it has one still there.
    if (!m.icon.empty()) {
        std::string shot;
        const std::string p = nro::sd_path(m.icon);
        std::vector<uint8_t> icon;
        if (!p.empty() && paths::read_file(on_sd(p), shot))
            icon = forwarder_icon::from_screenshot(std::vector<uint8_t>(shot.begin(), shot.end()), m.icon_x);
        if (nro::icon_usable(icon)) info.icon = icon;
        else brls::Logger::warning("forwarders: {} unreadable, the .nro's icon instead", m.icon);
    }
    forwarders::Made made = m;
    std::string gate_argv;
    Gate gate;
    if (made.pin) {
        gate = gate_nro();
        made.gate = gate.path;
        if (made.gate.empty() || !nro_exists(made.gate)) {
            ui::error("playguard/forwarders/add_err"_i18n, "playguard/forwarders/self_unknown"_i18n);
            return false;
        }
        // Its rules in the gate's argv, in the forwarder (core/gate.h's
        // GateRules): the SD card's files no longer say them.
        gate_argv = forwarder_gate::argv_for(made.gate, forwarder_gate::rules_of(made, pause_rule()), made.argv);
        made.carried = true;
    } else {
        made.gate.clear();
        made.free_days = made.free2_days = 0;
        made.limit = made.carried = false;
        made.verified = false;   // seen working through the gate: again once protected again
    }
    const ForwarderSpec spec = {
        made.nro.c_str(), made.argv.c_str(), made.pin ? made.gate.c_str() : nullptr,
        made.pin ? gate_argv.c_str() : nullptr, made.pin && gate.pinned ? gate.sha256.data() : nullptr,
        made.name.c_str(), made.author.c_str(),
        info.nacp.size() >= nro::NACP_SIZE ? info.nacp.data() : nullptr, info.icon.data(), info.icon.size(),
    };
    u64 id = 0;
    const Result rc = forwarder_install(&spec, &id);
    if (R_SUCCEEDED(rc)) {
        made.id = id;
        made.gone = false;   // on the HOME menu again
        std::string werr;
        if (!forwarders::save(made, &werr)) brls::Logger::warning("forwarders: not remembered ({})", werr);
        if (kind) history_flow::record_event(kind, "", history_label(made.name, id));
        write_rules();
    } else {
        brls::Logger::warning("forwarders: {} not installed ({})", made.nro, ui::rc_text(rc, false));
    }
    if (!quiet || R_FAILED(rc))
        ui::notify_result(rc, done_text.empty() ? brls::getStr("playguard/forwarders/added", made.name) : done_text,
                          "playguard/forwarders/add_err"_i18n);
    return R_SUCCEEDED(rc);
}

// Each of `rows` made again (`pin`: protected too), one after the other,
// quietly; one toast for them all. `kind` for each in the change history.
void install_many(const std::vector<forwarders_logic::Row>& rows, bool pin, const char* kind,
                  const std::string& done_text, std::function<void(bool)> done)
{
    int ok = 0;
    for (const auto& r : rows) {
        forwarders::Made m = made_of(r);
        if (m.nro.empty()) continue;
        if (pin) m.pin = true;
        if (m.name.empty()) m.name = r.name;
        if (install(m, r.self || m.nro == self_nro(), kind, "", true)) ok++;
    }
    if (ok > 0) ui::notify(brls::getStr(done_text, ok), ui::Toast::Done);
    if (done) done(ok > 0);
}

// The forwarder `row` remembered, or as the row says it (made here before
// PlayGuard remembered it).
forwarders::Made made_of(const forwarders_logic::Row& row)
{
    forwarders::Made m;
    if (!forwarders::find(row.id, &m)) {
        m.id = row.id;
        m.nro = row.nro;
        m.argv = argv_of(row.nro);
    }
    return m;
}

// Made here, still known (not PlayGuard's own when `with_self` is false).
std::vector<forwarders_logic::Row> made_rows(bool with_self)
{
    std::vector<forwarders_logic::Row> out;
    for (const auto& r : rows())
        if (r.state != forwarders_logic::State::Elsewhere && !r.nro.empty() && (with_self || !r.self)) out.push_back(r);
    return out;
}

// While the homebrew menu is locked, taking `row` away (deleting it, or its
// PIN: `what`) from the ways back: refused when it is the last one or
// PlayGuard's own (unlock first), confirmed in coral when others remain.
// `go` runs when it may go on.
void guard_way_back(const forwarders_logic::Row& row, const std::string& what, std::function<void()> go,
                    std::function<void()> stop)
{
    const std::vector<forwarders_logic::Row> all = rows();
    switch (forwarders_logic::way_back_guard(all, row, hbmenu_lock::locked())) {
        case forwarders_logic::Guard::Allowed: return go();
        case forwarders_logic::Guard::Refused:
            ui::notice("lock", ui::Check::Warn, "playguard/hbmenu/guard_title"_i18n,
                       brls::getStr(row.self ? "playguard/hbmenu/guard_self" : "playguard/hbmenu/guard_last", row.name));
            return stop();
        case forwarders_logic::Guard::Confirm: {
            std::string others;
            for (const std::string& n : forwarders_logic::other_menus(all, row)) others += (others.empty() ? "" : ", ") + n;
            ui::confirm(brls::getStr("playguard/hbmenu/guard_confirm", row.name, others), what, go, stop, true);
            return;
        }
    }
}

}   // namespace forwarder_flow::detail
