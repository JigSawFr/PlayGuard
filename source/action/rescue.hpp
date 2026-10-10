// rescue — the app side of the "last chance" (core/rescue.h): at start-up it
// picks up the report the playguard-rescue sysmodule leaves after it acted on
// a RESCUE file, so PlayGuard can tell the parent what happened and let them
// finish (show / reset the PIN, delete everything). RescueActivity shows it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <optional>
#include <string>

extern "C" {
#include "core/rescue.h"
}

namespace rescue
{

// The report left on the SD card, read and then removed (it is shown once).
// None when there is no report or it is damaged (data_notice then tells the
// parent at start-up). Reading it records a "rescue" entry in the change history.
// *proof gets its proof (core/rescue.h), present only when it answers the
// challenge on the card (RESCUE_NONCE_NAME). Called at every start, which
// replaces the challenge: a report answers the one of the start before it.
std::optional<RescueReport> take(RescueProof* proof);

// Whether the console confirms `report` (rescue_report_confirmed, from a
// fresh read of 1206 and 1006). An unlock also needs `proof`, its MAC checked
// with the stored PIN (pctl_pin_hmac): a console a parent left temporarily
// unlocked confirms a report written by hand just as well, and only the
// sysmodule makes the proof, so a report that has one shows it is installed
// too. Anything else (no PIN left) needs the sysmodule on the SD card
// (installed()). False when a read fails.
// *old_module, when given, says why an unlock is not confirmed: the console
// agrees, but the report has no proof and the module installed is
// outdated(), one from before the proof (it writes none, at every RESCUE).
bool confirmed(const RescueReport& report, const RescueProof& proof, bool* old_module = nullptr);

// Whether the sysmodule is on the SD card
// (atmosphere/contents/4200000000505247/exefs.nsp): while it is, anyone who
// can edit the card can unlock or wipe the parental controls at boot.
bool installed();

// The revision of the module this PlayGuard comes with (tools/rescue_rev.cmake).
const char* revision();
// Whether the module is installed and is not the one this PlayGuard comes
// with: its playguard-rescue.txt says another revision, or there is none.
bool outdated();
// MainActivity, once per start: when the module is installed and is not the
// one this PlayGuard comes with (its playguard-rescue.txt says another
// revision, or there is none: installed before it was written), an entry of
// the notification bell says so, with Update when this build carries the
// module (bundled), else to install this release's playguard-rescue.zip.
// After an update, another entry says to restart the console, until the new
// module has run (it removes RESCUE_RESTART_PATH at boot).
void note_version();

// Whether this build carries its module (romfs:/sysmodules/rescue/, the one
// Console › Optional modules installs: Switch builds; the desktop one does not).
bool bundled();
// Replaces the installed module with the bundled one and writes its
// playguard-rescue.txt (this version, revision()), as extracting the zip
// would; its flags/boot2.flag is left as it is. It runs from the next boot:
// RESCUE_RESTART_PATH says so until then.
// False, with a short English reason in *error, when it could not.
bool update(std::string* error);

}   // namespace rescue
