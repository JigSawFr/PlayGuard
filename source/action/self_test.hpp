// self_test — Preferences › Help › Developer › Console self-test: what
// PlayGuard checks on this console by itself before docs/hardware-test.md
// (the console, the forwarders, the homebrew menu's lock), shown, and
// written as Markdown to config/playguard/self-test.md for the pull
// request. Changes nothing. UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include "action/self_test_logic.hpp"

namespace self_test
{

// The checks, read now.
std::vector<self_test_logic::Item> run();

// run(), the report written, its lines shown.
void open();

}   // namespace self_test
