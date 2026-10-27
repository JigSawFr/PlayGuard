// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_flow.hpp"

#include <algorithm>
#include <ctime>
#include <borealis.hpp>

#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_modal.hpp>
#include <borealis/cadran/util/text.hpp>

#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/sigpatch_flow.hpp"
#include "action/forwarders_logic.hpp"
#include "app.hpp"
#include "core/forwarder.h"
#include "core/pctl_ops.h"
#include "ui/ui.hpp"
#include "core/gate.h"
#include "util/forwarder_gate.hpp"
#include "util/forwarder_icon.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"
#include "util/patches.hpp"
#include "util/paths.hpp"
#include "util/sha256.hpp"
#ifndef __SWITCH__
#include "sim/sim_homebrew.hpp"
#endif

#include "action/forwarder_flow_shared.hpp"

using namespace brls::literals;

namespace forwarder_flow
{

using namespace detail;

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

std::string loader_id()
{
#ifdef __SWITCH__
    static std::string id;
    static bool read = false;
    if (!read) {
        read = true;
        std::string main, npdm;
        if (paths::read_file(BRLS_ASSET("forwarder/main"), main) && paths::read_file(BRLS_ASSET("forwarder/main.npdm"), npdm)) {
            Sha256 hash;
            hash.update(main.data(), main.size());
            hash.update(npdm.data(), npdm.size());
            // How forwarder_pack patches the NPDM, which the files above do
            // not show: raised when that changes, so the forwarders made
            // before are offered to be made again. 2: ForceDebug in the
            // ACID too, without which Atmosphère 1.8.0+ refused to start them.
            static const char PACK_REVISION[] = "pack 2";
            hash.update(PACK_REVISION, sizeof(PACK_REVISION) - 1);
            id = hash.hex().substr(0, 16);
        }
    }
    return id;
#else
    return "";
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
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(28, 36, 22, 36);
    auto* tile = new brls::Box(brls::Axis::ROW);
    tile->setAlignItems(brls::AlignItems::CENTER);
    tile->setThemeColor("backgroundColor", "cadran/surf2");
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

std::string patches_problem()
{
    const patches::Report& report = ui::patch_report();
    switch (forwarders_logic::patches_state(report)) {
        case forwarders_logic::Patches::Ready:
            // sigpatch files beside sys-patch: a cause GBAtemp gives of
            // forwarders that do not start (patches::mixed).
            return patches::mixed(report) ? brls::getStr("playguard/forwarders/patches_mixed", text::join(report.files, ", "))
                                          : "";
        case forwarders_logic::Patches::Unknown: return "playguard/forwarders/patches_unknown"_i18n;
        case forwarders_logic::Patches::Incomplete:
            return brls::getStr("playguard/forwarders/patches_incomplete",
                                report.log_version.empty() ? std::string("?") : report.log_version,
                                forwarders_logic::patch_gaps(report));
        case forwarders_logic::Patches::Missing: break;
    }
    return "playguard/forwarders/patches_missing"_i18n;
}

void create_checked(const std::string& sd_path, std::function<void(bool)> done, bool pin);
void create_self_checked(std::function<void(bool)> done);

namespace
{
// Before a forwarder is made: said when it would not start (its patches),
// with the choice to make it anyway (it starts once they are in place).
// `go` makes it; `done(false)` when the user stops there.
void patches_first(std::function<void()> go, std::function<void(bool)> done)
{
    const std::string problem = patches_problem();
    if (problem.empty()) return go();
    std::vector<ui::Choice> choices;
    if (sigpatch_flow::offered())
        // Set aside first: the forwarder is made after the restart, not now.
        choices.emplace_back("playguard/sigpatch/set_aside"_i18n, [done]() {
            sigpatch_flow::set_aside([done](bool) {
                if (done) done(false);
            });
        }, "folder");
    choices.emplace_back("playguard/forwarders/patches_anyway"_i18n, go, "plus");
    choices.emplace_back("hints/cancel"_i18n, [done]() {
        if (done) done(false);
    });
    ui::pick_actions("playguard/forwarders/patches_title"_i18n, choices, problem);
}
}   // namespace

void create(const std::string& sd_path, std::function<void(bool)> done, bool pin)
{
    patches_first([sd_path, done, pin]() { create_checked(sd_path, done, pin); }, done);
}

void create_checked(const std::string& sd_path, std::function<void(bool)> done, bool pin)
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
    patches_first([done]() { create_self_checked(done); }, done);
}

void create_self_checked(std::function<void(bool)> done)
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

std::string options_label(const Options& o)
{
    std::string out;
    const auto add = [&out](const std::string& s) { out += (out.empty() ? "" : " · ") + s; };
    if (o.ask_user) add("playguard/forwarders/option_ask_user"_i18n);
    if (!o.screenshots) add("playguard/forwarders/option_no_screenshots"_i18n);
    if (!o.video) add("playguard/forwarders/option_no_video"_i18n);
    return out;
}

void set_options(const forwarders_logic::Row& row, const Options& o, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (m.nro.empty() || ui::refuse_read_only() || !pin_lock::confirm()) {
        if (done) done(false);
        return;
    }
    if (m.name.empty()) m.name = row.name;
    m.ask_user = o.ask_user;
    m.screenshots = o.screenshots;
    m.video = o.video;
    const bool ok = install(m, row.self || m.nro == self_nro(), nullptr,
                            brls::getStr("playguard/forwarders/options_set", m.name));
    if (ok) {
        const std::string what = options_label(o);
        history_flow::record_event("forwarder_options", "",
                                   history_label(m.name, m.id) + " · " +
                                       (what.empty() ? "playguard/forwarders/options_default"_i18n : what));
    }
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
        // Protected: the PIN first, whatever "Protect PlayGuard with the
        // PIN" says, as lifting its protection asks (set_pin).
        if (r.pin && !pin_lock::confirm()) {
            ui::refuse(pin_lock::refusal_text());
            if (done) done(false);
            return;
        }
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

std::vector<forwarders_logic::Row> to_refresh()
{
    std::vector<forwarders_logic::Row> out;
    const std::string current = loader_id();
    if (current.empty()) return out;
    const Gate gate = gate_nro();
    for (const auto& r : made_rows(true)) {
        forwarders::Made m;
        if (r.state != forwarders_logic::State::NotInstalled && r.state != forwarders_logic::State::Elsewhere &&
            forwarders::find(r.id, &m) &&
            (forwarders_logic::stale_loader(m.loader, current) ||
             forwarders_logic::stale_gate(m.pin, m.gate, gate.path, gate.pinned)))
            out.push_back(r);
    }
    return out;
}

void refresh(std::function<void(bool)> done)
{
    const std::vector<forwarders_logic::Row> todo = to_refresh();
    if (todo.empty()) {
        if (done) done(false);
        return;
    }
    std::string names;
    for (const auto& r : todo) names += (names.empty() ? "" : ", ") + r.name;
    patches_first([todo, names, done]() {
        ui::confirm(brls::getStr("playguard/forwarders/refresh_confirm", names), "playguard/forwarders/remake"_i18n,
                    [todo, done]() { install_many(todo, false, "add_forwarder", "playguard/forwarders/reinstalled_n", done); },
                    [done]() {
                        if (done) done(false);
                    });
    }, done);
}

void reinstall_all(std::function<void(bool)> done)
{
    const std::vector<forwarders_logic::Row> todo = made_rows(true);
    if (todo.empty()) {
        if (done) done(false);
        return;
    }
    patches_first([todo, done]() {
        ui::confirm(brls::getStr("playguard/forwarders/reinstall_all_confirm", (int)todo.size()),
                    "playguard/forwarders/reinstall_all"_i18n,
                    [todo, done]() { install_many(todo, false, "add_forwarder", "playguard/forwarders/reinstalled_n", done); },
                    [done]() {
                        if (done) done(false);
                    });
    }, done);
}

void set_icon(const forwarders_logic::Row& row, const std::string& shot, int x, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (m.nro.empty()) {
        if (done) done(false);
        return;
    }
    m.icon = shot;
    m.icon_x = shot.empty() ? forwarders::ICON_X_MIDDLE : std::clamp(x, 0, 100);
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
    std::vector<Job> jobs;
    for (const forwarders::Made& m : made)
        jobs.push_back({ m, m.nro == self_nro(), m.pin ? "protect_forwarder" : "add_forwarder" });
    install_batch(std::move(jobs), [done](int ok, int) {
        if (ok > 0) ui::notify(brls::getStr("playguard/forwarders/reinstalled_n", ok), ui::Toast::Done);
        if (done) done(ok > 0);
    });
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

std::vector<uint8_t> shot_bytes(const std::string& shot)
{
    std::string bytes;
    const std::string p = nro::sd_path(shot);
    if (p.empty() || !paths::read_file(on_sd(p), bytes)) return {};
    return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

std::vector<uint8_t> own_icon(const forwarders_logic::Row& row)
{
    nro::Info info;
    std::string error;
    if (!assets_of(row.nro, row.self, info, error)) return {};
    return info.icon;
}

}   // namespace forwarder_flow
