// prodinfo — the console's real serial number while Atmosphère blanks it
// (blank_prodinfo_{sys,emu}mmc: the system then reports XAW00000000000).
// Before blanking, Atmosphère's ams_mitm copies the console's PRODINFO (its
// calibration data, CAL0) to atmosphere/automatic_backups/ on the SD card,
// unencrypted (amsmitm_prodinfo_utils.cpp: SaveProdInfoBackup). The serial
// number is at 0x250 in it; the device certificate (ECC at 0x480, else RSA at
// 0x3FC0) names the console, compared with spl's DeviceId: an SD card moved
// between consoles holds the backups of each.
// Plain C++ (no Aurore), so the host tests can run it on a fake SD card.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

namespace prodinfo
{

// The serial number in one CAL0 image ("" when it is not one, or holds the
// blanked or an empty serial).
std::string serial_of(const std::string& cal0);
// The console the image belongs to (its device certificate's DeviceId, low
// 56 bits). False when neither certificate holds one.
bool device_id_of(const std::string& cal0, uint64_t* out);

enum class Source
{
    None,       // no backup of a PRODINFO on the card
    Matched,    // the backup of this console (its DeviceId)
    Single,     // DeviceId unknown: the only serial backed up on the card
    Ambiguous,  // DeviceId unknown and several consoles backed up
    Others,     // backups only of other consoles
};

struct Found
{
    Source      source = Source::None;
    std::string serial;   // "" unless Matched or Single
    std::string file;     // the backup it was read from, relative to the SD card
};

// Looks in <root>/atmosphere/automatic_backups/ (*PRODINFO*.bin).
Found find(const std::string& root, bool device_id_valid, uint64_t device_id);

}   // namespace prodinfo
