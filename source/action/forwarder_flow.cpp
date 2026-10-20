// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_flow.hpp"

#include <algorithm>
#include <borealis.hpp>

#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "app.hpp"
#include "core/forwarder.h"
#include "core/pctl_ops.h"
#include "ui/ui.hpp"
#include "core/gate.h"
#include "util/forwarder_gate.hpp"
#include "util/forwarder_icon.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"
#include "util/paths.hpp"
#ifndef __SWITCH__
#include "sim/sim_homebrew.hpp"
#endif

using namespace brls::literals;

namespace forwarder_flow
{

namespace
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

forwarders::Made made_of(const forwarders_logic::Row& row);
void write_rules();

// The gate a protected forwarder starts first: config/playguard/gate.nro,
// written from the romfs when this build carries it (again when PlayGuard
// was updated); else PlayGuard's own .nro (the desktop, or the card refused
// the write), which is a gate too (main.cpp).
std::string gate_nro()
{
    std::string bundled;
    if (paths::read_file(BRLS_ASSET("gate/gate.nro"), bundled) && !bundled.empty()) {
        const std::string path = paths::data_dir() + "/" + GATE_NRO_NAME;
        std::string on_card, err;
        if ((paths::read_file(path, on_card) && on_card == bundled) ||
            (paths::ensure_dir(paths::data_dir()) && paths::atomic_write(path, bundled, &err)))
            return nro::sdmc_path(path);
        brls::Logger::warning("forwarders: {} not written ({}): PlayGuard is the gate", GATE_NRO_NAME, err);
    }
    return self_nro();
}

// Builds and installs `m` from its .nro; says how it went (`done_text`, the
// change history's `kind`), only when it fails with `quiet`. Protected with
// the PIN, it goes through the gate as it is now (gate_nro).
bool install(const forwarders::Made& m, bool self, const char* kind = "add_forwarder",
             const std::string& done_text = "", bool quiet = false)
{
    nro::Info info;
    std::string error;
    if (!assets_of(m.nro, self, info, error)) {
        ui::error("playguard/forwarders/add_err"_i18n, error);
        return false;
    }
    forwarders::Made made = m;
    std::string gate_argv;
    if (made.pin) {
        made.gate = gate_nro();
        if (made.gate.empty() || !nro_exists(made.gate)) {
            ui::error("playguard/forwarders/add_err"_i18n, "playguard/forwarders/self_unknown"_i18n);
            return false;
        }
        gate_argv = forwarder_gate::argv_for(made.gate, made.argv);
    } else {
        made.gate.clear();
        made.free_days = 0;
        made.verified = false;   // seen working through the gate: again once protected again
    }
    const ForwarderSpec spec = {
        made.nro.c_str(), made.argv.c_str(), made.pin ? made.gate.c_str() : nullptr,
        made.pin ? gate_argv.c_str() : nullptr, made.name.c_str(), made.author.c_str(),
        info.nacp.size() >= nro::NACP_SIZE ? info.nacp.data() : nullptr, info.icon.data(), info.icon.size(),
    };
    u64 id = 0;
    const Result rc = forwarder_install(&spec, &id);
    if (R_SUCCEEDED(rc)) {
        made.id = id;
        made.gone = false;   // on the HOME menu again
        std::string werr;
        if (!forwarders::save(made, &werr)) brls::Logger::warning("forwarders: not remembered ({})", werr);
        history_flow::record_event(kind, "", history_label(made.name, id));
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

// The free hours of every protected forwarder, for the gate (GATE_RULES_NAME).
void write_rules()
{
    std::string text;
    for (const forwarders::Made& m : forwarders::list()) {
        if (!m.pin || !m.free_days) continue;
        const GateHours h = { (uint8_t)m.free_days, (uint16_t)m.free_from, (uint16_t)m.free_to };
        char line[0x400];
        if (gate_rule_format(&h, m.nro.c_str(), line, sizeof(line))) text += line;
    }
    const std::string path = paths::data_dir() + "/" + GATE_RULES_NAME;
    if (text.empty()) {
        std::remove(path.c_str());
        return;
    }
    std::string err;
    if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(path, text, &err))
        brls::Logger::warning("forwarders: {} not written ({})", GATE_RULES_NAME, err);
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
            ui::info(brls::getStr(row.self ? "playguard/hbmenu/guard_self" : "playguard/hbmenu/guard_last", row.name));
            return stop();
        case forwarders_logic::Guard::Confirm: {
            std::string others;
            for (const std::string& n : forwarders_logic::other_menus(all, row)) others += (others.empty() ? "" : ", ") + n;
            ui::confirm(brls::getStr("playguard/hbmenu/guard_confirm", row.name, others), what, go, stop, true);
            return;
        }
    }
}

}   // namespace

std::vector<forwarders_logic::Row> rows()
{
#ifndef __SWITCH__
    sim_homebrew::seed(paths::sd_root());   // PlayGuard's own .nro among them
#endif
    std::vector<ForwarderRecord> records(FORWARDER_MAX);
    u32 count = 0;
    const Result rc = forwarder_list(records.data(), FORWARDER_MAX, &count);
    if (R_FAILED(rc)) brls::Logger::warning("forwarders: HOME menu not read ({})", ui::rc_text(rc, false));
    std::vector<forwarders_logic::Installed> installed;
    for (u32 i = 0; i < count; i++)
        installed.push_back({ records[i].id, records[i].name, records[i].author, records[i].version });
    return forwarders_logic::merge(forwarders::list(), installed, nro_exists, self_nro());
}

std::vector<std::string> homebrew()
{
#ifndef __SWITCH__
    sim_homebrew::seed(paths::sd_root());
#endif
    return nro::find(paths::sd_root());
}

std::string self_nro()
{
    const std::string self = app::self_path();
#ifdef __SWITCH__
    return nro::sdmc_path(self);
#else
    return nro::sdmc_path(self.empty() ? std::string("/switch/playguard/playguard.nro") : self);
#endif
}

bool loader_present()
{
#ifdef __SWITCH__
    return paths::file_exists(BRLS_ASSET("forwarder/main")) && paths::file_exists(BRLS_ASSET("forwarder/main.npdm"));
#else
    return true;
#endif
}

void create(const std::string& sd_path, std::function<void(bool)> done)
{
    nro::Info info;
    std::string error;
    const std::string sdmc = nro::sdmc_path(sd_path);
    if (!assets_of(sdmc, false, info, error)) {
        ui::error("playguard/forwarders/add_err"_i18n, error);
        if (done) done(false);
        return;
    }
    const std::string initial = forwarders_logic::default_name(info.name, sd_path);
    const std::string author = forwarders::clean_text(info.author, forwarders::MAX_AUTHOR);
    ui::prompt_text("playguard/forwarders/name"_i18n, initial, (int)forwarders::MAX_NAME,
                    [sdmc, sd_path, author, done](std::string typed) {
        const std::string name = forwarders::clean_text(typed, forwarders::MAX_NAME);
        if (name.empty()) {
            if (done) done(false);
            return;
        }
        ui::confirm(brls::getStr("playguard/forwarders/confirm_add", name, sd_path), "playguard/forwarders/add"_i18n,
                    [sdmc, name, author, done]() {
                        forwarders::Made m;
                        m.nro = sdmc;
                        m.argv = argv_of(sdmc);
                        m.name = name;
                        m.author = author;
                        const bool ok = install(m, false);
                        if (done) done(ok);
                    },
                    [done]() {
                        if (done) done(false);
                    });
    });
}

void create_self(std::function<void(bool)> done)
{
    const std::string sdmc = self_nro();
    if (sdmc.empty()) {
        ui::error("playguard/forwarders/add_err"_i18n, "playguard/forwarders/self_unknown"_i18n);
        if (done) done(false);
        return;
    }
    ui::confirm(brls::getStr("playguard/forwarders/confirm_self", nro::sd_path(sdmc)), "playguard/forwarders/add"_i18n,
                [sdmc, done]() {
                    forwarders::Made m;
                    m.nro = sdmc;
                    m.argv = argv_of(sdmc);
                    m.name = "PlayGuard";
                    m.author = "JigSawFr";
                    const bool ok = install(m, true);
                    if (done) done(ok);
                },
                [done]() {
                    if (done) done(false);
                });
}

void rebuild(const forwarders_logic::Row& row, const std::string& name, const std::string& author,
             std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    m.name = name.empty() ? row.name : name;
    m.author = author.empty() ? row.author : author;
    const bool ok = !m.nro.empty() && install(m, row.self || m.nro == self_nro());
    if (done) done(ok);
}

void set_pin(const forwarders_logic::Row& row, bool on, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (m.nro.empty() || row.self) {
        if (done) done(false);
        return;
    }
    if (!on) {
        const std::string name = row.name;
        guard_way_back(row, "playguard/forwarders/action_pin_off"_i18n, [m, name, done]() mutable {
            // The PIN first, whatever PlayGuard's own lock says: the
            // protection is the parent's to lift.
            if (!pin_lock::confirm()) {
                if (done) done(false);
                return;
            }
            m.pin = false;
            const bool ok =
                install(m, false, "unprotect_forwarder", brls::getStr("playguard/forwarders/pin_removed", name));
            if (done) done(ok);
        }, [done]() {
            if (done) done(false);
        });
        return;
    }
    // Without a PIN on the console, there is nothing to ask for.
    u32 pin_length = 0;
    if (R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length == 0) {
        ui::info("playguard/forwarders/pin_no_pin"_i18n);
        if (done) done(false);
        return;
    }
    const std::string name = row.name;
    ui::confirm(brls::getStr("playguard/forwarders/pin_confirm", name), "playguard/forwarders/action_pin_on"_i18n,
                [m, name, done]() mutable {
                    m.pin = true;
                    const bool ok =
                        install(m, false, "protect_forwarder", brls::getStr("playguard/forwarders/pin_added", name));
                    if (done) done(ok);
                },
                [done]() {
                    if (done) done(false);
                });
}

void remove(const forwarders_logic::Row& row, std::function<void(bool)> done)
{
    const forwarders_logic::Row r = row;
    const auto go = [r, done]() {
        const Result rc = forwarder_remove(r.id);
        if (R_SUCCEEDED(rc)) {
            std::string err;
            if (!forwarders::forget(r.id, &err)) brls::Logger::warning("forwarders: not forgotten ({})", err);
            history_flow::record_event("remove_forwarder", "", history_label(r.name, r.id));
        }
        ui::notify_result(rc, brls::getStr("playguard/forwarders/deleted", r.name), "playguard/forwarders/delete_err"_i18n);
        if (done) done(R_SUCCEEDED(rc));
    };
    const auto stop = []() {};   // left as it was: the list too
    // One of the ways back while locked: its own warning stands for the
    // usual question.
    if (forwarders_logic::way_back_guard(rows(), r, hbmenu_lock::locked()) != forwarders_logic::Guard::Allowed) {
        guard_way_back(r, "playguard/common/delete"_i18n, go, stop);
        return;
    }
    ui::confirm(brls::getStr("playguard/forwarders/delete_body", r.name), "playguard/common/delete"_i18n, go, stop, true);
}

void forget(const forwarders_logic::Row& row, std::function<void(bool)> done)
{
    std::string err;
    const bool ok = forwarders::forget(row.id, &err);
    if (ok) ui::notify(brls::getStr("playguard/forwarders/forgotten", row.name), ui::Toast::Done);
    else ui::error("playguard/forwarders/delete_err"_i18n, err);
    if (done) done(ok);
}

std::vector<forwarders_logic::Row> unprotected()
{
    std::vector<forwarders_logic::Row> out;
    for (const auto& r : made_rows(false))
        if (!r.pin && r.state != forwarders_logic::State::NotInstalled) out.push_back(r);
    return out;
}

void protect_all(std::function<void(bool)> done)
{
    const std::vector<forwarders_logic::Row> todo = unprotected();
    if (todo.empty()) {
        ui::info("playguard/forwarders/all_protected"_i18n);
        if (done) done(false);
        return;
    }
    u32 pin_length = 0;
    if (R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length == 0) {
        ui::info("playguard/forwarders/pin_no_pin"_i18n);
        if (done) done(false);
        return;
    }
    std::string names;
    for (const auto& r : todo) names += (names.empty() ? "" : ", ") + r.name;
    ui::confirm(brls::getStr("playguard/forwarders/protect_all_confirm", names), "playguard/forwarders/protect_all"_i18n,
                [todo, done]() {
                    install_many(todo, true, "protect_forwarder", "playguard/forwarders/protected_n", done);
                },
                [done]() {
                    if (done) done(false);
                });
}

void reinstall_all(std::function<void(bool)> done)
{
    const std::vector<forwarders_logic::Row> todo = made_rows(true);
    if (todo.empty()) {
        if (done) done(false);
        return;
    }
    ui::confirm(brls::getStr("playguard/forwarders/reinstall_all_confirm", (int)todo.size()),
                "playguard/forwarders/reinstall_all"_i18n,
                [todo, done]() { install_many(todo, false, "add_forwarder", "playguard/forwarders/reinstalled_n", done); },
                [done]() {
                    if (done) done(false);
                });
}

void set_hours(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (!m.pin || !forwarders::find(row.id, nullptr)) {
        if (done) done(false);
        return;
    }
    // The PIN first: free hours let the homebrew start without it.
    if (ui::refuse_read_only() || !pin_lock::confirm() || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    m.free_days = days;
    m.free_from = days ? from : 0;
    m.free_to = days ? to : 0;
    std::string err;
    if (!forwarders::save(m, &err)) {
        const std::string hint = hbmenu_lock::write_hint(paths::nand_dir());
        ui::error("playguard/forwarders/hours_err"_i18n, hint.empty() ? err : err + "\n\n" + hint);
        if (done) done(false);
        return;
    }
    write_rules();
    history_flow::record_event("gate_hours", "", history_label(m.name, m.id) +
                                                     (days ? " · " + hours_label(days, from, to) : std::string()));
    ui::notify(days ? brls::getStr("playguard/forwarders/hours_set", m.name, hours_label(days, from, to))
                    : brls::getStr("playguard/forwarders/hours_none", m.name),
               ui::Toast::Done);
    if (done) done(true);
}

std::string hours_label(int days, int from, int to)
{
    const std::vector<std::string> names = {
        "playguard/forwarders/day_sun"_i18n, "playguard/forwarders/day_mon"_i18n, "playguard/forwarders/day_tue"_i18n,
        "playguard/forwarders/day_wed"_i18n, "playguard/forwarders/day_thu"_i18n, "playguard/forwarders/day_fri"_i18n,
        "playguard/forwarders/day_sat"_i18n,
    };
    return forwarders_logic::hours_text(days, from, to, names);
}

std::vector<forwarders_logic::Refusals> take_log()
{
    // Each try the gate logged since: into the change history, then gone.
    const std::string path = paths::data_dir() + "/" + GATE_LOG_NAME;
    std::string text;
    if (!paths::read_file(path, text)) return {};
    std::remove(path.c_str());
    std::vector<forwarders::Made> made = forwarders::list();
    std::vector<std::pair<std::string, std::string>> refused;
    std::vector<size_t> seen_working;
    size_t at = 0;
    int n = 0;
    while (at < text.size() && n < 200) {
        size_t end = text.find('\n', at);
        if (end == std::string::npos) end = text.size();
        const char *when, *nro;
        size_t wn, nn;
        GateResult r;
        if (gate_log_parse(text.data() + at, end - at, &when, &wn, &r, &nro, &nn)) {
            const std::string path_s(nro, nn);
            std::string name = forwarders_logic::file_title(path_s);
            for (size_t i = 0; i < made.size(); i++) {
                if (made[i].nro != path_s) continue;
                name = made[i].name;
                // The gate opened it: this way back works.
                if ((r == GateResult_Pin || r == GateResult_Free) && made[i].pin && !made[i].verified)
                    seen_working.push_back(i);
            }
            const char* kind = r == GateResult_Pin ? "gate_open" : r == GateResult_Free ? "gate_free" : "gate_refused";
            std::string w(when, wn);
            if (w == "?") w.clear();
            if (r == GateResult_Refused) refused.push_back({ name, w });
            history_flow::record_seen(kind, w, r == GateResult_Error ? name + " (error)" : name);
            n++;
        }
        at = end + 1;
    }
    for (const size_t i : seen_working) {
        if (made[i].verified) continue;
        made[i].verified = true;
        std::string err;
        if (!forwarders::save(made[i], &err)) brls::Logger::warning("forwarders: not remembered ({})", err);
    }
    return forwarders_logic::repeated_refusals(refused);
}

std::vector<forwarders_logic::Refusals> at_start()
{
    // The gate as this PlayGuard carries it, and the free hours, while a
    // forwarder is protected.
    bool any = false;
    for (const forwarders::Made& m : forwarders::list()) any = any || m.pin;
    if (any) {
        gate_nro();
        write_rules();
    }
    return take_log();
}

}   // namespace forwarder_flow
