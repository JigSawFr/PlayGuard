// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/security_tab.hpp"

#include <borealis/cadran/ui/cadran_frame.hpp>
#include <borealis/cadran/ui/cadran_kit.hpp>

#include "action/console_actions.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pairing_state.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_block_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/session_pause.hpp"
#include "activity/forwarders_activity.hpp"
#include "app.hpp"
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
    pr_note->setSingleLine(false);
    pin_lock_note->setSingleLine(false);
    // The homebrew menu's lock is made on Console › Forwarders, after its
    // checks: a line to it here, by the PIN that guards it.
    hbmenu_lock->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ForwardersActivity());
        return true;
    });
    // Deleting everything is the one irreversible action here: its line is
    // drawn in the danger colour, not only its section title (refresh()).
    pin_lock_cell->registerClickAction([this](brls::View*) {
        pin_lock::choose(this->refresher());
        return true;
    });
    // Off by default: once entered, the PIN holds until PlayGuard closes.
    pin_ask_again->init("playguard/pin_lock/ask_again"_i18n, config::get().pin_ask_again, [](bool) {});
    pin_ask_again->registerClickAction([this](brls::View*) {
        pin_lock::set_ask_again(!config::get().pin_ask_again, this->refresher());
        return true;
    });
    // Off by default: entering the PIN leaves parental controls as they are.
    pin_pauses->init("playguard/pin_lock/pauses"_i18n, config::get().pin_pauses, [](bool) {});
    pin_pauses->registerClickAction([this](brls::View*) {
        pin_lock::set_pauses(!config::get().pin_pauses, this->refresher());
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
        console_actions::unlink("", this->refresher());
        return true;
    });
    set_pin->registerClickAction([this](brls::View*) {
        this->change_pin();
        return true;
    });
    show_pin->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        // Each step a frame later, after a dialog: nothing once the tab is gone.
        const auto change = ui::guarded(this->alive, [this]() { this->change_pin(); });
        const auto offer_change = ui::guarded(this->alive, [change]() {
            ui::confirm("playguard/security/show_pin_change_body"_i18n,
                        "playguard/security/show_pin_change_confirm"_i18n, [change]() { brls::sync(change); });
        });
        const auto show = ui::guarded(this->alive, [offer_change]() {
            // The PIN every time (pin_lock.hpp); then, as it may have been
            // seen over a shoulder, a new one is offered.
            if (!pin_lock::before_show_pin()) return;
            const bool shown = ui::show_pin_dialog([offer_change]() { brls::sync(offer_change); });
            if (shown) history_flow::record_event("pin_shown");
        });
        ui::confirm("playguard/security/show_pin_body"_i18n, "playguard/security/show_pin_confirm"_i18n,
                    [show]() { brls::sync(show); });
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
    // After a change PlayGuard made with a temporary unlock: on at once, or offered.
    ui::guard_switch(auto_relock);
    auto_relock->init("playguard/tools/auto_relock"_i18n, config::get().auto_relock, [](bool on) {
        config::get().auto_relock = on;
        ui::save_config();
    });
    del->registerClickAction([this](brls::View*) {
        console_actions::delete_all("", this->refresher());
        return true;
    });
    this->list.tiles({ { "sc_pin", "key" }, { "sc_restrictions", "shield" }, { "sc_temp", "unlock" }, { "sc_hbmenu", "home" } }, true);
    this->list.apply({ { "sc_set_pin", "key" }, { "sc_pin_lock", "lock" }, { "sc_pin_ask_again", "timer" },
                       { "sc_pin_pauses", "pause" },
                       { "sc_hbmenu_lock", "home" }, { "sc_show_pin", "eye" }, { "sc_unlock", "unlock" },
                       { "sc_auto_relock", "refresh" }, { "pr_active", "phone" },
                       { "pr_updated", "refresh" }, { "pr_decode", "code" }, { "pr_unlink", "stop" }, { "sc_delete", "trash" } });
    if (cadran::Card* danger = this->list.card_of("sc_delete"))
        danger->setAccent(brls::Application::getTheme()["cadran/coral_line"]);
}

void SecurityTab::willAppear(bool resetState)
{
    this->hbmenu_locked = hbmenu_lock::locked();
    TabBase::willAppear(resetState);
}

void SecurityTab::change_pin()
{
    console_actions::set_pin("", this->refresher());
}

void SecurityTab::refresh()
{
    perf::Timed timed("PIN refresh");   // the diagnostic report's timings
    PctlStatus s;
    pctl_status_fetch(&s);
    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);
    const std::string na = "cadran/common/unavailable"_i18n;

    if (!s.pin_length_ok) ui::set_detail(pin, na);
    else if (s.pin_length == 0) ui::set_detail(pin, "playguard/common/not_set"_i18n);
    else ui::set_detail(pin, "playguard/dashboard/pin_set"_i18n);   // not its length: it narrows a guess

    ui::set_detail(restrictions, ui::bool_text(s.restriction_enabled_ok, s.restriction_enabled,
                                               "cadran/common/yes"_i18n, "cadran/common/no"_i18n));
    // Said as a state, not as "unlocked: no" under a check. PlayGuard's own
    // pause while the parent is here is no warning: it ends as it is left.
    const bool paused = session_pause::active() && s.temp_unlocked_ok && s.temp_unlocked;
    if (paused) ui::set_detail(temp, "playguard/cadran/pill/paused"_i18n);
    else ui::set_detail(temp, ui::bool_text(s.temp_unlocked_ok, s.temp_unlocked,
                                            "playguard/cadran/pill/unlocked"_i18n, "playguard/dashboard/pc_active"_i18n));
    temp->setDetailTextColor(s.temp_unlocked_ok && s.temp_unlocked && !paused ? cadran::kit::color("cadran/amber")
                                                                               : cadran::kit::color("cadran/text2"));
    // Each tile's state at a glance: a check when it is as it should be, a
    // warning when it needs the parent; nothing when it could not be read.
    constexpr const char* OK = "cadran/teal_fill";
    constexpr const char* WARN = "cadran/amber_fill";
    this->list.state("sc_pin", !s.pin_length_ok ? nullptr : s.pin_length ? OK : WARN);
    this->list.state("sc_restrictions", !s.restriction_enabled_ok ? nullptr : s.restriction_enabled ? OK : WARN);
    this->list.state("sc_temp", !s.temp_unlocked_ok ? nullptr : s.temp_unlocked && !paused ? WARN : OK);
    // Open is how most consoles are, and fine without forwarders: no warning,
    // only a check once it is locked.
    const std::string hb_state = this->hbmenu_locked ? "playguard/hbmenu/state_locked"_i18n : "playguard/hbmenu/state_open"_i18n;
    ui::set_detail(hbmenu, hb_state);
    this->list.state("sc_hbmenu", this->hbmenu_locked ? OK : nullptr);
    hbmenu_lock->setDetailText(ui::opens(hb_state));

    // Companion app: while linked, its next sync overwrites what is set here,
    // unless Atmosphère's DNS blocks its server (then the note says so).
    const bool paired = s.pairing_active_ok && s.pairing_active;
    const bool syncs = pairing_state::syncs(s);
    std::string active = ui::bool_text(s.pairing_active_ok, s.pairing_active, "cadran/common/yes"_i18n, "cadran/common/no"_i18n);
    if (paired && !syncs) active += " · " + "playguard/pairing/dns_blocked"_i18n;
    ui::set_detail(pr_active, active);
    pr_active->setDetailTextColor(syncs ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    ui::set_detail(pr_updated, s.last_updated_ok ? ui::time_text(s.last_updated) : "cadran/common/unavailable"_i18n);
    ui::set_text(pr_note, paired && !syncs ? "playguard/pairing/dns_note"_i18n : "playguard/pairing/note"_i18n);
    pr_note->setTextColor(syncs ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text3"));

    ui::set_detail(pin_lock_cell, pin_lock::mode_text());
    ui::set_text(pin_lock_note, pin_lock::note_text());
    pin_ask_again->setOn(config::get().pin_ask_again, false);
    pin_pauses->setOn(config::get().pin_pauses, false);

    const bool writable = !app::read_only();
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    // Read-only: the actions stay in sight, greyed (A says why).
    ui::set_visible_all({ { show_pin.getView(), has_pin },
                          { unlock.getView(), has_pin && !unlocked },
                          // Nothing to unlink when the read says not linked.
                          { pr_unlink.getView(), !s.pairing_active_ok || paired },
                          // The block comparison needs the app to sync.
                          { pr_decode.getView(), syncs } });
    for (brls::DetailCell* c : { (brls::DetailCell*)set_pin.getView(), (brls::DetailCell*)show_pin.getView(),
                                 (brls::DetailCell*)unlock.getView(), (brls::DetailCell*)pr_unlink.getView(),
                                 (brls::DetailCell*)auto_relock.getView() })
        ui::show_writable(c, writable);
    auto_relock->setOn(config::get().auto_relock, false);
    ui::show_writable(del, writable, cadran::kit::color("cadran/coral"), cadran::kit::color("cadran/text2"));
    this->list.sync();
    if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", cadran::Tone::WARN);
    else if (has_pin) cadran::set_pill("playguard/cadran/pill/pin_set"_i18n, "key");
    else if (s.pin_length_ok) cadran::set_pill("playguard/cadran/pill/pin_none"_i18n, "key", cadran::Tone::WARN);
    else cadran::set_pill("");
}

brls::View* SecurityTab::create()
{
    return new SecurityTab();
}
