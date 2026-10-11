// backup_logic — the decisions behind backup_flow, without any UI: what a
// backup keeps of the console's answers, how the restore list names a
// backup, and the tally of a restore's writes. Plain C++ over the
// service-layer structs so the host tests can run it (tests/backup_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include "util/backup.hpp"
#include "util/pctl_ops_c.hpp"

namespace backup_logic
{

// The console's settings as read (pctl_status_fetch, pctl_play_timer_query):
// a value that was not read, or that a restore would refuse, stays unset.
// `created` and `firmware` are left to the caller.
backup::Snapshot snapshot_of(const PctlStatus& st, const PtState& pt);

// A readable backup in the restore list: "2026-10-07 18:30 · 23.0.1", the
// date alone without a firmware, the file name without a date.
std::string list_label(const std::string& name, const backup::Snapshot& s);

// The writes of a restore: it goes on after a failure, and says at the end
// what failed (and the first error).
struct Tally
{
    Result                   first = 0;
    std::vector<std::string> failed;   // what failed, in the order tried
    size_t                   tried = 0;

    void add(Result rc, const std::string& what);
    // At least one write went through: the restore is worth recording.
    bool any_written() const { return tried > failed.size(); }
    // "Restriction level, VR mode".
    std::string failed_list() const;
};

}   // namespace backup_logic
