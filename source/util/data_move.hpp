// data_move — the data folder moved from sd:/switch/playguard (next to the
// .nro) to sd:/config/playguard, where a reinstall (or a folder removed to
// "start clean") does not take the backups, the profiles and the change
// history with it. Run once at start-up, before config::load(); then the
// files of the logs/ layout before its subfolders are moved into them
// (log_files::migrate). Plain C++ (no borealis): host-tested (tests/log_files).
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
