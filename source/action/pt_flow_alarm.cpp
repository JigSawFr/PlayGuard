// pt_flow: the "time's up" alarm, what the console does when time is up
// (suspend, or a warning only), and pausing or resuming the countdown.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <borealis.hpp>

#include "action/history_flow.hpp"
#include "action/pt_logic.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace pt_flow
{

std::string time_up_text(const PtState& pt)
{
    if (!pt.valid) return "";
    return pt.alarm_only ? "playguard/play_timer/time_up_alarm"_i18n : "playguard/play_timer/time_up_suspend"_i18n;
}

Result write_alarm_only(bool alarm_only, const std::string& source)
{
    PtState before;
    pctl_play_timer_query(&before);
    const Result rc = pctl_play_timer_set_alarm_only(alarm_only);
    if (R_SUCCEEDED(rc) && before.valid && before.alarm_only != alarm_only)
        history_flow::record_values("time_up", { before.alarm_only ? 1 : 0 }, { alarm_only ? 1 : 0 }, source);
    if (R_FAILED(rc))
        history_flow::record_failure("time_up", rc, before.valid ? std::vector<int>{ before.alarm_only ? 1 : 0 } : std::vector<int>{},
                                     { alarm_only ? 1 : 0 }, source);
    return rc;
}

void choose_time_up(const PtState& pt, std::function<void()> refresh)
{
    if (ui::refuse_read_only()) return;
    if (!pt.fw_supported) {
        ui::notify(ui::rc_text(NXM_RC_FW_UNSUPPORTED), ui::Toast::Refused);
        return;
    }
    if (!pt.valid) {
        ui::notify(ui::rc_text(NXM_RC_STATE_UNKNOWN), ui::Toast::Refused);
        return;
    }
    if (!pt_configured(pt.block)) {
        ui::info("playguard/play_timer/time_up_needs_limit"_i18n);
        return;
    }
    const std::vector<std::string> labels = { "playguard/play_timer/time_up_suspend"_i18n,
                                              "playguard/play_timer/time_up_alarm"_i18n };
    const bool now = pt.alarm_only;
    ui::pick("playguard/play_timer/time_up_title"_i18n, labels, now ? 1 : 0, [now, refresh](int index) {
        const bool alarm_only = index == 1;
        if (alarm_only == now) return;   // already so
        const std::string body = alarm_only ? "playguard/play_timer/time_up_alarm_body"_i18n
                                            : "playguard/play_timer/time_up_suspend_body"_i18n;
        confirm_write(body, "playguard/play_timer/confirm_set"_i18n, [alarm_only, refresh](bool did_unlock) {
            const Result rc = write_alarm_only(alarm_only, "");
            finish_write(rc, did_unlock, "playguard/common/applied"_i18n, "playguard/play_timer/write_err"_i18n, refresh);
        });
    });
}

bool alarm_off(const PtState& pt)
{
    return pt_logic::alarm_off(pt);
}

Result write_alarm_disabled(bool disabled, const std::string& source)
{
    PtState before;
    pctl_play_timer_query(&before);
    const Result rc = pctl_play_timer_set_alarm_disabled(disabled);
    if (R_SUCCEEDED(rc) && before.alarm_disabled_valid)
        history_flow::record_values("alarm", { before.alarm_disabled ? 1 : 0 }, { disabled ? 1 : 0 }, source);
    if (R_FAILED(rc))
        history_flow::record_failure("alarm", rc,
                                     before.alarm_disabled_valid ? std::vector<int>{ before.alarm_disabled ? 1 : 0 } : std::vector<int>{},
                                     { disabled ? 1 : 0 }, source);
    return rc;
}

void set_alarm(bool on, const std::string& source, std::function<void()> refresh)
{
    if (ui::refuse_read_only()) return;
    const std::string body = on ? "playguard/play_timer/alarm_on_body"_i18n : "playguard/play_timer/alarm_off_body"_i18n;
    const std::string label = on ? "playguard/play_timer/alarm_on_confirm"_i18n : "playguard/play_timer/alarm_off_confirm"_i18n;
    confirm_write(body, label, [on, source, refresh](bool did_unlock) {
        const Result rc = write_alarm_disabled(!on, source);
        finish_write(rc, did_unlock, on ? "playguard/play_timer/alarm_on_done"_i18n : "playguard/common/applied"_i18n,
                     "playguard/play_timer/write_err"_i18n, refresh);
    }, nullptr, !on);
}

void turn_alarm_on(const std::string& source, std::function<void()> refresh)
{
    set_alarm(true, source, refresh);
}

void set_countdown(bool running, std::function<void()> refresh)
{
    if (ui::refuse_read_only()) return;
    const std::string body = running ? "playguard/play_timer/resume_body"_i18n : "playguard/play_timer/pause_body"_i18n;
    const std::string label = running ? "playguard/play_timer/resume_confirm"_i18n : "playguard/play_timer/pause_confirm"_i18n;
    confirm_write(body, label, [running, refresh](bool did_unlock) {
        PtState before;
        pctl_play_timer_query(&before);
        const Result rc = running ? pctl_play_timer_start() : pctl_play_timer_stop();
        // Paused or resumed: the countdown (1453) as it was, and as asked.
        std::vector<int> was;
        if (before.enabled_valid) was = { before.enabled ? 1 : 0 };
        if (R_SUCCEEDED(rc)) {
            if (was.empty()) history_flow::record_event("countdown", "", running ? "playguard/history/counting"_i18n
                                                                              : "playguard/history/paused"_i18n);
            else history_flow::record_values("countdown", was, { running ? 1 : 0 });
        } else {
            history_flow::record_failure("countdown", rc, was, { running ? 1 : 0 });
        }
        finish_write(rc, did_unlock, "playguard/common/applied"_i18n, "playguard/play_timer/write_err"_i18n, refresh);
    }, nullptr, !running);
}

}   // namespace pt_flow
