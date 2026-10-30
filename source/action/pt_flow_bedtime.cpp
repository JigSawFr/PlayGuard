// pt_flow: the bedtime, its text, its pickers and its write.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <array>
#include <format>
#include <aurore.hpp>

#include <aurore/cadran/ui/cadran_modal.hpp>

#include "action/history_flow.hpp"
#include "action/pt_logic.hpp"
#include "ui/ui.hpp"
#include "util/date.hpp"

using namespace aurore::literals;

namespace pt_flow
{

static std::string bed_text(const PtBedtime& b)
{
    return b.on ? aurore::getStr("playguard/play_timer/bedtime_summary", date::hh_mm(b.hour, b.minute), date::hh_mm(b.end_hour, b.end_minute))
                : "playguard/common/off"_i18n;
}

std::string bedtime_days_text(const PtState& pt)
{
    // Up to three runs of days, each with its alarm: "Mon–Fri 21:30, Sat–Sun 22:30".
    const std::vector<pt_logic::BedRun> runs = pt_logic::bedtime_runs(pt);
    if (runs.size() < 2 || runs.size() > 3) return "playguard/play_timer/bedtime_varies"_i18n;
    std::string text;
    for (const pt_logic::BedRun& r : runs) {
        if (!text.empty()) text += ", ";
        text += ui::day_short(r.first);
        if (r.last != r.first) text += "–" + ui::day_short(r.last);
        text += " " + (r.bed.on ? date::hh_mm(r.bed.hour, r.bed.minute) : "playguard/common/off"_i18n);
    }
    return text;
}

std::string bedtime_text(const PtState& pt)
{
    PtBedtime b;
    return pt_logic::bedtime_uniform(pt, &b) ? bed_text(b) : bedtime_days_text(pt);
}

Result write_bedtime(const PtBedtime bed[7], const std::string& source)
{
    PtState before;
    pctl_play_timer_query(&before);
    const Result rc = pctl_play_timer_set_bedtime(bed, ui::today_weekday());
    if (R_SUCCEEDED(rc) && before.valid) {
        PtState after = before;
        for (int n = 0; n < 7; n++) after.bed[n] = bed[n];
        const std::string from = bedtime_text(before), to = bedtime_text(after);
        if (from != to) history_flow::record_event("bedtime", source, from + " → " + to);
    }
    if (R_FAILED(rc)) {
        PtState asked = before;
        for (int n = 0; n < 7; n++) asked.bed[n] = bed[n];
        history_flow::record_failure("bedtime", rc, {}, {}, source,
                                     before.valid ? bedtime_text(before) + " → " + bedtime_text(asked) : "");
    }
    return rc;
}

// The bedtime can be changed from this state; says why not otherwise.
static bool bedtime_editable(const PtState& pt)
{
    if (ui::refuse_read_only()) return false;
    if (!pt.fw_supported) {
        ui::refuse(NXM_RC_FW_UNSUPPORTED);
        return false;
    }
    if (!pt_logic::bedtime_layout_ok(pt, ui::today_weekday())) {
        ui::info("playguard/play_timer/bedtime_unknown"_i18n);
        return false;
    }
    return true;
}

static void apply_bedtime(const std::array<PtBedtime, 7>& bed, const std::string& body, std::function<void()> refresh)
{
    confirm_write(body, "playguard/play_timer/confirm_set"_i18n, [bed, refresh](bool did_unlock) {
        Result rc = write_bedtime(bed.data(), "");
        finish_write(rc, did_unlock, "playguard/play_timer/bedtime_written"_i18n,
                     "playguard/play_timer/bedtime_err"_i18n, refresh);
    });
}

// The alarm's times, 16:00 to 23:45 (pt_logic::bedtime_times without Off).
static std::vector<int> alarm_times()
{
    std::vector<int> times;
    for (int t : pt_logic::bedtime_times())
        if (t >= 0) times.push_back(t);
    return times;
}

// Where the stick starts for the days of `days`: their alarm when they share
// one, else 21:00 (pt_logic::bedtime_selected_days).
static int start_time(const PtState& pt, uint8_t days)
{
    const std::vector<int> times = pt_logic::bedtime_times();
    const int t = times[(size_t)pt_logic::bedtime_selected_days(pt, days, times)];
    return t >= 0 ? t : 21 * 60;
}

// The days a choice of "By day of the week…" stands for, and its name.
struct DaySet
{
    uint8_t     days;
    std::string name;
};

static std::vector<DaySet> day_sets()
{
    std::vector<DaySet> sets = { { pt_logic::WEEKDAYS, "playguard/play_timer/bedtime_weekdays"_i18n },
                                 { pt_logic::WEEKEND, "playguard/play_timer/bedtime_weekend"_i18n } };
    for (int i = 0; i < 7; i++) {
        const int d = (i + 1) % 7;   // Monday first
        sets.push_back({ (uint8_t)(1u << d), aurore::getStr(std::format("cadran/days/{}", d)) });
    }
    return sets;
}

// The bedtime set with the stick for the days of `days` (or Off), then
// confirm_write and write_bedtime; the other days keep theirs.
static void choose_bedtime_days(const PtState& pt, uint8_t days, const std::string& name, std::function<void()> refresh)
{
    const PtState copy = pt;
    const auto apply = [copy, days, name, refresh](int t) {
        std::array<PtBedtime, 7> bed;
        pt_logic::bedtime_days(copy, days, t >= 0, (uint8_t)(t >= 0 ? t / 60 : 0), (uint8_t)(t >= 0 ? t % 60 : 0), bed.data());
        int first = 0;
        while (!(days & (1u << first))) first++;
        const std::string body = t < 0 ? aurore::getStr("playguard/play_timer/bedtime_days_confirm_off", name)
                                       : aurore::getStr("playguard/play_timer/bedtime_days_confirm", name, date::hh_mm(t / 60, t % 60),
                                                      date::hh_mm(bed[first].end_hour, bed[first].end_minute));
        apply_bedtime(bed, body, refresh);
    };
    cadran::prompt_time(aurore::getStr("playguard/play_timer/bedtime_days_title", name), alarm_times(),
                        start_time(pt, days), { "playguard/common/off"_i18n, [apply]() { apply(-1); } }, {}, apply);
}

// "By day of the week…": Monday to Friday, the weekend or one day, each
// with what it has now, then its bedtime picker.
static void choose_bedtime_by_day(const PtState& pt, std::function<void()> refresh)
{
    const std::vector<DaySet> sets = day_sets();
    std::vector<std::string> labels, details;
    for (const DaySet& set : sets) {
        labels.push_back(set.name);
        PtBedtime b;
        details.push_back(!pt_logic::bedtime_shared(pt, set.days, &b) ? "playguard/play_timer/bedtime_varies"_i18n
                          : b.on ? date::hh_mm(b.hour, b.minute) : "playguard/common/off"_i18n);
    }
    const PtState copy = pt;
    ui::pick("playguard/play_timer/bedtime_by_day_title"_i18n, labels, 0, [copy, sets, refresh](int index) {
        const DaySet& set = sets[(size_t)index];
        choose_bedtime_days(copy, set.days, set.name, refresh);
    }, false, details);
}

void choose_bedtime(const PtState& pt, std::function<void()> refresh)
{
    if (!bedtime_editable(pt)) return;
    const PtState copy = pt;
    const auto apply = [copy, refresh](int t) {
        std::array<PtBedtime, 7> bed;
        pt_logic::bedtime_every_day(copy, t >= 0, (uint8_t)(t >= 0 ? t / 60 : 0), (uint8_t)(t >= 0 ? t % 60 : 0), bed.data());
        const std::string body = t < 0 ? "playguard/play_timer/bedtime_confirm_off"_i18n
                                       : aurore::getStr("playguard/play_timer/bedtime_confirm", date::hh_mm(t / 60, t % 60),
                                                      date::hh_mm(bed[0].end_hour, bed[0].end_minute));
        apply_bedtime(bed, body, refresh);
    };
    // Every day with the stick, Off, or by day of the week on Y.
    cadran::prompt_time("playguard/play_timer/bedtime_title"_i18n, alarm_times(), start_time(pt, pt_logic::ALL_DAYS),
                        { "playguard/common/off"_i18n, [apply]() { apply(-1); } },
                        { "playguard/play_timer/bedtime_by_day"_i18n, [copy, refresh]() { choose_bedtime_by_day(copy, refresh); } },
                        apply);
}

void choose_bedtime_end(const PtState& pt, std::function<void()> refresh)
{
    if (!bedtime_editable(pt)) return;
    std::array<PtBedtime, 7> probe;
    if (!pt_logic::bedtime_end_every_day(pt, 6, 0, probe.data())) {
        ui::info("playguard/play_timer/bedtime_end_needs_alarm"_i18n);
        return;
    }
    // 05:00 to 09:00 every quarter of an hour.
    const std::vector<int> times = pt_logic::bedtime_end_times();
    const int initial = times[(size_t)pt_logic::bedtime_end_selected(pt, times)];
    const PtState copy = pt;
    cadran::prompt_time("playguard/play_timer/bedtime_end_title"_i18n, times, initial, {}, {}, [copy, refresh](int t) {
        std::array<PtBedtime, 7> bed;
        pt_logic::bedtime_end_every_day(copy, (uint8_t)(t / 60), (uint8_t)(t % 60), bed.data());
        apply_bedtime(bed, aurore::getStr("playguard/play_timer/bedtime_end_confirm", date::hh_mm(t / 60, t % 60)), refresh);
    });
}

}   // namespace pt_flow
