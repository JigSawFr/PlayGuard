// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/nintendo_dns.hpp"

#include <borealis.hpp>

#include "util/diagnostics.hpp"
#include "util/dns_hosts.hpp"
#include "util/paths.hpp"

namespace nintendo_dns
{

namespace
{
std::string sd(const std::string& path)
{
    const std::string root = paths::sd_root();
    return root + (root.back() == '/' ? "" : "/") + path;
}

// DNS.mitm turned off, and the hosts file Atmosphère reads.
const dns_hosts::Seen& seen()
{
    static const dns_hosts::Seen state = []() {
        dns_hosts::Seen out;
        std::string read;
        if (paths::read_file(sd("atmosphere/config/system_settings.ini"), read) && dns_hosts::mitm_disabled(read)) {
            out.mitm_off = true;
            return out;
        }
        int id = -1;
        if (paths::nand_key() == "emummc" && paths::read_file(sd("emuMMC/emummc.ini"), read)) id = dns_hosts::emummc_id(read);
        // The first hosts file there is the one Atmosphère reads.
        for (const std::string& file : dns_hosts::candidates(paths::nand_key(), id))
            if (paths::read_file(sd(file), read)) {
                out.file = file;
                out.text = read;
                break;
            }
        return out;
    }();
    return state;
}
}   // namespace

bool redirected(const char* host)
{
    return !seen().mitm_off && !seen().file.empty() && dns_hosts::redirects(seen().text, host);
}

bool pctl_blocked() { return redirected(dns_hosts::PCTL_HOST); }
bool time_blocked() { return redirected(dns_hosts::TIME_HOST); }

void at_start()
{
    diagnostic::set_dns(&report);
    brls::Logger::info("{}", dns_hosts::summary(seen()));
}

std::string report()
{
    std::string log;
    const bool has_log = paths::read_file(sd(dns_hosts::STARTUP_LOG), log);
    return dns_hosts::report(seen(), has_log, log);
}

}   // namespace nintendo_dns
