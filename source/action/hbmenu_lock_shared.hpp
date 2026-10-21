// hbmenu_lock's pieces its three files share (hbmenu_lock.cpp: locking,
// unlocking, the checks at start; hbmenu_check.cpp: the check and the
// diagnostic's report; hbmenu_rescue.cpp: the rescue sheet). Not for
// anything else: hbmenu_lock.hpp is the way in.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"
#include "ui/ui.hpp"

namespace hbmenu_lock::detail
{

// `rel` from the SD card root ("/" on the console).
std::string on_sd(const char* rel);
std::string config_path();
std::string in_data(const char* name);
std::string read_config();

// What Console › Forwarders opens from the bell (at_start's `open`).
extern std::function<void()> s_open;

std::string joined(const std::vector<std::string>& names);

// Tried and refused at the gate three times or more on one day: the bell.
// A later read with none leaves it, until it is put away.
void tell_refusals(const std::vector<forwarders_logic::Refusals>& refusals);

// The checks, read now (with `take_log`, the gate's log first: a menu it
// opened is a way back seen working; UI thread only then).
forwarders_logic::LockCheck check(bool take_log = true);
// What locking still needs, one line each.
std::string missing(const forwarders_logic::LockCheck& c);
// The checks as a dialog's lines (the lock's confirmation, the check).
// With `open_now` (the check, not locked), the album and R said open first
// under what is still open.
std::vector<ui::CheckLine> check_lines(const forwarders_logic::LockCheck& c, bool open_now = false);

// The rescue sheet; `locked`: just locked, said in its title. Its page to
// print written again (the language, the module, as they are now).
void rescue_sheet(bool locked);

}   // namespace hbmenu_lock::detail
