// diagnostics — build and save the report users attach to bug reports.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "util/diagnostics_logic.hpp"
#include "util/pctl_ops_c.hpp"

namespace diagnostic
{

// Header (app version, read-only / developer modes, firmware, Atmosphère, launch mode) +
// this run's start-up times + the logs (this run's session file, how the
// last one ended, memory, SD card, the newest crash files) + system clocks +
// every pctl query + the last lines logged. Read-only; never contains the
// PIN.
std::string current_report();

// What the header's firmware_choice says (action/fw_gate's summary(): "none",
// "read_only", "probe (remembered)" …), given by that layer at start-up so
// this one does not reach up to it. "unknown" until then.
void set_firmware_choice(std::string (*summary)());

// The homebrew menu's section (action/hbmenu_lock's report()), given the
// same way; none until then.
void set_homebrew_lock(std::string (*section)());

// Atmosphère's DNS.mitm section (action/nintendo_dns's report()), given the
// same way; none until then.
void set_dns(std::string (*section)());

// Saves as logs/reports/report_<timestamp>.txt (util/log_files.hpp). Returns the
// saved path, or an empty string (and *error set) on failure.
std::string save(const std::string& report, std::string* error = nullptr);

}   // namespace diagnostic
