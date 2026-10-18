// pt_flow (see pt_flow.hpp): the write gate (confirm_write, finish_write),
// the limit writes and the limit pickers. The rest of pt_flow by subject:
// pt_flow_unlock.cpp (the unlock record and locking again), pt_flow_bedtime.cpp,
// pt_flow_extra.cpp (extra time, no more play today, putting it back),
// pt_flow_alarm.cpp (the alarm, what happens when time is up, the countdown).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <borealis.hpp>

#include "action/console_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pt_flow_internal.hpp"
#include "action/pt_logic.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "view/pt_week.hpp"

using namespace brls::literals;

namespace pt_flow
{

std::string with_link_note(const std::string& body, const PctlStatus* st)
{
    PctlStatus read;
    if (!st) {
        pctl_status_fetch(&read);
        st = &read;
    }
    if (!st->pairing_active_ok || !st->pairing_active) return body;
    return body.empty() ? "playguard/pairing/note"_i18n : body + "\n\n" + "playguard/pairing/note"_i18n;
}

void confirm_write(const std::string& body_in, const std::string& confirm_label,
                   std::function<void(bool did_unlock)> write, const uint16_t* new_days, bool danger,
                   const std::string& title)
{
    if (ui::refuse_read_only()) return;

    PtState pt;
    pctl_play_timer_query(&pt);
    if (!pt.fw_supported) {
        ui::notify(ui::rc_text(NXM_RC_FW_UNSUPPORTED));
        return;
    }
    if (!pt_logic::state_known(pt)) {
        ui::notify(ui::rc_text(NXM_RC_STATE_UNKNOWN));
        return;
    }

    // A limit below what was already played suspends the game at the relock.
    std::string body = body_in;
    const int played = pt_logic::suspend_warning_min(pt, ui::today_weekday(), new_days);
    if (played > 0) {
        const std::string warning = brls::getStr("playguard/play_timer/suspend_warning", ui::fmt_minutes((uint16_t)played)) +
                                    " " + "playguard/play_timer/unsaved"_i18n;
        body = body.empty() ? warning : body + "\n\n" + warning;
    }
    // Any dialog shown ends with the phone-app note, while it is linked.
    const bool unlock = pt_logic::needs_unlock(pt);
    if (!body.empty() || unlock) body = with_link_note(body);

    // The week as it will be: the days that change in amber.
    auto preview = [&pt, new_days]() -> brls::View* {
        if (!new_days || !pt.valid) return nullptr;
        auto* week = new PtWeekView();
        week->show(new_days, pt.day_min);
        return week;
    };

    const auto titled = [&title](const std::string& text) { return title.empty() ? text : title + "\n\n" + text; };
    if (!unlock) {
        if (body.empty()) {
            write(false);
            return;
        }
        body = titled(body);
        if (brls::View* week = preview()) ui::confirm_with(body, week, confirm_label, [write]() { write(false); }, nullptr, danger);
        else ui::confirm(body, confirm_label, [write]() { write(false); }, nullptr, danger);
        return;
    }

    std::string text = body.empty() ? std::string() : body + "\n\n";
    text += "playguard/play_timer/gate/body"_i18n + " " +
            (config::get().auto_relock ? "playguard/play_timer/gate/relock_auto"_i18n
                                       : "playguard/play_timer/gate/relock_manual"_i18n);
    text = titled(text);
    auto unlock_and_write = [write]() {
        // Should the app stop before finish_write, the next start locks again:
        // without that record saved, no unlock (as for the extra time).
        if (!set_relock_pending(true)) {
            config::get().relock_pending = false;   // not on the card: not in memory either
            ui::error("playguard/play_timer/gate/failed"_i18n + " — " + ui::rc_text(NXM_RC_NOT_SAVED));
            return;
        }
        Result rc = pctl_unlock_restriction_temporarily();
        if (R_FAILED(rc)) {
            settle_failed_unlock();   // not unlocked, or locked again by the service layer
            ui::error("playguard/play_timer/gate/failed"_i18n + " — " + ui::rc_text(rc));
            return;
        }
        write(true);
    };
    if (brls::View* week = preview()) ui::confirm_with(text, week, "playguard/play_timer/gate/confirm"_i18n, unlock_and_write, nullptr, danger);
    else ui::confirm(text, "playguard/play_timer/gate/confirm"_i18n, unlock_and_write, nullptr, danger);
}

// Limits written by anything but the console lock replace it: its record
// would otherwise put older limits back when it is turned off.
static void limits_replaced(const std::string& source)
{
    if (source != "console_lock") console_lock::forget();
}

Result write_days(const uint16_t days[7], const std::string& source, const std::string& detail)
{
    PtState before;
    pctl_play_timer_query(&before);
    const Result rc = pctl_play_timer_set_days(days);
    if (R_SUCCEEDED(rc)) limits_replaced(source);
    if (R_SUCCEEDED(rc) && before.valid)
        history_flow::record_values("limits", history_flow::days_values(before.day_min), history_flow::days_values(days),
                                    source, detail);
    if (R_FAILED(rc))
        history_flow::record_failure("limits", rc, before.valid ? history_flow::days_values(before.day_min) : std::vector<int>{},
                                     history_flow::days_values(days), source, detail);
    return rc;
}

Result clear_days(const std::string& source)
{
    PtState before;
    pctl_play_timer_query(&before);
    const Result rc = pctl_play_timer_clear();
    if (R_SUCCEEDED(rc)) limits_replaced(source);
    uint16_t none[7];
    for (auto& d : none) d = PT_DAY_NOLIMIT;
    if (R_SUCCEEDED(rc) && before.valid)
        history_flow::record_values("limits", history_flow::days_values(before.day_min), history_flow::days_values(none), source);
    if (R_FAILED(rc))
        history_flow::record_failure("limits", rc, before.valid ? history_flow::days_values(before.day_min) : std::vector<int>{},
                                     history_flow::days_values(none), source);
    return rc;
}

static void offer_relock(std::function<void()> after)
{
    auto* dialog = ui::dialog("playguard/play_timer/relock/body"_i18n + " " + "playguard/play_timer/unsaved"_i18n);
    // "Later" is a choice to stay unlocked: the record goes. Closed before
    // an answer, it stays, and the next start locks again.
    auto later = [after]() {
        set_relock_pending(false);
        if (after) after();
    };
    dialog->addButton("playguard/play_timer/relock/later"_i18n, later);
    dialog->addButton("playguard/play_timer/relock/now"_i18n, [after]() {
        Result rc = pctl_relock();
        if (R_SUCCEEDED(rc)) set_relock_pending(false);
        ui::notify_result(rc, "playguard/toast/relocked"_i18n, "playguard/toast/relock_err"_i18n);
        if (after) after();
    });
    ui::on_cancel(dialog, later);   // B: "Later"
    dialog->open();
}

void finish_write(Result rc, bool did_unlock, const std::string& ok_text,
                  const std::string& error_prefix, std::function<void()> refresh)
{
    if (!did_unlock) {
        ui::notify_result(rc, ok_text, error_prefix);
        if (refresh) refresh();
        return;
    }
    // The unlock was only for this write: never leave the console unlocked
    // behind the user's back, even when the write failed. The record of the
    // unlock stays until the console is locked again (or the user says later).
    if (config::get().auto_relock) {
        Result relock = pctl_relock();
        if (R_SUCCEEDED(relock)) set_relock_pending(false);
        if (R_SUCCEEDED(rc) && R_SUCCEEDED(relock)) {
            ui::notify(ok_text + " " + "playguard/toast/relocked"_i18n);
        } else {
            // One dialog says it all: what failed, and what went through.
            std::string text;
            if (R_FAILED(rc)) text = error_prefix + " — " + ui::rc_text(rc);
            else if (!ok_text.empty()) text = ok_text;
            if (R_FAILED(relock)) text += (text.empty() ? "" : "\n\n") + "playguard/toast/relock_err"_i18n + " — " + ui::rc_text(relock);
            ui::error(text);
        }
        if (refresh) refresh();
        return;
    }
    ui::notify_result(rc, ok_text, error_prefix);
    if (refresh) refresh();
    offer_relock(refresh);
}

static void apply_uniform(uint16_t minutes, std::function<void()> refresh)
{
    std::string body = minutes == 0 ? "playguard/play_timer/confirm_uniform_zero"_i18n
                                    : brls::getStr("playguard/play_timer/confirm_uniform", ui::fmt_minutes(minutes));
    uint16_t new_days[7];
    for (auto& d : new_days) d = minutes;
    confirm_write(body, "playguard/play_timer/confirm_set"_i18n, [minutes, refresh](bool did_unlock) {
        uint16_t days[7];
        for (auto& d : days) d = minutes;
        Result rc = write_days(days, "uniform");
        finish_write(rc, did_unlock, brls::getStr("playguard/play_timer/written_uniform", ui::fmt_minutes(minutes)),
                     "playguard/play_timer/write_err"_i18n, refresh);
    }, new_days);
}

void choose_uniform_limit(const PtState& pt, std::function<void()> refresh)
{
    const pt_logic::UniformPick pick = pt_logic::uniform_pick(pt);
    std::vector<std::string> labels;
    for (uint16_t v : pt_logic::quick_values()) labels.push_back(ui::fmt_minutes(v));
    labels.push_back("playguard/common/custom"_i18n);

    const std::string title = pick.uniform ? "playguard/play_timer/quick_title"_i18n
                                           : "playguard/play_timer/quick_title_varies"_i18n;
    const uint16_t seed = pick.seed;
    ui::pick(title, labels, pick.selected, [refresh, seed](int index) {
        const auto& values = pt_logic::quick_values();
        if ((size_t)index < values.size()) {
            apply_uniform(values[index], refresh);
            return;
        }
        ui::prompt_minutes("playguard/play_timer/quick_title"_i18n, seed,
                           [refresh](uint16_t v) { apply_uniform(v, refresh); });
    });
}

void pick_limit(const std::string& title, uint16_t current, std::function<void(uint16_t)> on_value)
{
    // Quick values first (as in "Same limit every day"), then any value, then
    // no limit; the current value is pre-selected.
    const pt_logic::LimitPick pick = pt_logic::limit_pick(current);
    std::vector<std::string> options;
    for (uint16_t v : pt_logic::quick_values()) options.push_back(ui::fmt_minutes(v));
    options.push_back("playguard/play_timer/perday/pick_minutes"_i18n);
    options.push_back("playguard/play_timer/perday/pick_no_limit"_i18n);

    const int custom_index = pick.custom, nolimit_index = pick.no_limit;
    ui::pick(title, options, pick.selected, [title, current, on_value, custom_index, nolimit_index](int index) {
        if (index == nolimit_index) {
            on_value(PT_DAY_NOLIMIT);
        } else if (index == custom_index) {
            ui::prompt_minutes(title, pt_logic::limit_seed(current), [on_value](uint16_t v) { on_value(v); });
        } else {
            on_value(pt_logic::quick_values()[index]);
        }
    });
}

}   // namespace pt_flow
