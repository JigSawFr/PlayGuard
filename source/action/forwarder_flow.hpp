// forwarder_flow — Console › Forwarders' changes: a forwarder made for an
// .nro (its icon and NACP read from it, the name asked, confirmed), made
// again with another name or publisher, deleted; each one through
// core/forwarder.h (the PIN asked before it, read-only mode refusing it),
// remembered in util/forwarders.hpp and in the change history. UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"

namespace forwarder_flow
{

// The forwarders as the screen lists them: the HOME menu's and those
// PlayGuard remembers (forwarders_logic::merge). Reads the console.
std::vector<forwarders_logic::Row> rows();

// The .nro files a forwarder can be made for (/switch, as "/switch/…").
std::vector<std::string> homebrew();

// PlayGuard's own .nro, as "sdmc:/…" (the running one; on the desktop, where
// a console would keep it).
std::string self_nro();

// This build carries the forwarders' program (romfs:/forwarder/: a Switch
// build from this repository).
bool loader_present();

// Asks the name (the .nro's own first), confirms, installs. `sd_path`:
// "/switch/…". `done(installed)` once it is over, also when cancelled.
void create(const std::string& sd_path, std::function<void(bool)> done);

// PlayGuard's own, named "PlayGuard": confirmed, then installed.
void create_self(std::function<void(bool)> done);

// PlayGuard's own with a launch-API request (docs/api.md) as its argument,
// under `name`: "Set the clock" starts PlayGuard with
// --api "POST /v1/clock/sync". Confirmed, then installed (its own program
// ID: the same .nro, other arguments).
void create_request(const std::string& name, const std::string& request, std::function<void(bool)> done);

// The forwarder `row` made again from its .nro with this name and publisher
// (its own when empty), or as it was (`name` and `author` its own): the same
// program ID, the HOME menu's entry replaced.
void rebuild(const forwarders_logic::Row& row, const std::string& name, const std::string& author,
             std::function<void(bool)> done);

// Deletes it from the HOME menu (asked first), and from what PlayGuard
// remembers.
void remove(const forwarders_logic::Row& row, std::function<void(bool)> done);

// Forgets one no longer on the HOME menu (nothing on the console changes).
void forget(const forwarders_logic::Row& row, std::function<void(bool)> done);

}   // namespace forwarder_flow
