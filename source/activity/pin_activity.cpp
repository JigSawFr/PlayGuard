// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "activity/pin_activity.hpp"

#include "action/console_actions.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "app.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

void PinActivity::onContentAvailable()
{
    lock_note->setSingleLine(false);
    set_pin->registerClickAction([this](aurore::View*) {
        this->change_pin();
        return true;
    });
    show_pin->registerClickAction([this](aurore::View*) {
        if (ui::refuse_read_only()) return true;
        // Each step a frame later, after a dialog: nothing once the screen is gone.
        const auto change = ui::guarded(this->alive, [this]() { this->change_pin(); });
        const auto offer_change = ui::guarded(this->alive, [change]() {
            ui::confirm("playguard/security/show_pin_change_body"_i18n,
                        "playguard/security/show_pin_change_confirm"_i18n, [change]() { aurore::sync(change); });
        });
        const auto show = ui::guarded(this->alive, [offer_change]() {
            // The PIN every time (pin_lock.hpp); then, as it may have been
            // seen over a shoulder, a new one is offered.
            if (!pin_lock::before_show_pin()) return;
            const bool shown = ui::show_pin_dialog([offer_change]() { aurore::sync(offer_change); });
            if (shown) history_flow::record_event("pin_shown");
        });
        ui::confirm("playguard/security/show_pin_body"_i18n, "playguard/security/show_pin_confirm"_i18n,
                    [show]() { aurore::sync(show); });
        return true;
    });
    const std::function<void()> again = ui::guarded(this->alive, [this]() { this->refresh(); });
    lock_cell->registerClickAction([again](aurore::View*) {
        pin_lock::choose(again);
        return true;
    });
    // Off by default: once entered, the PIN holds until PlayGuard closes.
    ask_again->init("playguard/pin_lock/ask_again"_i18n, config::get().pin_ask_again, [](bool) {});
    ask_again->registerClickAction([again](aurore::View*) {
        pin_lock::set_ask_again(!config::get().pin_ask_again, again);
        return true;
    });
    // Off by default: entering the PIN leaves parental controls as they are.
    pauses->init("playguard/pin_lock/pauses"_i18n, config::get().pin_pauses, [](bool) {});
    pauses->registerClickAction([again](aurore::View*) {
        pin_lock::set_pauses(!config::get().pin_pauses, again);
        return true;
    });
    this->list = std::make_unique<cadran::List>(this->getContentView(), true);
    this->list->apply({ { "pn_set", "key" }, { "pn_show", "eye" }, { "pn_lock", "lock" }, { "pn_ask_again", "timer" },
                        { "pn_pauses", "pause" } });
    this->refresh();
}

void PinActivity::onResume()
{
    this->refresh();
}

void PinActivity::change_pin()
{
    console_actions::set_pin("", ui::guarded(this->alive, [this]() { this->refresh(); }));
}

void PinActivity::refresh()
{
    PctlStatus s;
    pctl_status_fetch(&s);
    const bool has_pin = s.pin_length_ok && s.pin_length > 0;
    ui::set_detail(lock_cell, pin_lock::mode_text());
    ui::set_text(lock_note, pin_lock::note_text());
    ask_again->setOn(config::get().pin_ask_again, false);
    pauses->setOn(config::get().pin_pauses, false);
    ui::set_visible(show_pin.getView(), has_pin);
    // Read-only: the actions stay in sight, greyed (A says why).
    const bool writable = !app::read_only();
    ui::show_writable(set_pin, writable);
    ui::show_writable(show_pin, writable);
    if (this->list) this->list->sync();
}
