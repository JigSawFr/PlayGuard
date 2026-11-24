// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/security_tab.hpp"

#include <aurore/cadran/ui/cadran_frame.hpp>
#include <aurore/cadran/ui/cadran_kit.hpp>

#include "action/console_actions.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pairing_state.hpp"
#include "action/pt_flow.hpp"
#include "action/session_pause.hpp"
#include "activity/companion_activity.hpp"
#include "activity/forwarders_activity.hpp"
#include "activity/modules_activity.hpp"
#include "activity/pin_activity.hpp"
#include "app.hpp"
#include "ui/play_ui.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace aurore::literals;

SecurityTab::SecurityTab()
    : TabBase("xml/tab/security.xml"), list(this)
{
    this->enable_auto_refresh(5000);
    // What the notification bell says is to do here, on its row.
    this->mark(pin.getView(), { "pin_missing", "pin_weak", "pin_lock_off" });
    // The PIN and the companion phone app each have a screen of their own.
    pin->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new PinActivity());
        return true;
    });
    companion->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new CompanionActivity());
        return true;
    });
    // The homebrew menu's lock is made on Forwarders, after its checks: a
    // line to it here, by the PIN that guards it, and one to Forwarders
    // itself (homebrew on the HOME menu, protected by the PIN).
    forwarders->setDetailText(ui::opens());
    forwarders->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new ForwardersActivity());
        return true;
    });
    hbmenu_lock->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new ForwardersActivity());
        return true;
    });
    unlock->registerClickAction([this](aurore::View*) {
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
    del->registerClickAction([this](aurore::View*) {
        console_actions::delete_all("", this->refresher());
        return true;
    });
    rescue_about->setSingleLine(false);
    rescue->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new ModuleActivity(modules::Id::Rescue));
        return true;
    });
    this->list.tiles({ { "sc_pin", "key" }, { "sc_companion", "phone" }, { "sc_hbmenu", "home" } }, true);
    this->list.apply({ { "sc_hbmenu_lock", "home" }, { "sc_forwarders", "home" }, { "sc_unlock", "unlock" },
                       { "sc_auto_relock", "refresh" }, { "sc_delete", "trash" }, { "sc_rescue", "tools" } });
    if (cadran::Card* danger = this->list.card_of("sc_delete"))
        danger->setAccent("cadran/coral_line");
}

void SecurityTab::willAppear(bool resetState)
{
    this->hbmenu_locked = hbmenu_lock::locked();
    // Locked out? › Optional modules installs and removes the sysmodule while
    // PlayGuard runs: its state read as the tab comes back.
    rescue->setDetailText(ui::opens(ModulesActivity::summary(modules::Id::Rescue)));
    TabBase::willAppear(resetState);
}

void SecurityTab::refresh()
{
    perf::Timed timed("PIN refresh");   // the diagnostic report's timings
    PctlStatus s;
    pctl_status_fetch(&s);
    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);
    const std::string na = "cadran/common/unavailable"_i18n;

    if (!s.pin_length_ok) ui::set_detail(pin, ui::opens(na));
    else if (s.pin_length == 0) ui::set_detail(pin, ui::opens("playguard/common/not_set"_i18n));
    else ui::set_detail(pin, ui::opens("playguard/dashboard/pin_set"_i18n));   // not its length: it narrows a guess

    // Each tile's state at a glance: a check when it is as it should be, a
    // warning when it needs the parent; nothing when it could not be read.
    constexpr const char* OK = "cadran/teal_fill";
    constexpr const char* WARN = "cadran/amber_fill";
    this->list.state("sc_pin", !s.pin_length_ok ? nullptr : s.pin_length ? OK : WARN);
    // Open is how most consoles are, and fine without forwarders: no warning,
    // only a check once it is locked.
    const std::string hb_state = this->hbmenu_locked ? "playguard/hbmenu/state_locked"_i18n : "playguard/hbmenu/state_open"_i18n;
    ui::set_detail(hbmenu, hb_state);
    this->list.state("sc_hbmenu", this->hbmenu_locked ? OK : nullptr);
    hbmenu_lock->setDetailText(ui::opens(hb_state));

    // Companion app: while linked, its next sync overwrites what is set here.
    const bool paired = s.pairing_active_ok && s.pairing_active;
    const bool syncs = pairing_state::syncs(s);
    std::string linked = ui::bool_text(s.pairing_active_ok, s.pairing_active, "cadran/common/yes"_i18n, "cadran/common/no"_i18n);
    if (paired && !syncs) linked += " · " + "playguard/pairing/dns_blocked"_i18n;
    ui::set_detail(companion, ui::opens(linked));
    this->list.state("sc_companion", !s.pairing_active_ok ? nullptr : paired && syncs ? WARN : OK);

    const bool writable = !app::read_only();
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    // Read-only: the actions stay in sight, greyed (A says why).
    ui::set_visible(unlock.getView(), has_pin && !unlocked);
    for (aurore::DetailCell* c : { (aurore::DetailCell*)unlock.getView(), (aurore::DetailCell*)auto_relock.getView() })
        ui::show_writable(c, writable);
    auto_relock->setOn(config::get().auto_relock, false);
    ui::show_writable(del, writable, "cadran/coral", "cadran/text2");
    this->list.sync();
    if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", cadran::Tone::WARN);
    else if (has_pin) cadran::set_pill("playguard/cadran/pill/pin_set"_i18n, "key");
    else if (s.pin_length_ok) cadran::set_pill("playguard/cadran/pill/pin_none"_i18n, "key", cadran::Tone::WARN);
    else cadran::set_pill("");
}

aurore::View* SecurityTab::create()
{
    return new SecurityTab();
}
