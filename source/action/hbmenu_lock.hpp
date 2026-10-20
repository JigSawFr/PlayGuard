// hbmenu_lock — Console › Forwarders › Lock the homebrew menu: the album and
// a game started while R is held no longer open the homebrew menu
// (core/hbl_lock.h, atmosphere/config/override_config.ini), so a homebrew
// starts only from its forwarder, which can ask for the PIN.
//
// Refused until there are two ways back to PlayGuard from the HOME menu:
// its own forwarder, and a menu of homebrew (hbmenu, sphaira) protected
// with the PIN (forwarders_logic::lock_check); and without a PIN on the
// console or the forwarders' signature patches. Locking and unlocking ask
// for the PIN whatever "Protect PlayGuard with the PIN" says, and are in
// the change history. The file as it was is kept in the data directory
// (HBL_LOCK_BACKUP_NAME); from a computer, deleting override_config.ini (or
// putting the backup back) and restarting undoes it.
//
// The rescue sysmodule, when it is installed, can lift it at boot (a RESCUE
// request; Atmosphère updated since, whose signature patches may lag): it
// leaves a report, told here at the next start. Nothing of the lock needs it.
// UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>

namespace hbmenu_lock
{

// Whether override_config.ini holds PlayGuard's lock.
bool locked();

// Locks it (the checks, what it changes, confirmed) or unlocks it.
// `done(changed)` once it is over, also when refused or cancelled.
void lock(std::function<void(bool)> done);
void unlock(std::function<void(bool)> done);

// Once per start (the main screen): the sysmodule's report told and
// recorded; while locked, a way back gone told (the bell); a forwarder
// PlayGuard made that is no longer on the HOME menu told once (the change
// history, the bell). `open` opens Console › Forwarders, from the bell.
void at_start(std::function<void()> open);

}   // namespace hbmenu_lock
