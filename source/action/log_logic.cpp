// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/log_logic.hpp"

#include <cctype>

namespace log_logic
{

namespace
{
bool digit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

bool has_time(std::string_view s)
{
    return s.size() > 9 && digit(s[0]) && digit(s[1]) && s[2] == ':' && digit(s[3]) && digit(s[4]) && s[5] == ':' &&
           digit(s[6]) && digit(s[7]) && s[8] == ' ';
}

bool starts(std::string_view s, std::string_view p) { return s.substr(0, p.size()) == p; }
bool has(std::string_view s, std::string_view p) { return s.find(p) != std::string_view::npos; }
}   // namespace

Line parse(std::string_view raw)
{
    Line l;
    std::string_view text = raw;
    if (has_time(raw)) {
        l.time = std::string(raw.substr(0, 8));
        text = raw.substr(9);
        // PlayGuard's own lines are written with two spaces after the time.
        while (!text.empty() && text.front() == ' ') text.remove_prefix(1);
    }
    l.text = std::string(text);
    if (starts(text, "remote link") || starts(text, "link started") || starts(text, "order ") ||
        starts(text, "stopped:") || starts(text, "resumed:") || starts(text, "icon of")) {
        l.kind = Kind::Link;
    } else if (starts(text, "the loop") || starts(text, "agent ") || has(text, "watchdog")) {
        l.kind = Kind::Loop;
        l.alert = has(text, "has not turned");
    } else if (starts(text, "PlayGuard ")) {
        l.kind = Kind::App;
    }
    return l;
}

bool matches(const Line& l, Filter f)
{
    switch (f) {
        case Filter::All: return true;
        case Filter::App: return l.kind == Kind::App;
        case Filter::Link: return l.kind == Kind::Link;
        case Filter::Loop: return l.kind == Kind::Loop;
        case Filter::Count: break;
    }
    return false;
}

}   // namespace log_logic
