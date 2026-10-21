// nintendo_dns — which of Nintendo's servers Atmosphère's DNS.mitm sends
// elsewhere on this console (util/dns_hosts): the parental-control one (the
// phone app cannot sync, action/pairing_state) and the time one (the network
// clock cannot set itself). The hosts file is read from the SD card once a
// run: Atmosphère itself reads it once, at start-up. What was read goes to
// the session log and to the diagnostic report (at_start()).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace nintendo_dns
{

// `host` sent elsewhere: DNS.mitm on (system_settings.ini) and the hosts file
// Atmosphère reads naming it.
bool redirected(const char* host);

bool pctl_blocked();   // dns_hosts::PCTL_HOST
bool time_blocked();   // dns_hosts::TIME_HOST

// Logs what was read (dns_hosts::summary()) and gives the diagnostic report
// its section (report()).
void at_start();

// The diagnostic report's section: dns_hosts::report(), with the end of
// Atmosphère's start-up log read now.
std::string report();

}   // namespace nintendo_dns
