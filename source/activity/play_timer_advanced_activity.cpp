// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "activity/play_timer_advanced_activity.hpp"

#include "action/pt_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/diagnostics.hpp"

using namespace brls::literals;

void PlayTimerAdvancedActivity::onContentAvailable()
{
    note->setSingleLine(false);

    // Only PlayGuard's own setting: also changed in read-only mode.
    show->init("playguard/tools/advanced"_i18n, config::get().advanced, [this](bool on) {
        config::get().advanced = on;
        ui::save_config();
        this->refresh();
    });

    ui::guard_switch(alarm);
    alarm->init("playguard/play_timer/alarm"_i18n, true, [this](bool on) {
        // The switch shows the console's value until the change is made
        // (after a confirmation, and the unlock when the timer counts down).
        this->alarm->setOn(!on, false);
        pt_flow::set_alarm(on, "", [this]() { this->refresh(); });
    });
    pause->registerClickAction([this](brls::View*) {
        pt_flow::set_countdown(false, [this]() { this->refresh(); });
        return true;
    });
    resume->registerClickAction([this](brls::View*) {
        pt_flow::set_countdown(true, [this]() { this->refresh(); });
        return true;
    });
    diag->registerClickAction([](brls::View*) {
        std::string err;
        const std::string path = diagnostic::save(diagnostic::current_report(), &err);
        if (path.empty()) ui::error("playguard/toast/diag_err"_i18n + ": " + err);
        else ui::notify(brls::getStr("playguard/toast/diag_saved", path));
        return true;
    });
    this->refresh();
}

void PlayTimerAdvancedActivity::refresh()
{
    pctl_play_timer_query(&this->pt);
    const bool shown    = config::get().advanced;
    const bool writable = this->pt.fw_supported && !app::read_only();
    show->setOn(shown, false);
    for (brls::View* v : { (brls::View*)alarm.getView(), (brls::View*)pause.getView(), (brls::View*)resume.getView() })
        ui::set_visible(v, shown);
    ui::set_visible(diag.getView(), app::dev_mode());
    if (this->pt.alarm_disabled_valid) alarm->setOn(!this->pt.alarm_disabled, false);
    ui::show_writable(alarm, writable);
    ui::show_writable(pause, writable);
    ui::show_writable(resume, writable);
}
