// backup — snapshots of the parental-control settings saved on the SD card
// (sd:/config/playguard/backups/<YYYYMMDD_HHMMSS>.json) and restored from the
// History tab: restriction level, custom settings, VR mode and the per-day
// play-time limits. The PIN is never saved. Plain C++ (no libnx, no UI) so the
// host tests can run it; reading and writing the console is backup_flow's job.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "util/forwarders.hpp"

namespace backup
{

// The values a restore accepts (also checked before a history undo, which
// writes the same settings).
constexpr int MAX_LEVEL       = 4;        // PctlSafetyLevel_Teen
constexpr int MAX_RATING_AGE  = 21;
constexpr int MAX_RATING_ORG  = 12;
constexpr int MAX_DAY_MINUTES = 1440;
constexpr int DAY_NO_LIMIT    = 0xFFFF;

// The settings of the "Custom" level.
struct Custom
{
    uint8_t rating_age      = 0;   // 0 == no age restriction, else up to 21
    bool    sns_restricted  = false;
    bool    comm_restricted = false;
    bool    operator==(const Custom&) const = default;
};

// Console › Forwarders, not on the console's side: what PlayGuard made
// (forwarders.json's, as util/forwarders.hpp keeps it) and whether the
// homebrew menu was locked.
struct Forwarders
{
    std::vector<forwarders::Made> made;
    bool                          hbmenu_locked = false;
};

// Each value is there when it was read from the console when the backup was
// made: one that was not is neither saved nor restored. (Mind the
// std::optional<bool>s: `if (s.vr_restricted)` asks whether it is known,
// `*s.vr_restricted` what it is.)
struct Snapshot
{
    std::string created;    // local time, "2026-10-07 18:30" (shown in the list)
    std::string firmware;   // "23.0.1" (information only)

    std::optional<uint32_t> level;   // PctlSafetyLevel, 0..4
    std::optional<Custom>   custom;
    std::optional<bool>     vr_restricted;

    std::optional<std::array<uint16_t, 7>> days;   // Sun..Sat minutes, 0xFFFF == no limit

    // "Time's up" alarm off (1458 / 1953, a debug-class command: restored
    // only with the advanced play-timer actions on). Saved with the limits.
    std::optional<bool> alarm_disabled;

    // The default rating body (1037 / 1038), which the age limit is read with.
    std::optional<uint32_t> rating_org;

    // For the record, not restored: the 0x44 PlayTimerSettings block as read
    // (136 hex digits), to tell later what the fields PlayGuard does not
    // decode held.
    std::string raw_block;

    // Restored apart, after the rest (backup_flow): the ones missing
    // remembered again, then installed again if wanted.
    std::optional<Forwarders> forwarders;

    // Nothing a restore would write.
    bool empty() const
    {
        return !level.has_value() && !custom.has_value() && !vr_restricted.has_value() && !days.has_value() &&
               !rating_org.has_value();
    }
};

// What a restore would do, value by value, as its summary compares them with
// the console's: a value the console does not report counts as different,
// and one the restore leaves out (the alarm with the advanced play-timer
// actions off) writes nothing.
struct RestoreTally
{
    int same    = 0;   // already as in the backup
    int differ  = 0;   // listed as a change
    int written = 0;   // of those, the ones the restore writes

    void add(bool known, bool equal, bool restored = true);
    // Nothing to write: the restore only says so (no unlock, no write, no
    // history event).
    bool nothing_to_write() const { return written == 0; }
};

std::string to_json(const Snapshot& s);

// False when the text is not a backup of this format or a value is out of
// range (nothing from a damaged file is ever written to the console).
bool from_json(const std::string& text, Snapshot& out);

// Writes a new file in paths::backups_dir(). Returns its path, or "" and a
// short English reason in *error.
[[nodiscard]] std::string save(const Snapshot& s, std::string* error = nullptr);

// File names (not paths) in paths::backups_dir(), newest first.
std::vector<std::string> list();

bool load(const std::string& name, Snapshot& out);

// Deletes all but the `keep` newest backups (keep 0: deletes nothing).
// `keep_path` (the path save() just returned) is never deleted and counts as
// one of the `keep`. Returns how many were deleted.
size_t prune(size_t keep, const std::string& keep_path = "");

}   // namespace backup
