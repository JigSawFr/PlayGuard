// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rollback_hooks.hpp"

#include <format>
#include <aurore.hpp>
#include <string>

#include "action/migration_flow.hpp"
#include "action/rollback_flow.hpp"
#include "core/nx_types.h"
#include "util/pctl_ops_c.hpp"
#include "util/rollback.hpp"

namespace rollback_hooks
{

void install()
{
    rollback_flow::Hooks& h = rollback_flow::hooks();
    h.release_level = [](const std::string& label) { return migration_flow::release_level(label); };
    h.undoable = [](int level, int previous) { return migration_flow::undoable(level, previous); };
    h.going_back = [](int previous_level, const migrations::Log& log) { migration_flow::going_back(previous_level, log); };
    h.verify_all = []() { migration_flow::verify_all(); };
    // The system's PIN screen, not PlayGuard's (which may be what crashed);
    // config and Aurore are not needed.
    h.boot_gate = [](const std::string& previous) -> std::string {
        const Result pin = pctl_ask_pin();
        if (rollback::go_back_allowed(R_SUCCEEDED(pin), pin == NXM_RC_NO_PIN)) return "";
        aurore::Logger::info("rollback: not gone back, the PIN not entered (0x{:08X})", (unsigned)pin);
        // Left with B: said plainly; another failure with its code.
        const std::string why =
            pin == NXM_RC_CANCELLED ? std::string() : " (" + std::format("0x{:08X}", (unsigned)pin) + ")";
        return "The PIN was not entered" + why + ": " + previous + " stays where it is.";
    };
}

}   // namespace rollback_hooks
