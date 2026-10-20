// forwarder_gate — a forwarder protected with the PIN (Console › Forwarders):
// its loader starts PlayGuard first, with "--pin-gate" and the homebrew's
// own argv; PlayGuard shows the system's PIN screen and nothing else, then
// hands over to the homebrew (envSetNextLoad) or ends the forwarder, back to
// the HOME menu (main.cpp). The loader returns to the homebrew, not to
// PlayGuard, once it exits (core/forwarder_pack.h's gate). Plain C++ for the
// host tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace forwarder_gate
{

constexpr const char* FLAG = "--pin-gate";

// The argv a forwarder's loader gives PlayGuard (`self`, "sdmc:/…") for the
// homebrew started with `argv` (its path first, quoted when it holds a space).
std::string argv_for(const std::string& self, const std::string& argv);

// The homebrew to start, from the argv PlayGuard was given as it is (one
// string): its path ("sdmc:/…", unquoted) and its argv. False when PlayGuard
// was not started as a gate.
bool parse(const std::string& raw, std::string& nro, std::string& argv);

}   // namespace forwarder_gate
