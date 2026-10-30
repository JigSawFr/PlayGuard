// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sync_orders.hpp"

#include <aurore.hpp>
#include <format>
#include <vector>

#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "action/session_pause.hpp"
#include "action/session_pause_logic.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/profiles.hpp"
#include "util/sync_files.hpp"

using namespace aurore::literals;

namespace sync_orders
{

namespace
{
std::string hm(int hour, int minute)
{
    return std::format("{:02d}:{:02d}", hour, minute);
}

std::string on_off(const char* on_key, const char* off_key, bool on)
{
    return aurore::getStr(on ? on_key : off_key);
}

// A saved profile's limits, by its display name (sync_exec's callback).
bool profile_days(void*, const char* name, uint16_t days[7])
{
    for (const auto& p : profiles::list())
        if (p.name == name) {
            for (int d = 0; d < 7; d++) days[d] = p.days[(size_t)d];
            return true;
        }
    return false;
}

std::vector<int> values(const int* v, int n)
{
    return std::vector<int>(v, v + n);
}

// The change history, as for the same change made on the console. `before`
// is the play timer before a bedtime change; none for a change the agent made
// (at `when`), whose bedtime is recorded as it was asked.
// A change the console refused with its error code: in the history as the
// same change refused on the console is (amber, never put back). PlayGuard's
// own refusals (the play timer counting, the parent declining, read-only, a
// value out of range) reached nothing on the console and are only answered.
void record_refused(const Order& o, const SyncOutcome& out, const std::string& source)
{
    const std::string detail = o.intent.kind == SyncIntent_Profile ? std::string(o.intent.profile) : std::string();
    const auto one = [](int v) { return v >= 0 ? std::vector<int>{ v } : std::vector<int>{}; };
    switch (out.change) {
        case SyncChange_Limits:
            history_flow::record_failure("limits", out.rc, values(out.before, 7), values(out.after, 7), source, detail);
            break;
        case SyncChange_Level:
            history_flow::record_failure("level", out.rc, one(out.before[0]), { out.after[0] }, source);
            break;
        case SyncChange_Vr: history_flow::record_failure("vr", out.rc, one(out.before[0]), { out.after[0] }, source); break;
        case SyncChange_Custom: history_flow::record_failure("custom", out.rc, {}, {}, source); break;
        case SyncChange_Alarm: history_flow::record_failure("alarm", out.rc, {}, {}, source); break;
        case SyncChange_Bedtime:
            history_flow::record_failure("bedtime", out.rc, {}, {}, source, o.entity + " = " + o.payload);
            break;
        case SyncChange_Unlock: history_flow::record_failure("unlock", out.rc, {}, {}, source); break;
        case SyncChange_Relock: history_flow::record_failure("relock", out.rc, {}, {}, source); break;
        default: break;   // nothing was asked of the console yet (it could not be read)
    }
}

void record(const Order& o, const SyncOutcome& out, const PtState* before, const std::string& when = "")
{
    const std::string source = out.source ? out.source : "remote";
    if (!out.applied && out.reason == SyncReason_PctlError) record_refused(o, out, source);
    if (!out.applied || !out.changed) return;
    const std::string detail = o.intent.kind == SyncIntent_Profile ? std::string(o.intent.profile) : std::string();
    switch (out.change) {
        case SyncChange_Limits:
            history_flow::record_values("limits", values(out.before, 7), values(out.after, 7), source, detail, when);
            break;
        case SyncChange_Level:
            if (out.before[0] >= 0)
                history_flow::record_values("level", { out.before[0] }, { out.after[0] }, "remote", "", when);
            break;
        case SyncChange_Custom:
            history_flow::record_values("custom", values(out.before, 3), values(out.after, 3), "remote", "", when);
            break;
        case SyncChange_Vr:
            if (out.before[0] >= 0) history_flow::record_values("vr", { out.before[0] }, { out.after[0] }, "remote", "", when);
            break;
        case SyncChange_Alarm:
            if (out.before[0] >= 0)
                history_flow::record_values("alarm", { out.before[0] }, { out.after[0] }, "remote", "", when);
            break;
        case SyncChange_Bedtime: {
            if (!before) {
                history_flow::record_event("bedtime", "remote", o.entity + " = " + o.payload, when);
                break;
            }
            PtState after;
            pctl_play_timer_query(&after);
            if (!before->valid || !after.valid) break;
            const std::string from = pt_flow::bedtime_text(*before), to = pt_flow::bedtime_text(after);
            if (from != to) history_flow::record_event("bedtime", "remote", from + " → " + to);
            break;
        }
        case SyncChange_Unlock: history_flow::record_event("unlock", "remote", "", when); break;
        case SyncChange_Relock: history_flow::record_event("relock", "remote", "", when); break;
        default: break;
    }
    if (o.intent.kind == SyncIntent_ConsoleLock && out.console_lock_after >= 0)
        history_flow::record_values("console_lock", { out.console_lock_after ? 0 : 1 }, { out.console_lock_after },
                                    "remote", "", when);
}

// What must be put back later (an unlock, the extra time, the console lock's
// limits) on the SD card before the console changes, as PlayGuard's own
// changes do (pt_flow, console_lock): state.json written now, not queued.
bool save_records(void*, const SyncRecords* r)
{
    if (!sync_files::records_into(*r, config::get())) return true;
    return ui::save_config_now();
}

SyncOutcome execute(const Order& o, bool remote_timer_writes)
{
    // Security › Pause parental controls while I'm in PlayGuard: the
    // console's unlock is then PlayGuard's own, ended as the parent leaves.
    const SyncIntentKind kind = o.intent.kind;
    const auto pause = session_pause_logic::on_remote(session_pause::active(), kind == SyncIntent_Unlock,
                                                      kind == SyncIntent_LockNow);
    // Already unlocked: kept so once PlayGuard is left (before the records,
    // which say whether to undo it at the next start, are read).
    if (pause == session_pause_logic::Remote::Adopt) session_pause::adopt();
    auto& cfg = config::get();
    SyncRecords rec = sync_files::records_from(cfg);
    const std::string today = ui::today_date();
    SyncExecCtx ctx{};
    ctx.remote_timer_writes = remote_timer_writes;
    ctx.weekday = ui::today_weekday();
    ctx.today = today.c_str();
    ctx.rec = &rec;
    ctx.profile_days = profile_days;
    ctx.save_records = save_records;
    ctx.ctx = nullptr;

    PtState before{};
    if (kind == SyncIntent_Bedtime || kind == SyncIntent_BedtimeEnd || kind == SyncIntent_BedtimeEnabled)
        pctl_play_timer_query(&before);

    SyncOutcome out;
    sync_exec(&o.intent, &ctx, &out);
    if (pause == session_pause_logic::Remote::End && out.applied) session_pause::stop();
    if (sync_files::records_into(rec, cfg)) ui::save_config();
    record(o, out, &before);
    aurore::Logger::info("sync: {}={} -> rc 0x{:08X} {}", o.entity, o.payload, (unsigned)out.rc,
                       out.applied ? "applied" : sync_reason_name(out.reason));
    return out;
}

// Lock now and a report change nothing a child would want: never asked.
bool asks(SyncIntentKind k)
{
    return k != SyncIntent_LockNow;
}
}   // namespace

std::string describe(const SyncIntent& in)
{
    switch (in.kind) {
        case SyncIntent_LimitDay:
            return aurore::getStr("playguard/sync/orders/limit_day", ui::day_name_in_text(in.day), ui::fmt_minutes(in.minutes));
        case SyncIntent_LimitUniform:
            return aurore::getStr("playguard/sync/orders/limit_uniform", ui::fmt_minutes(in.minutes));
        case SyncIntent_LimitsWeek: {
            if (in.mask == 0x7F) return aurore::getStr("playguard/sync/orders/limits_week", ui::days_summary(in.days));
            std::string list;
            for (int d = 0; d < 7; d++)
                if (in.mask & (1u << d))
                    list += (list.empty() ? "" : ", ") +
                            aurore::getStr("playguard/common/line", ui::day_name(d), ui::fmt_minutes(in.days[d]));
            return aurore::getStr("playguard/sync/orders/limits_days", list);
        }
        case SyncIntent_LimitToday:
            return aurore::getStr("playguard/sync/orders/limit_today", ui::fmt_minutes(in.minutes));
        case SyncIntent_RemoveLimit: return "playguard/sync/orders/remove_limit"_i18n;
        case SyncIntent_ConsoleLock:
            return on_off("playguard/sync/orders/console_lock_on", "playguard/sync/orders/console_lock_off", in.on);
        case SyncIntent_BonusTime: return aurore::getStr("playguard/sync/orders/bonus", ui::fmt_played(in.minutes));
        case SyncIntent_StopToday: return "playguard/sync/orders/stop_today"_i18n;
        case SyncIntent_Unlock: return "playguard/sync/orders/unlock"_i18n;
        case SyncIntent_LockNow: return "playguard/sync/orders/lock_now"_i18n;
        case SyncIntent_Alarm: return on_off("playguard/sync/orders/alarm_on", "playguard/sync/orders/alarm_off", in.on);
        case SyncIntent_BedtimeEnabled:
            return on_off("playguard/sync/orders/bedtime_on", "playguard/sync/orders/bedtime_off", in.on);
        case SyncIntent_Bedtime: return aurore::getStr("playguard/sync/orders/bedtime", hm(in.hour, in.minute));
        case SyncIntent_BedtimeEnd: return aurore::getStr("playguard/sync/orders/bedtime_end", hm(in.hour, in.minute));
        case SyncIntent_Profile: return aurore::getStr("playguard/sync/orders/profile", std::string(in.profile));
        case SyncIntent_Level: return aurore::getStr("playguard/sync/orders/level", ui::level_name(in.level));
        case SyncIntent_Vr: return on_off("playguard/sync/orders/vr_on", "playguard/sync/orders/vr_off", in.on);
        case SyncIntent_Sns: return on_off("playguard/sync/orders/sns_on", "playguard/sync/orders/sns_off", in.on);
        case SyncIntent_Comm: return on_off("playguard/sync/orders/comm_on", "playguard/sync/orders/comm_off", in.on);
        case SyncIntent_ExportReport: return "playguard/sync/orders/export_report"_i18n;
        default: return "?";
    }
}

void run(const Order& o, SyncPolicy policy, bool remote_timer_writes, std::function<void(const SyncOutcome&)> done)
{
    if (policy == SyncPolicy_Auto || !asks(o.intent.kind)) {
        // The broker authenticated the sender: Security's "ask for the PIN
        // before a change" is for someone at the console.
        pin_lock::remote_bypass(true);
        const SyncOutcome out = execute(o, remote_timer_writes);
        pin_lock::remote_bypass(false);
        done(out);
        return;
    }
    const std::string body = aurore::getStr("playguard/sync/ask_body", describe(o.intent));
    ui::confirm(body, "playguard/sync/ask_confirm"_i18n,
                [o, remote_timer_writes, done]() { done(execute(o, remote_timer_writes)); },
                [done]() {
                    SyncOutcome out;
                    sync_outcome_init(&out);
                    out.reason = SyncReason_NotConfirmed;
                    done(out);
                });
}

void tell(const Order& o, const SyncOutcome& out)
{
    const std::string what = describe(o.intent);
    if (out.relock_failed) {
        ui::error(aurore::getStr("playguard/sync/done", what) + "\n\n" + "playguard/toast/relock_err"_i18n);
        return;
    }
    if (out.applied) {
        ui::notify(aurore::getStr("playguard/sync/done", what));
        return;
    }
    if (out.reason == SyncReason_NotConfirmed && !out.rc) return;   // declined here: nothing to say
    std::string why = out.reason == SyncReason_NotConfirmed
                          ? pin_lock::refusal_text()   // the PIN screen's answer
                          : aurore::getStr(std::string("playguard/sync/reasons/") + sync_reason_name(out.reason));
    if (out.reason == SyncReason_PctlError && out.rc) why += " — " + ui::rc_text(out.rc);
    ui::notify(aurore::getStr("playguard/sync/refused", what, why));
}

void import(const sync_files::AgentEvent& ev, const std::string& when)
{
    if (!ev.applied) return;
    Order o;
    o.entity = ev.entity;
    o.payload = ev.payload;
    // The intent only says which kind of order it was (a profile's name, the
    // console lock); the usual limit put back after midnight is none.
    if (sync_apply_parse(o.entity.c_str(), o.payload.c_str(), o.payload.size(), &o.intent) != SyncReason_None)
        o.intent = SyncIntent{};
    SyncOutcome out;
    sync_outcome_init(&out);
    out.applied = true;
    out.changed = true;
    out.change = (SyncChange)ev.change;
    out.n = (int)ev.before.size();
    for (size_t i = 0; i < ev.before.size() && i < 7; i++) out.before[i] = ev.before[i];
    for (size_t i = 0; i < ev.after.size() && i < 7; i++) out.after[i] = ev.after[i];
    out.source = ev.source.c_str();
    out.console_lock_after = ev.console_lock_after;
    record(o, out, nullptr, when);
}

}   // namespace sync_orders
