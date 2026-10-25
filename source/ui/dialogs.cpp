// ui — PlayGuard's own dialogs: a duration chosen with the stick and the PIN
// shown, in Aurore's Cadran look (borealis/cadran/ui/ui.hpp has the rest).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#include <algorithm>
#include <memory>

#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_modal.hpp>
#include <borealis/cadran/view/cadran_digits.hpp>
#include <borealis/cadran/view/cadran_icon.hpp>

#include "action/pt_logic.hpp"
#include "ui/format_text.hpp"

using namespace brls::literals;

namespace
{

// A duration chosen with the stick, in place of the system number pad: up /
// down 15 min, L / R 5 min (or a tap on the arrows), from 0 to 24:00, OK
// or Cancel; `on_value` runs once the dialog has closed.
void stick_minutes(const std::string& header, uint16_t initial, std::function<void(uint16_t)> on_value)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    auto minutes = std::make_shared<int>(std::min<int>(initial, 24 * 60));
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setAlignItems(brls::AlignItems::CENTER);
    body->setPadding(30, 40, 22, 40);
    brls::Label* title = kit::display(header, kit::TEXT_TITLE);
    title->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    body->addView(title);
    CadranIcon* up = kit::icon("caret-up", 30, "cadran/teal");
    up->setMarginTop(18);
    body->addView(up);
    auto* value = new CadranDigits();
    value->setFontSize(56);
    value->setSmallUnits(true);
    value->setTextColor(theme["cadran/text"]);
    value->setText(ui::fmt_played((uint16_t)*minutes));
    body->addView(value);
    CadranIcon* down = kit::icon("caret-down", 30, "cadran/teal");
    body->addView(down);
    // The keys, as the week's: the arrows 15 min, L and R 5 min.
    auto* keys = new brls::Box(brls::Axis::ROW);
    keys->setAlignItems(brls::AlignItems::CENTER);
    keys->setMarginTop(16);
    const auto key = [](const char* letter) {
        brls::Box* bubble = kit::key(letter);
        bubble->setMarginRight(4);
        return bubble;
    };
    keys->addView(kit::icon("caret-up", 18, "cadran/text3"));
    keys->addView(kit::icon("caret-down", 18, "cadran/text3"));
    brls::Label* quarter = kit::label(ui::fmt_played(15), 16, "cadran/text3");
    quarter->setMarginLeft(6);
    quarter->setMarginRight(22);
    keys->addView(quarter);
    keys->addView(key("L"));
    keys->addView(key("R"));
    brls::Label* five = kit::label(ui::fmt_played(5), 16, "cadran/text3");
    five->setMarginLeft(4);
    keys->addView(five);
    body->addView(keys);

    auto* dialog = new brls::Dialog(body);
    const auto step = [minutes, value](int delta) {
        *minutes = std::max(0, std::min(24 * 60, *minutes + delta));
        value->setText(ui::fmt_played((uint16_t)*minutes));
    };
    brls::View* frame = dialog->getAppletFrame();
    // An arrow held: 15 min at a time, then whole hours (pt_logic::held_step).
    auto held = std::make_shared<pt_logic::Held>();
    for (const auto& [button, delta] : { std::pair<brls::ControllerButton, int>{ brls::BUTTON_NAV_UP, 15 },
                                         { brls::BUTTON_NAV_DOWN, -15 }, { brls::BUTTON_RB, 5 }, { brls::BUTTON_LB, -5 } })
        frame->registerAction("", button, [step, held, minutes, delta = delta](brls::View*) {
            step(pt_logic::held_step(*held, *minutes, delta, brls::getCPUTimeUsec()) - *minutes);
            return true;
        }, true, true, brls::SOUND_FOCUS_CHANGE);
    up->addGestureRecognizer(new brls::TapGestureRecognizer(up, [step]() { step(15); }));
    down->addGestureRecognizer(new brls::TapGestureRecognizer(down, [step]() { step(-15); }));
    dialog->addButton("hints/cancel"_i18n, []() {});
    dialog->addButton("hints/ok"_i18n, [minutes, on_value]() {
        if (on_value) on_value((uint16_t)*minutes);
    });
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    // A value is set, then confirmed: A right after the arrows is OK, not Cancel.
    if (brls::View* ok = dialog->getView("brls/dialog/button2")) dialog->setLastFocusedView(ok);
    dialog->open();
}

}   // namespace

namespace ui
{

void prompt_minutes(const std::string& header, uint16_t current, std::function<void(uint16_t)> on_value)
{
    // The stick, not the number pad.
    stick_minutes(header, current == PT_DAY_NOLIMIT ? 60 : current, std::move(on_value));
}

namespace
{
void wipe(char* p, size_t n)
{
    volatile char* b = p;
    while (n--) *b++ = 0;
}
}   // namespace

bool show_pin_dialog(std::function<void()> on_close)
{
    char pin[16];
    Result rc = pctl_get_pin(pin, sizeof(pin));
    brls::Logger::info("pctl_get_pin returned 0x{:08X}", (unsigned)rc);
    if (R_FAILED(rc)) {
        notify_result(rc, "", "playguard/security/show_pin_err"_i18n);
        return false;
    }
    std::string spaced;   // "1 2 3 4": easier to read out and to type
    spaced.reserve(2 * sizeof(pin));   // no reallocation, so no stray copy
    for (const char* c = pin; *c; c++) {
        if (!spaced.empty()) spaced += ' ';
        spaced += *c;
    }
    wipe(pin, sizeof(pin));

    // The digits in the display face, large, under what they are.
    brls::Label* digits = cadran::kit::display(spaced, 56);
    digits->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    digits->setMarginBottom(8);
    wipe(&spaced[0], spaced.size());

    auto* d = dialog("playguard/security/show_pin_title"_i18n, digits);
    d->addButton("hints/ok"_i18n, [on_close]() { if (on_close) on_close(); });
    on_cancel(d, on_close);
    d->open();
    return true;
}

}   // namespace ui
