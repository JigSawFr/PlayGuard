// dns_hosts — whether Atmosphère's DNS.mitm sends a host name elsewhere, read
// the way it reads its hosts file (docs/features/dns_mitm.md): the phone app's
// synchronization goes through Nintendo's parental-control server
// (api-lp1.pctl.srv.nintendo.net), which a hosts file that blocks Nintendo
// redirects, so the app can no longer overwrite what is set here.
// Plain C++ (no libnx, no file access): host-tested (tests/dns_hosts).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

namespace dns_hosts
{

// The parental-control server the console synchronizes with ("api-%" on
// switchbrew's server list, "lp1" on production consoles).
constexpr const char* PCTL_HOST = "api-lp1.pctl.srv.nintendo.net";

// A host name against one of a hosts file's names: "*" any run of characters
// (none included), "%" the environment ("lp1"), letters in either case.
bool matches(const std::string& pattern, const std::string& host);

// Whether the hosts file `text` sends `host` elsewhere: a line "address
// name name …" naming it ("#" starts a comment).
bool redirects(const std::string& text, const std::string& host);

// Whether system_settings.ini turns DNS.mitm off ([atmosphere]
// enable_dns_mitm = u8!0x0). On by default.
bool mitm_disabled(const std::string& ini);

// The emuMMC's id from emummc.ini ("id = 0x0002"), -1 when not given.
int emummc_id(const std::string& ini);

// The hosts files Atmosphère tries, in order, relative to the SD card's root,
// for `nand` ("sysmmc", "emummc", anything else: none, no Atmosphère seen):
// it reads the first one there.
std::vector<std::string> candidates(const std::string& nand, int emummc_id);

}   // namespace dns_hosts
