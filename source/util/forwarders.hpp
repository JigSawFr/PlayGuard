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
constexpr int    ICON_X_MIDDLE = 50;   // an icon from the album: the screenshot's middle square
// "schema" in forwarders.json (none before it: read as 1).
constexpr int    SCHEMA = 1;

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
    // Protected, and started through the gate once since (its log said
    // so): a way back seen working (forwarders_logic::LockCheck::verified).
    bool        verified = false;
    // Protected: hours when the PIN is not asked (core/gate.h's GateHours):
    // days 0 for none.
    int         free_days = 0;
    int         free_from = 0, free_to = 0;
    // A second span of them (the weekend's, the first one the weekdays'):
    // days 0 for none.
    int         free2_days = 0;
    int         free2_from = 0, free2_to = 0;
    // Protected: hours when it never opens, the PIN or not (GATE_CLOSED_NAME).
    int         closed_days = 0;
    int         closed_from = 0, closed_to = 0;
    // Protected: closed once the day's play time is used up (the console's
    // play timer), the PIN or not (core/gate.h's GateRules.limit).
    bool        limit = false;
    // Protected, built with its rules in its gate's argv and the gate's
    // SHA-256 for its loader (GateRules.carried): one made before reads them
    // from the SD card, and is made again (forwarder_flow::at_start).
    bool        carried = false;
    // Its HOME menu icon from a screenshot of the album ("sdmc:/Nintendo/
    // Album/…jpg"), or from PlayGuard's library ("lib:<id>",
    // forwarder_library), else the .nro's own. Protected, with a badge over
    // it (forwarder_icon::with_badge).
    std::string icon;
    // The loader it was built with (forwarder_flow::loader_id: 16 hex
    // digits of its SHA-256); empty for one made before it was kept. One
    // built with another loader is offered to be made again
    // (forwarders_logic::stale_loader).
    std::string loader;
    // Where its square sits along the screenshot: 0 its left edge, 100 its
    // right one (forwarder_icon::from_screenshot).
    int         icon_x = ICON_X_MIDDLE;
    // How the HOME menu starts it, as sphaira's forwarder options: a user
    // picked at launch (its play time then that user's in the Activity),
    // screenshots and videos allowed (core/forwarder_pack.h's FWD_*).
    bool        ask_user = false;
    bool        screenshots = true;
    bool        video = true;
};

// Every one remembered, in the file's order. A damaged file reads as none,
// and so does one a newer version wrote (its "schema" above SCHEMA), which
// save() and forget() then leave as it is; an entry without an ID or an .nro
// is skipped. A save keeps the keys this version does not know.
std::vector<Made> list();

// The list as a JSON array (a settings backup's), and back: entries as
// list() keeps them; anything else, none.
std::string       list_text(const std::vector<Made>& all);
std::vector<Made> parse_list(const std::string& text);

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
