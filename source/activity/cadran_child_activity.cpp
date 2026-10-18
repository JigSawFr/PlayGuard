// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/cadran_child_activity.hpp"

#include <fmt/format.h>

#include "action/frame_stats_flow.hpp"
#include "action/pin_lock.hpp"
#include "activity/main_activity.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

void CadranChildActivity::onContentAvailable()
{
    frame_stats_flow::set_screen("activity/cadran_child");
    auto* frame = dynamic_cast<brls::AppletFrame*>(this->getContentView());
    if (frame) frame->addView(cadran::make_glow(640.0f, 330.0f, 330.0f, "cadran/glow_strong"), 0);
    cadran::style_header(frame, "lock", "playguard/cadran/child/header"_i18n, false, false);

    ring->setShowLeft(true);
    ring->setValueSize(92);
    ring->setCaptionSize(20);

    // The screen itself takes the focus (highlight hidden): otherwise the
    // previous screen's focused button keeps its ring, drawn over this one.
    if (brls::View* root = this->getView("child_root")) {
        root->setHideHighlight(true);
        brls::Application::giveFocus(root);
    }
    // Back to the parent's screens only with the PIN (none on the console: at once).
    this->getContentView()->registerAction("playguard/cadran/child/back"_i18n, brls::BUTTON_A, [](brls::View*) {
        if (!pin_lock::ask()) {
            ui::notify(pin_lock::refusal_text());
            return true;
        }
        brls::Application::popActivity();
        return true;
    });
    this->getContentView()->registerAction("playguard/cadran/child/play"_i18n, brls::BUTTON_B, [](brls::View*) {
        MainActivity::quit_or_relock();
        return true;
    });

    this->refresh();
    this->timer.setPeriod(5000);
    this->timer.setCallback([this]() { this->refresh(); });
    this->timer.start();
}

void CadranChildActivity::refresh()
{
    PtState pt;
    pctl_play_timer_query(&pt);
    const int today = ui::today_weekday();
    float frac = -1.0f;
    std::string left = "—", caption = "";
    if (pt.fw_supported && pt.valid && pt.enabled_valid) {
        const uint16_t limit = pt.day_min[today];
        if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            left = "playguard/cadran/today/free"_i18n;
            caption = "playguard/cadran/today/no_limit"_i18n;
        } else if (pt.restricted_valid && pt.restricted) {
            frac = 1.0f;
            left = ui::fmt_played(0);
            caption = brls::getStr("playguard/cadran/child/of", ui::fmt_minutes(limit)) + " · " + "playguard/cadran/today/time_up"_i18n;
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left_min = pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left_min >= limit ? 0 : (uint16_t)(limit - left_min);
            frac = (float)used / (float)limit;
            left = ui::fmt_duration_ns(pt.remaining_ns);
            caption = brls::getStr("playguard/cadran/child/of", ui::fmt_minutes(limit));
        } else {
            frac = 0.0f;
            left = ui::fmt_minutes(limit);
            caption = "playguard/cadran/today/today"_i18n;
        }
    }
    ring->setFraction(frac);
    ring->setValue(left);
    ring->setCaption(caption);
    const bool bed = pt.bedtime_valid && pt.bedtime_enabled;
    ui::set_visible(bed_chip.getView(), bed);
    if (bed) bedtime->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
    // What the console does then (When the time is up).
    time_up->setText(pt.valid && pt.alarm_only ? "playguard/cadran/child/warns"_i18n : "playguard/cadran/child/stops"_i18n);
}
