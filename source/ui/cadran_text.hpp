// cadran_text — the words the Cadran screens take apart before drawing them,
// with no view: a restriction's line and its state, a week's identical lines
// as one, a failure's title, reason and code. Translated through ui/tr.hpp, so
// a host suite checks them against the real catalogs (tests/cadran_text).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace cadran_text
{

// "Communication · restricted" (or "交流・制限") as what it is about and its
// state: { "Communication", "restricted" }; the whole text and "" when it has
// no separator. The last separator splits ("A · B · C": { "A · B", "C" }).
std::pair<std::string, std::string> split_state(const std::string& text);

// A table of five lines or more (a week): the lines that follow each other
// with the same value as one, their labels joined with " – " ("Monday –
// Friday"). Fewer lines are kept as they are.
std::vector<std::pair<std::string, std::string>> group_rows(const std::vector<std::pair<std::string, std::string>>& rows);

// A failure as ui::error() words it: "Could not unlock — error 0x0000C88E:
// refused by …\n\nIf it happens again, send a report: …" (playguard/error/*).
struct Failure
{
    std::string title;    // what failed, before " — " ("" when there is none)
    std::string reason;   // why, its first letter in capitals
    std::string code;     // "error 0x0000C88E" ("" when there is none)
    bool report = false;  // the text said to send a report
};
Failure failure(const std::string& text);

}   // namespace cadran_text
