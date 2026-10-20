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

// The forwarder `row` made again from its .nro with this name and publisher
// (its own when empty), or as it was (`name` and `author` its own): the same
// program ID, the HOME menu's entry replaced.
void rebuild(const forwarders_logic::Row& row, const std::string& name, const std::string& author,
             std::function<void(bool)> done);

// Protects it with the PIN (`on`, confirmed first; refused without a PIN on
// the console), or no longer: made again, the same program ID, starting
// PlayGuard first, which asks for the PIN (util/forwarder_gate.hpp). Not for
// PlayGuard's own, which has its own lock (pin_lock).
void set_pin(const forwarders_logic::Row& row, bool on, std::function<void(bool)> done);

// The forwarders made here (not PlayGuard's own) not protected yet.
std::vector<forwarders_logic::Row> unprotected();

// Every one of them protected with the PIN (confirmed first). `done(any)`.
void protect_all(std::function<void(bool)> done);

// Every forwarder made here built again (confirmed first): a new .nro's
// icon and version, the gate as PlayGuard carries it now, one no longer on
// the HOME menu put back.
void reinstall_all(std::function<void(bool)> done);

// Hours when a protected forwarder starts without the PIN (core/gate.h):
// `days` (bit 0 Sunday) from `from` to `to` (minutes); days 0 for none. The
// PIN asked first; in the change history.
void set_hours(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done);
// "Sat–Sun 09:00–20:00", in the UI's language.
std::string hours_label(int days, int from, int to);

// Once per start (hbmenu_lock::at_start): the gate and its rules written
// again while a forwarder is protected; the gate's log into the history.
void at_start();

// Deletes it from the HOME menu (asked first), and from what PlayGuard
// remembers.
void remove(const forwarders_logic::Row& row, std::function<void(bool)> done);

// Forgets one no longer on the HOME menu (nothing on the console changes).
void forget(const forwarders_logic::Row& row, std::function<void(bool)> done);

}   // namespace forwarder_flow
