// titles — what PlayGuard knows of the titles that are not games: the NRO
// forwarders on the HOME menu (PlayGuard's own, those it made, those another
// app made), each with its name, kept after the forwarder is deleted, so its
// play time keeps a name and stays apart from the games' (sync/sync_titles.h
// for the kinds and the rules from the ID). Two files in this system's folder:
// nand/<system>/titles.txt, the record with the names ("<16 hex>=homebrew:
// <name>"), PlayGuard's alone; and nand/<system>/sync/titles.txt, the same
// without the names ("<16 hex>=homebrew"), which the agent reads to keep
// homebrews off the broker. No UI and no console calls: host-tested
// (tests/titles).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "sync/sync_titles.h"

namespace titles
{

// The record's bound: the oldest go past it.
constexpr size_t MAX = SYNC_TITLES_MAX;

struct Title
{
    SyncTitleKind kind = SyncTitle_Homebrew;
    std::string   name;   // "" unknown
};
using Record = std::map<uint64_t, Title>;

// What the record says of `id`, else its ID (sync_title_kind_of_id).
SyncTitleKind kind(const Record& r, uint64_t id);
// The name the record keeps for `id`, "" none.
std::string name(const Record& r, uint64_t id);

// Notes a title seen: kept when it is not a game (`kind`; a game the ID
// tells already is not kept), its name when `name` is not empty (the newest
// wins). True when the record changed.
bool note(Record& r, uint64_t id, SyncTitleKind kind, const std::string& name);

// nand/<system>/titles.txt, and back; lines it does not understand are left out.
std::string text(const Record& r);
Record      parse(const std::string& text);
// The agent's file: the kinds, no names.
std::string agent_text(const Record& r);

}   // namespace titles
