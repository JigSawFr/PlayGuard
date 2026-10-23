// forwarders_text — the words of Console › Forwarders that need no view: a
// forwarder's state, its rules in a line (Details) or in a word (the list),
// the icons at the end of its line, the days and hours of a rule.
// Translated through ui/tr.hpp, so a host suite checks them against the
// real catalogs (tests/forwarders_text).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"

namespace forwarders_text
{

// The days' short names, Sunday first (7 of them).
std::vector<std::string> day_names();
// "Sat–Sun 09:00–20:00"-like (forwarders_logic::hours_text in these names);
// with a second span (`days2`, only after a first one), both, ", " between.
std::string hours_label(int days, int from, int to);
std::string hours_label(int days, int from, int to, int days2, int from2, int to2);

// Its state alone: ready, its .nro missing, …
std::string state_only(const forwarders_logic::Row& r);

// Its state, and whether it asks for the PIN (and when it does not); a
// protected menu seen working through its gate. In the list (`full` false)
// a forwarder with more than one rule says so in a word: Details has them.
// `until`: when its pause ends (forwarder_flow::until_label), shown while
// it is paused (Row::paused_until).
std::string state_text(const forwarders_logic::Row& r, const std::string& until, bool full = false);

// In the list, a protected forwarder's rules are icons (list_marks), so its
// text is only what they cannot say: a state other than ready, the pause's
// end, made before it carried its rules.
std::string list_text(const forwarders_logic::Row& r, const std::string& until);

// One icon per thing turned on: the PIN (a key), hours without it (open
// lock), hours it never opens (stop), closed once today's time is used up
// (timer); paused after refused PINs, only that, as state_text says. A
// Cadran icon's name and its colour (GameCell::Mark's).
struct Mark
{
    const char* icon;
    const char* color_key;
};
std::vector<Mark> list_marks(const forwarders_logic::Row& r);

}   // namespace forwarders_text
