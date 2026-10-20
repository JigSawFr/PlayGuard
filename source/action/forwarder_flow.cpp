// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_flow.hpp"

#include <algorithm>
#include <ctime>
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
#include "util/text.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
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
    // Its icon from the album, when it has one still there.
    if (!m.icon.empty()) {
        std::string shot;
        const std::string p = nro::sd_path(m.icon);
        std::vector<uint8_t> icon;
        if (!p.empty() && paths::read_file(on_sd(p), shot))
            icon = forwarder_icon::from_screenshot(std::vector<uint8_t>(shot.begin(), shot.end()));
        if (nro::icon_usable(icon)) info.icon = icon;
        else brls::Logger::warning("forwarders: {} unreadable, the .nro's icon instead", m.icon);
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

// The free hours of every protected forwarder, and the hours it never
// opens, for the gate (GATE_RULES_NAME, GATE_CLOSED_NAME).
void write_rules()
{
    std::string free_text, closed_text;
    for (const forwarders::Made& m : forwarders::list()) {
        if (!m.pin) continue;
        char line[0x400];
        const GateHours f = { (uint8_t)m.free_days, (uint16_t)m.free_from, (uint16_t)m.free_to };
        if (m.free_days && gate_rule_format(&f, m.nro.c_str(), line, sizeof(line))) free_text += line;
        const GateHours c = { (uint8_t)m.closed_days, (uint16_t)m.closed_from, (uint16_t)m.closed_to };
        if (m.closed_days && gate_rule_format(&c, m.nro.c_str(), line, sizeof(line))) closed_text += line;
    }
    for (const auto& [name, text] : { std::pair<const char*, std::string>{ GATE_RULES_NAME, free_text },
                                      { GATE_CLOSED_NAME, closed_text } }) {
        const std::string path = paths::data_dir() + "/" + name;
        if (text.empty()) {
            std::remove(path.c_str());
            continue;
        }
        std::string err;
        if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(path, text, &err))
            brls::Logger::warning("forwarders: {} not written ({})", name, err);
    }
}

std::string pause_path()
{
    return paths::data_dir() + "/" + GATE_PAUSE_NAME;
}

std::string read_pause()
{
    std::string text;
    if (!paths::read_file(pause_path(), text)) text.clear();
    return text;
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
    std::vector<forwarders_logic::Row> out = forwarders_logic::merge(forwarders::list(), installed, nro_exists, self_nro());
    // Paused after refusals (the gate's file), until a time still to come.
    const std::string pause = read_pause();
    const uint64_t now = (uint64_t)std::time(nullptr);
    for (auto& r : out) {
        uint64_t until = 0;
        if (r.pin && gate_pause_find(pause.data(), pause.size(), r.nro.c_str(), nullptr, &until) && until > now)
            r.paused_until = until;
    }
    return out;
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

namespace
{
// The question before a forwarder is made, with what the HOME menu will
// show: its icon, its name, its publisher.
void confirm_preview(const std::vector<uint8_t>& icon, const std::string& name, const std::string& author,
                     const std::string& body, std::function<void()> yes, std::function<void()> no)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(28, 36, 22, 36);
    auto* tile = new brls::Box(brls::Axis::ROW);
    tile->setAlignItems(brls::AlignItems::CENTER);
    tile->setBackgroundColor(theme["cadran/surf2"]);
    tile->setCornerRadius(18);
    tile->setPadding(14, 18, 14, 14);
    if (!icon.empty()) {
        auto* image = new brls::Image();
        image->setWidth(96);
        image->setHeight(96);
        image->setCornerRadius(14);
        image->setScalingType(brls::ImageScalingType::FIT);
        image->setImageFromMem(const_cast<unsigned char*>(icon.data()), (int)icon.size());
        image->setMarginRight(18);
        image->setShrink(0);
        tile->addView(image);
    }
    auto* words = new brls::Box(brls::Axis::COLUMN);
    words->setGrow(1.0f);
    words->setWidth(0);
    words->addView(kit::label(name, 22, "cadran/text"));
    if (!author.empty()) words->addView(kit::label(author, 16, "cadran/text3"));
    brls::Label* as = kit::label("playguard/forwarders/preview_note"_i18n, 14, "cadran/text3");
    as->setMarginTop(6);
    words->addView(as);
    tile->addView(words);
    box->addView(tile);
    brls::Label* text = kit::label(body, 18, "cadran/text2");
    text->setMarginTop(16);
    box->addView(text);
    auto* dialog = ui::box_dialog(box);
    dialog->addButton("hints/cancel"_i18n, [no]() { if (no) no(); });
    dialog->addButton("playguard/forwarders/add"_i18n, [yes]() { if (yes) yes(); });
    ui::on_cancel(dialog, no);
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    dialog->open();
}
}   // namespace

void create(const std::string& sd_path, std::function<void(bool)> done, bool pin)
{
    nro::Info info;
    std::string error;
    const std::string sdmc = nro::sd_path(sd_path).empty() ? nro::sdmc_path(sd_path) : sd_path;
    if (!assets_of(sdmc, false, info, error)) {
        ui::error("playguard/forwarders/add_err"_i18n, error);
        if (done) done(false);
        return;
    }
    // Protected: a PIN on the console to ask for.
    u32 pin_length = 0;
    if (pin && R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length == 0) {
        ui::info("playguard/forwarders/pin_no_pin"_i18n);
        if (done) done(false);
        return;
    }
    const std::string shown = nro::sd_path(sdmc);
    const std::string initial = forwarders_logic::default_name(info.name, shown);
    const std::string author = forwarders::clean_text(info.author, forwarders::MAX_AUTHOR);
    const std::vector<uint8_t> icon = info.icon;
    ui::prompt_text("playguard/forwarders/name"_i18n, initial, (int)forwarders::MAX_NAME,
                    [sdmc, shown, author, icon, pin, done](std::string typed) {
        const std::string name = forwarders::clean_text(typed, forwarders::MAX_NAME);
        if (name.empty()) {
            if (done) done(false);
            return;
        }
        std::string body = brls::getStr("playguard/forwarders/confirm_add", name, shown);
        if (pin) body += "\n\n" + brls::getStr("playguard/forwarders/pin_confirm", name);
        brls::sync([icon, name, author, body, sdmc, pin, done]() {
            confirm_preview(icon, name, author, body,
                            [sdmc, name, author, pin, done]() {
                                forwarders::Made m;
                                m.nro = sdmc;
                                m.argv = argv_of(sdmc);
                                m.name = name;
                                m.author = author;
                                m.pin = pin;
                                const bool ok = pin ? install(m, false, "protect_forwarder",
                                                              brls::getStr("playguard/forwarders/pin_added", name))
                                                    : install(m, false);
                                if (done) done(ok);
                            },
                            [done]() {
                                if (done) done(false);
                            });
        });
    });
}

std::string menu_nro()
{
    std::string sphaira, hbmenu;
    for (const std::string& p : homebrew()) {
        const std::string sdmc = nro::sdmc_path(p);
        const std::string name = text::lower_ascii(forwarders_logic::file_title(sdmc));
        if (name == "sphaira" && sphaira.empty()) sphaira = p;
        if (name == "hbmenu" && hbmenu.empty()) hbmenu = p;
    }
    if (hbmenu.empty() && paths::file_exists(on_sd("/hbmenu.nro"))) hbmenu = "/hbmenu.nro";
    return sphaira.empty() ? hbmenu : sphaira;
}

void protect_a_menu(std::function<void(bool)> done)
{
    // One made here already: its PIN.
    for (const auto& r : rows())
        if (forwarders_logic::is_menu(r.nro) && !r.pin && r.state == forwarders_logic::State::Ready) return set_pin(r, true, done);
    const std::string path = menu_nro();
    if (path.empty()) {
        ui::notice("home", ui::Check::Warn, "playguard/hbmenu/step_menu"_i18n, "playguard/forwarders/no_menu"_i18n);
        if (done) done(false);
        return;
    }
    create(path, done, true);
}

void put_back(const forwarders::Made& m, bool reinstall, std::function<void(bool)> done)
{
    bool ok;
    if (reinstall) {
        ok = install(m, m.nro == self_nro(), m.pin ? "protect_forwarder" : "add_forwarder",
                     brls::getStr("playguard/forwarders/put_back", m.name));
    } else {
        std::string err;
        ok = forwarders::save(m, &err);
        write_rules();
        if (ok) ui::notify(brls::getStr("playguard/forwarders/put_back", m.name), ui::Toast::Done);
        else ui::error("playguard/forwarders/delete_err"_i18n, err);
    }
    if (done) done(ok);
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

namespace
{
// Free hours (`closed` false) or the hours it never opens: saved, the
// gate's files written, in the change history.
void set_span(const forwarders_logic::Row& row, bool closed, int days, int from, int to, std::function<void(bool)> done,
              bool ask)
{
    forwarders::Made m = made_of(row);
    if (!m.pin || !forwarders::find(row.id, nullptr)) {
        if (done) done(false);
        return;
    }
    // The PIN first: these hours change when it starts (not to put them back
    // as they were a moment ago: ask false).
    if (ui::refuse_read_only() || (ask && !pin_lock::confirm()) || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    int& d = closed ? m.closed_days : m.free_days;
    int& f = closed ? m.closed_from : m.free_from;
    int& t = closed ? m.closed_to : m.free_to;
    d = days;
    f = days ? from : 0;
    t = days ? to : 0;
    std::string err;
    if (!forwarders::save(m, &err)) {
        const std::string hint = hbmenu_lock::write_hint(paths::nand_dir());
        ui::error("playguard/forwarders/hours_err"_i18n, hint.empty() ? err : err + "\n\n" + hint);
        if (done) done(false);
        return;
    }
    write_rules();
    history_flow::record_event(closed ? "gate_closed_hours" : "gate_hours", "",
                               history_label(m.name, m.id) + (days ? " · " + hours_label(days, from, to) : std::string()));
    if (closed)
        ui::notify(days ? brls::getStr("playguard/forwarders/closed_set", m.name, hours_label(days, from, to))
                        : brls::getStr("playguard/forwarders/closed_none", m.name),
                   ui::Toast::Done);
    else
        ui::notify(days ? brls::getStr("playguard/forwarders/hours_set", m.name, hours_label(days, from, to))
                        : brls::getStr("playguard/forwarders/hours_none", m.name),
                   ui::Toast::Done);
    if (done) done(true);
}
}   // namespace

void set_hours(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done, bool ask)
{
    set_span(row, false, days, from, to, std::move(done), ask);
}

void set_closed(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done, bool ask)
{
    set_span(row, true, days, from, to, std::move(done), ask);
}

GatePauseRule pause_rule()
{
    const std::string text = read_pause();
    return gate_pause_rule(text.data(), text.size());
}

void set_pause_rule(unsigned tries, unsigned minutes, std::function<void(bool)> done)
{
    if (ui::refuse_read_only() || !pin_lock::confirm() || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    const std::string text = read_pause();
    const GatePauseRule rule = { tries, tries ? minutes : 0 };
    std::string out(GATE_PAUSE_MAX, '\0');
    const size_t n = gate_pause_edit(text.data(), text.size(), &rule, nullptr, &out[0], out.size());
    std::string err;
    if (n == 0 || !paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(pause_path(), out.substr(0, n), &err)) {
        const std::string hint = hbmenu_lock::write_hint(paths::data_dir());
        ui::error("playguard/forwarders/pause_err"_i18n, hint.empty() ? err : err + "\n\n" + hint);
        if (done) done(false);
        return;
    }
    history_flow::record_event("gate_pause", "", pause_label(rule.tries, rule.minutes));
    ui::notify(brls::getStr("playguard/forwarders/pause_set", pause_label(rule.tries, rule.minutes)), ui::Toast::Done);
    if (done) done(true);
}

std::string until_label(uint64_t posix)
{
    // The time alone today, else the day too ("2026-10-21 07:00").
    const std::string full = ui::time_text(posix);
    const std::string today = ui::today_date();
    if (full.size() >= 16 && full.compare(0, 10, today) == 0) return full.substr(11, 5);
    return full.size() >= 16 ? full.substr(0, 16) : full;
}

std::string pause_label(unsigned tries, unsigned minutes)
{
    if (!tries) return "playguard/forwarders/pause_off"_i18n;
    return brls::getStr("playguard/forwarders/pause_rule", (int)tries, ui::fmt_played((uint16_t)minutes));
}

void lift_pause(const forwarders_logic::Row& row, std::function<void(bool)> done)
{
    // The PIN first: lifting it lets the PIN be tried again at once.
    if (ui::refuse_read_only() || !pin_lock::confirm()) {
        if (done) done(false);
        return;
    }
    const std::string text = read_pause();
    std::string out(GATE_PAUSE_MAX, '\0');
    const size_t n = gate_pause_edit(text.data(), text.size(), nullptr, row.nro.c_str(), &out[0], out.size());
    std::string err;
    if ((n == 0 && !text.empty() && std::remove(pause_path().c_str()) != 0) ||
        (n > 0 && !paths::atomic_write(pause_path(), out.substr(0, n), &err))) {
        ui::error("playguard/forwarders/pause_err"_i18n, err);
        if (done) done(false);
        return;
    }
    history_flow::record_event("gate_pause_lifted", "", history_label(row.name, row.id));
    ui::notify(brls::getStr("playguard/forwarders/pause_lifted", row.name), ui::Toast::Done);
    if (done) done(true);
}

void set_icon(const forwarders_logic::Row& row, const std::string& shot, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (m.nro.empty()) {
        if (done) done(false);
        return;
    }
    m.icon = shot;
    const bool ok = install(m, row.self || m.nro == self_nro(), "add_forwarder",
                            brls::getStr("playguard/forwarders/icon_set", row.name));
    if (done) done(ok);
}

std::vector<forwarders::Made> missing_from(const std::vector<forwarders::Made>& made)
{
    std::vector<forwarders::Made> out;
    const std::vector<forwarders_logic::Row> now = rows();
    for (const forwarders::Made& m : made) {
        bool there = false;
        for (const auto& r : now) there = there || (r.id == m.id && r.state != forwarders_logic::State::NotInstalled);
        if (!there && !m.nro.empty()) out.push_back(m);
    }
    return out;
}

void put_back_all(const std::vector<forwarders::Made>& made, std::function<void(bool)> done)
{
    if (ui::refuse_read_only()) {
        if (done) done(false);
        return;
    }
    int ok = 0;
    for (const forwarders::Made& m : made)
        if (install(m, m.nro == self_nro(), m.pin ? "protect_forwarder" : "add_forwarder", "", true)) ok++;
    if (ok > 0) ui::notify(brls::getStr("playguard/forwarders/reinstalled_n", ok), ui::Toast::Done);
    if (done) done(ok > 0);
}

std::vector<std::string> album_shots()
{
    std::vector<std::string> found;
    // The album's folders: year, month, day; a JPEG per screenshot.
    std::function<void(const std::string&, int)> walk = [&found, &walk](const std::string& dir, int depth) {
        for (const std::string& name : paths::list_dirs(dir)) {
            if (depth < 3) walk(dir + "/" + name, depth + 1);
        }
        if (depth == 3)
            for (const std::string& f : paths::list_files(dir, ".jpg")) found.push_back(dir + "/" + f);
    };
    walk(on_sd("/Nintendo/Album"), 0);
    // As "sdmc:/…", the newest first.
    std::vector<std::string> out;
    for (const std::string& p : forwarders_logic::newest_shots(found, 30)) {
        const std::string root = paths::sd_root() == "/" ? "" : paths::sd_root();
        out.push_back(nro::sdmc_path(root.empty() ? p : p.substr(root.size())));
    }
    return out;
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
    struct Tries
    {
        const char*              kind;
        std::string              when, detail;
        std::vector<std::string> times;
    };
    std::vector<Tries> tries;
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
            const char* kind = r == GateResult_Pin      ? "gate_open"
                               : r == GateResult_Free   ? "gate_free"
                               : r == GateResult_Paused ? "gate_paused"
                               : r == GateResult_Closed ? "gate_closed"
                                                        : "gate_refused";
            std::string w(when, wn);
            if (w == "?") w.clear();
            if (r == GateResult_Refused) refused.push_back({ name, w });
            const std::string detail = r == GateResult_Error ? name + " (error)" : name;
            // One entry per kind, forwarder and day: the times in it.
            bool joined = false;
            for (Tries& t : tries)
                if (t.kind == kind && t.detail == detail && t.when.substr(0, 10) == w.substr(0, 10) && !w.empty()) {
                    t.times.push_back(w.size() > 11 ? w.substr(11) : w);
                    joined = true;
                }
            if (!joined) tries.push_back({ kind, w, detail, { w.size() > 11 ? w.substr(11) : w } });
            n++;
        }
        at = end + 1;
    }
    for (const Tries& t : tries) {
        if (t.times.size() < 2) {
            history_flow::record_seen(t.kind, t.when, t.detail);
            continue;
        }
        std::string times;
        for (const std::string& h : t.times) times += (times.empty() ? "" : ", ") + h;
        history_flow::record_seen(t.kind, t.when,
                                  brls::getStr("playguard/forwarders/tries_grouped", t.detail, (int)t.times.size(), times));
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
