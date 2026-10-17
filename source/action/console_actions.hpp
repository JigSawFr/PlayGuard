// console_actions — the console's parental-control actions offered from more
// than one screen (the PIN tab, First steps, the recovery screen, the unlock
// banner and the bell): set a new PIN, unlink the phone app, delete
// everything, lock again. Each one refuses in read-only mode, asks what its
// screen used to ask, records what happened in the change history (a refusal
// of the console too) under `source` ("" for the PIN tab, "first_steps",
// "rescue"…), says it, then runs `after` (the screen refreshed).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

namespace console_actions
{

// The system PIN screen, which sets a new PIN (one when there is none); it
// blocks while shown. Left with B: nothing changed, nothing said or
// recorded. A failure is said, not recorded.
void set_pin(const std::string& source, std::function<void()> after);
// The phone app unlinked, once confirmed (the note on decoding the settings
// first: the block comparison needs the link).
void unlink(const std::string& source, std::function<void()> after);
// Every parental control deleted, after two confirmations and a settings
// backup (backup_flow::backup_then).
void delete_all(const std::string& source, std::function<void()> after);
// Locked again after a temporary unlock: no question.
void relock(const std::string& source, std::function<void()> after);

}   // namespace console_actions
