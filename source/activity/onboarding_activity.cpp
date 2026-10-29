// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/onboarding_activity.hpp"

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "action/clock_flow.hpp"
#include "action/console_actions.hpp"
#include "action/fw_gate.hpp"
#include "action/pairing_state.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/funding.hpp"

using namespace aurore::literals;

bool OnboardingActivity::wanted_at_start()
{
    if (!config::get().onboarding_at_start || app::read_only() || fw_gate::needed()) return false;
    PctlStatus s;
    pctl_status_fetch(&s);
    return s.pin_length_ok && s.pin_length == 0;
}

void OnboardingActivity::onContentAvailable()
{
    headline->setSingleLine(false);
    note->setSingleLine(false);
    // Not a step: the other system's controls are out of reach from here.
    SysInfo si;
    sysinfo_get(&si);
    const std::string other = ui::other_storage_note(si);
    storage_note->setSingleLine(false);
    storage_note->setText(other);
    ui::set_visible(storage_note.getView(), !other.empty());
    at_start->init("playguard/onboarding/at_start"_i18n, config::get().onboarding_at_start, [](bool on) {
        config::get().onboarding_at_start = on;
        ui::save_config();
    });
    unlink->registerClickAction([this](aurore::View*) {
        console_actions::unlink("first_steps", [this]() { this->refresh(); });
        return true;
    });
    alarm->setDetailText("playguard/dashboard/alarm_off"_i18n);   // the title says why
    alarm->detail->setThemeColor("textColor", "cadran/amber");
    alarm->registerClickAction([this](aurore::View*) {
        pt_flow::turn_alarm_on("first_steps", [this]() { this->refresh(); });
        return true;
    });

    // The PIN tab's own choice: the PIN is the console's, this keeps a child out
    // of PlayGuard itself.
    protect->registerClickAction([this](aurore::View*) {
        pin_lock::choose([this]() { this->refresh(); });
        return true;
    });

    pin->registerClickAction([this](aurore::View*) {
        console_actions::set_pin("first_steps", [this]() { this->refresh(); });
        return true;
    });
    limit->registerClickAction([this](aurore::View*) {
        PtState pt;
        pctl_play_timer_query(&pt);
        if (!pt.fw_supported) {
            ui::error("", NXM_RC_FW_UNSUPPORTED);
            return true;
        }
        pt_flow::choose_uniform_limit(pt, [this]() { this->refresh(); });
        return true;
    });
    clock->registerClickAction([this](aurore::View*) {
        if (this->clock_inaccurate) clock_flow::guided([this]() { this->refresh(); });
        else ui::info("playguard/clock/guided_fine"_i18n);
        return true;
    });
    close->registerClickAction([](aurore::View*) {
        aurore::Application::popActivity();
        return true;
    });
    // Not a step: below Close, for whoever wants it.
    support->setDetailText("GitHub Sponsors · Ko-fi");
    support->registerClickAction([](aurore::View*) {
        funding::open_dialog();
        return true;
    });
    this->refresh();
}

void OnboardingActivity::willAppear(bool resetState)
{
    aurore::Activity::willAppear(resetState);
    this->refresh();
}

void OnboardingActivity::refresh()
{
    PctlStatus s;
    pctl_status_fetch(&s);
    PtState pt;
    pctl_play_timer_query(&pt);
    bool accurate = false;
    const Result accuracy_rc = time_network_accuracy(&accurate);
    const std::string todo = "playguard/onboarding/todo"_i18n;

    const bool has_pin = s.pin_length_ok && s.pin_length > 0;
    pin->setDetailText(!s.pin_length_ok ? "cadran/common/unavailable"_i18n
                       : has_pin ? "playguard/dashboard/pin_set"_i18n : todo);
    pin->detail->setThemeColor("textColor", has_pin ? "cadran/teal" : "cadran/amber");

    bool any_limit = false;
    for (int d = 0; d < 7 && pt.valid; d++) any_limit |= pt.day_min[d] != PT_DAY_NOLIMIT;
    if (!pt.fw_supported) {
        limit->setDetailText("playguard/play_timer/fw_too_old_short"_i18n);
        limit->detail->setThemeColor("textColor", "cadran/text2");
    } else if (!pt.valid) {
        limit->setDetailText("cadran/common/unavailable"_i18n);
        limit->detail->setThemeColor("textColor", "cadran/text2");
    } else {
        limit->setDetailText(any_limit ? ui::days_summary(pt.day_min) : todo);
        limit->detail->setThemeColor("textColor", any_limit ? "cadran/teal" : "cadran/amber");
    }

    this->clock_inaccurate = R_SUCCEEDED(accuracy_rc) && !accurate;
    clock->setDetailText(R_FAILED(accuracy_rc) ? "cadran/common/unavailable"_i18n
                         : accurate ? "playguard/dashboard/clock_ok"_i18n : "playguard/dashboard/clock_bad"_i18n);
    clock->detail->setThemeColor("textColor", R_FAILED(accuracy_rc) ? "cadran/text2" : accurate ? "cadran/teal" : "cadran/amber");

    // Step 4 only while the phone app is linked: its next sync would undo the
    // rest (none when Atmosphère's DNS blocks its server).
    const bool paired = pairing_state::syncs(s);
    unlink->setDetailText("playguard/dashboard/pairing_on"_i18n);
    unlink->detail->setThemeColor("textColor", "cadran/amber");
    ui::set_visible(unlink.getView(), paired);

    // Then, while the timer runs with its alarm off: nothing says time is up.
    const bool alarm_off = pt_flow::alarm_off(pt);
    alarm->setText(aurore::getStr("playguard/onboarding/step_alarm", paired ? 5 : 4));
    ui::set_visible(alarm.getView(), alarm_off);

    // Last, once a PIN is set: Protect PlayGuard with the PIN (any mode but "Never").
    const bool protected_ = config::get().pin_lock != "off";
    protect->setText(aurore::getStr("playguard/onboarding/step_protect", 4 + (paired ? 1 : 0) + (alarm_off ? 1 : 0)));
    protect->setDetailText(protected_ ? pin_lock::mode_text() : todo);
    protect->detail->setThemeColor("textColor", protected_ ? "cadran/teal" : "cadran/amber");
    ui::set_visible(protect.getView(), has_pin);

    const bool done = has_pin && (!pt.fw_supported || any_limit) && (R_FAILED(accuracy_rc) || accurate) && !paired
                      && !alarm_off && protected_;
    headline->setText(done ? "playguard/onboarding/all_done"_i18n : "playguard/onboarding/headline"_i18n);
}
