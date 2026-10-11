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
// history.json, watch.json, github_token, rescue_report.txt, the RESCUE
// request, each with its .tmp / .bad / .bad.1) and folders (profiles,
// backups, exports, cache, logs). The .nro, LICENSE.txt and anything else
// stay. A folder already in `to` gets what it lacks, file by file; a file
// already in `to` is not replaced. Returns how many entries were moved.
size_t run(const std::string& from, const std::string& to);

}   // namespace data_move
