// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/pairing_state.hpp"

#include "util/dns_hosts.hpp"
#include "util/paths.hpp"

namespace pairing_state
{

bool dns_blocked()
{
    static const bool blocked = []() {
        const std::string root = paths::sd_root();
        const auto sd = [&root](const std::string& path) { return root + (root.back() == '/' ? "" : "/") + path; };
        std::string text;
        if (paths::read_file(sd("atmosphere/config/system_settings.ini"), text) && dns_hosts::mitm_disabled(text))
            return false;
        int id = -1;
        if (paths::nand_key() == "emummc" && paths::read_file(sd("emuMMC/emummc.ini"), text)) id = dns_hosts::emummc_id(text);
        // The first hosts file there is the one Atmosphère reads.
        for (const std::string& file : dns_hosts::candidates(paths::nand_key(), id))
            if (paths::read_file(sd(file), text)) return dns_hosts::redirects(text, dns_hosts::PCTL_HOST);
        return false;
    }();
    return blocked;
}

bool syncs(const PctlStatus& s)
{
    return s.pairing_active_ok && s.pairing_active && !dns_blocked();
}

}   // namespace pairing_state
