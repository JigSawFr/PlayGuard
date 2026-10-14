// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/pt_logic.hpp"

#include <algorithm>

namespace pt_logic
{

bool state_known(const PtState& pt)
{
    return pt.valid && pt.enabled_valid && pt.restricted_valid && pt.temporary_unlocked_valid;
}

bool needs_unlock(const PtState& pt)
{
    return (pt.enabled || pt.restricted) && !pt.temporary_unlocked;
}

int played_today_min(const PtState& pt, int weekday)
{
    if (!pt.valid || !pt.enabled_valid || !pt.enabled || weekday < 0 || weekday > 6) return -1;
    const uint16_t limit = pt.day_min[weekday];
    if (limit == PT_DAY_NOLIMIT) return -1;
    // The time over the limit first: restricted can read true with it (a
    // limit passed while unlocked).
    const int over = over_limit_min(pt, weekday);
    if (over >= 0) return limit + over;
    if (pt.restricted_valid && pt.restricted) return limit;
    // The remaining time reads 0 until a game has been counted today.
    if (!pt.remaining_valid || pt.remaining_ns <= 0) return -1;
    const uint64_t left = (uint64_t)pt.remaining_ns / 60000000000ULL;
    return left >= limit ? 0 : (int)(limit - left);
}

int over_limit_min(const PtState& pt, int weekday)
{
    if (!pt.valid || !pt.enabled_valid || !pt.enabled || weekday < 0 || weekday > 6) return -1;
    if (pt.day_min[weekday] == PT_DAY_NOLIMIT || !pt.remaining_valid || pt.remaining_ns >= 0) return -1;
    // Negated as unsigned: INT64_MIN has no positive s64.
    const uint64_t over_ns = (uint64_t)0 - (uint64_t)pt.remaining_ns;
    return (int)std::min<uint64_t>(1440, (over_ns + 30000000000ULL) / 60000000000ULL);
}

int suspend_warning_min(const PtState& pt, int weekday, const uint16_t new_days[7])
{
    if (!new_days || weekday < 0 || weekday > 6 || pt.alarm_only) return -1;
    const int played = played_today_min(pt, weekday);
    const uint16_t next = new_days[weekday];
    return played > 0 && next != PT_DAY_NOLIMIT && next < played ? played : -1;
}

bool can_add_extra_time(const PtState& pt, int weekday, bool read_only)
{
    if (read_only || weekday < 0 || weekday > 6) return false;
    return pt.fw_supported && pt.valid && pt.enabled_valid && pt.enabled &&
           pt.day_min[weekday] != PT_DAY_NOLIMIT && pt.day_min[weekday] < 1440;
}

ExtraPlan plan_extra(uint16_t base, uint16_t extra, bool again, uint16_t recorded_base)
{
    return { (uint16_t)std::min<int>(1440, base + extra), again ? recorded_base : base };
}

bool can_stop_today(const PtState& pt, int weekday, bool read_only)
{
    if (read_only || weekday < 0 || weekday > 6) return false;
    return pt.fw_supported && pt.valid && pt.day_min[weekday] != 0;
}

ExtraPlan plan_stop(uint16_t base, bool again, uint16_t recorded_base)
{
    return { 0, again ? recorded_base : base };
}

int stop_soon_limit(const PtState& pt, int weekday, int minutes)
{
    const int played = played_today_min(pt, weekday);
    if (played < 0 || minutes <= 0) return -1;
    const int value = played + minutes;
    return value < pt.day_min[weekday] ? value : -1;
}

Restore restore_action(const ExtraRecord& rec, const std::string& today, const PtState& pt, bool read_only)
{
    if (rec.weekday < 0 || rec.weekday > 6) return Restore::None;
    if (rec.date == today) return Restore::None;           // still the day it was added
    if (read_only || !pt.fw_supported || !pt.valid) return Restore::Later;
    if (pt.day_min[rec.weekday] != rec.value) return Restore::Forget;
    return Restore::Offer;
}

static bool reported(const PtState& pt, const PtBedtime& b)
{
    return b.on == pt.bedtime_enabled && (!b.on || (b.hour == pt.bedtime_hour && b.minute == pt.bedtime_minute));
}

bool bedtime_layout_ok(const PtState& pt, int weekday)
{
    if (weekday < 0 || weekday > 6 || !pt.fw_supported || !pt.valid || !pt.bedtime_valid) return false;
    return reported(pt, pt.bed[weekday]) || reported(pt, pt.bed[(weekday + 6) % 7]);
}

static bool same(const PtBedtime& a, const PtBedtime& b)
{
    return a.on == b.on && (!a.on || (a.hour == b.hour && a.minute == b.minute && a.end_hour == b.end_hour &&
                                      a.end_minute == b.end_minute));
}

bool bedtime_uniform(const PtState& pt, PtBedtime* out)
{
    if (!pt.valid) return false;
    for (int n = 1; n < 7; n++)
        if (!same(pt.bed[n], pt.bed[0])) return false;
    if (out) *out = pt.bed[0];
    return true;
}

static bool end_in_range(uint8_t hour, uint8_t minute)
{
    const int t = hour * 60 + minute;
    return minute < 60 && t >= 5 * 60 && t <= 9 * 60;
}

void bedtime_every_day(const PtState& pt, bool on, uint8_t hour, uint8_t minute, PtBedtime out[7])
{
    for (int n = 0; n < 7; n++) {
        PtBedtime b = pt.bed[n];
        b.on = on;
        b.hour = on ? hour : 0;
        b.minute = on ? minute : 0;
        if (on && !end_in_range(b.end_hour, b.end_minute)) {
            b.end_hour = 6;
            b.end_minute = 0;
        }
        out[n] = b;
    }
}

bool bedtime_end_every_day(const PtState& pt, uint8_t hour, uint8_t minute, PtBedtime out[7])
{
    bool any = false;
    for (int n = 0; n < 7; n++) any = any || pt.bed[n].on;
    if (!any) return false;
    for (int n = 0; n < 7; n++) {
        out[n] = pt.bed[n];
        if (out[n].on) {
            out[n].end_hour = hour;
            out[n].end_minute = minute;
        }
    }
    return true;
}

const std::vector<uint16_t>& quick_values()
{
    static const std::vector<uint16_t> values = { 30, 45, 60, 90, 120, 180, 240, 0 };
    return values;
}

UniformPick uniform_pick(const PtState& pt)
{
    bool uniform = pt.valid;
    for (int i = 1; i < 7 && uniform; i++) uniform = pt.day_min[i] == pt.day_min[0];
    const auto& values = quick_values();
    int selected = -1;
    for (size_t i = 0; i < values.size(); i++)
        if (uniform && pt.day_min[0] == values[i]) selected = (int)i;
    // Days that differ (or a value outside the list): start on "Custom…"
    // rather than pretending the first value is the current one.
    if (selected < 0) selected = (int)values.size();
    const uint16_t seed = (pt.valid && pt.day_min[0] != PT_DAY_NOLIMIT) ? pt.day_min[0] : 60;
    return { uniform, selected, seed };
}

LimitPick limit_pick(uint16_t current)
{
    const auto& values = quick_values();
    LimitPick p;
    p.selected = -1;
    for (size_t i = 0; i < values.size(); i++)
        if (values[i] == current) p.selected = (int)i;
    p.custom   = (int)values.size();
    p.no_limit = p.custom + 1;
    if (current == PT_DAY_NOLIMIT) p.selected = p.no_limit;
    else if (p.selected < 0) p.selected = p.custom;
    return p;
}

uint16_t limit_seed(uint16_t current)
{
    return current == PT_DAY_NOLIMIT ? 60 : current;
}

// The row of `t` in `times`, else `fallback`.
static int index_of(const std::vector<int>& times, int t, int fallback)
{
    for (size_t i = 0; i < times.size(); i++)
        if (times[i] == t) return (int)i;
    return fallback;
}

std::vector<int> bedtime_times()
{
    std::vector<int> times = { -1 };
    for (int t = 16 * 60; t < 24 * 60; t += 15) times.push_back(t);
    return times;
}

int bedtime_selected(const PtState& pt, const std::vector<int>& times)
{
    PtBedtime now;
    if (bedtime_uniform(pt, &now) && now.on) return index_of(times, now.hour * 60 + now.minute, 0);
    if (!bedtime_uniform(pt, nullptr)) return index_of(times, 21 * 60, 0);   // days differ: start on 21:00
    return 0;
}

std::vector<int> bedtime_end_times()
{
    std::vector<int> times;
    for (int t = 5 * 60; t <= 9 * 60; t += 15) times.push_back(t);
    return times;
}

int bedtime_end_selected(const PtState& pt, const std::vector<int>& times)
{
    const int usual = index_of(times, 6 * 60, 0);
    PtBedtime now;
    if (!bedtime_uniform(pt, &now)) return usual;
    return index_of(times, now.end_hour * 60 + now.end_minute, usual);
}

bool recorded_today(const ExtraRecord& rec, int weekday, const std::string& today)
{
    return rec.weekday == weekday && rec.date == today;
}

int extra_today_min(const ExtraRecord& rec, const std::string& today, int weekday, const PtState& pt)
{
    if (weekday < 0 || weekday > 6 || !recorded_today(rec, weekday, today) || !pt.valid ||
        pt.day_min[weekday] != rec.value || rec.value <= rec.base)
        return 0;
    return rec.value - rec.base;
}

bool pending_relock_needs_state(bool pending, bool this_run, bool read_only)
{
    return pending && !this_run && !read_only;
}

PendingRelock pending_relock(bool pending, bool this_run, bool read_only, bool state_read, bool unlocked)
{
    if (!pending_relock_needs_state(pending, this_run, read_only)) return PendingRelock::None;
    if (!state_read) return PendingRelock::Keep;
    return unlocked ? PendingRelock::Lock : PendingRelock::Forget;
}

bool alarm_off(const PtState& pt)
{
    return pt.fw_supported && pt.enabled_valid && pt.enabled && pt.alarm_disabled_valid && pt.alarm_disabled;
}

}   // namespace pt_logic
