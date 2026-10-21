// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/hbmenu_lock.hpp"

#include <borealis.hpp>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

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
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/diagnostics.hpp"
#include "util/forwarders.hpp"
#include "util/paths.hpp"
#include "util/rescue_page.hpp"
#include "view/qr_view.hpp"
#include "util/pctl_ops_c.hpp"

#include "action/hbmenu_lock_shared.hpp"

using namespace brls::literals;

namespace hbmenu_lock
{

using namespace detail;

namespace detail
{

// `rel` from the SD card root ("/" on the console).
std::string on_sd(const char* rel)
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';
    return path + rel;
}

std::string config_path()
{
    return on_sd(HBL_LOCK_CONFIG_PATH);
}

std::string in_data(const char* name)
{
    return paths::data_dir() + "/" + name;
}

// What Console › Forwarders opens from the bell (at_start's `open`).
std::function<void()> s_open;

std::string read_config()
{
    std::string text;
    if (!paths::read_file(config_path(), text)) text.clear();
    return text;
}

std::string joined(const std::vector<std::string>& names)
{
    std::string out;
    for (const std::string& n : names) out += (out.empty() ? "" : ", ") + n;
    return out;
}

// What locking still needs, one line each.
std::string missing(const forwarders_logic::LockCheck& c)
{
    std::string out;
    const auto add = [&out](const std::string& line) { out += "\n• " + line; };
    if (!c.pin) add("playguard/hbmenu/need_pin"_i18n);
    if (!c.patches) add("playguard/hbmenu/need_patches"_i18n);
    if (!c.self) add("playguard/hbmenu/need_self"_i18n);
    if (c.menus.empty()) add("playguard/hbmenu/need_menu"_i18n);
    else if (c.verified.empty()) add(brls::getStr("playguard/hbmenu/need_verified", joined(c.menus)));
    return out;
}

// Tried and refused at the gate three times or more on one day: the bell.
// A later read with none leaves it, until it is put away.
void tell_refusals(const std::vector<forwarders_logic::Refusals>& refusals)
{
    if (refusals.empty()) return;
    std::string detail;
    for (const auto& r : refusals)
        detail += (detail.empty() ? "" : "\n") + brls::getStr("playguard/hbmenu/refusals_line", r.name,
                                                              cadran::kit::long_date(r.day), r.count);
    notices::set("gate_refusals", notices::Level::Info, "playguard/hbmenu/refusals"_i18n, detail,
                 s_open ? "playguard/hbmenu/open"_i18n : "", s_open);
}

}   // namespace detail

namespace
{

// The file after `op`; false (logged) when it is too large to handle.
bool apply(const std::string& text, HblLockOp op, std::string& out)
{
    std::string buf(HBL_LOCK_MAX_FILE + 2048, '\0');
    size_t n = 0;
    if (text.size() > HBL_LOCK_MAX_FILE ||
        !hbl_lock_apply(text.data(), text.size(), op, &buf[0], buf.size(), &n)) {
        brls::Logger::error("hbmenu lock: {} is too large ({} bytes)", HBL_LOCK_CONFIG_PATH, text.size());
        return false;
    }
    out.assign(buf.data(), n);
    return true;
}

// The Atmosphère version, for the sysmodule to tell an update; the file
// also says PlayGuard holds the lock (one gone from override_config.ini
// while it is there was taken away outside PlayGuard).
void write_state()
{
    SysInfo si;
    sysinfo_get(&si);
    std::string error;
    char text[32];
    const std::string path = in_data(HBL_LOCK_STATE_NAME);
    // The file is there while PlayGuard holds the lock, the version or not
    // (without one, the sysmodule lifts nothing for an update).
    if (!(si.ams_valid && hbl_lock_state_format(si.ams_major, si.ams_minor, si.ams_micro, text, sizeof(text))) &&
        !hbl_lock_state_format_unknown(text, sizeof(text)))
        return;
    if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(path, text, &error))
        brls::Logger::warning("hbmenu lock: {} not written: {}", HBL_LOCK_STATE_NAME, error);
}

bool write_config(const std::string& text, std::string* error)
{
    if (paths::ensure_dir(on_sd("atmosphere/config")) && paths::atomic_write(config_path(), text, error)) return true;
    const std::string hint = write_hint(on_sd("atmosphere/config"));
    if (error && !hint.empty()) *error += "\n\n" + hint;
    return false;
}

// The sysmodule's report, told and recorded, then removed.
void take_report(const std::function<void()>& open)
{
    const std::string path = in_data(HBL_LOCK_REPORT_NAME);
    std::string text;
    if (!paths::read_file(path, text)) return;
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());
    HblLockReport r;
    if (!hbl_lock_report_parse(text.c_str(), text.size(), &r)) {
        brls::Logger::warning("hbmenu lock: the sysmodule's report is unreadable");
        return;
    }
    brls::Logger::info("hbmenu lock: lifted at boot by the sysmodule ({}, {})",
                       r.reason == HblLockReason_Atmosphere ? "Atmosphère updated" : "rescue", r.ok ? "ok" : "failed");
    if (!r.ok) {
        notices::set("hbmenu_lifted", notices::Level::Action, "playguard/hbmenu/lift_failed"_i18n,
                     "playguard/hbmenu/lift_failed_detail"_i18n, "playguard/hbmenu/open"_i18n, open);
        return;
    }
    history_flow::record_event("hbmenu_restored", "",
                               r.reason == HblLockReason_Atmosphere ? "Atmosphère " + std::string(r.from) + " → " + r.to
                                                                    : std::string(RESCUE_REQUEST_NAME));
    const std::string detail = r.reason == HblLockReason_Atmosphere
        ? brls::getStr("playguard/hbmenu/lifted_atmosphere", r.from, r.to)
        : "playguard/hbmenu/lifted_rescue"_i18n;
    notices::set("hbmenu_lifted", notices::Level::Info, "playguard/hbmenu/lifted"_i18n, detail,
                 "playguard/hbmenu/open"_i18n, open);
}

}   // namespace

bool locked()
{
    const std::string text = read_config();
    return hbl_lock_locked(text.data(), text.size());
}

namespace
{

// Writes the lock, the checks done and confirmed.
void apply_lock(std::function<void(bool)> finish)
{
    // The PIN whatever PlayGuard's own lock says, then the change check
    // (read-only mode).
    if (!pin_lock::confirm() || R_FAILED(core_change_allowed())) return finish(false);
    const std::string before = read_config();
    std::string after, error;
    if (!apply(before, HblLock_Lock, after)) {
        ui::error("playguard/hbmenu/lock_err"_i18n, "playguard/hbmenu/too_large"_i18n);
        return finish(false);
    }
    // The file as it was, once: not one already locked.
    if (!before.empty() && !hbl_lock_locked(before.data(), before.size())) {
        std::string berr;
        if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(in_data(HBL_LOCK_BACKUP_NAME), before, &berr))
            brls::Logger::warning("hbmenu lock: the backup was not written: {}", berr);
    }
    if (!write_config(after, &error)) {
        ui::error("playguard/hbmenu/lock_err"_i18n, error);
        return finish(false);
    }
    write_state();
    notices::clear("hbmenu_lifted");
    notices::clear("hbmenu_removed");
    history_flow::record_event("lock_hbmenu");
    brls::Logger::info("hbmenu lock: locked");
    // Said by the rescue sheet's title: how to undo it without the
    // console, while it is fresh.
    brls::sync([]() { rescue_sheet(true); });
    finish(true);
}

// What it changes, what it leaves open, confirmed.
void confirm_lock(std::function<void(bool)> finish)
{
    const forwarders_logic::LockCheck c = check();
    if (!c.ready()) return setup(finish);
    ui::hold_confirm(ui::checklist_body("playguard/hbmenu/confirm_title"_i18n, check_lines(c),
                                        brls::getStr("playguard/hbmenu/escape", HBL_LOCK_CONFIG_PATH),
                                        "playguard/hbmenu/confirm_sub"_i18n),
                     "playguard/hbmenu/lock"_i18n, [finish]() { apply_lock(finish); }, [finish]() { finish(false); });
}

}   // namespace

std::vector<SetupStep> setup_steps(const forwarders_logic::LockCheck& c)
{
    return {
        { SetupStep::Pin, "playguard/hbmenu/step_pin"_i18n, c.pin },
        { SetupStep::Patches, "playguard/hbmenu/step_patches"_i18n, c.patches },
        { SetupStep::Self, "playguard/hbmenu/step_self"_i18n, c.self },
        { SetupStep::Menu, "playguard/hbmenu/step_menu"_i18n, !c.menus.empty() },
        { SetupStep::Verify, c.menus.empty() ? "playguard/hbmenu/step_verify"_i18n
                                             : brls::getStr("playguard/hbmenu/step_verify_named", joined(c.menus)),
          !c.verified.empty() },
    };
}

void run_step(SetupStep::Kind kind, std::function<void(bool)> done)
{
    const auto finish = [done](bool changed) {
        if (done) done(changed);
    };
    // Said only (nothing changes): no `done`, which would rebuild the screen
    // under the dialog.
    switch (kind) {
        case SetupStep::Pin:
            ui::notice("key", ui::Check::Warn, "playguard/hbmenu/step_pin"_i18n, "playguard/hbmenu/need_pin"_i18n);
            return;
        case SetupStep::Patches:
            ui::notice("chip", ui::Check::Warn, "playguard/hbmenu/step_patches"_i18n, "playguard/hbmenu/need_patches"_i18n);
            return;
        case SetupStep::Self: return forwarder_flow::create_self(finish);
        case SetupStep::Menu: return forwarder_flow::protect_a_menu(finish);
        case SetupStep::Verify: {
            const forwarders_logic::LockCheck c = check();
            if (c.menus.empty())
                ui::notice("home", ui::Check::Warn, "playguard/hbmenu/step_menu"_i18n, "playguard/hbmenu/need_menu"_i18n);
            else
                ui::notice("home", ui::Check::Info, brls::getStr("playguard/hbmenu/verify_title", joined(c.menus)),
                           brls::getStr("playguard/hbmenu/verify_how", joined(c.menus)));
            return;
        }
    }
    finish(false);
}

void setup(std::function<void(bool)> done)
{
    const auto finish = [done](bool changed) {
        if (done) done(changed);
    };
    const forwarders_logic::LockCheck c = check();
    if (c.ready()) return confirm_lock(finish);
    // Each step, numbered: done (a check), the next one (amber), the rest.
    const std::vector<SetupStep> steps = setup_steps(c);
    std::vector<ui::CheckLine> lines;
    int next = -1;
    for (size_t i = 0; i < steps.size(); i++) {
        if (!steps[i].done && next < 0) next = (int)i;
        const ui::Check mark = steps[i].done ? ui::Check::Ok : (int)i == next ? ui::Check::Warn : ui::Check::Info;
        lines.push_back({ mark, fmt::format("{}. {}", i + 1, steps[i].label) });
    }
    lines.push_back({ ui::Check::Info, fmt::format("{}. {}", steps.size() + 1, "playguard/hbmenu/step_lock"_i18n) });
    const SetupStep::Kind kind = steps[(size_t)next].kind;
    auto* dialog = ui::box_dialog(ui::checklist_body("playguard/hbmenu/setup_title"_i18n, lines,
                                                       "playguard/hbmenu/setup_note"_i18n,
                                                       brls::getStr("playguard/hbmenu/setup_progress", next + 1,
                                                                    (int)steps.size() + 1)));
    dialog->addButton("hints/cancel"_i18n, [finish]() { finish(false); });
    dialog->addButton("playguard/hbmenu/setup_next"_i18n, [kind, done, finish]() {
        brls::sync([kind, done, finish]() {
            run_step(kind, [kind, done, finish](bool changed) {
                // A step made (a forwarder): the list again, and the next step.
                finish(changed);
                if (changed && (kind == SetupStep::Self || kind == SetupStep::Menu))
                    brls::sync([done]() { setup(done); });
            });
        });
    });
    ui::on_cancel(dialog, [finish]() { finish(false); });
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    if (brls::View* go = dialog->getView("brls/dialog/button2")) dialog->setLastFocusedView(go);
    dialog->open();
}

void lock(std::function<void(bool)> done)
{
    const auto finish = [done](bool changed) {
        if (done) done(changed);
    };
    if (ui::refuse_read_only()) return finish(false);
    // Forwarders made here and not protected: each starts its homebrew
    // without the PIN, and one that writes to the SD card could change the
    // lock's file. Protected first, in one go, when the parent wants.
    const std::vector<forwarders_logic::Row> open = forwarder_flow::unprotected();
    if (open.empty() || !check().ready()) return confirm_lock(finish);
    std::string names;
    for (const auto& r : open) names += (names.empty() ? "" : ", ") + r.name;
    std::vector<ui::Choice> choices;
    choices.push_back({ "playguard/hbmenu/protect_then_lock"_i18n, [finish]() {
                           forwarder_flow::protect_all([finish](bool) {
                               brls::sync([finish]() { confirm_lock(finish); });
                           });
                       } });
    choices.push_back({ "playguard/hbmenu/lock_anyway"_i18n, [finish]() { confirm_lock(finish); } });
    ui::pick_actions(brls::getStr("playguard/hbmenu/unprotected", names), choices);
}

void unlock(std::function<void(bool)> done)
{
    const auto finish = [done](bool changed) {
        if (done) done(changed);
    };
    if (ui::refuse_read_only()) return finish(false);
    if (!pin_lock::confirm() || R_FAILED(core_change_allowed())) return finish(false);
    std::string after, error;
    if (!apply(read_config(), HblLock_Unlock, after)) {
        ui::error("playguard/hbmenu/unlock_err"_i18n, "playguard/hbmenu/too_large"_i18n);
        return finish(false);
    }
    if (!write_config(after, &error)) {
        ui::error("playguard/hbmenu/unlock_err"_i18n, error);
        return finish(false);
    }
    std::remove(in_data(HBL_LOCK_STATE_NAME).c_str());
    notices::clear("hbmenu_route");
    history_flow::record_event("unlock_hbmenu");
    brls::Logger::info("hbmenu lock: unlocked");
    ui::notify("playguard/hbmenu/unlocked_done"_i18n, ui::Toast::Done);
    finish(true);
}

void at_start(std::function<void()> open)
{
    s_open = open;
    diagnostic::set_homebrew_lock(&report);
    take_report(open);
    tell_refusals(forwarder_flow::at_start());
    const bool is_locked = locked();
    // PlayGuard held the lock and the file no longer says so: taken away
    // outside PlayGuard (a computer, a file manager). Told, recorded once;
    // never put back by itself (deleting the file is the way back from a
    // computer).
    const std::string state = in_data(HBL_LOCK_STATE_NAME);
    if (!is_locked && paths::exists_or_pending(state)) {
        std::remove(state.c_str());
        std::remove((state + ".tmp").c_str());
        history_flow::record_seen("hbmenu_lock_removed", "");
        notices::set("hbmenu_removed", notices::Level::Info, "playguard/hbmenu/removed"_i18n,
                     "playguard/hbmenu/removed_detail"_i18n, "playguard/hbmenu/open"_i18n, open);
    }
    // Nothing made here and nothing locked: the HOME menu is not read.
    if (!is_locked && forwarders::list().empty()) {
        notices::clear("hbmenu_route");
        notices::clear("forwarder_gone");
        return;
    }
    const std::vector<forwarders_logic::Row> rows = forwarder_flow::rows();

    // Made here, no longer on the HOME menu (deleted in System Settings):
    // recorded once, told while it lasts.
    std::vector<std::string> gone;
    for (const auto& r : rows) {
        if (r.state != forwarders_logic::State::NotInstalled) continue;
        gone.push_back(r.name);
        if (r.gone_seen) continue;
        forwarders::Made m;
        if (forwarders::find(r.id, &m)) {
            m.gone = true;
            std::string err;
            if (!forwarders::save(m, &err)) brls::Logger::warning("forwarders: not remembered ({})", err);
        }
        history_flow::record_event("forwarder_gone", "", r.name + " (" + forwarders::hex_id(r.id) + ")");
    }
    // Paused after refused PINs, until a time still to come.
    std::string paused;
    for (const auto& r : rows)
        if (r.paused_until)
            paused += (paused.empty() ? "" : "\n") +
                      brls::getStr("playguard/forwarders/paused_line", r.name, forwarder_flow::until_label(r.paused_until));
    if (paused.empty()) notices::clear("gate_paused");
    else
        notices::set("gate_paused", notices::Level::Info, "playguard/forwarders/paused_title"_i18n, paused,
                     "playguard/hbmenu/open"_i18n, open);
    if (gone.empty()) notices::clear("forwarder_gone");
    else
        notices::set("forwarder_gone", notices::Level::Info, "playguard/hbmenu/gone"_i18n,
                     brls::getStr("playguard/hbmenu/gone_detail", joined(gone)), "playguard/hbmenu/open"_i18n, open);

    if (!is_locked) {
        notices::clear("hbmenu_route");
        return;
    }
    u32 pin_length = 0;
    const bool pin = R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length > 0;
    const forwarders_logic::LockCheck c =
        forwarders_logic::lock_check(rows, pin, forwarders_logic::patches_state(ui::patch_report()));
    if (c.routes()) {
        notices::clear("hbmenu_route");
        return;
    }
    notices::set("hbmenu_route", notices::Level::Action, "playguard/hbmenu/route"_i18n,
                 "playguard/hbmenu/route_detail"_i18n + missing(c), "playguard/hbmenu/open"_i18n, open);
}

}   // namespace hbmenu_lock
