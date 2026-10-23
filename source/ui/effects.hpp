// effects — Preferences › Visual effects: "full" (the default: the background
// glow, the rings' arcs on the move, the screens' and lists' entrances, the
// small answers to a press, on the battery as on the dock), "auto" (lighter
// while the console runs on its battery: the glow, the ring's pulse and
// every other movement left out) or "reduced" (no decorative movement at
// all, the glow still: for whom motion tires). On the battery: no charger
// connected (psm), read again every few seconds; the desktop:
// PLAYGUARD_SIM_BATTERY=1.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace effects
{
// The console runs on its battery.
bool on_battery();
// What the setting and the power ask for now: the lighter effects (no glow,
// nothing moving).
bool light();
// The setting is "reduced": nothing moves for show (the glow stays).
bool reduced();
// A decorative movement may be drawn now: neither light() nor reduced(),
// and not the desktop's still frames (PLAYGUARD_SIM_STILL_FOCUS, for the
// screenshots that compare). Every animation of the look asks this first.
bool motion();
}   // namespace effects
