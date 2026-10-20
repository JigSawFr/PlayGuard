// sim_homebrew — the made-up homebrew of the desktop and web simulators, as
// .nro files on the simulated SD card (paths::sd_root()), so Console ›
// Forwarders has something to make a forwarder for. sim_forwarder.cpp.
// Never compiled for the Switch.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace sim_homebrew
{

// Writes those of them that are not there yet under `sd_root`/switch.
void seed(const std::string& sd_root);

}   // namespace sim_homebrew
