// log_logic — the remote link's journal (Accès à distance › Journal), without
// any UI: a line of the agent's log ("HH:MM:SS text", sync_flow::log_lines())
// split into its time and text, what it is about (the app, the link, the
// agent's loop) and the screen's filter. Host-tested (tests/log_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <string_view>

namespace log_logic
{

enum class Kind
{
    App,     // PlayGuard opened / closed
    Link,    // the remote link: online, the console slept, connecting again
    Loop,    // the agent's loop and its watchdog
    Other,   // a line this version does not know
};

struct Line
{
    std::string time;   // "HH:MM:SS", or {} when the line carries none
    std::string text;
    Kind        kind = Kind::Other;
    bool        alert = false;   // the loop stopped: shown apart
};

Line parse(std::string_view raw);

// The screen's filter: everything, or one kind.
enum class Filter
{
    All,
    App,
    Link,
    Loop,
    Count,
};

bool matches(const Line& l, Filter f);

}   // namespace log_logic
