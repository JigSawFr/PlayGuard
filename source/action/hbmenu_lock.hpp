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
// The menu's way back counts once the gate has opened it (its log): started
// from the HOME menu once. While locked, deleting PlayGuard's own forwarder,
// or the last protected menu or its PIN, is refused (forwarder_flow::remove).
// Locking is held 2 s on a list of the checks; the rescue sheet follows.
//
// Before locking, the forwarders made here and not protected are offered to
// be protected in one go (one that writes to the SD card could change the
// file). While locked, the lock gone from the file (a computer, a file
// manager) is told at the next start, never put back by itself.
//
// The rescue sysmodule, when it is installed, can lift it at boot (a RESCUE
// request; Atmosphère updated since, whose signature patches may lag): it
// leaves a report, told here at the next start. Nothing of the lock needs it.
// UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"

namespace hbmenu_lock
{

// Whether override_config.ini holds PlayGuard's lock.
bool locked();

// The way to a lock, step by step: a PIN, the patches, PlayGuard's
// forwarder, a protected menu, that menu started once from the HOME menu.
struct SetupStep
{
    enum Kind
    {
        Pin,
        Patches,
        Self,
        Menu,
        Verify,
    } kind;
    std::string label;
    bool        done = false;
};
std::vector<SetupStep> setup_steps(const forwarders_logic::LockCheck& c);
// A step done (`done(changed)`), or said how (a dialog, no `done`).
void run_step(SetupStep::Kind kind, std::function<void(bool)> done);
// The steps in a dialog, the next one a press away, again after each that
// makes a forwarder; the lock's confirmation once they are all done.
// `done(changed)` after each change.
void setup(std::function<void(bool)> done);

// Locks it (the checks, what it changes, confirmed) or unlocks it.
// `done(changed)` once it is over, also when refused or cancelled.
void lock(std::function<void(bool)> done);
void unlock(std::function<void(bool)> done);

// Once per start (the main screen): the sysmodule's report told and
// recorded; while locked, a way back gone told (the bell); a forwarder
// PlayGuard made that is no longer on the HOME menu told once (the change
// history, the bell). `open` opens Console › Forwarders, from the bell.
void at_start(std::function<void()> open);

// What Atmosphère will read and what the lock rests on, in a dialog
// (Console › Forwarders › Check the homebrew menu). Not locked, it offers
// the way to the lock; an outdated rescue module, its update. `done(changed)`
// after either.
void show_check(std::function<void(bool)> done = nullptr);
// The same for the diagnostic report (util/diagnostics), in English.
std::string report();

// How to undo the lock without the console (a computer, the rescue module),
// a QR code to the help: shown once it is locked, and from the check.
void show_rescue();

// What Console › Forwarders' card says.
struct Summary
{
    bool                        locked = false;
    bool                        module = false;   // the rescue module installed
    forwarders_logic::LockCheck check;
};
Summary summary();

// After a write under `dir` (an absolute path) failed: why, read now (the SD
// card full, or written to by nothing: a locked card, or the archive bit
// hekate fixes), in the UI's language; "" when neither shows.
std::string write_hint(const std::string& dir);

}   // namespace hbmenu_lock
