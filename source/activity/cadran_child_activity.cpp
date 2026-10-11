// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/cadran_child_activity.hpp"

#include <fmt/format.h>

#include "action/frame_stats_flow.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"

void CadranChildActivity::onContentAvailable()
{
    frame_stats_flow::set_screen("activity/cadran_child");
    auto* frame = dynamic_cast<brls::AppletFrame*>(this->getContentView());
    if (frame) frame->addView(cadran::make_glow(640.0f, 330.0f, 330.0f, "cadran/glow_strong"), 0);
    cadran::style_header(frame, "lock", "Écran enfant · code PIN pour revenir", false, false);

    ring->setShowLeft(true);
    ring->setValueSize(92);
    ring->setCaptionSize(20);

    // The screen itself takes the focus (highlight hidden): otherwise the
    // previous screen's focused button keeps its ring, drawn over this one.
    if (brls::View* root = this->getView("child_root")) {
        root->setHideHighlight(true);
        brls::Application::giveFocus(root);
    }
    this->getContentView()->registerAction("Retour parent · code PIN", brls::BUTTON_A, [](brls::View*) {
        brls::Application::popActivity();
        return true;
    });
    this->getContentView()->registerAction("Rendre la console et jouer", brls::BUTTON_B, [](brls::View*) {
        brls::Application::quit();
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
            left = "libre";
            caption = "pas de limite aujourd'hui";
        } else if (pt.restricted_valid && pt.restricted) {
            frac = 1.0f;
            left = "0 min";
            caption = "sur " + ui::fmt_minutes(limit) + " · temps écoulé";
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left_min = pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left_min >= limit ? 0 : (uint16_t)(limit - left_min);
            frac = (float)used / (float)limit;
            left = ui::fmt_duration_ns(pt.remaining_ns);
            caption = "sur " + ui::fmt_minutes(limit);
        } else {
            frac = 0.0f;
            left = ui::fmt_minutes(limit);
            caption = "aujourd'hui";
        }
    }
    ring->setFraction(frac);
    ring->setValue(left);
    ring->setCaption(caption);
    const bool bed = pt.bedtime_valid && pt.bedtime_enabled;
    ui::set_visible(bed_chip.getView(), bed);
    if (bed) bedtime->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
}
