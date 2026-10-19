// nintendo_dns — which of Nintendo's servers Atmosphère's DNS.mitm sends
// elsewhere on this console (util/dns_hosts): the parental-control one (the
// phone app cannot sync, action/pairing_state) and the time one (the network
// clock cannot set itself). The hosts file is read from the SD card once a
// run: Atmosphère itself reads it once, at start-up.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

namespace nintendo_dns
{

// `host` sent elsewhere: DNS.mitm on (system_settings.ini) and the hosts file
// Atmosphère reads naming it.
bool redirected(const char* host);

bool pctl_blocked();   // dns_hosts::PCTL_HOST
bool time_blocked();   // dns_hosts::TIME_HOST

}   // namespace nintendo_dns
