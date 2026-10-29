// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/session_pause.hpp"

#include <aurore.hpp>

#include "action/pin_lock.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_flow_internal.hpp"
#include "action/session_pause_logic.hpp"
#include "app.hpp"
#include "core/sysinfo.h"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"

namespace session_pause
{

namespace
{
bool s_active = false;     // parental controls unlocked by the pause, now
bool s_failed = false;     // the last unlock failed: not tried again until the PIN is entered again
bool s_stopped = false;    // the parent locked them by hand: not until the PIN is entered again
bool s_quitting = false;
bool s_focused = true;     // as the last focus event said (app::in_focus reads it now, as the console does)
int s_holds = 0;

bool wanted()
{
    SysInfo si;
    sysinfo_get(&si);
    session_pause_logic::Inputs in;
    in.enabled = config::get().pin_pauses;
    in.pin_holds = pin_lock::holds();
    in.in_focus = s_focused && app::in_focus();
    in.holds = s_holds;
    in.application = !si.applet_mode;
    in.read_only = app::read_only();
    in.quitting = s_quitting;
    return !s_failed && !s_stopped && session_pause_logic::wanted(in);
}

// What the screens show (the header's pill, Today), at once.
void shown(bool unlocked)
{
    ui::note_paused(unlocked);
    ui::note_unlocked(true, unlocked);
    aurore::sync([]() { ui::refresh_shown_tab(); });
}

void pause()
{
    // Never over an unlock PlayGuard did not make (Security › Unlock
    // temporarily, an earlier run): that one stays the parent's.
    u32 pin_length = 0;
    bool unlocked = false;
    if (R_FAILED(pctl_lock_state(&pin_length, &unlocked)) || pin_length == 0 || unlocked) return;
    const Result rc = pt_flow::unlock_temporarily();
    if (R_FAILED(rc)) {
        aurore::Logger::error("session pause: the unlock failed ({:#x})", (unsigned)rc);
        s_failed = true;
        return;
    }
    aurore::Logger::info("session pause: parental controls paused while PlayGuard is open");
    s_active = true;
    shown(true);
}

void resume()
{
    s_active = false;
    ui::note_paused(false);
    const Result rc = pctl_relock();
    if (R_FAILED(rc)) {
        // Still recorded (relock_pending): the next start locks again.
        aurore::Logger::error("session pause: locking again failed ({:#x})", (unsigned)rc);
        return;
    }
    aurore::Logger::info("session pause: parental controls back on");
    pt_flow::set_relock_pending(false);
    if (!s_quitting) shown(false);
}

void apply()
{
    const bool want = wanted();
    if (want && !s_active) pause();
    else if (!want && s_active) resume();
}
}   // namespace

void install()
{
    aurore::Application::getWindowFocusChangedEvent()->subscribe([](bool focused) {
        s_focused = focused;
        // Left: at once, before anything can start behind it.
        if (!focused) {
            apply();
            return;
        }
        // Back: after the lock screen, if any, has opened (pin_lock::watch_focus,
        // subscribed before this one, waits a frame too).
        aurore::sync([]() { apply(); });
    });
}

void pin_entered()
{
    s_failed = false;
    s_stopped = false;
    aurore::sync([]() { apply(); });
}

void setting_changed()
{
    // Chosen now: a pause stopped by hand, or failed, may start again.
    s_failed = false;
    s_stopped = false;
    apply();
}

bool active() { return s_active; }

void shown_as_locked(PctlStatus* status, PtState* pt)
{
    if (!s_active) return;
    if (status && status->temp_unlocked_ok) status->temp_unlocked = false;
    if (pt && pt->temporary_unlocked_valid && pt->temporary_unlocked) {
        pt->temporary_unlocked = false;
        if (pt->enabled_valid) pt->enabled = true;
    }
}

void stop()
{
    s_stopped = true;
    s_active = false;
    ui::note_paused(false);
}

void adopt()
{
    stop();
    pt_flow::set_relock_pending(false);
    aurore::Logger::info("session pause: the unlock kept, as a remote order asked");
}

void end()
{
    s_quitting = true;
    if (s_active) resume();
}

Hold::Hold()
{
    s_holds++;
    apply();
}

Hold::~Hold()
{
    s_holds--;
    aurore::sync([]() { apply(); });
}

}   // namespace session_pause
