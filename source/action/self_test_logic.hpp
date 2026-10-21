// self_test_logic — the console self-test's report (action/self_test.hpp):
// what PlayGuard can check by itself before docs/hardware-test.md is gone
// through by hand, as Markdown to paste in a pull request. Plain C++ for the
// host tests (tests/self_test_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

namespace self_test_logic
{

enum class Result
{
    Pass,   // checked, as it should be
    Warn,   // works, with something to know
    Fail,   // not as it should be
    Skip,   // not checked here (not set up, or nothing to check)
};

struct Item
{
    std::string section;   // "Forwarders", "Homebrew menu", …
    Result      result = Result::Skip;
    std::string text;      // what was checked, and what was found
};

struct Header
{
    std::string version, firmware, storage, when;   // "1.4.0", "23.0.1 · Atmosphère 1.12.0", "sysMMC", "2026-10-21 08:30"
};

// "- [x] …" for a pass, "- [ ] ⚠ …" or "- [ ] ✗ …", "- [ ] – …" (not
// checked), under a "### <section>" per section in the order first met;
// a heading with the header's lines first, and the tally last.
std::string markdown(const Header& h, const std::vector<Item>& items);

// How many of each.
struct Tally
{
    int pass = 0, warn = 0, fail = 0, skip = 0;
};
Tally tally(const std::vector<Item>& items);

}   // namespace self_test_logic
