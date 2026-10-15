// diagnostics — build and save the report users attach to bug reports.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace diagnostic
{

// Header (app version, read-only / developer modes, firmware, Atmosphère, launch mode) +
// this run's start-up times + system clocks + every pctl query. Read-only;
// never contains the PIN.
std::string current_report();

// Saves as logs/reports/report_<timestamp>.txt (util/log_files.hpp). Returns the
// saved path, or an empty string (and *error set) on failure.
std::string save(const std::string& report, std::string* error = nullptr);

}   // namespace diagnostic
