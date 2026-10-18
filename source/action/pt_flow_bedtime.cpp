// pt_flow: the bedtime, its text, its pickers and its write.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <array>
#include <borealis.hpp>
#include <fmt/format.h>

#include "action/history_flow.hpp"
#include "action/pt_logic.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace pt_flow
{

static std::string hm(int hour, int minute)
{
    return fmt::format("{:02d}:{:02d}", hour, minute);
}

static std::string bed_text(const PtBedtime& b)
{
    return b.on ? brls::getStr("playguard/play_timer/bedtime_summary", hm(b.hour, b.minute), hm(b.end_hour, b.end_minute))
                : "playguard/common/off"_i18n;
}

std::string bedtime_text(const PtState& pt)
{
    PtBedtime b;
    if (!pt_logic::bedtime_uniform(pt, &b)) return "playguard/play_timer/bedtime_varies"_i18n;
    return bed_text(b);
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
        ui::notify(ui::rc_text(NXM_RC_FW_UNSUPPORTED), ui::Toast::Refused);
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

void choose_bedtime(const PtState& pt, std::function<void()> refresh)
{
    if (!bedtime_editable(pt)) return;
    // Off, then 16:00 to 23:45 every quarter of an hour.
    const std::vector<int> times = pt_logic::bedtime_times();
    std::vector<std::string> labels;
    for (int t : times) labels.push_back(t < 0 ? "playguard/common/off"_i18n : hm(t / 60, t % 60));
    const int selected = pt_logic::bedtime_selected(pt, times);
    const PtState copy = pt;
    ui::pick("playguard/play_timer/bedtime_title"_i18n, labels, selected, [copy, times, refresh](int index) {
        const int t = times[(size_t)index];
        std::array<PtBedtime, 7> bed;
        pt_logic::bedtime_every_day(copy, t >= 0, (uint8_t)(t >= 0 ? t / 60 : 0), (uint8_t)(t >= 0 ? t % 60 : 0), bed.data());
        const std::string body = t < 0 ? "playguard/play_timer/bedtime_confirm_off"_i18n
                                       : brls::getStr("playguard/play_timer/bedtime_confirm", hm(t / 60, t % 60),
                                                      hm(bed[0].end_hour, bed[0].end_minute));
        apply_bedtime(bed, body, refresh);
    });
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
    std::vector<std::string> labels;
    for (int t : times) labels.push_back(hm(t / 60, t % 60));
    const int selected = pt_logic::bedtime_end_selected(pt, times);
    const PtState copy = pt;
    ui::pick("playguard/play_timer/bedtime_end_title"_i18n, labels, selected, [copy, times, refresh](int index) {
        const int t = times[(size_t)index];
        std::array<PtBedtime, 7> bed;
        pt_logic::bedtime_end_every_day(copy, (uint8_t)(t / 60), (uint8_t)(t % 60), bed.data());
        apply_bedtime(bed, brls::getStr("playguard/play_timer/bedtime_end_confirm", hm(t / 60, t % 60)), refresh);
    });
}

}   // namespace pt_flow
