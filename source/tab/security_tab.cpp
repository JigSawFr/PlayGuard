// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/security_tab.hpp"

#include "action/console_actions.hpp"
#include "action/console_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pairing_state.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_block_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/rescue.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace brls::literals;

SecurityTab::SecurityTab()
    : TabBase("xml/tab/security.xml"), list(this)
{
    this->enable_auto_refresh(5000);
    // What the notification bell says is to do here, on its row.
    this->mark(set_pin.getView(), { "pin_missing", "pin_weak" });
    this->mark(pin_lock_cell.getView(), { "pin_lock_off" });
    this->mark(relock.getView(), { "unlocked" });
    pr_note->setSingleLine(false);
    rescue_note->setSingleLine(false);
    // Checked once: the sysmodule only comes and goes with the SD card out.
    ui::set_visible(rescue_note.getView(), rescue::installed());
    pin_lock_note->setSingleLine(false);
    console_lock_note->setSingleLine(false);
    console_lock_cell->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        PtState pt;
        pctl_play_timer_query(&pt);
        if (!pt.fw_supported) {
            ui::error("", NXM_RC_FW_UNSUPPORTED);
            return true;
        }
        PctlStatus s;
        pctl_status_fetch(&s);
        if (!(s.pin_length_ok && s.pin_length > 0)) {   // nothing gates play without a PIN
            ui::info("playguard/console_lock/needs_pin"_i18n);
            return true;
        }
        console_lock::set(!console_lock::active(), pt, [this]() { this->refresh(); });
        return true;
    });
    // Deleting everything is the one irreversible action here: its line is
    // drawn in the danger colour, not only its section title (refresh()).
    pin_lock_cell->registerClickAction([this](brls::View*) {
        pin_lock::choose([this]() { this->refresh(); });
        return true;
    });
    // Off by default: once entered, the PIN holds until PlayGuard closes.
    pin_ask_again->init("playguard/pin_lock/ask_again"_i18n, config::get().pin_ask_again, [](bool) {});
    pin_ask_again->registerClickAction([this](brls::View*) {
        pin_lock::set_ask_again(!config::get().pin_ask_again, [this]() { this->refresh(); });
        return true;
    });
    // While linked, anyone can save the block before and after a change made
    // in the phone app: the data ROADMAP.md waits for (Developer tools has it too).
    ui::set_detail(pr_decode, ui::fmt_played(2));   // how long it takes
    pr_decode->registerClickAction([](brls::View*) {
        pt_block_flow::open();
        return true;
    });
    pr_unlink->registerClickAction([this](brls::View*) {
        console_actions::unlink("", [this]() { this->refresh(); });
        return true;
    });
    set_pin->registerClickAction([this](brls::View*) {
        this->change_pin();
        return true;
    });
    show_pin->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        ui::confirm("playguard/security/show_pin_body"_i18n, "playguard/security/show_pin_confirm"_i18n, [this]() {
            brls::sync([this]() {
                // The PIN every time (pin_lock.hpp); then, as it may have been
                // seen over a shoulder, a new one is offered.
                if (!pin_lock::before_show_pin()) return;
                const bool shown = ui::show_pin_dialog([this]() {
                    brls::sync([this]() {
                        ui::confirm("playguard/security/show_pin_change_body"_i18n,
                                    "playguard/security/show_pin_change_confirm"_i18n, [this]() {
                                        brls::sync([this]() { this->change_pin(); });
                                    });
                    });
                });
                if (shown) history_flow::record_event("pin_shown");
            });
        });
        return true;
    });
    unlock->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        ui::confirm("playguard/security/unlock_body"_i18n, "playguard/security/unlock_confirm"_i18n, [this]() {
            Result rc = pt_flow::unlock_temporarily();
            if (R_SUCCEEDED(rc)) history_flow::record_event("unlock");
            else history_flow::record_failure("unlock", rc);
            ui::notify_result(rc, "playguard/security/unlocked"_i18n, "playguard/security/unlock_err"_i18n);
            this->refresh();
        });
        return true;
    });
    relock->registerClickAction([this](brls::View*) {
        console_actions::relock("", [this]() { this->refresh(); });
        return true;
    });
    // After a change PlayGuard made with a temporary unlock: on at once, or offered.
    ui::guard_switch(auto_relock);
    auto_relock->init("playguard/tools/auto_relock"_i18n, config::get().auto_relock, [](bool on) {
        config::get().auto_relock = on;
        ui::save_config();
    });
    del->registerClickAction([this](brls::View*) {
        console_actions::delete_all("", [this]() { this->refresh(); });
        return true;
    });
    this->list.tiles({ { "sc_pin", "key" }, { "sc_restrictions", "shield" }, { "sc_temp", "unlock" } }, true);
    this->list.apply({ { "sc_set_pin", "key" }, { "sc_pin_lock", "lock" }, { "sc_pin_ask_again", "timer" },
                       { "sc_show_pin", "eye" }, { "sc_unlock", "unlock" }, { "sc_relock", "lock" },
                       { "sc_auto_relock", "refresh" }, { "sc_console_lock", "gamepad" }, { "pr_active", "phone" },
                       { "pr_updated", "refresh" }, { "pr_decode", "code" }, { "pr_unlink", "stop" }, { "sc_delete", "trash" } });
    this->headline = this->list.headline("playguard/cadran/today/pc_title"_i18n);
    if (cadran::Card* danger = this->list.card_of("sc_delete"))
        danger->setAccent(brls::Application::getTheme()["cadran/coral_line"]);
}

void SecurityTab::change_pin()
{
    console_actions::set_pin("", [this]() { this->refresh(); });
}

void SecurityTab::refresh()
{
    perf::Timed timed("PIN refresh");   // the diagnostic report's timings
    PctlStatus s;
    pctl_status_fetch(&s);
    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);
    const std::string na = "playguard/common/unavailable"_i18n;

    if (!s.pin_length_ok) ui::set_detail(pin, na);
    else if (s.pin_length == 0) ui::set_detail(pin, "playguard/common/not_set"_i18n);
    else ui::set_detail(pin, "playguard/dashboard/pin_set"_i18n);   // not its length: it narrows a guess

    ui::set_detail(restrictions, ui::bool_text(s.restriction_enabled_ok, s.restriction_enabled,
                                               "playguard/common/yes"_i18n, "playguard/common/no"_i18n));
    // Said as a state, not as "unlocked: no" under a check.
    ui::set_detail(temp, ui::bool_text(s.temp_unlocked_ok, s.temp_unlocked,
                                       "playguard/cadran/pill/unlocked"_i18n, "playguard/dashboard/pc_active"_i18n));
    temp->setDetailTextColor(s.temp_unlocked_ok && s.temp_unlocked ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    // Each tile's state at a glance: a check when it is as it should be, a
    // warning when it needs the parent; nothing when it could not be read.
    constexpr const char* OK = "cadran/teal_fill";
    constexpr const char* WARN = "cadran/amber_fill";
    this->list.state("sc_pin", !s.pin_length_ok ? nullptr : s.pin_length ? OK : WARN);
    this->list.state("sc_restrictions", !s.restriction_enabled_ok ? nullptr : s.restriction_enabled ? OK : WARN);
    this->list.state("sc_temp", !s.temp_unlocked_ok ? nullptr : s.temp_unlocked ? WARN : OK);

    // Companion app: while linked, its next sync overwrites what is set here,
    // unless Atmosphère's DNS blocks its server (then the note says so).
    const bool paired = s.pairing_active_ok && s.pairing_active;
    const bool syncs = pairing_state::syncs(s);
    std::string active = ui::bool_text(s.pairing_active_ok, s.pairing_active, "playguard/common/yes"_i18n, "playguard/common/no"_i18n);
    if (paired && !syncs) active += " · " + "playguard/pairing/dns_blocked"_i18n;
    ui::set_detail(pr_active, active);
    pr_active->setDetailTextColor(syncs ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    ui::set_detail(pr_updated, s.last_updated_ok ? ui::time_text(s.last_updated) : "playguard/common/unavailable"_i18n);
    ui::set_text(pr_note, paired && !syncs ? "playguard/pairing/dns_note"_i18n : "playguard/pairing/note"_i18n);
    pr_note->setTextColor(syncs ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text3"));

    ui::set_detail(pin_lock_cell, pin_lock::mode_text());
    ui::set_text(pin_lock_note, pin_lock::note_text());
    pin_ask_again->setOn(config::get().pin_ask_again, false);

    const bool locked = console_lock::active();
    ui::set_detail(console_lock_cell, locked ? "playguard/common/on"_i18n : "playguard/common/off"_i18n);
    console_lock_cell->setDetailTextColor(locked ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    ui::set_text(console_lock_note, locked ? "playguard/console_lock/note_on"_i18n : "playguard/console_lock/note_off"_i18n);

    const bool writable = !app::read_only();
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    // The three tiles in one line: what matters first (no PIN, then unlocked).
    if (this->headline) {
        const bool no_pin = s.pin_length_ok && s.pin_length == 0;
        ui::set_text(this->headline, no_pin     ? "playguard/security/headline_none"_i18n
                                     : unlocked ? "playguard/security/headline_unlocked"_i18n
                                     : has_pin  ? "playguard/security/headline_active"_i18n
                                                : "playguard/common/unavailable"_i18n);
        this->headline->setTextColor(cadran::kit::color(no_pin || unlocked ? "cadran/amber" : "cadran/text"));
    }
    // Read-only: the actions stay in sight, greyed (A says why).
    ui::set_visible_all({ { show_pin.getView(), has_pin },
                          { unlock.getView(), has_pin && !unlocked },
                          { relock.getView(), unlocked },
                          // Nothing to unlink when the read says not linked.
                          { pr_unlink.getView(), !s.pairing_active_ok || paired },
                          // The block comparison needs the app to sync.
                          { pr_decode.getView(), syncs } });
    for (brls::DetailCell* c : { (brls::DetailCell*)set_pin.getView(), (brls::DetailCell*)show_pin.getView(),
                                 (brls::DetailCell*)unlock.getView(), (brls::DetailCell*)relock.getView(),
                                 (brls::DetailCell*)console_lock_cell.getView(), (brls::DetailCell*)pr_unlink.getView(),
                                 (brls::DetailCell*)auto_relock.getView() })
        ui::show_writable(c, writable);
    auto_relock->setOn(config::get().auto_relock, false);
    ui::show_writable(del, writable, cadran::kit::color("cadran/coral"), cadran::kit::color("cadran/text2"));
    this->list.sync();
    if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", true);
    else if (has_pin) cadran::set_pill("playguard/cadran/pill/pin_set"_i18n, "key");
    else if (s.pin_length_ok) cadran::set_pill("playguard/cadran/pill/pin_none"_i18n, "key", true);
    else cadran::set_pill("");
}

brls::View* SecurityTab::create()
{
    return new SecurityTab();
}
