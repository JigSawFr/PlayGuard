// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/nintendo_dns.hpp"

#include <string>

#include "util/dns_hosts.hpp"
#include "util/paths.hpp"

namespace nintendo_dns
{

namespace
{
// The hosts file Atmosphère reads, "" when DNS.mitm is off or none is there.
const std::string& hosts()
{
    static const std::string text = []() {
        const std::string root = paths::sd_root();
        const auto sd = [&root](const std::string& path) { return root + (root.back() == '/' ? "" : "/") + path; };
        std::string read;
        if (paths::read_file(sd("atmosphere/config/system_settings.ini"), read) && dns_hosts::mitm_disabled(read))
            return std::string();
        int id = -1;
        if (paths::nand_key() == "emummc" && paths::read_file(sd("emuMMC/emummc.ini"), read)) id = dns_hosts::emummc_id(read);
        // The first hosts file there is the one Atmosphère reads.
        for (const std::string& file : dns_hosts::candidates(paths::nand_key(), id))
            if (paths::read_file(sd(file), read)) return read;
        return std::string();
    }();
    return text;
}
}   // namespace

bool redirected(const char* host)
{
    return !hosts().empty() && dns_hosts::redirects(hosts(), host);
}

bool pctl_blocked() { return redirected(dns_hosts::PCTL_HOST); }
bool time_blocked() { return redirected(dns_hosts::TIME_HOST); }

}   // namespace nintendo_dns
