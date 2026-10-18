// pt_flow: extra time today, no more play today, and putting the usual
// limit back afterwards (the record in config extra_*).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <borealis.hpp>

#include "action/console_lock.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow_internal.hpp"
#include "action/pt_logic.hpp"
#include "app.hpp"
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
static bool save_extra_record(int wd, uint16_t base, uint16_t value)
{
    auto& c = config::get();
    c.extra_weekday = wd;
    c.extra_date    = ui::today_date();
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

// A list of actions, not of values: no row is checked as the current one
// (borealis' Dropdown checks one, ui::pick the `selected`). The first row
// takes the focus.
class ActionList : public brls::Dropdown
{
  public:
    ActionList(const std::string& title, const std::vector<std::string>& values, std::function<void(int)> on_pick)
        : brls::Dropdown(
              title, values, [this](int index) { this->chosen = index; }, 0,
              [this, on_pick](int) {
                  if (this->chosen >= 0 && on_pick) {
                      const int index = this->chosen;
                      brls::sync([on_pick, index]() { on_pick(index); });
                  }
              }),
          labels(values)
    {
        // The rows Dropdown's constructor already made used its own
        // cellForRow (this one did not exist yet): unchecked here.
        uncheck(this);
    }

  private:
    std::vector<std::string> labels;
    int chosen = -1;

    static void uncheck(brls::View* view)
    {
        if (auto* cell = dynamic_cast<brls::RadioCell*>(view)) cell->setSelected(false);
        if (auto* box = dynamic_cast<brls::Box*>(view))
            for (brls::View* child : box->getChildren()) uncheck(child);
    }

    brls::RecyclerCell* cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath index) override
    {
        auto* cell = (brls::RadioCell*)recycler->dequeueReusableCell("Cell");
        cell->title->setText(this->labels[index.row]);
        cell->setSelected(false);
        return cell;
    }
};

// `extra` minutes more today, after the confirmation (the picker below, or a
// chip of the Cadran Overview).
static void add_extra(int wd, uint16_t base, uint16_t extra, std::function<void()> refresh)
{
    // What gets put back later is the limit before any extra time today.
    const pt_logic::ExtraRecord rec = extra_record();
    const bool again = pt_logic::recorded_today(rec, wd, ui::today_date());
    const pt_logic::ExtraPlan plan = pt_logic::plan_extra(base, extra, again, rec.base);
    const uint16_t value = plan.value, original = plan.original;
    // Put back by itself the next day, whatever the preference (pt_logic::restore_way).
    const std::string body = brls::getStr("playguard/dashboard/extra_body_auto", ui::fmt_minutes(extra),
                                          ui::day_name_in_text(wd), ui::fmt_minutes(base), ui::fmt_minutes(value),
                                          ui::fmt_minutes(original));
    confirm_write(body, "playguard/dashboard/extra_confirm"_i18n, [refresh, wd, base, value, original](bool did_unlock) {
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
        if (!save_extra_record(wd, original, value)) {
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
    const int wd = ui::today_weekday();
    if (!pt.valid || pt.day_min[wd] == PT_DAY_NOLIMIT) return;
    // Play timer › Extra time amounts.
    const std::vector<int> amounts = config::get().extra_amounts;
    std::vector<std::string> labels;
    for (int e : amounts) labels.push_back("+" + ui::fmt_minutes((uint16_t)e));
    const uint16_t base = pt.day_min[wd];
    auto* list = new ActionList("playguard/dashboard/extra_title"_i18n, labels, [refresh, wd, base, amounts](int index) {
        add_extra(wd, base, (uint16_t)amounts[index], refresh);
    });
    brls::Application::pushActivity(new brls::Activity(list));
}

void add_extra_minutes(const PtState& pt, uint16_t minutes, std::function<void()> refresh)
{
    if (refuse_console_lock()) return;
    const int wd = ui::today_weekday();
    if (!pt.valid || pt.day_min[wd] == PT_DAY_NOLIMIT) return;
    add_extra(wd, pt.day_min[wd], minutes, refresh);
}

// Today's limit to `value`: 0 (no more play now) or a few minutes more than
// played (pt_logic::stop_soon_limit). Put back the next day like extra time.
static void stop_at(const PtState& pt, uint16_t value, std::function<void()> refresh)
{
    const int wd = ui::today_weekday();
    const uint16_t base = pt.day_min[wd];
    const pt_logic::ExtraRecord rec = extra_record();
    const bool again = pt_logic::recorded_today(rec, wd, ui::today_date());
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
    confirm_write(body, "playguard/dashboard/stop_confirm"_i18n, [refresh, wd, base, original, value](bool did_unlock) {
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
        if (!save_extra_record(wd, original, value)) {
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
    // Suspending can lose unsaved progress: when the time played is known,
    // "in 5 minutes" leaves time to save.
    const int soon = pt_logic::stop_soon_limit(pt, ui::today_weekday(), STOP_SOON_MIN);
    if (soon < 0) {
        stop_at(pt, 0, refresh);
        return;
    }
    const PtState copy = pt;
    auto* list = new ActionList("playguard/dashboard/stop_title"_i18n,
                                { "playguard/dashboard/stop_now"_i18n, "playguard/dashboard/stop_soon"_i18n },
                                [copy, soon, refresh](int index) { stop_at(copy, index == 0 ? 0 : (uint16_t)soon, refresh); });
    brls::Application::pushActivity(new brls::Activity(list));
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

std::string extra_today_text(const PtState& pt)
{
    const int extra = pt_logic::extra_today_min(extra_record(), ui::today_date(), ui::today_weekday(), pt);
    return extra > 0 ? "+" + ui::fmt_minutes((uint16_t)extra) : "";
}

std::string restore_label()
{
    const pt_logic::ExtraRecord rec = extra_record();
    if (rec.weekday < 0 || rec.weekday > 6) return "";
    return brls::getStr("playguard/dashboard/extra_pending", ui::day_name_in_text(rec.weekday), ui::fmt_minutes(rec.base));
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
// `pt` was just read (restore_action said Offer). `asked`: from the Overview
// or the bell, where a failure is said; at start-up it is only logged, and the
// bell keeps its entry.
static void put_back_by_itself(const pt_logic::ExtraRecord& rec, const PtState& pt, bool asked,
                               std::function<void()> refresh)
{
    if (!pt_logic::state_known(pt)) {
        s_restore_retry = true;
        if (asked) ui::notify(ui::rc_text(NXM_RC_STATE_UNKNOWN));
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
            if (asked) ui::error("playguard/play_timer/write_err"_i18n + " — " + ui::rc_text(NXM_RC_NOT_SAVED));
            return;
        }
        const Result rc = pctl_unlock_restriction_temporarily();
        if (R_FAILED(rc)) {
            settle_failed_unlock();
            brls::Logger::warning("extra time: not put back, the unlock failed ({:#x})", (unsigned)rc);
            if (asked) ui::error("playguard/play_timer/gate/failed"_i18n + " — " + ui::rc_text(rc));
            return;
        }
    }
    const Result rc = write_days(days, "restore_extra");
    if (R_SUCCEEDED(rc)) clear_extra_record();
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
        const std::string text = R_SUCCEEDED(rc) ? done : "playguard/play_timer/write_err"_i18n + " — " + ui::rc_text(rc);
        ui::error(text + "\n\n" + "playguard/toast/relock_err"_i18n + " — " + ui::rc_text(relock));
    } else if (R_SUCCEEDED(rc)) {
        ui::notify(done);
    } else {
        brls::Logger::warning("extra time: not put back ({:#x})", (unsigned)rc);
        if (asked) ui::notify_result(rc, "", "playguard/play_timer/write_err"_i18n);
    }
    if (refresh) refresh();
}

void offer_extra_time_restore(std::function<void()> refresh, bool asked)
{
    const pt_logic::ExtraRecord rec = extra_record();
    if (rec.weekday < 0 || rec.weekday > 6 || rec.date == ui::today_date()) return;
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
            if (asked) ui::notify(ui::rc_text(pt.fw_supported ? NXM_RC_STATE_UNKNOWN : NXM_RC_FW_UNSUPPORTED));
            return;
        case pt_logic::Restore::Forget: clear_extra_record(); return;  // changed since
        case pt_logic::Restore::Offer:  break;
    }
    const int wd = rec.weekday;
    const uint16_t base = rec.base, value = rec.value;
    switch (pt_logic::restore_way(rec, pt, ui::today_weekday(), config::get().extra_auto_restore)) {
        case pt_logic::RestoreWay::ByItself:
            put_back_by_itself(rec, pt, asked, refresh);
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
    // B: decide later (the record stays; the Overview keeps a line for it).
    dialog->setCancelable(true);
    dialog->open();
}

void retry_extra_time_restore()
{
    if (!s_restore_retry) return;
    offer_extra_time_restore();
}

}   // namespace pt_flow
