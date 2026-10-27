// sync_flow — the remote link (MQTT, Home Assistant) while PlayGuard runs.
//
// When the agent sysmodule runs (util/agent_client, its pg:agent service), it
// holds the link: PlayGuard pushes the documents built below and carries out
// the orders the agent hands it, then answers them; nothing else changes.
// Otherwise PlayGuard holds the link itself ("autonomous mode",
// docs/sync-design.md):
//
// A worker thread owns the session (source/sync/sync_engine.c over TCP or
// the console's TLS) and never touches pctl or the play log: everything it
// publishes is built here on the UI thread, while the app is in the
// foreground (the state every poll_s seconds and right after a change,
// today's activity, the names, the week and the finished days after each
// read of the play log; the games' icons go over a connection of their own,
// action/sync_icons), and every order it receives is carried out here on
// the UI thread (action/sync_orders), one at a time, then answered. The
// settings are sync.conf (util/sync_files), written by the Sync screen.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "sync/sync_engine.h"

namespace sync_flow
{

// At start-up, before the first screen (main.cpp's choose_first(), after
// the PIN check is installed): reads sync.conf and starts the link when it is
// on; the agent's records are adopted then.
void start();
// At exit, after the main loop and before the background tasks are waited
// for: "offline", then the worker ends and is joined (waited for a few
// seconds; one still in a connection then is left running, never detached:
// the console cannot).
void stop();

// sync.conf changed (the Sync screen saved it): start, reconnect or stop.
void reload();
// Publish everything again now (the Sync screen, Home Assistant's button).
void sync_now();
// The console changed (a write, an unlock seen by a tab): publish the state
// at the next second rather than at the next poll.
void changed();

// Console › Optional modules, around the agent: before it is stopped (PlayGuard
// ends its session, the agent says "offline"; `then` once it did, or after
// 2 s; reload() afterwards hands the link to PlayGuard's own), and once
// started (its service looked for every 100 ms for 3 s, then the agent's
// session takes the link over). Neither waits on the UI thread.
void agent_stopping(std::function<void()> then);
void agent_started();
// While an update of the agent runs (action/agent_update): the link does not
// look for the agent, which comes and goes.
void hold(bool on);

struct Status
{
    bool        enabled  = false;   // sync.conf turns the link on
    bool        ready    = false;   // and has what it needs (host, user)
    bool        running  = false;   // the worker runs
    SyncStatus  link{};             // the worker's (or the agent's) last view
    size_t      pending  = 0;       // orders received, not answered yet
    bool        agent    = false;   // the agent sysmodule holds the link, PlayGuard pushes to it
    bool        agent_refused = false;   // it runs, but speaks another protocol (an update is due)
    bool        agent_read_only = false; // it refuses changes (sync/nro_state.txt)
    std::string agent_version;
    uint64_t    agent_quiet_ms = 0;      // since the agent's link last ran a turn (its status says)
};
Status status();

// The worker's last lines (newest last), for the Sync screen's log.
std::vector<std::string> log_lines();

// The diagnostic report's lines on the link: never the password, the
// broker's address or the user name.
std::string report_section();

}   // namespace sync_flow
