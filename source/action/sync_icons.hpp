// sync_icons — the game icons on the broker (docs/sync-protocol.md,
// `icon/<app id>`): the raw JPEG of each game played lately
// (action/sync_icons_logic picks them), retained, published once and
// cleared when the game leaves the set or publish_icons is turned off.
//
// PlayGuard's alone, whoever holds the link: an icon is up to 128 KiB, which
// the agent sysmodule's memory cannot spare. They go over a short connection
// of their own (`pg-<id>-icons`, no last will, nothing subscribed) on a
// thread of their own, one icon read at a time, so a batch never holds the
// session (its orders, its keep-alive) nor needs the agent: started while
// the link is online, closed once the batch is sent. UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "action/sync_icons_logic.hpp"
#include "sync/sync_conf.h"

namespace sync_icons
{

// After each read of the play log (sync_flow): the games played, the set
// is chosen from.
void set_played(const std::vector<sync_icons_logic::Played>& games);

// Every second while the link runs (sync_flow): takes a finished batch's
// results (icons.txt), then starts the next one when the link is `online`
// and something is left to send or clear (a failed connection waits a few
// minutes).
void step(const SyncConf& conf, bool online);

// Sync now: every icon of the set sent again.
void again();

// At exit: the batch still running stops after its current icon (waited for
// a few seconds, then left running, never detached: the console cannot).
void stop();

}   // namespace sync_icons
