// effects — Preferences › Visual effects, with the Cadran appearance: "auto"
// (the default: lighter while the console runs on its battery, the
// background glow, the ring's pulse and its arc's animation left out) or
// "full". On the battery: no charger connected (psm), read again every few
// seconds; the desktop: PLAYGUARD_SIM_BATTERY=1.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace effects
{
// The console runs on its battery.
bool on_battery();
// What the setting and the power ask for now: the lighter effects.
bool light();
}   // namespace effects
