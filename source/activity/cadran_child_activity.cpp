// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/cadran_child_activity.hpp"

#include <fmt/format.h>

#include "action/frame_stats_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "activity/main_activity.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
void keep_size(brls::View* view)
{
    if (auto* label = dynamic_cast<brls::Label*>(view)) label->setFontScaled(false);
    if (auto* box = dynamic_cast<brls::Box*>(view))
        for (brls::View* child : box->getChildren()) keep_size(child);
}

constexpr int LAST_MINUTES = 5;   // the end chip in amber, the ring beating
constexpr int BED_SOON     = 30;  // the bedtime chip in amber from then on
}   // namespace

void CadranChildActivity::onContentAvailable()
{
    frame_stats_flow::set_screen("activity/cadran_child");
    auto* frame = dynamic_cast<brls::AppletFrame*>(this->getContentView());
    if (frame) frame->addView(cadran::make_glow(640.0f, 330.0f, 330.0f, "cadran/glow_strong"), 0);
    cadran::style_header(frame, "lock", "playguard/cadran/child/header"_i18n, false, false);

    ring->setShowLeft(true);
    ring->setValueSize(96);
    ring->setCaptionSize(22);
    // Already sized for the room: the larger text leaves it as it is.
    if (brls::View* root = this->getView("child_root")) keep_size(root);

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
    // Extra time from here, the parent by the child: the PIN, then the
    // amounts, and the child screen stays.
    const auto extra = [this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        PtState pt;
        pctl_play_timer_query(&pt);
        const int wd = ui::today_weekday();
        if (!pt.fw_supported || !pt.valid || pt.day_min[wd] == PT_DAY_NOLIMIT || ui::known_unlocked()) {
            ui::notify("playguard/cadran/child/no_extra"_i18n);
            return true;
        }
        if (!pin_lock::ask()) {
            ui::notify(pin_lock::refusal_text());
            return true;
        }
        pt_flow::add_extra_time(pt, [this]() { this->refresh(); });
        return true;
    };
    this->getContentView()->registerAction("playguard/cadran/child/extra"_i18n, brls::BUTTON_Y, extra);
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    // The desktop build: the keyboard has no Ⓨ, so the Y key (as the main screen).
    this->getContentView()->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Y), extra);
#endif

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
    int left_min = -1;   // counting down: the minutes left
    if (pt.fw_supported && pt.valid && pt.enabled_valid) {
        const uint16_t limit = pt.day_min[today];
        // Unlocked first, as on Today: the console reports the timer off
        // while unlocked, though today's limit still stands.
        const bool unlocked = ui::known_unlocked() && limit != PT_DAY_NOLIMIT;
        if (limit == 0 || unlocked) {
            frac = unlocked ? -1.0f : 1.0f;
            left = limit == 0 ? "playguard/cadran/today/no_play"_i18n : ui::fmt_minutes(limit);
            caption = unlocked ? "playguard/cadran/child/on_hold"_i18n : "playguard/cadran/today/today"_i18n;
        } else if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            left = "playguard/cadran/today/free"_i18n;
            caption = "playguard/cadran/today/no_limit"_i18n;
        } else if (pt.restricted_valid && pt.restricted) {
            frac = 1.0f;
            left = ui::fmt_played(0);
            caption = brls::getStr("playguard/cadran/child/of", ui::fmt_minutes(limit)) + " · " + "playguard/cadran/today/time_up"_i18n;
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            left_min = (int)(pt.remaining_ns / 60000000000ULL);
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
    // When it ends, by the clock, as Today says it (not after the bedtime
    // alarm: its chip says when); amber and the ring beating near the end.
    const LocalTime now = ui::local_now();
    const pt_logic::NextStop n = pt_logic::next_stop(now.hour * 60 + now.minute, left_min, pt.bed[today]);
    const bool ending = n.kind == pt_logic::NextStop::TIME_UP;
    const bool last = ending && n.in <= LAST_MINUTES;
    if (ending) ui::set_text(end_text, brls::getStr("playguard/cadran/child/end", fmt::format("{:02d}:{:02d}", n.at / 60, n.at % 60)));
    brls::Theme theme = brls::Application::getTheme();
    end_icon->setColor(theme[last ? "cadran/amber" : "cadran/teal"]);
    end_text->setTextColor(theme[last ? "cadran/amber" : "cadran/text"]);
    ui::set_visible(end_chip.getView(), ending);
    ring->setPulse(last);
    const bool bed = pt.bedtime_valid && pt.bedtime_enabled;
    ui::set_visible(bed_chip.getView(), bed);
    if (bed) {
        bedtime->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
        // Amber only once it is near: before, nothing calls for attention.
        const int until = (pt.bedtime_hour * 60 + pt.bedtime_minute - (now.hour * 60 + now.minute) + 24 * 60) % (24 * 60);
        const bool soon = until <= BED_SOON;
        bed_icon->setColor(theme[soon ? "cadran/amber" : "cadran/text2"]);
        bedtime->setTextColor(theme[soon ? "cadran/amber" : "cadran/text"]);
    }
    // What the console does then (When the time is up).
    time_up->setText(pt.valid && pt.alarm_only ? "playguard/cadran/child/warns"_i18n : "playguard/cadran/child/stops"_i18n);
}
