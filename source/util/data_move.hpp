// data_move — the data folder moved from sd:/switch/playguard (next to the
// .nro) to sd:/config/playguard, where a reinstall (or a folder removed to
// "start clean") does not take the backups, the profiles and the change
// history with it. Run at start-up, before config::load(), as the steps of
// util/migrations.hpp (once each, not at every start); then the files of the
// logs/ layout before its subfolders are moved into them (log_files::migrate).
// Plain C++ (no Aurore): host-tested (tests/log_files).
//
// A shim for the 1.0.x layout, which 1.1.0 (October 2026) left: run() and
// to_nand() can go in the first release after April 2027, when an install
// still on 1.0.x is unlikely (it would then start without its old data,
// still in /switch/playguard, where a release note can point to it). Their
// steps' numbers in util/migrations.hpp go unused then.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace data_move
{

// The folder the app and its data shared before ("/switch/playguard").
std::string old_dir();

// Moves PlayGuard's data from `from` to `to`: its files (config.json,
// history.json, watch.json, github_token, rescue_report.txt, the remote
// link's sync.conf, the RESCUE request, each with its .tmp / .bad / .bad.1)
// and folders (profiles, backups, exports, cache, logs, sync). The .nro, LICENSE.txt and anything else
// stay. A folder already in `to` gets what it lacks, file by file; a file
// already in `to` is not replaced. Returns how many entries were moved.
size_t run(const std::string& from, const std::string& to);

// What belongs to one system, from `data_dir` (where it was before the
// per-system folders) to `nand_dir` (paths::nand_dir()): history.json and
// watch.json (with their .tmp / .bad / .bad.1), backups/, the play-data
// cache (cache/activity_*.bin) and the remote link's records and agent files
// (sync/ but profiles.txt). The first system PlayGuard starts on takes
// them; the others start without. Same rules as run(). Returns how many
// entries were moved.
size_t to_nand(const std::string& data_dir, const std::string& nand_dir);

}   // namespace data_move
