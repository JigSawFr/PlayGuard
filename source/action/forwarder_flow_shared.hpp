// forwarder_flow's pieces its two files share (forwarder_flow.cpp: making,
// deleting, the icon; forwarder_rules.cpp: the gate's rules, its pause, its
// log). Not for anything else: forwarder_flow.hpp is the way in.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"
#include "util/forwarders.hpp"
#include "util/gate_tally.hpp"
#include "util/nro.hpp"

namespace forwarder_flow::detail
{

// "/switch/…" under the SD card's root as the app reads it.
std::string on_sd(const std::string& sd_path);
bool        nro_exists(const std::string& sdmc);
// The argv a forwarder hands its .nro: its path, quoted when it holds a
// space (libnx splits the argv there), as sphaira writes it.
std::string argv_of(const std::string& sdmc);
// The detail the history keeps: the name, and the program ID.
std::string history_label(const std::string& name, uint64_t id);

// What a forwarder is built from: the .nro's icon and NACP. PlayGuard's own
// falls back to the icon it ships with when its .nro cannot be read (the
// desktop has none).
bool assets_of(const std::string& sdmc, bool self, nro::Info& info, std::string& error);

// The gate a protected forwarder starts first: config/playguard/gate-<its
// SHA-256's first 16 hex digits>.nro, written from the romfs when this build
// carries it, its hash for the forwarder's loader to check (`pinned`); a
// new PlayGuard's gate is another file, so a forwarder made before keeps
// its own until it is made again. gate.nro too, for those made before the
// hash. Else PlayGuard's own .nro (the desktop, or the card refused the
// write), which is a gate too (main.cpp), not pinned: it changes with each
// update.
struct Gate
{
    std::string                path;   // "sdmc:/…"
    std::array<uint8_t, 32>    sha256{};
    bool                       pinned = false;
};
Gate gate_nro();
// The gate files PlayGuard wrote that no forwarder starts any more: gone.
void tidy_gates();

// Builds and installs `m` from its .nro; says how it went (`done_text`, the
// change history's `kind`, nullptr for none), only when it fails with `quiet`. Protected with
// the PIN, it goes through the gate as it is now (gate_nro).
bool install(const forwarders::Made& m, bool self, const char* kind = "add_forwarder", const std::string& done_text = "",
             bool quiet = false);
// Each of `rows` made again (`pin`: protected too), one after the other,
// quietly; one toast for them all. `kind` for each in the change history.
void install_many(const std::vector<forwarders_logic::Row>& rows, bool pin, const char* kind,
                  const std::string& done_text, std::function<void(bool)> done);

// The forwarder `row` remembered, or as the row says it (made here before
// PlayGuard remembered it).
forwarders::Made made_of(const forwarders_logic::Row& row);
// Made here, still known (not PlayGuard's own when `with_self` is false).
std::vector<forwarders_logic::Row> made_rows(bool with_self);

// While the homebrew menu is locked, taking `row` away (deleting it, or its
// PIN: `what`) from the ways back: refused when it is the last one or
// PlayGuard's own (unlock first), confirmed in coral when others remain.
// `go` runs when it may go on.
void guard_way_back(const forwarders_logic::Row& row, const std::string& what, std::function<void()> go,
                    std::function<void()> stop);

// The free hours of every protected forwarder, and the hours it never
// opens, for the gate (GATE_RULES_NAME, GATE_CLOSED_NAME): forwarder_rules.cpp.
void write_rules();
// The gate's pause file (GATE_PAUSE_NAME), its text ("" when there is none).
std::string pause_path();
std::string read_pause();
// The week's tries (util/gate_tally.hpp): nand_dir()/gate_tally.json.
std::string       tally_path();
gate_tally::Tally read_tally();

}   // namespace forwarder_flow::detail
