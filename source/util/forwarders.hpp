// forwarders — the NRO forwarders PlayGuard installed on this system, as it
// made them (core/forwarder.h): the .nro each starts, its argv, the name and
// author it was given. Kept in nand_dir()/forwarders.json (one per system:
// sysMMC and emuMMC have their own HOME menu), so a forwarder can be edited
// (built again from its .nro) and told apart from one another tool made.
// What is on the HOME menu is the console's to say (forwarder_list); this
// is only what PlayGuard remembers. Plain C++ (no libnx, no UI) for the host
// tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace forwarders
{

constexpr size_t MAX_NAME   = 64;   // characters (code points) of a name or an author
constexpr size_t MAX_AUTHOR = 64;

struct Made
{
    uint64_t    id = 0;
    std::string nro;      // "sdmc:/switch/x/x.nro"
    std::string argv;     // what the .nro gets, its path first
    std::string name;
    std::string author;
    // Protected with the PIN: the loader starts `gate` (PlayGuard's .nro,
    // "sdmc:/…") first, which asks for it (util/forwarder_gate.hpp).
    bool        pin = false;
    std::string gate;
    // No longer on the HOME menu, and the parent told so (the change
    // history, the bell): told once (action/hbmenu_lock.hpp).
    bool        gone = false;
};

// Every one remembered, in the file's order. A damaged file reads as none;
// an entry without an ID or an .nro is skipped.
std::vector<Made> list();

// Adds `m`, or replaces the one with its ID. A short English reason in
// *error when the file cannot be written.
bool save(const Made& m, std::string* error = nullptr);

// Forgets the one with `id` (nothing when there is none).
bool forget(uint64_t id, std::string* error = nullptr);

// The one with `id`; false when none.
bool find(uint64_t id, Made* out);

// A name or an author: printable characters only, spaces trimmed and not
// doubled, at most `max` characters; invalid UTF-8 dropped.
std::string clean_text(const std::string& text, size_t max);

// "050537e2542e2000", and back (false when it is not 16 hex digits).
std::string hex_id(uint64_t id);
bool        parse_id(const std::string& hex, uint64_t* id);

}   // namespace forwarders
