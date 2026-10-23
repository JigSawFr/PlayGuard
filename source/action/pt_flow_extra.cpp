// pt_flow: extra time today, no more play today, extra time planned for a
// later day, and putting the usual limit back afterwards (the records in
// config extra_* and bonus_*).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <borealis.hpp>

#include "action/console_lock.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow_internal.hpp"
#include "action/pt_logic.hpp"
#include "app.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace pt_flow
{

// "No more play today" can wait this long, for a save.
static constexpr int STOP_SOON_MIN = 5;

// Extra time and "no more play" are for one day of the usual limits: with
// every day at 0 they would open a hole in the console lock.
static bool refuse_console_lock()
{
    if (!console_lock::active()) return false;
    ui::info("playguard/console_lock/blocks_change"_i18n);
    return true;
}

static void clear_extra_record()
{
    auto& cfg = config::get();
    cfg.extra_weekday = -1;
    cfg.extra_date.clear();
    ui::save_config();
}

static pt_logic::ExtraRecord extra_record()
{
    const auto& cfg = config::get();
    pt_logic::ExtraRecord rec;
    rec.weekday = cfg.extra_weekday;
    rec.date    = cfg.extra_date;
    rec.base    = (uint16_t)cfg.extra_base;
    rec.value   = (uint16_t)cfg.extra_value;
    return rec;
}

// What to put back later, saved *before* the write it describes: should the
// app stop between the two, the limit still comes back. False when it could
// not be saved, and then nothing must be written (it would never be put back).
// `date`: the day of `wd`, read with it (not at the confirmation, which may
// come after midnight).
static bool save_extra_record(int wd, const std::string& date, uint16_t base, uint16_t value)
{
    auto& c = config::get();
    c.extra_weekday = wd;
    c.extra_date    = date;
    c.extra_base    = base;
    c.extra_value   = value;
    return ui::save_config_now();
}

// The write failed: the record it was saved for goes back to what it was.
static void put_back_extra_record(const pt_logic::ExtraRecord& prev)
{
    auto& c = config::get();
    c.extra_weekday = prev.weekday;
    c.extra_date    = prev.date;
    c.extra_base    = prev.base;
    c.extra_value   = prev.value;
    ui::save_config();
}

static pt_logic::BonusRecord bonus_record()
{
    const auto& cfg = config::get();
    pt_logic::BonusRecord rec;
    rec.weekday = cfg.bonus_weekday;
    rec.date    = cfg.bonus_date;
    rec.base    = (uint16_t)cfg.bonus_base;
    rec.value   = (uint16_t)cfg.bonus_value;
    return rec;
}

static void clear_bonus_record()
{
    auto& cfg = config::get();
    cfg.bonus_weekday = -1;
    cfg.bonus_date.clear();
    cfg.bonus_base = cfg.bonus_value = 0;
    ui::save_config();
}

// Saved before the write, as the extra-time record (and put back as it was
// when the write fails).
static bool save_bonus_record(const pt_logic::BonusRecord& rec)
{
    auto& c = config::get();
    c.bonus_weekday = rec.weekday;
    c.bonus_date    = rec.date;
    c.bonus_base    = rec.base;
    c.bonus_value   = rec.value;
    return ui::save_config_now();
}

static void plan_bonus(std::function<void()> refresh);
static void cancel_bonus(std::function<void()> refresh);

// `extra` minutes more today (weekday `wd`, on `date`), after the
// confirmation (the picker below, or an amount's chip on Today).
static void add_extra(int wd, const std::string& date, uint16_t base, uint16_t extra, std::function<void()> refresh)
{
    // What gets put back later is the limit before any extra time today.
    const pt_logic::ExtraRecord rec = extra_record();
    const bool again = pt_logic::recorded_today(rec, wd, date);
    const pt_logic::ExtraPlan plan = pt_logic::plan_extra(base, extra, again, rec.base);
    const uint16_t value = plan.value, original = plan.original;
    // Put back by itself the next day, whatever the preference (pt_logic::restore_way).
    const std::string body = brls::getStr("playguard/dashboard/extra_body_auto", ui::fmt_minutes(extra),
                                          ui::day_name_in_text(wd), ui::fmt_minutes(base), ui::fmt_minutes(value),
                                          ui::fmt_minutes(original));
    confirm_write(body, "playguard/dashboard/extra_confirm"_i18n, [refresh, wd, date, base, value, original](bool did_unlock) {
        PtState now;
        pctl_play_timer_query(&now);
        if (!now.valid || now.day_min[wd] != base) {   // changed meanwhile: write nothing
            finish_write(NXM_RC_STATE_UNKNOWN, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        uint16_t days[7];
        for (int i = 0; i < 7; i++) days[i] = now.day_min[i];
        days[wd] = value;
        const pt_logic::ExtraRecord prev = extra_record();
        if (!save_extra_record(wd, date, original, value)) {
            finish_write(NXM_RC_NOT_SAVED, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        Result rc = write_days(days, "extra");
        if (R_FAILED(rc)) put_back_extra_record(prev);
        finish_write(rc, did_unlock, brls::getStr("playguard/dashboard/extra_done", ui::fmt_minutes(value)),
                     "playguard/play_timer/write_err"_i18n, refresh);
    });
}

void add_extra_time(const PtState& pt, std::function<void()> refresh)
{
    if (refuse_console_lock()) return;
    ui::SameTime same;   // the weekday and the date, of one moment
    const int wd = ui::today_weekday();
    const std::string date = ui::today_date();
    if (!pt.valid || pt.day_min[wd] == PT_DAY_NOLIMIT) return;
    // Play timer › Extra time amounts.
    const std::vector<int> amounts = config::get().extra_amounts;
    std::vector<std::string> labels;
    for (int e : amounts) labels.push_back("+" + ui::fmt_minutes((uint16_t)e));
    // Then another day: a bonus planned for it, or the one planned already
    // (which can be cancelled).
    const pt_logic::BonusRecord bonus = bonus_record();
    const bool planned = bonus.weekday >= 0;
    labels.push_back(planned ? brls::getStr("playguard/dashboard/bonus_planned_row", ui::day_name(bonus.weekday),
                                            "+" + ui::fmt_minutes((uint16_t)(bonus.value - bonus.base)))
                             : "playguard/dashboard/bonus_row"_i18n);
    const uint16_t base = pt.day_min[wd];
    // A list of actions, not of values: no row is checked.
    ui::pick("playguard/dashboard/extra_title"_i18n, labels, 0, [refresh, wd, date, base, amounts, planned](int index) {
        if ((size_t)index == amounts.size()) {
            if (planned) cancel_bonus(refresh);
            else plan_bonus(refresh);
            return;
        }
        add_extra(wd, date, base, (uint16_t)amounts[index], refresh);
    }, false);
}

// Extra time on a later day: the day (tomorrow and the five after, each with
// its limit), the amount, the confirmation; that day's limit is raised now
// and put back once the day is over.
static void plan_bonus(std::function<void()> refresh)
{
    PtState pt;
    pctl_play_timer_query(&pt);
    ui::SameTime same;   // the date and the weekday, of one moment
    const std::vector<pt_logic::PlanDay> days = pt_logic::bonus_days(ui::today_date(), ui::today_weekday());
    if (!pt.valid || days.empty()) {
        ui::refuse(NXM_RC_STATE_UNKNOWN);
        return;
    }
    std::vector<std::string> labels, details;
    for (const pt_logic::PlanDay& d : days) {
        labels.push_back(ui::day_name(d.weekday));
        details.push_back(ui::fmt_minutes(pt.day_min[d.weekday]));
    }
    ui::pick("playguard/dashboard/bonus_day_title"_i18n, labels, 0, [days, pt, refresh](int index) {
        const pt_logic::PlanDay day = days[(size_t)index];
        if (!pt_logic::can_plan_bonus(pt, day.weekday, app::read_only())) {
            ui::info(brls::getStr("playguard/dashboard/bonus_no_limit", ui::day_name(day.weekday)));
            return;
        }
        const std::vector<int> amounts = config::get().extra_amounts;
        std::vector<std::string> labels;
        for (int e : amounts) labels.push_back("+" + ui::fmt_minutes((uint16_t)e));
        const uint16_t base = pt.day_min[day.weekday];
        ui::pick(brls::getStr("playguard/dashboard/bonus_amount_title", ui::day_name(day.weekday)), labels, 0,
                 [day, base, amounts, refresh](int i) {
            const uint16_t value = pt_logic::plan_extra(base, (uint16_t)amounts[(size_t)i], false, base).value;
            const std::string body = brls::getStr("playguard/dashboard/bonus_body", ui::day_name(day.weekday),
                                                  ui::fmt_minutes(base), ui::fmt_minutes(value), ui::fmt_minutes(base));
            confirm_write(body, "playguard/dashboard/extra_confirm"_i18n, [day, base, value, refresh](bool did_unlock) {
                PtState now;
                pctl_play_timer_query(&now);
                if (!now.valid || now.day_min[day.weekday] != base) {   // changed meanwhile: write nothing
                    finish_write(NXM_RC_STATE_UNKNOWN, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
                    return;
                }
                uint16_t days[7];
                for (int d = 0; d < 7; d++) days[d] = now.day_min[d];
                days[day.weekday] = value;
                const pt_logic::BonusRecord prev = bonus_record();
                pt_logic::BonusRecord rec;
                rec.weekday = day.weekday;
                rec.date    = day.date;
                rec.base    = base;
                rec.value   = value;
                if (!save_bonus_record(rec)) {
                    finish_write(NXM_RC_NOT_SAVED, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
                    return;
                }
                const Result rc = write_days(days, "bonus");
                if (R_FAILED(rc)) save_bonus_record(prev);
                finish_write(rc, did_unlock,
                             brls::getStr("playguard/dashboard/bonus_done", ui::day_name(day.weekday),
                                          "+" + ui::fmt_minutes((uint16_t)(value - base))),
                             "playguard/play_timer/write_err"_i18n, refresh);
            });
        }, false);
    }, false, details);
}

// The bonus planned, cancelled before its day: the limit put back now (a
// tightening, asked as any change).
static void cancel_bonus(std::function<void()> refresh)
{
    const pt_logic::BonusRecord rec = bonus_record();
    if (rec.weekday < 0) return;
    const std::string body = brls::getStr("playguard/dashboard/bonus_cancel_body", ui::day_name(rec.weekday),
                                          "+" + ui::fmt_minutes((uint16_t)(rec.value - rec.base)), ui::fmt_minutes(rec.base));
    confirm_write(body, "playguard/dashboard/bonus_cancel"_i18n, [rec, refresh](bool did_unlock) {
        PtState now;
        pctl_play_timer_query(&now);
        if (!now.valid) {
            finish_write(NXM_RC_STATE_UNKNOWN, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        if (now.day_min[rec.weekday] != rec.value) {   // changed since: nothing to put back
            clear_bonus_record();
            finish_write(0, did_unlock, "playguard/dashboard/bonus_cancelled"_i18n, "", refresh);
            return;
        }
        uint16_t days[7];
        for (int d = 0; d < 7; d++) days[d] = now.day_min[d];
        days[rec.weekday] = rec.base;
        const Result rc = write_days(days, "restore_bonus");
        if (R_SUCCEEDED(rc)) clear_bonus_record();
        finish_write(rc, did_unlock, "playguard/dashboard/bonus_cancelled"_i18n, "playguard/play_timer/write_err"_i18n, refresh);
    });
}

void add_extra_minutes(const PtState& pt, uint16_t minutes, std::function<void()> refresh)
{
    if (refuse_console_lock()) return;
    ui::SameTime same;   // the weekday and the date, of one moment
    const int wd = ui::today_weekday();
    if (!pt.valid || pt.day_min[wd] == PT_DAY_NOLIMIT) return;
    add_extra(wd, ui::today_date(), pt.day_min[wd], minutes, refresh);
}

// Today's limit (weekday `wd`, on `date`) to `value`: 0 (no more play now) or
// a few minutes more than played (pt_logic::stop_soon_limit). Put back the
// next day like extra time.
static void stop_at(const PtState& pt, int wd, const std::string& date, uint16_t value, std::function<void()> refresh)
{
    const uint16_t base = pt.day_min[wd];
    const pt_logic::ExtraRecord rec = extra_record();
    const bool again = pt_logic::recorded_today(rec, wd, date);
    const pt_logic::ExtraPlan plan = pt_logic::plan_stop(base, again, rec.base);
    const uint16_t original = plan.original;
    uint16_t new_days[7];
    for (int i = 0; i < 7; i++) new_days[i] = pt.day_min[i];
    new_days[wd] = value;
    const bool auto_restore = config::get().extra_auto_restore;
    std::string body = value == 0
        ? brls::getStr(auto_restore ? "playguard/dashboard/stop_body_auto" : "playguard/dashboard/stop_body",
                       ui::day_name_in_text(wd), ui::fmt_minutes(original))
        : brls::getStr(auto_restore ? "playguard/dashboard/stop_soon_body_auto" : "playguard/dashboard/stop_soon_body",
                       ui::day_name_in_text(wd), ui::fmt_minutes(value), ui::fmt_minutes(original));
    // Said once: confirm_write adds it to its own warning when the time
    // played is known.
    // A warning only at the limit: nothing is stopped, the console warns.
    if (pt.alarm_only) body += "\n\n" + "playguard/dashboard/stop_alarm_only"_i18n;
    else if (value == 0 && pt_logic::suspend_warning_min(pt, wd, new_days) < 0)
        body += " " + "playguard/play_timer/unsaved"_i18n;
    confirm_write(body, "playguard/dashboard/stop_confirm"_i18n, [refresh, wd, date, base, original, value](bool did_unlock) {
        PtState now;
        pctl_play_timer_query(&now);
        if (!now.valid || now.day_min[wd] != base) {   // changed meanwhile: write nothing
            finish_write(NXM_RC_STATE_UNKNOWN, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        uint16_t days[7];
        for (int i = 0; i < 7; i++) days[i] = now.day_min[i];
        days[wd] = value;
        const pt_logic::ExtraRecord prev = extra_record();
        if (!save_extra_record(wd, date, original, value)) {
            finish_write(NXM_RC_NOT_SAVED, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        Result rc = write_days(days, "stop");
        if (R_FAILED(rc)) put_back_extra_record(prev);
        finish_write(rc, did_unlock, value == 0 ? "playguard/dashboard/stop_done"_i18n : "playguard/dashboard/stop_soon_done"_i18n,
                     "playguard/play_timer/write_err"_i18n, refresh);
    }, new_days, true);
}

void stop_today(const PtState& pt, std::function<void()> refresh)
{
    if (refuse_console_lock()) return;
    if (!pt.valid) return;
    ui::SameTime same;   // the weekday and the date, of one moment
    const int wd = ui::today_weekday();
    const std::string date = ui::today_date();
    // Suspending can lose unsaved progress: when the time played is known,
    // "in 5 minutes" leaves time to save.
    const int soon = pt_logic::stop_soon_limit(pt, wd, STOP_SOON_MIN);
    if (soon < 0) {
        stop_at(pt, wd, date, 0, refresh);
        return;
    }
    const PtState copy = pt;
    ui::pick("playguard/dashboard/stop_title"_i18n,
             { "playguard/dashboard/stop_now"_i18n, "playguard/dashboard/stop_soon"_i18n }, 0,
             [copy, wd, date, soon, refresh](int index) { stop_at(copy, wd, date, index == 0 ? 0 : (uint16_t)soon, refresh); },
             false);
}

// Puts `base` back on weekday `wd`, if it still holds the extra time (`value`).
static void restore_extra(int wd, uint16_t base, uint16_t value, std::function<void()> refresh)
{
    confirm_write("", "playguard/play_timer/confirm_set"_i18n, [wd, base, value, refresh](bool did_unlock) {
        PtState now;
        pctl_play_timer_query(&now);
        if (!now.valid || now.day_min[wd] != value) {
            // Changed meanwhile: nothing left to put back. Unreadable: the
            // record stays, to try again (pt_logic::restore_action says Later).
            if (now.valid) clear_extra_record();
            finish_write(NXM_RC_STATE_UNKNOWN, did_unlock, "", "playguard/play_timer/write_err"_i18n, refresh);
            return;
        }
        uint16_t days[7];
        for (int i = 0; i < 7; i++) days[i] = now.day_min[i];
        days[wd] = base;
        Result rc = write_days(days, "restore_extra");
        if (R_SUCCEEDED(rc)) clear_extra_record();
        finish_write(rc, did_unlock,
                     brls::getStr("playguard/dashboard/extra_restored", ui::day_name_in_text(wd), ui::fmt_minutes(base)),
                     "playguard/play_timer/write_err"_i18n, refresh);
    });
}

bool restore_pending(const PtState& pt)
{
    if (console_lock::active()) return false;   // put back once the lock is off
    return pt_logic::restore_action(extra_record(), ui::today_date(), pt, app::read_only()) == pt_logic::Restore::Offer;
}

int extra_today_minutes(const PtState& pt)
{
    ui::SameTime same;   // the date and the weekday, of one moment
    return pt_logic::extra_today_min(extra_record(), ui::today_date(), ui::today_weekday(), pt);
}

std::string restore_label()
{
    const pt_logic::ExtraRecord rec = extra_record();
    if (rec.weekday < 0 || rec.weekday > 6) return "";
    return brls::getStr("playguard/dashboard/extra_pending", ui::day_name_in_text(rec.weekday), ui::fmt_minutes(rec.base));
}

std::string bonus_text()
{
    const pt_logic::BonusRecord rec = bonus_record();
    if (rec.weekday < 0) return "";
    return brls::getStr("playguard/cadran/week/bonus", ui::day_name(rec.weekday), "+" + ui::fmt_minutes((uint16_t)(rec.value - rec.base)));
}

bool restore_lowers()
{
    return pt_logic::restore_tightens(extra_record());
}

// Set when a restore had to wait for a state that could not be read: the
// main screen's 30-second check tries again (retry_extra_time_restore).
static bool s_restore_retry = false;

// Extra time from an earlier day, put back with no question: the limit only
// goes down, so, as for locking again, the PIN is not asked
// (pin_lock::Tightening), and nobody keeps the extra time by answering a
// dialog. The play timer is still never written while it counts down: the
// temporary unlock comes first, and the console is locked again right after.
// `pt` was just read (restore_action said Offer). `asked`: from Today
// or the bell, where a failure is said; at start-up it is only logged, and the
// bell keeps its entry. `clear` drops the record once put back, `source` names
// the change in the history (a planned bonus is put back the same way).
static void put_back_by_itself(const pt_logic::ExtraRecord& rec, const PtState& pt, bool asked,
                               std::function<void()> refresh, std::function<void()> clear = clear_extra_record,
                               const char* source = "restore_extra")
{
    if (!pt_logic::state_known(pt)) {
        s_restore_retry = true;
        if (asked) ui::refuse(NXM_RC_STATE_UNKNOWN);
        return;
    }
    const int wd = rec.weekday;
    uint16_t days[7];
    for (int i = 0; i < 7; i++) days[i] = pt.day_min[i];
    days[wd] = rec.base;

    pin_lock::Tightening no_pin;
    const bool unlock = pt_logic::needs_unlock(pt);
    if (unlock) {
        // As in confirm_write: the record first, so that a stop before the
        // lock below locks again at the next start.
        if (!set_relock_pending(true)) {
            config::get().relock_pending = false;   // not on the card: not in memory either
            brls::Logger::warning("extra time: not put back, the unlock could not be recorded");
            if (asked) ui::error("playguard/play_timer/write_err"_i18n, NXM_RC_NOT_SAVED);
            return;
        }
        const Result rc = pctl_unlock_restriction_temporarily();
        if (R_FAILED(rc)) {
            settle_failed_unlock();
            brls::Logger::warning("extra time: not put back, the unlock failed ({:#x})", (unsigned)rc);
            if (asked) ui::error("playguard/play_timer/gate/failed"_i18n, rc);
            return;
        }
    }
    const Result rc = write_days(days, source);
    if (R_SUCCEEDED(rc)) clear();
    // The unlock was this function's own, never the user's: locked again
    // whatever the "lock again automatically" preference says.
    Result relock = 0;
    if (unlock) {
        relock = pctl_relock();
        if (R_SUCCEEDED(relock)) set_relock_pending(false);
    }
    const std::string done =
        brls::getStr("playguard/dashboard/extra_restored", ui::day_name_in_text(wd), ui::fmt_minutes(rec.base));
    if (R_FAILED(relock)) {
        // Said even at start-up: the console may be left unlocked.
        ui::error("playguard/toast/relock_err"_i18n, relock,
                  R_SUCCEEDED(rc) ? done : "playguard/play_timer/write_err"_i18n + " — " + ui::rc_text(rc, false));
    } else if (R_SUCCEEDED(rc)) {
        ui::notify(done);
    } else {
        brls::Logger::warning("extra time: not put back ({:#x})", (unsigned)rc);
        if (asked) ui::notify_result(rc, "", "playguard/play_timer/write_err"_i18n);
    }
    if (refresh) refresh();
}

// A bonus planned for a day now over: its limit put back by itself (it only
// goes down), once no extra-time record waits on its own day's limit.
static void offer_bonus_restore(std::function<void()> refresh, bool asked)
{
    const pt_logic::BonusRecord bonus = bonus_record();
    if (bonus.weekday < 0 || app::read_only() || console_lock::active()) return;
    s_restore_retry = false;
    PtState pt;
    pctl_play_timer_query(&pt);
    switch (pt_logic::bonus_action(bonus, ui::today_date(), pt, app::read_only())) {
        case pt_logic::Restore::None:   return;
        case pt_logic::Restore::Later:  s_restore_retry = pt.fw_supported; return;
        case pt_logic::Restore::Forget: clear_bonus_record(); return;   // changed since
        case pt_logic::Restore::Offer:  break;
    }
    pt_logic::ExtraRecord rec;
    rec.weekday = bonus.weekday;
    rec.date    = bonus.date;
    rec.base    = bonus.base;
    rec.value   = bonus.value;
    // On its weekday again (a week on) and below the time played: the next day.
    if (pt_logic::restore_way(rec, pt, ui::today_weekday(), false) == pt_logic::RestoreWay::Wait) return;
    put_back_by_itself(rec, pt, asked, refresh, clear_bonus_record, "restore_bonus");
}

void offer_extra_time_restore(std::function<void()> refresh, bool asked)
{
    const pt_logic::ExtraRecord rec = extra_record();
    // No extra time to put back (yet): a bonus planned may be.
    if (rec.weekday < 0 || rec.weekday > 6 || !pt_logic::day_over(rec.date, ui::today_date())) {
        offer_bonus_restore(refresh, asked);
        return;
    }
    // Kept for when the firmware screen leaves read-only mode.
    if (app::read_only()) {
        if (asked) ui::refuse_read_only();
        return;
    }
    // The lock holds every day at 0; the record waits for its limits to come back.
    if (console_lock::active()) return;

    PtState pt;
    pctl_play_timer_query(&pt);
    s_restore_retry = false;
    switch (pt_logic::restore_action(rec, ui::today_date(), pt, app::read_only())) {
        case pt_logic::Restore::None:   return;
        case pt_logic::Restore::Later:                                  // not read: again soon
            s_restore_retry = pt.fw_supported;
            if (asked) ui::refuse(pt.fw_supported ? NXM_RC_STATE_UNKNOWN : NXM_RC_FW_UNSUPPORTED);
            return;
        case pt_logic::Restore::Forget:                                 // changed since
            clear_extra_record();
            offer_bonus_restore(refresh, asked);
            return;
        case pt_logic::Restore::Offer:  break;
    }
    const int wd = rec.weekday;
    const uint16_t base = rec.base, value = rec.value;
    switch (pt_logic::restore_way(rec, pt, ui::today_weekday(), config::get().extra_auto_restore)) {
        case pt_logic::RestoreWay::ByItself:
            put_back_by_itself(rec, pt, asked, refresh);
            if (config::get().extra_weekday < 0) offer_bonus_restore(refresh, asked);   // put back: the bonus next
            return;
        case pt_logic::RestoreWay::Wait:
            // Asked for: as any change, the suspend warning first (and the PIN).
            if (asked) restore_extra(wd, base, value, refresh);
            return;
        case pt_logic::RestoreWay::Direct:   // no question (the unlock and the PIN, if needed, still ask)
            restore_extra(wd, base, value, refresh);
            return;
        case pt_logic::RestoreWay::Ask:
            break;
    }
    // Lowered for that day only (no more play): keeping that is stricter
    // still; putting the limit back raises it, through the PIN as any change.
    auto* dialog = ui::dialog(brls::getStr("playguard/dashboard/stop_restore_body", rec.date, ui::day_name_in_text(wd),
                                           ui::fmt_minutes(value), ui::fmt_minutes(base)));
    dialog->addButton(brls::getStr("playguard/dashboard/extra_keep", ui::fmt_minutes(value)), [refresh]() {
        clear_extra_record();
        if (refresh) refresh();
    });
    dialog->addButton("playguard/dashboard/extra_restore"_i18n, [wd, base, value, refresh]() {
        brls::sync([wd, base, value, refresh]() { restore_extra(wd, base, value, refresh); });
    });
    // B: decide later (the record stays; Today keeps a line for it).
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    dialog->open();
}

void retry_extra_time_restore()
{
    if (!s_restore_retry) return;
    offer_extra_time_restore();
}

}   // namespace pt_flow
