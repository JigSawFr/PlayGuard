// sync_files — the remote link's files on the SD card, from PlayGuard's side:
// sync.conf (the link's settings, written by the Sync screen) and sync/
// (profiles.txt, the saved profiles for profile orders), shared by every
// system on the SD card like the preferences and the profiles; and, in each
// system's own folder (nand/<system>/sync/, util/nand.hpp), what the agent
// sysmodule reads to act on PlayGuard's behalf while it is closed:
// nro_state.txt (the records of state.json it needs) and names.txt (game
// names, for "now playing"), and what it writes there. One writer per file,
// flat "key=value" text the agent's C parser reads (source/sync/sync_conf.h).
// No UI and no console calls: host-tested (tests/sync_files).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "sync/sync_conf.h"
#include "sync/sync_exec.h"
#include "util/config.hpp"
#include "util/profiles.hpp"

namespace sync_files
{

std::string conf_file();    // data_dir()/sync.conf
std::string dir();          // data_dir()/sync: shared by every system
std::string system_dir();   // nand_dir()/sync: this system's
bool exists();              // sync.conf is there: the link was set up once

// The defaults, then what sync.conf holds; `emummc` set from the system
// PlayGuard runs on (paths::nand_key()), for its own device id.
SyncConf load();
bool save(const SyncConf& c, std::string* error = nullptr);
// Gives the console its id ("a1b2c3d4", from 4 random bytes) when it has
// none. True when it changed.
bool ensure_id(SyncConf& c, const uint8_t random[4]);

// How the files the agent reads are written (nro_state.txt, profiles.txt,
// names.txt): their text is made at the call, on the caller's thread; the
// write itself at once by default (the host tests), or handed to `writer`
// (PlayGuard: ui::write_later, off the UI thread; a write of the same file
// still waiting is replaced by the newer one).
using Writer = void (*)(const std::string& file, std::function<void()> write);
void set_writer(Writer writer);
// Told of a file the SD card refused (`why`), on the thread that wrote it:
// the app logs it and says so; none in the tests and the agent.
using Failed = void (*)(const std::string& file, const std::string& why);
void set_on_failure(Failed failed);

// config.json's records <-> the shared form sync_exec updates.
SyncRecords records_from(const config::Config& c);
// True when something changed.
bool records_into(const SyncRecords& r, config::Config& c);

// nand/<system>/sync/nro_state.txt: the records, extra_auto_restore, the firmware choice,
// the PIN lock, and what the agent needs to know before it changes anything:
// whether PlayGuard is read-only and the firmware it last ran on (the agent
// refuses writes on another one). Written after every config save while
// sync.conf exists (config::set_saved_hook, at the end of config::snapshot()
// on the UI thread), and when read-only mode changes.
// set_console() says the firmware and the mode (main.cpp, and the mode_changed hook of activity/screen_hooks.cpp).
void set_console(const std::string& firmware, bool read_only);
// The serial number the agent publishes with publish_serial (the one
// PlayGuard read: util/prodinfo), in nro_state.txt; "" leaves it out.
void set_serial(const std::string& serial);
std::string nro_state_text(const config::Config& c);
void export_nro_state();

// sync/profiles.txt: one "60,90,120,120,120,180,180=School week" line per
// profile (minutes Sunday first, 65535 no limit).
std::string profiles_text(const std::vector<profiles::Profile>& list);
void write_profiles(const std::vector<profiles::Profile>& list);

// nand/<system>/sync/names.txt: one "0100000000010000=Super Mario Odyssey" line per game.
std::string names_text(const std::vector<std::pair<uint64_t, std::string>>& names);
void write_names(const std::vector<std::pair<uint64_t, std::string>>& names);

// nand/<system>/sync/icons.txt: the game icons PlayGuard published, and to
// which broker (action/sync_icons_logic's text), so a new run sends only
// what changed. PlayGuard writes it, the agent only reads it.
std::string icons_file();
void write_icons(const std::string& text);
// nand/<system>/sync/agent_icons.txt: the icons the agent published itself
// (the game played, from PlayGuard's icon cache), in the same form. Taken as
// agent_events.log is (renamed, read, removed): its text, "" when none.
std::string take_agent_icons();

// nand/<system>/sync/integration.json: PlayGuard's Home Assistant integration's
// document, as the link got it from the broker (docs/sync-protocol.md):
// written by PlayGuard's own link or by the agent, removed when cleared (the
// integration removed). PlayGuard warns of a newer release of the integration.
std::string integration_file();
struct Integration
{
    std::string version;         // "0.2.0"; "" when unknown
    std::string homeassistant;   // "2026.10.1"; "" when unknown
};
// The integration document's "schema" this version reads.
constexpr int INTEGRATION_SCHEMA = 1;
// Its two versions; each "" when missing or not a version (digits first, then
// at most 24 of [0-9A-Za-z.+-]). A document that is not JSON, or of a newer
// schema than INTEGRATION_SCHEMA, gives both "".
Integration parse_integration(const std::string& doc);
// Writes `doc` as it came, or removes the file when "" (through the writer).
void write_integration(const std::string& doc);

// nand/<system>/sync/agent_state.txt (what the agent changed itself) is at
// least as recent as nro_state.txt next to it (what PlayGuard saved last): the agent's records are
// then the newer ones, for PlayGuard to adopt. False when the agent never
// wrote any (its records are PlayGuard's).
bool agent_records_newer();

// nand/<system>/sync/agent_events.log: the orders the agent carried out itself while
// PlayGuard was closed, one tab-separated line each (docs/sync-protocol.md),
// for PlayGuard's change history.
struct AgentEvent
{
    uint64_t         ts = 0;           // POSIX
    std::string      entity, payload, reason, source;
    bool             applied = false;
    int              change = 0;       // SyncChange
    std::vector<int> before, after;
    int              console_lock_after = -1;
};
// The lines that parse, oldest first (a line with more fields than this
// version knows is read for the ones it knows). The others go to `rejected`,
// one per line, when given.
std::vector<AgentEvent> parse_agent_events(const std::string& text, std::string* rejected = nullptr);
// Takes the file (renamed first, so a line the agent appends meanwhile waits
// for the next time), parses it, and removes it once the lines it could not
// read are kept aside in agent_events.log.rejected.
std::vector<AgentEvent> take_agent_events();

}   // namespace sync_files
