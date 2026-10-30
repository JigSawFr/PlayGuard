// DashboardTab — Today's cards: the changes seen outside PlayGuard, the first steps, the
// extra time to put back and the alarm turned off, the three state cards.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <format>

#include <aurore/cadran/ui/motion.hpp>
#include <aurore/cadran/util/text.hpp>

#include "action/clock_flow.hpp"
#include "action/console_actions.hpp"
#include "action/nintendo_dns.hpp"
#include "action/outside_watch.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_logic.hpp"
#include "app.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

void DashboardTab::build_outside_card(aurore::Box* column, int at)
{
    namespace kit = cadran::kit;
    // What changed outside PlayGuard, as one card above the ring: its title,
    // the changes one under the other, Ⓐ to dismiss it.
    auto* card = kit::card(true, false);
    card->setAxis(aurore::Axis::ROW);
    card->setAlignItems(aurore::AlignItems::FLEX_START);
    card->setThemeColor("backgroundColor", "cadran/amber_surf");
    card->setThemeColor("borderColor", "cadran/amber_line");
    card->setPadding(18, 24, 18, 20);
    card->setMarginBottom(16);
    card->setVisibility(aurore::Visibility::GONE);
    CadranIcon* icon = kit::icon("alert", 22, "cadran/amber");
    icon->setMarginTop(1);
    icon->setMarginRight(14);
    icon->setShrink(0);
    card->addView(icon);
    auto* text = new aurore::Box(aurore::Axis::COLUMN);
    text->setGrow(1.0f);
    text->setWidth(0);
    text->addView(kit::label("playguard/notices/outside"_i18n, 18, "cadran/amber"));
    this->outside_lines = new aurore::Box(aurore::Axis::COLUMN);
    this->outside_lines->setMarginTop(6);
    text->addView(this->outside_lines);
    // Ⓐ and what it does, as on the network clock's card.
    auto* hint = new aurore::Box(aurore::Axis::ROW);
    hint->setAlignItems(aurore::AlignItems::CENTER);
    hint->setMarginTop(10);
    aurore::Box* key = kit::key("A", 22);
    key->setMarginRight(8);
    hint->addView(key);
    hint->addView(kit::label("playguard/dashboard/outside_dismiss"_i18n, 15, "cadran/text2"));
    text->addView(hint);
    card->addView(text);
    card->registerClickAction([this](aurore::View*) {
        outside_watch::dismiss();
        this->refresh();
        return true;
    });
    column->addView(card, at);
    this->outside_card = card;
}

void DashboardTab::build_steps_card(aurore::Box* column, int at)
{
    namespace kit = cadran::kit;
    auto* card = kit::card(false, false);
    card->setPadding(16, 16, 10, 16);
    card->setMarginBottom(16);
    card->setVisibility(aurore::Visibility::GONE);
    auto* head = new aurore::Box(aurore::Axis::ROW);
    head->setAlignItems(aurore::AlignItems::CENTER);
    head->setPadding(0, 8, 6, 8);
    CadranIcon* icon = kit::icon("sparkle", 22, "cadran/teal");
    icon->setMarginRight(12);
    head->addView(icon);
    aurore::Label* title = kit::label("playguard/onboarding/title"_i18n, 18);
    title->setGrow(1.0f);
    head->addView(title);
    this->steps_count = kit::label("", 15, "cadran/text3");
    head->addView(this->steps_count);
    card->addView(head);

    // The onboarding's steps, their words; Ⓐ does each one as it does.
    const char* words[3] = { "playguard/onboarding/step_pin", "playguard/onboarding/step_limit",
                             "playguard/onboarding/step_clock" };
    for (int i = 0; i < 3; i++) {
        Step& s = this->steps[i];
        s.row = new aurore::Box(aurore::Axis::ROW);
        s.row->setAlignItems(aurore::AlignItems::CENTER);
        s.row->setPadding(10, 8, 10, 8);
        s.row->setFocusable(true);
        s.row->setHideHighlightBackground(true);
        s.row->setHighlightCornerRadius(12);
        s.done = kit::icon("check", 20, "cadran/teal");
        s.done->setMarginRight(12);
        s.row->addView(s.done);
        s.todo = new aurore::Box();
        s.todo->setWidth(20);
        s.todo->setHeight(20);
        s.todo->setCornerRadius(10);
        s.todo->setThemeColor("borderColor", "cadran/amber");
        s.todo->setBorderThickness(2);
        s.todo->setMarginRight(12);
        s.row->addView(s.todo);
        aurore::Label* text = kit::label(aurore::getStr(words[i]), 17);
        text->setGrow(1.0f);
        text->setWidth(0);
        s.row->addView(text);
        s.state = kit::label("", 15, "cadran/text3");
        s.state->setMarginLeft(12);
        s.state->setSingleLine(true);
        s.row->addView(s.state);
        card->addView(s.row);
    }
    steps[0].row->registerClickAction([this](aurore::View*) {
        console_actions::set_pin("first_steps", this->refresher());
        return true;
    });
    steps[1].row->registerClickAction([this](aurore::View*) {
        if (!this->pt.fw_supported) ui::error("", NXM_RC_FW_UNSUPPORTED);
        else pt_flow::choose_uniform_limit(this->pt, this->refresher());
        return true;
    });
    steps[2].row->registerClickAction([this](aurore::View*) {
        if (this->clock_inaccurate) clock_flow::guided(this->refresher());
        else ui::info("playguard/clock/guided_fine"_i18n);
        return true;
    });
    column->addView(card, at);
    this->steps_card = card;
}

void DashboardTab::show_steps_card(bool has_pin)
{
    // Until the PIN and the limits are set (the clock is Today's own card
    // afterwards); not in read-only mode, where none can be done.
    const bool limits = this->pt.valid && pt_configured(this->pt.block);
    const bool shown = !app::read_only() && this->status.pin_length_ok && this->pt.fw_supported && this->pt.valid &&
                       (!has_pin || !limits);
    if (!shown) {
        // All done (or no longer the parent's to do): the card bows out,
        // then goes; nothing more to show in it meanwhile.
        if (this->steps_shown) {
            this->steps_shown = false;
            motion::dismiss(this->steps_card, ui::guarded(this->alive, [this]() { ui::set_visible(this->steps_card, false); }));
        } else {
            ui::set_visible(this->steps_card, false);
        }
        return;
    }
    // Up again after bowing out: whole and in its place.
    if (!this->steps_shown) {
        this->steps_card->setAlpha(1.0f);
        this->steps_card->setTranslationY(0.0f);
    }
    ui::set_visible(this->steps_card, shown);
    this->steps_shown = true;
    const bool clock_ok = R_SUCCEEDED(this->accuracy_rc) && this->accurate;
    const bool done[3] = { has_pin, limits, clock_ok };
    const std::string state[3] = {
        has_pin ? "playguard/dashboard/pin_set"_i18n : "playguard/onboarding/todo"_i18n,
        limits ? ui::days_summary(this->pt.day_min) : "playguard/onboarding/todo"_i18n,
        clock_ok ? "playguard/dashboard/clock_ok"_i18n : "playguard/dashboard/clock_bad"_i18n,
    };
    int count = 0;
    for (int i = 0; i < 3; i++) {
        Step& s = this->steps[i];
        count += done[i];
        // A step just done (seen to do before): its check pops in.
        if (done[i] && this->steps_read && !this->steps_done[i]) s.done->pop();
        this->steps_done[i] = done[i];
        ui::set_visible(s.done, done[i]);
        ui::set_visible(s.todo, !done[i]);
        ui::set_text(s.state, state[i]);
        s.state->setThemeColor("textColor", done[i] ? "cadran/teal" : "cadran/amber");
    }
    this->steps_read = true;
    ui::set_text(this->steps_count, aurore::getStr("playguard/cadran/today/steps_count", count, 3));
}

void DashboardTab::show_outside_card(const std::vector<std::string>& lines)
{
    namespace kit = cadran::kit;
    const bool shown = !lines.empty();
    const std::string text = text::join(lines, "\n");
    if (text != this->outside_shown) {
        this->outside_shown = text;
        while (!this->outside_lines->getChildren().empty())
            this->outside_lines->removeView(this->outside_lines->getChildren().back());
        size_t from = 0;
        while (from < text.size()) {
            size_t to = text.find('\n', from);
            if (to == std::string::npos) to = text.size();
            auto* row = new aurore::Box(aurore::Axis::ROW);
            row->setAlignItems(aurore::AlignItems::FLEX_START);
            row->setMarginTop(4);
            auto* dot = new aurore::Rectangle();
            dot->setThemeColor("color", "cadran/amber_fill");
            dot->setWidth(6);
            dot->setHeight(6);
            dot->setCornerRadius(3);
            dot->setMarginTop(7);
            dot->setMarginRight(10);
            dot->setShrink(0);
            row->addView(dot);
            aurore::Label* label = kit::label(text.substr(from, to - from), 16, "cadran/text2");
            label->setGrow(1.0f);
            label->setWidth(0);
            row->addView(label);
            this->outside_lines->addView(row);
            from = to + 1;
        }
    }
    ui::set_visible(this->outside_card, shown);
}

void DashboardTab::show_low(bool unlocked)
{
    // Little or no time left today: the first amount stands out, the one
    // most likely wanted: ringed in teal on the soft fill, not filled (a
    // filled pill is the screen's main action, and a chosen chip).
    const PtState& timer = this->pt;
    const int today = ui::today_weekday();
    bool low = false;
    if (!unlocked && timer.fw_supported && timer.valid && timer.enabled_valid && timer.enabled && timer.day_min[today] != PT_DAY_NOLIMIT) {
        constexpr int64_t QUARTER_NS = 15LL * 60 * 1000000000LL;
        const bool up = pt_logic::over_limit_min(timer, today) >= 0 || (timer.restricted_valid && timer.restricted);
        low = up || (timer.remaining_valid && timer.remaining_ns > 0 && timer.remaining_ns <= QUARTER_NS);
    }
    // Lit anew: one breath of the chip, so the eye finds it.
    if (low && !this->low_lit && !this->amount_chips.empty() && chips->getVisibility() == aurore::Visibility::VISIBLE &&
        this->amount_chips[0]->getVisibility() == aurore::Visibility::VISIBLE)
        motion::breathe(this->amount_chips[0], 0.5f, 800);
    this->low_lit = low;
    for (size_t i = 0; i < this->amount_chips.size(); i++) {
        aurore::Box* chip = this->amount_chips[i];
        const bool lit = low && i == 0;
        chip->setThemeColor("backgroundColor", lit ? "cadran/teal_soft" : "cadran/surf2");
        chip->setThemeColor("borderColor", lit ? "cadran/teal_fill" : "cadran/line");
        chip->setBorderThickness(lit ? 2 : 1);
        for (aurore::View* v : chip->getChildren())
            if (auto* digits = dynamic_cast<CadranDigits*>(v)) digits->setThemeColor("textColor", lit ? "cadran/teal" : "cadran/text");
    }
}

void DashboardTab::show_cards(bool unlocked, bool has_pin)
{
    const std::string na = "cadran/common/unavailable"_i18n;
    const PtState& timer = this->pt;
    const PctlStatus& s = this->status;

    // Bedtime. Off, the card says so in large rather than an alarm time that
    // does not ring, and A sets it.
    const bool bed_off = timer.bedtime_valid && !timer.bedtime_enabled;
    ui::set_visible(bed_key.getView(), bed_off && !app::read_only());
    bed_value->setThemeColor("textColor", bed_off ? "cadran/text2" : "cadran/text");
    if (bed_off) {
        bed_value->setText("playguard/cadran/today/bed_off_value"_i18n);
        bed_state->setText("");
        bed_sub->setText(app::read_only() ? "" : "playguard/cadran/today/bed_set"_i18n);
    } else if (timer.bedtime_valid) {
        bed_value->setText(std::format("{:02d}:{:02d}", timer.bedtime_hour, timer.bedtime_minute));
        bed_state->setText("playguard/cadran/today/on"_i18n);
        bed_state->setThemeColor("textColor", "cadran/teal");
        std::string sub;
        if (timer.bedtime_reset_valid) {
            // The time kept with the word before it: "again at / 06:00" never
            // breaks there (Aurore, patch 0010).
            const std::string at = std::format("{:02d}:{:02d}", timer.bedtime_reset_hour, timer.bedtime_reset_minute);
            sub = aurore::getStr("playguard/cadran/today/bed_sub", at);
            const size_t pos = sub.find(" " + at);
            if (pos != std::string::npos) sub.replace(pos, 1, "\u00A0");
        } else if (timer.bedtime_enabled) {
            // No morning time from the console: in how long the alarm rings.
            const LocalTime now = ui::local_now();
            const int in = (timer.bedtime_hour * 60 + timer.bedtime_minute) - (now.hour * 60 + now.minute);
            if (in > 0) sub = aurore::getStr("playguard/cadran/today/bed_in", ui::fmt_limit((uint16_t)in));
        }
        bed_sub->setText(sub);
    } else {
        bed_value->setText("—");
        bed_state->setText("");
        bed_sub->setText(timer.fw_supported ? na : "");
    }

    // Parental controls: the level, the PIN, the overall state.
    pc_value->setText(s.safety_level_ok ? ui::level_name(s.safety_level) : na);
    if (unlocked) {
        pc_state->setText("playguard/cadran/today/pc_unlocked"_i18n);
        pc_state->setThemeColor("textColor", "cadran/amber");
    } else {
        pc_state->setText(has_pin ? "playguard/cadran/today/pin_set"_i18n : "playguard/cadran/today/no_pin"_i18n);
        pc_state->setThemeColor("textColor", has_pin ? "cadran/teal" : "cadran/amber");
    }
    pc_sub->setText(!s.restriction_enabled_ok ? na
                    : unlocked                ? "playguard/cadran/today/pc_lifted"_i18n
                    : s.restriction_enabled   ? "playguard/dashboard/pc_active"_i18n
                    : has_pin                 ? "playguard/dashboard/pc_pin_only"_i18n
                                              : "playguard/dashboard/pc_off"_i18n);

    // Network clock: amber, and a call to action, only when it needs one.
    const bool bad = this->clock_inaccurate;
    clock_value->setText(R_FAILED(this->accuracy_rc) ? na
                         : this->accurate              ? "playguard/cadran/today/clock_ok"_i18n
                                                       : "playguard/cadran/today/clock_bad"_i18n);
    clock_value->setThemeColor("textColor", bad ? "cadran/amber" : "cadran/text");
    clock_title->setThemeColor("textColor", bad ? "cadran/amber" : "cadran/text3");
    clock_state->setText(bad ? "playguard/cadran/today/to_set"_i18n : "playguard/cadran/today/ok"_i18n);
    clock_state->setThemeColor("textColor", bad ? "cadran/amber" : "cadran/teal");
    // Nintendo's time server blocked by the DNS: why it went wrong.
    clock_sub->setText(!bad                         ? "playguard/cadran/today/clock_fine"_i18n
                       : nintendo_dns::time_blocked() ? "playguard/cadran/today/clock_fix_dns"_i18n
                                                    : "playguard/cadran/today/clock_fix"_i18n);
    clock_sub->setThemeColor("textColor", bad ? "cadran/text" : "cadran/text2");
    ui::set_visible(clock_key.getView(), bad && !app::read_only());
    clock_card->setThemeColor("backgroundColor", bad ? "cadran/amber_surf" : "cadran/surf");
    clock_card->setThemeColor("borderColor", bad ? "cadran/amber_line" : "cadran/line");
}
