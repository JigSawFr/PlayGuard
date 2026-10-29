// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/pin_lock.hpp"

#include <aurore.hpp>
#include <chrono>
#include <vector>

#include "action/pin_lock_logic.hpp"
#include "action/session_pause.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace pin_lock
{

namespace
{
using pin_lock_logic::MODES;
using pin_lock_logic::rank;

std::chrono::steady_clock::time_point s_refused_until;
std::chrono::steady_clock::time_point s_pin_at = pin_lock_logic::NEVER;   // when the PIN was last entered
std::chrono::steady_clock::time_point s_reveal_until;  // before_show_pin() just asked
std::chrono::steady_clock::time_point s_away_since;    // out of focus since (s_away)
bool s_away = false;
bool s_remote_bypass = false;   // an order PlayGuard carries out for the remote link
Result s_last_rc = 0;   // what the last ask() got
int s_tightening = 0;   // Tightening scopes alive

// The service layer's change check (write_guard.h). Runs on the UI thread,
// with no pctl session open.
bool check()
{
    if (s_remote_bypass) return true;   // checked by the link's policy (sync_flow)
    // Putting back a limit that extra time raised: asked of no one, like
    // locking again. It grants no grace to the changes after it.
    if (s_tightening > 0) return true;
    using pin_lock_logic::Check;
    const Check c = pin_lock_logic::check(config::get().pin_lock, std::chrono::steady_clock::now(), s_pin_at,
                                          config::get().pin_ask_again, s_refused_until);
    if (c == Check::Allow) return true;
    if (c == Check::Refuse) return false;
    if (ask()) return true;
    // Held from the end of the PIN screen, not from when it opened.
    s_refused_until = std::chrono::steady_clock::now() + pin_lock_logic::REFUSAL_HOLDS;
    return false;
}

// The service layer's reveal check: the PIN every time, whatever the mode,
// the grace or the lock screen let through. Whoever may see the PIN knows it.
bool reveal_check()
{
    const auto now = std::chrono::steady_clock::now();
    if (now < s_reveal_until) {   // asked by before_show_pin() a moment ago: once
        s_reveal_until = {};
        return true;
    }
    return ask();
}
}   // namespace

Tightening::Tightening()  { s_tightening++; }
Tightening::~Tightening() { s_tightening--; }

void install()
{
    core_set_change_check(check);
    core_set_reveal_check(reveal_check);
}

void watch_focus(std::function<void()> lock)
{
    aurore::Application::getWindowFocusChangedEvent()->subscribe([lock](bool focused) {
        const auto now = std::chrono::steady_clock::now();
        if (!focused) {
            if (!s_away) s_away_since = now;
            s_away = true;
            return;
        }
        if (!s_away) return;
        s_away = false;
        const auto since = s_away_since;
        // After this frame: a PIN screen that took the focus has returned by then.
        aurore::sync([lock, since, now]() {
            if (pin_lock_logic::lock_again(config::get().pin_lock, since, now, s_pin_at, config::get().pin_ask_again))
                lock();
        });
    });
}

void remote_bypass(bool on)
{
    s_remote_bypass = on;
}

bool allow_change()
{
    return check();
}

bool at_start()
{
    return config::get().pin_lock == "open";
}

bool ask()
{
    const Result rc = pctl_ask_pin();
    s_last_rc = rc;
    aurore::Logger::info("pctl_ask_pin returned 0x{:08X}", (unsigned)rc);
    if (R_SUCCEEDED(rc)) {
        s_pin_at = std::chrono::steady_clock::now();
        session_pause::pin_entered();
        return true;
    }
    return rc == NXM_RC_NO_PIN;   // nothing to ask for
}

bool holds()
{
    return pin_lock_logic::holds(s_pin_at, std::chrono::steady_clock::now(), config::get().pin_ask_again);
}

bool confirm()
{
    if (pin_lock_logic::holds(s_pin_at, std::chrono::steady_clock::now(), config::get().pin_ask_again)) return true;
    return ask();
}

bool before_handing_over(bool menu_locked)
{
    if (menu_locked ? confirm() : check()) return true;
    ui::refuse(refusal_text());
    return false;
}

bool before_show_pin()
{
    if (!ask()) {
        ui::refuse(refusal_text());
        return false;
    }
    // The read that follows at once does not ask a second time.
    s_reveal_until = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    return true;
}

std::string refusal_text()
{
    if (R_SUCCEEDED(s_last_rc) || NXM_IS_APP_RESULT(s_last_rc)) return ui::rc_text(NXM_RC_NOT_CONFIRMED);
    return aurore::getStr("playguard/error/code_hint", fmt::format("0x{:08X}", (unsigned)s_last_rc),
                        "playguard/error/not_confirmed"_i18n);
}

std::string mode_text()
{
    return aurore::getStr(std::string("playguard/pin_lock/modes/") + MODES[rank(config::get().pin_lock)]);
}

std::string note_text()
{
    const int mode = rank(config::get().pin_lock);
    std::string text = aurore::getStr(std::string("playguard/pin_lock/notes/") + MODES[mode]);
    if (mode > 0)
        text += " " + (config::get().pin_ask_again ? aurore::getStr(std::string("playguard/pin_lock/again_on/") + MODES[mode])
                                                   : "playguard/pin_lock/again_off"_i18n);
    return text;
}

void choose(std::function<void()> done)
{
    std::vector<std::string> labels;
    for (const char* m : MODES) labels.push_back(aurore::getStr(std::string("playguard/pin_lock/modes/") + m));
    ui::pick("playguard/pin_lock/title"_i18n, labels, rank(config::get().pin_lock), [done, labels](int index) {
        const std::string from = config::get().pin_lock, to = MODES[index];
        if (to == from) return;
        PctlStatus st;
        pctl_status_fetch(&st);
        const bool has_pin = st.pin_length_ok && st.pin_length > 0;
        const pin_lock_logic::Change change = pin_lock_logic::change(from, index, has_pin);
        if (change == pin_lock_logic::Change::NoPin) {
            ui::notify("playguard/pin_lock/no_pin"_i18n);
            return;
        }
        if (change == pin_lock_logic::Change::AskPin && !confirm()) {
            ui::refuse(refusal_text());
            return;
        }
        config::get().pin_lock = to;
        ui::save_config(aurore::getStr("playguard/pin_lock/set", labels[index]));
        if (done) done();
    });
}

void set_ask_again(bool on, std::function<void()> done)
{
    if (on == config::get().pin_ask_again) return;
    // Less protection: only the parent may choose it.
    if (!on && !confirm()) {
        ui::refuse(refusal_text());
        if (done) done();   // the switch shows the setting again
        return;
    }
    config::get().pin_ask_again = on;
    ui::save_config();
    if (done) done();
}

void set_pauses(bool on, std::function<void()> done)
{
    if (on == config::get().pin_pauses) return;
    // It unlocks parental controls: only the parent may choose it.
    if (on && !confirm()) {
        ui::refuse(refusal_text());
        if (done) done();   // the switch shows the setting again
        return;
    }
    config::get().pin_pauses = on;
    ui::save_config();
    session_pause::setting_changed();
    if (done) done();
}

}   // namespace pin_lock
