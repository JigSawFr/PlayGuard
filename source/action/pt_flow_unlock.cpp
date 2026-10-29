// pt_flow: the record of an unlock (config relock_pending), the temporary
// unlock of PIN › Unlock temporarily and locking again (after an
// interrupted run, at exit).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/pt_flow.hpp"

#include <aurore.hpp>

#include "action/history_flow.hpp"
#include "action/pt_flow_internal.hpp"
#include "action/pt_logic.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace aurore::literals;

namespace pt_flow
{

// A record made by this run is an unlock still in use, not an interrupted one.
static bool s_pending_this_run = false;

// False when the record could not be saved to the SD card. Setting it waits
// for the write: it must be on the card before the unlock it is for.
bool set_relock_pending(bool on)
{
    auto& cfg = config::get();
    if (on) s_pending_this_run = true;
    if (cfg.relock_pending == on) return true;
    cfg.relock_pending = on;
    return on ? ui::save_config_now() : ui::save_config();
}

void relock_if_interrupted()
{
    // Read-only refuses every write, the lock too: keep the record for a
    // start that can lock.
    const bool pending = config::get().relock_pending;
    if (!pt_logic::pending_relock_needs_state(pending, s_pending_this_run, app::read_only())) return;
    PctlStatus st;
    pctl_status_fetch(&st);
    switch (pt_logic::pending_relock(pending, s_pending_this_run, app::read_only(), st.temp_unlocked_ok, st.temp_unlocked)) {
        case pt_logic::PendingRelock::Forget:
            set_relock_pending(false);
            return;
        case pt_logic::PendingRelock::Lock:
            break;
        default:
            return;   // state unknown: keep the record, try again next time
    }
    Result rc = pctl_relock();
    // The record goes only once the console is locked again.
    if (R_SUCCEEDED(rc)) set_relock_pending(false);
    ui::notify_result(rc, "playguard/toast/relocked_after_stop"_i18n, "playguard/toast/relock_err"_i18n);
}

// After an unlock attempt that failed: drop the record only when the console
// reads back as locked (the unlock may have gone through and its lock failed).
void settle_failed_unlock()
{
    PctlStatus st;
    pctl_status_fetch(&st);
    if (st.temp_unlocked_ok && !st.temp_unlocked) set_relock_pending(false);
}

Result unlock_temporarily()
{
    // Recorded first, as for a change: closed from the HOME menu (no
    // question then), PlayGuard locks again at its next start. A record that
    // cannot be saved (config_err) does not stop a parent's unlock.
    set_relock_pending(true);
    Result rc = pctl_unlock_restriction_temporarily();
    if (R_FAILED(rc)) settle_failed_unlock();
    return rc;
}

void keep_unlocked()
{
    set_relock_pending(false);
}

static bool s_lock_at_exit = false;

void lock_at_exit()
{
    set_relock_pending(true);
    s_lock_at_exit = true;
}

void lock_at_exit_if_asked()
{
    if (!s_lock_at_exit) return;
    const Result rc = pctl_relock();
    if (R_FAILED(rc)) {
        aurore::Logger::error("lock at exit: failed ({:#x}), locked again at the next start", (unsigned)rc);
        return;
    }
    aurore::Logger::info("lock at exit: locked");
    history_flow::record_event("relock");
    set_relock_pending(false);
    platform_exit_to_home();
}

}   // namespace pt_flow
