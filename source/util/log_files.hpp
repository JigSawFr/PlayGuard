// log_files — how logs/ is laid out, and how much of it is kept:
//
//   logs/reports/report_20261011_142530.txt   saved diagnostic reports
//   logs/crashes/crash_20261011_142530.txt     one file per crash (main.cpp)
//   logs/play_timer/log.csv, log.old.csv       Developer › record the play timer
//   logs/play_timer/block.json                 Developer › play-timer block reference
//   logs/uploads.txt                           links of the reports sent online
//
// The reports and the crashes are pruned with the limits of the Tools tab
// (config: logs_keep, logs_keep_days, logs_keep_mb), each folder on its own.
// The date in the name (local time, as the console shows it) is what counts,
// not the file's date, which the SD card does not always keep.
// Plain C++ (no borealis): host-tested (tests/log_files).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace log_files
{

constexpr const char* REPORT = "report";
constexpr const char* CRASH  = "crash";

// "report_20261011_142530.txt", "report_20261011_142530_2.txt" for n = 2
// (another one in the same second). `stamp` is "YYYYMMDD_HHMMSS".
std::string name(const char* prefix, const std::string& stamp, unsigned n = 0);

// `t` as a name's stamp, in local time ("19700101_000000" when it cannot be
// converted).
std::string stamp(std::time_t t);

// Whether `name` is `prefix`_YYYYMMDD_HHMMSS[_n].txt; *stamp gets its date.
bool parse(const std::string& name, const char* prefix, std::string* stamp = nullptr);

// The names of `prefix` files in `dir`, newest first (the stamp sorts them;
// in a second, _n after the one without).
std::vector<std::string> list(const std::string& dir, const char* prefix);

// What is kept; 0: no limit for that one. A file goes when any limit says so.
struct Limits
{
    int count = 0;   // the newest `count`
    int days  = 0;   // dated less than `days` days before now
    int mb    = 0;   // the newest ones that fit in `mb` MB together
};

struct File
{
    std::string name;
    std::string stamp;
    uint64_t    bytes = 0;
};

// Which of `files` (newest first) go under `limits`, `cutoff` being the stamp
// `days` before now (stamp(now - days * 86400)). The newest file never goes:
// the report just saved, the crash that just happened.
std::vector<std::string> to_prune(const std::vector<File>& files, const Limits& limits, const std::string& cutoff);

// Removes the `prefix` files of `dir` that `limits` leave out at time `now`.
// Returns how many were removed.
size_t prune(const std::string& dir, const char* prefix, const Limits& limits, std::time_t now);

// Moves the files of the layout before the subfolders into them, in
// `logs_dir`: <stamp>[_n].txt to reports/report_<stamp>[_n].txt, crash.txt to
// crashes/crash_<date of its first line>.txt, play_timer_log.csv,
// play_timer_log.old.csv and play_timer_block.json to play_timer/. A file
// whose new name is taken stays where it is. Returns how many were moved.
size_t migrate(const std::string& logs_dir);

}   // namespace log_files
