// play_data — the console's play data (core/playstats.h) read off the main
// thread and kept for the whole app: the Activity tab, its account filter and
// Today's "played today" share it, so the log is walked once per
// account and per few minutes rather than once per screen. The last read of
// each is also kept on the SD card (data_dir()/cache/, util/play_cache.hpp),
// so the next run shows it at once while it reads the log again. UI thread
// only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "util/pctl_ops_c.hpp"

namespace play_data
{

// Which data: every account ("") or one ("<uid hex>"), see key_of().
using Key = std::string;
Key key_of(const PlayAccount* account);   // nullptr: every account

// The last read for `key` (the last run's, from the SD card, before this
// run's first), or null when there is none.
std::shared_ptr<const PlayStats> latest(const Key& key = "");
// The last read for `key` was made by this run and is at most `max_age` old.
bool fresh(const Key& key, std::chrono::seconds max_age);
// A read for `key` is queued or running.
bool busy(const Key& key);

// Reads `account`'s data (nullptr: every account) unless a read for it already
// runs. Every listener is told when any read ends.
void fetch(const PlayAccount* account);

// Called on the UI thread after each read; returns an id for unlisten().
int  listen(std::function<void()> on_read);
void unlisten(int id);
// Calls every listener now, as at the end of a read.
void notify();

// A title's name: its own, a system program's (the HOME menu, the Album …:
// util/system_titles), the one recorded for a forwarder deleted since
// (action/title_names), "PlayGuard" for its own forwarder, "Homebrew (<title
// ID>)" for another homebrew, or
// "Deleted game (<title ID>)" for a game with none.
std::string game_name(const GameStat& g);
// A system program's name, translated; "" for any other title ID.
std::string system_name(u64 app_id);
// What an installed title without a name is called, from its 16-digit ID: a
// system program's name, a forwarder's recorded one, "Homebrew (<ID>)", else
// "Unknown game (<ID>)".
std::string unnamed(const std::string& id);
// The title's own name, a system program's or the one recorded; "" when none
// is known (names.txt, which the agent reads).
std::string known_name(const GameStat& g);

// The console's user accounts, read once per run (acc:u0 is quick).
const std::vector<PlayAccount>& accounts();

// Seconds played today, every game added up.
uint64_t today_total_s(const PlayStats& stats);

}   // namespace play_data
