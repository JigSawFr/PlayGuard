// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/pairing_state.hpp"

#include "action/nintendo_dns.hpp"

namespace pairing_state
{

bool dns_blocked()
{
    return nintendo_dns::pctl_blocked();
}

bool syncs(const PctlStatus& s)
{
    return s.pairing_active_ok && s.pairing_active && !dns_blocked();
}

}   // namespace pairing_state
