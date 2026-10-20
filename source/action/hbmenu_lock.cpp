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
#include "view/qr_view.hpp"
#include "util/pctl_ops_c.hpp"

using namespace brls::literals;

namespace hbmenu_lock
{

namespace
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

// uLaunch, which replaces the HOME menu and starts homebrew its own way:
// the lock does not reach it.
bool ulaunch_present()
{
    struct stat st;
    return stat(on_sd("atmosphere/contents/0100000000001000/exefs.nsp").c_str(), &st) == 0;
}

std::string read_config()
{
    std::string text;
    if (!paths::read_file(config_path(), text)) text.clear();
    return text;
}

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

void tell_refusals(const std::vector<forwarders_logic::Refusals>& refusals);

// The checks, read now (with `take_log`, the gate's log first: a menu it
// opened is a way back seen working; UI thread only then).
forwarders_logic::LockCheck check(bool take_log = true)
{
    if (take_log) tell_refusals(forwarder_flow::take_log());
    u32 pin_length = 0;
    const bool pin = R_SUCCEEDED(pctl_lock_state(&pin_length, nullptr)) && pin_length > 0;
    return forwarders_logic::lock_check(forwarder_flow::rows(), pin,
                                        forwarders_logic::patches_state(ui::patch_report()));
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

// Locking on emuMMC, or on a sysMMC with an emuMMC set up: the lock is on
// the SD card, both systems read it; the forwarders are each system's.
bool other_system()
{
    SysInfo si;
    sysinfo_get(&si);
    return !ui::other_storage_note(si).empty();
}

// The checks as a dialog's lines (the lock's confirmation, the check).
std::vector<ui::CheckLine> check_lines(const forwarders_logic::LockCheck& c)
{
    using ui::Check;
    std::vector<ui::CheckLine> out;
    out.push_back({ Check::Header, "playguard/hbmenu/sec_ready"_i18n });
    out.push_back({ c.pin ? Check::Ok : Check::Bad, c.pin ? "playguard/hbmenu/line_pin"_i18n : "playguard/hbmenu/need_pin"_i18n });
    out.push_back({ c.patches ? Check::Ok : Check::Bad,
                    c.patches ? "playguard/hbmenu/line_patches"_i18n : "playguard/hbmenu/need_patches"_i18n });
    out.push_back({ c.self ? Check::Ok : Check::Bad, c.self ? "playguard/hbmenu/line_self"_i18n : "playguard/hbmenu/need_self"_i18n });
    if (!c.verified.empty())
        out.push_back({ Check::Ok, brls::getStr("playguard/hbmenu/line_menu", joined(c.verified)) });
    else if (!c.menus.empty())
        out.push_back({ Check::Bad, brls::getStr("playguard/hbmenu/need_verified", joined(c.menus)) });
    else
        out.push_back({ Check::Bad, "playguard/hbmenu/need_menu"_i18n });
    out.push_back({ Check::Header, "playguard/hbmenu/sec_open"_i18n });
    if (c.open_doors.empty()) out.push_back({ Check::Ok, "playguard/hbmenu/line_no_doors"_i18n });
    else out.push_back({ Check::Warn, brls::getStr("playguard/hbmenu/open_doors", joined(c.open_doors)) });
    if (!c.sd_writers.empty()) out.push_back({ Check::Warn, brls::getStr("playguard/hbmenu/sd_writers", joined(c.sd_writers)) });
    if (ulaunch_present()) out.push_back({ Check::Warn, "playguard/hbmenu/ulaunch"_i18n });
    if (other_system()) out.push_back({ Check::Warn, "playguard/hbmenu/other_system"_i18n });
    return out;
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
// A step of the rescue sheet: its number in a bubble, its title, its text.
brls::Box* rescue_step(const char* number, const std::string& title, const std::string& text)
{
    namespace kit = cadran::kit;
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::FLEX_START);
    brls::Box* n = kit::key(number, 26);
    n->setMarginRight(14);
    n->setMarginTop(1);
    row->addView(n);
    auto* words = new brls::Box(brls::Axis::COLUMN);
    words->setGrow(1.0f);
    words->setWidth(0);
    words->addView(kit::label(title, 18, "cadran/text"));
    brls::Label* detail = kit::label(text, 15.5f, "cadran/text2");
    detail->setMarginTop(3);
    words->addView(detail);
    row->addView(words);
    return row;
}

// The rescue sheet; `locked`: just locked, said in its title.
void rescue_sheet(bool locked)
{
    namespace kit = cadran::kit;
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setAlignItems(brls::AlignItems::STRETCH);
    body->setPadding(30, 40, 22, 40);
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    brls::Box* badge = locked ? cadran::dialog_badge("check", "cadran/teal", "cadran/teal_soft")
                              : cadran::dialog_badge("shield", "cadran/teal", "cadran/teal_soft");
    badge->setMarginRight(16);
    head->addView(badge);
    auto* titles = new brls::Box(brls::Axis::COLUMN);
    titles->setGrow(1.0f);
    titles->setWidth(0);
    titles->addView(cadran::dialog_title(locked ? "playguard/hbmenu/locked_done"_i18n : "playguard/hbmenu/rescue"_i18n));
    brls::Label* sub = kit::label("playguard/hbmenu/rescue_sub"_i18n, 16, "cadran/text2");
    sub->setMarginTop(4);
    titles->addView(sub);
    head->addView(titles);
    body->addView(head);

    // The ways, numbered, beside the help's QR code.
    auto* main = new brls::Box(brls::Axis::ROW);
    main->setAlignItems(brls::AlignItems::FLEX_START);
    main->setMarginTop(20);
    auto* steps = new brls::Box(brls::Axis::COLUMN);
    steps->setGrow(1.0f);
    steps->setWidth(0);
    steps->setBackgroundColor(brls::Application::getTheme()["cadran/surf2"]);
    steps->setCornerRadius(14);
    steps->setPadding(16, 18, 16, 18);
    steps->addView(rescue_step("1", "playguard/hbmenu/rescue_pc_title"_i18n,
                               brls::getStr("playguard/hbmenu/rescue_pc", HBL_LOCK_CONFIG_PATH)));
    brls::Box* second = rescue_step("2", "playguard/hbmenu/rescue_mod_title"_i18n,
                                    rescue::installed() ? brls::getStr("playguard/hbmenu/rescue_mod", RESCUE_REQUEST_NAME)
                                                        : "playguard/hbmenu/rescue_mod_none"_i18n);
    second->setMarginTop(14);
    steps->addView(second);
    main->addView(steps);
    auto* side = new brls::Box(brls::Axis::COLUMN);
    side->setAlignItems(brls::AlignItems::CENTER);
    side->setMarginLeft(20);
    side->setShrink(0);
    auto* qr = new QrView(std::string(app::repo_url()) + "/blob/main/docs/troubleshooting.md#homebrew-menu-locked", 168);
    side->addView(qr);
    brls::Label* caption = kit::label("playguard/hbmenu/rescue_qr"_i18n, 13.5f, "cadran/text3");
    caption->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    caption->setMaxWidth(168);
    caption->setMarginTop(8);
    side->addView(caption);
    main->addView(side);
    body->addView(main);

    auto* dialog = ui::box_dialog(body);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    dialog->open();
}
}   // namespace

void show_rescue()
{
    rescue_sheet(false);
}

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

namespace
{
// What the check reads, once for both the dialog and the report.
struct State
{
    bool file = false, lock_block = false, defaults = false, album = true, any_app = true, state = false;
    std::string locked_on, ams, patches, module, gate;
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
    if (st.state && hbl_lock_state_parse(s.data(), s.size(), &a, &b, &c)) st.locked_on = fmt::format("{}.{}.{}", a, b, c);
    SysInfo si;
    sysinfo_get(&si);
    if (si.ams_valid) st.ams = fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro);
    const auto p = forwarders_logic::patches_state(ui::patch_report());
    st.patches = p == forwarders_logic::Patches::Ready ? "ready" : p == forwarders_logic::Patches::Unknown ? "unknown" : "missing";
    st.module = !rescue::installed() ? "none" : rescue::outdated() ? "outdated" : "current";
    st.gate = paths::file_exists(in_data(GATE_NRO_NAME)) ? "present" : "none";
    st.c = check(take_log);
    return st;
}

std::string yes_no(bool v)
{
    return v ? "playguard/hbmenu/yes"_i18n : "playguard/hbmenu/no"_i18n;
}
}   // namespace

void show_check()
{
    using ui::Check;
    const State st = read_state(true);
    const std::string unknown = "playguard/forwarders/unknown"_i18n;
    std::vector<ui::CheckLine> lines;
    // What Atmosphère will read, then the checks, then the system's.
    lines.push_back({ Check::Header, "playguard/hbmenu/sec_reads"_i18n });
    lines.push_back({ st.lock_block ? Check::Ok : Check::Info,
                      brls::getStr("playguard/hbmenu/check_file", HBL_LOCK_CONFIG_PATH,
                                   !st.file ? "playguard/hbmenu/check_none"_i18n
                                   : st.lock_block ? "playguard/hbmenu/state_locked"_i18n
                                                   : "playguard/hbmenu/state_open"_i18n) });
    lines.push_back({ st.album ? Check::Info : Check::Ok, brls::getStr("playguard/hbmenu/check_album", yes_no(st.album)) });
    lines.push_back({ st.any_app ? Check::Info : Check::Ok, brls::getStr("playguard/hbmenu/check_r", yes_no(st.any_app)) });
    for (const ui::CheckLine& l : check_lines(st.c)) lines.push_back(l);
    lines.push_back({ Check::Header, "playguard/hbmenu/sec_system"_i18n });
    lines.push_back({ Check::Info, brls::getStr("playguard/hbmenu/check_ams", st.ams.empty() ? unknown : st.ams,
                                                st.locked_on.empty() ? unknown : st.locked_on) });
    lines.push_back({ st.module == "outdated" ? Check::Warn : Check::Info,
                      brls::getStr("playguard/hbmenu/check_module",
                                   st.module == "none"      ? "playguard/hbmenu/check_none"_i18n
                                   : st.module == "current" ? "playguard/hbmenu/check_current"_i18n
                                                            : "playguard/hbmenu/check_outdated"_i18n) });
    auto* dialog = ui::box_dialog(ui::checklist_body("playguard/hbmenu/check"_i18n, lines));
    dialog->addButton("playguard/hbmenu/rescue"_i18n, []() { brls::sync([]() { show_rescue(); }); });
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    if (brls::View* ok = dialog->getView("brls/dialog/button2")) dialog->setLastFocusedView(ok);
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
    return fmt::format(
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
