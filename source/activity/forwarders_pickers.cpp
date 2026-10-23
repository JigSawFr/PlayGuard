// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/forwarders_pickers.hpp"

#include <algorithm>
#include <borealis.hpp>
#include <fmt/format.h>
#include <vector>

#include "ui/ui.hpp"

using namespace brls::literals;
using forwarders_logic::Row;

namespace forwarders_pickers
{

void span(const std::string& title, int from_hour, int to_hour, std::weak_ptr<bool> alive,
          std::function<void(int, int)> done)
{
    std::weak_ptr<bool> weak = alive;
    std::vector<std::string> starts, ends;
    for (int h = 0; h < 24; h++) starts.push_back(fmt::format("{:02}:00", h));
    for (int h = 1; h <= 24; h++) ends.push_back(fmt::format("{:02}:00", h));
    const std::string from_title = title.empty() ? "playguard/forwarders/hours_from"_i18n
                                                 : brls::getStr("playguard/forwarders/hours_from_of", title);
    const std::string to_title = title.empty() ? "playguard/forwarders/hours_to"_i18n
                                               : brls::getStr("playguard/forwarders/hours_to_of", title);
    ui::pick(from_title, starts, from_hour, [weak, ends, to_title, to_hour, done](int from) {
        if (weak.expired() || from < 0 || from > 23) return;
        brls::sync([weak, ends, to_title, to_hour, from, done]() {
            ui::pick(to_title, ends, to_hour, [weak, from, done](int to) {
                if (weak.expired() || to < 0 || to > 23) return;
                const int end = (to + 1) * 60, start = from * 60;
                if (end == start) return;
                done(start, end);
            });
        });
    });
}

void hours(const Row& row, bool closed, std::weak_ptr<bool> alive, std::function<void(const forwarder_flow::Hours&)> set)
{
    const Row r = row;
    const int cur_days = closed ? r.closed_days : r.free_days, cur_from = closed ? r.closed_from : r.free_from,
              cur_to = closed ? r.closed_to : r.free_to;
    // Never, every day, the school days, the weekend; free hours, the school
    // days and the weekend apart too.
    static const int DAYS[] = { 0, 0x7F, 0x3E, 0x41 };
    std::vector<std::string> names = { "playguard/forwarders/hours_never"_i18n, "playguard/forwarders/hours_every_day"_i18n,
                                       "playguard/forwarders/hours_weekdays"_i18n, "playguard/forwarders/hours_weekend"_i18n };
    if (!closed) names.push_back("playguard/forwarders/hours_apart"_i18n);
    int current = 0;
    for (int i = 1; i < 4; i++)
        if (cur_days == DAYS[i]) current = i;
    if (!closed && r.free2_days) current = 4;
    std::weak_ptr<bool> weak = alive;
    ui::pick(closed ? "playguard/forwarders/closed_title"_i18n : "playguard/forwarders/hours_title"_i18n, names, current,
             [weak, r, closed, cur_days, cur_from, cur_to, set](int index) {
        if (weak.expired() || index < 0 || index > (closed ? 3 : 4)) return;
        if (index == 0) {
            set({});
            return;
        }
        // Free: 09:00 to 20:00 at first; closed: 21:00 to 07:00.
        const int from_now = cur_days ? cur_from / 60 : closed ? 21 : 9;
        const int to_now = cur_days ? std::max(0, cur_to / 60 - 1) : closed ? 6 : 19;
        if (index == 4) {
            // The school days (16:00 to 19:00 at first), then the weekend (9:00 to 20:00).
            const bool had = r.free2_days != 0;
            const int wd_from = had ? r.free_from / 60 : 16, wd_to = had ? std::max(0, r.free_to / 60 - 1) : 18;
            const int we_from = had ? r.free2_from / 60 : 9, we_to = had ? std::max(0, r.free2_to / 60 - 1) : 19;
            brls::sync([weak, wd_from, wd_to, we_from, we_to, set]() {
                span("playguard/forwarders/hours_weekdays"_i18n, wd_from, wd_to, weak,
                     [weak, we_from, we_to, set](int from, int to) {
                    brls::sync([weak, from, to, we_from, we_to, set]() {
                        if (weak.expired()) return;
                        span("playguard/forwarders/hours_weekend"_i18n, we_from, we_to, weak,
                             [weak, from, to, set](int from2, int to2) {
                            if (weak.expired()) return;
                            set({ 0x3E, from, to, 0x41, from2, to2 });
                        });
                    });
                });
            });
            return;
        }
        const int days = DAYS[index];
        brls::sync([weak, days, from_now, to_now, set]() {
            span("", from_now, to_now, weak, [weak, days, set](int from, int to) {
                if (weak.expired()) return;
                set({ days, from, to });
            });
        });
    });
}

void pause(std::weak_ptr<bool> alive, std::function<void()> done)
{
    // Never, then tries in a row and how long: the default (5, 15 min) among them.
    static const unsigned TRIES[] = { 0, 3, 5, 5, 10 }, MINUTES[] = { 0, 15, 15, 60, 60 };
    const GatePauseRule now = forwarder_flow::pause_rule();
    std::vector<std::string> names;
    int current = -1;
    for (size_t i = 0; i < 5; i++) {
        names.push_back(forwarder_flow::pause_label(TRIES[i], MINUTES[i]));
        if (now.tries == TRIES[i] && (!now.tries || now.minutes == MINUTES[i])) current = (int)i;
    }
    std::weak_ptr<bool> weak = alive;
    ui::pick("playguard/forwarders/pause_cell"_i18n, names, current, [weak, done](int index) {
        if (weak.expired() || index < 0 || index > 4) return;
        forwarder_flow::set_pause_rule(TRIES[index], MINUTES[index], [weak, done](bool) {
            brls::sync([weak, done]() {
                if (!weak.expired()) done();
            });
        });
    }, true, { "", "", "playguard/forwarders/pause_default"_i18n, "", "" });
}

void wait(std::weak_ptr<bool> alive, std::function<void()> done)
{
    // Off, then seconds: the default (2 s, as PlayGuard's intro) among them.
    static const unsigned MS[] = { 0, 1000, 2000, 3000, 5000 };
    const unsigned now = forwarder_flow::launch_wait();
    std::vector<std::string> names;
    int current = -1;
    for (size_t i = 0; i < 5; i++) {
        names.push_back(forwarder_flow::wait_label(MS[i]));
        if (now == MS[i]) current = (int)i;
    }
    std::weak_ptr<bool> weak = alive;
    ui::pick("playguard/forwarders/wait_cell"_i18n, names, current, [weak, done](int index) {
        if (weak.expired() || index < 0 || index > 4) return;
        forwarder_flow::set_launch_wait(MS[index], [weak, done](bool) {
            brls::sync([weak, done]() {
                if (!weak.expired()) done();
            });
        });
    }, true, { "", "", "playguard/forwarders/pause_default"_i18n, "", "" });
}

}   // namespace forwarders_pickers
