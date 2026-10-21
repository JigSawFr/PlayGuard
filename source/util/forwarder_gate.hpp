// forwarder_gate — a forwarder protected with the PIN (Console › Forwarders):
// its loader starts the gate first (config/playguard/gate.nro, or PlayGuard
// itself for one made before), with "--pin-gate" and the homebrew's own
// argv (core/gate.h). The loader returns to the homebrew, not to the gate,
// once it exits (core/forwarder_pack.h's gate). Plain C++ for the host
// tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "core/gate.h"
#include "util/forwarders.hpp"

namespace forwarder_gate
{

// The argv a forwarder's loader gives the gate (`self`, "sdmc:/…") for the
// homebrew started with `argv` (its path first, quoted when it holds a
// space); with `rules`, those it carries (core/gate.h's GateRules).
std::string argv_for(const std::string& self, const std::string& argv);
std::string argv_for(const std::string& self, const GateRules& rules, const std::string& argv);

// The rules `m` carries: its free hours (both spans), its hours without any
// access, the day's limit, and the pause after refusals as `pause` says.
GateRules rules_of(const forwarders::Made& m, const GatePauseRule& pause);

// The homebrew to start, from the argv PlayGuard was given as it is (one
// string): its path ("sdmc:/…", unquoted) and its argv. False when PlayGuard
// was not started as a gate.
bool parse(const std::string& raw, std::string& nro, std::string& argv);

}   // namespace forwarder_gate
