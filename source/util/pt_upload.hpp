// pt_upload — the play-timer files "Send a report…" adds before the debug
// files of util/log_upload.hpp: logs/play_timer/block.json (Developer ›
// Compare the play-timer block) and logs/play_timer/log.csv (Developer ›
// Record the play timer: its last LOG_BYTES, header kept) only with
// `play_timer_log` (the recorder is on: it is what the developer asked to be
// sent; otherwise a child's play time stays home). Missing ones are left
// out. Plain C++ (no Aurore): host-tested (tests/log_upload).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <vector>

#include "util/log_upload.hpp"

namespace pt_upload
{

// The end of logs/play_timer/log.csv sent, at most (util/pt_log.hpp).
constexpr size_t LOG_BYTES = 48 * 1024;

std::vector<log_upload::Part> debug_files(bool play_timer_log = false);

}   // namespace pt_upload
