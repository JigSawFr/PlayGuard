// report_sections — what PlayGuard adds to the diagnostic report
// (util/diagnostics.hpp): the header's firmware choice, launch by a
// forwarder and compatibility level; then the homebrew menu's lock,
// Atmosphère's DNS.mitm, the system clocks, every pctl query and the
// remote link. Given once at start-up, before any report is made.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace report_sections
{

void install();

}   // namespace report_sections
