// forwarders_logic — the UI-free part of Console › Forwarders: one row per
// forwarder, from what the HOME menu has (core/forwarder.h) and what
// PlayGuard remembers making (util/forwarders.hpp); whether the console can
// start one (its signature patches, util/patches.hpp); the name a new one
// gets. Plain C++ for the host tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "util/forwarders.hpp"
#include "util/patches.hpp"

namespace forwarders_logic
{

// A forwarder the HOME menu has (core/forwarder.h's ForwarderRecord).
struct Installed
{
    uint64_t    id = 0;
    std::string name, author, version;
};

enum class State
{
    Ready,         // on the HOME menu, its .nro there
    NroMissing,    // on the HOME menu, its .nro gone: it would not start
    NotInstalled,  // made here, no longer on the HOME menu (deleted in System Settings)
    Elsewhere,     // on the HOME menu, made by another tool: PlayGuard knows not its .nro
};

struct Row
{
    uint64_t    id = 0;
    std::string name, author, version;
    std::string nro;    // "sdmc:/…"; empty for one made elsewhere
    State       state = State::Ready;
    bool        self = false;   // PlayGuard's own
};

// One row per forwarder on the HOME menu and per one remembered that is no
// longer there, by name (ignoring case), then ID. The name the console reads
// first, then the one remembered, then the .nro's file name, then the ID.
// `nro_exists` says whether an "sdmc:/…" path is on the SD card;
// `self_nro` is PlayGuard's own ("sdmc:/…", may be empty).
std::vector<Row> merge(const std::vector<forwarders::Made>& made, const std::vector<Installed>& installed,
                       const std::function<bool(const std::string&)>& nro_exists, const std::string& self_nro);

// The row of PlayGuard's own forwarder, or nullptr.
const Row* self_row(const std::vector<Row>& rows);

// The .nro's file name without its folder and ".nro" ("ftpd" for
// "sdmc:/switch/ftpd/ftpd.nro").
std::string file_title(const std::string& path);

// The name a new forwarder gets: the .nro's own (its NACP), else its file
// name; cut to forwarders::MAX_NAME.
std::string default_name(const std::string& nacp_name, const std::string& path);

// Whether the console can start a forwarder: its fs and loader signature
// checks patched (sys-patch at this boot, or sigpatch files for both, or
// hekate's).
enum class Patches
{
    Ready,
    Unknown,   // sys-patch is there, its log does not say for this boot
    Missing,
};
Patches patches_state(const patches::Report& report);

}   // namespace forwarders_logic
