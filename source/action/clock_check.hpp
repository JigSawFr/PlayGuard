// clock_check — Console › Network clock › "Check the network clock at start-up": one NTP sample
// in the background at start-up (none when the agent compared the clocks less
// than 4 h ago: its latest.conf says how far off), a toast when the network clock (the one the
// play timer counts on) is more than a minute off. It never sets anything:
// setting the clock stays in Console › Network clock.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace clock_check
{

void at_start();

}   // namespace clock_check
