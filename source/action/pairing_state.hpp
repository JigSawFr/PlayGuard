// pairing_state — whether the phone app (Nintendo Switch Parental Controls)
// can overwrite what is set here: linked, and its server not sent elsewhere
// by Atmosphère's DNS.mitm (util/dns_hosts, a hosts file blocking Nintendo).
// Only then do the dialogs, the bell, the Overview and the first steps
// mention it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "util/pctl_ops_c.hpp"

namespace pairing_state
{

// The parental-control server redirected by DNS.mitm (action/nintendo_dns).
bool dns_blocked();

// Linked, as `s` read it, and not blocked.
bool syncs(const PctlStatus& s);

}   // namespace pairing_state
