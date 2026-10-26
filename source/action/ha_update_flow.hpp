// ha_update_flow — PlayGuard's Home Assistant integration (playguard-ha): the
// version Home Assistant runs, as the integration says it on the broker
// (docs/sync-protocol.md; kept in sync_files::integration_file() by the link
// that got it, PlayGuard's or the agent's), and its newer releases: in the
// notification bell and on Remote access, with what changed since the
// installed one and the QR code that opens it in HACS
// (activity/ha_update_activity). Checked with PlayGuard's other updates
// (once a day with "Check for updates every day", and by "Check for
// updates"), and only once the integration is known: without it, nothing
// goes online for it. UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include "util/changelog.hpp"

namespace ha_update_flow
{

// Start-up: what the link kept, the bell's entry, then a check in the
// background when one is due.
void at_start();
// Check for updates: a new check now, when the integration is known.
void check_now();

// PlayGuard's own link got the integration's document ("" cleared: the
// integration removed): kept on the SD card, then the bell's entry, and a
// check when the version is new and one is due.
void received(const std::string& doc);
// The agent holds the link: what it kept, read again.
void reload();

std::string installed();       // the integration's version ("0.1.0"), "" unknown
std::string homeassistant();   // Home Assistant's ("2026.10.1"), "" unknown
std::string newer();           // the newer release the last check found, "" none

// The newer release's screen.
void open();
// What changed since the installed version, from the changelog the last
// check kept; empty when it could not be fetched.
std::vector<changelog::Line> changes();

}   // namespace ha_update_flow
