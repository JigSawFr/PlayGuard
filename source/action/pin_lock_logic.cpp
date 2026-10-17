// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/pin_lock_logic.hpp"

namespace pin_lock_logic
{

const char* const MODES[3] = { "off", "changes", "open" };

int rank(const std::string& mode)
{
    for (int i = 0; i < 3; i++)
        if (mode == MODES[i]) return i;
    return 0;
}

bool holds(Clock::time_point pin_at, Clock::time_point now, bool ask_again)
{
    if (pin_at == NEVER) return false;
    return !ask_again || now - pin_at < GRACE;
}

Check check(const std::string& mode, Clock::time_point now, Clock::time_point pin_at, bool ask_again,
            Clock::time_point refused_until)
{
    if (mode != "changes") return Check::Allow;
    if (holds(pin_at, now, ask_again)) return Check::Allow;
    if (now < refused_until) return Check::Refuse;
    return Check::Ask;
}

Change change(const std::string& from, int to, bool has_pin)
{
    if (to > 0 && !has_pin) return Change::NoPin;
    // Without a PIN there is nothing to ask: turning it down just goes ahead.
    if (to < rank(from) && has_pin) return Change::AskPin;
    return Change::Allowed;
}

bool lock_again(const std::string& mode, Clock::time_point away_since, Clock::time_point now,
                Clock::time_point pin_at, bool ask_again)
{
    if (mode != "open" || !ask_again) return false;
    if (now - away_since < GRACE) return false;
    return pin_at < away_since;
}

}   // namespace pin_lock_logic
