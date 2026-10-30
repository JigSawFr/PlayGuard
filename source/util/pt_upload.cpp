// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/pt_upload.hpp"

#include <string>

#include "util/paths.hpp"
#include "util/pt_block.hpp"
#include "util/pt_log.hpp"

namespace pt_upload
{

std::vector<log_upload::Part> debug_files(bool play_timer_log)
{
    std::vector<log_upload::Part> out;
    std::string content;
    if (paths::read_file(pt_block::reference_path(), content))
        out.push_back({ "logs/play_timer/block.json", content });
    // The recorder's file can reach megabytes: its last lines only, and
    // only those read (one byte more than kept: tail() then cuts the same).
    if (play_timer_log && paths::read_ends(pt_log::path(), 0, LOG_BYTES + 1, content))
        out.push_back({ "logs/play_timer/log.csv", pt_log::tail(content, LOG_BYTES) });
    return out;
}

}   // namespace pt_upload
