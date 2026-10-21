// dns_hosts — whether Atmosphère's DNS.mitm sends a host name elsewhere, read
// the way it reads its hosts file (docs/features/dns_mitm.md). A hosts file
// that blocks Nintendo redirects its parental-control server (the phone app
// can no longer overwrite what is set here) and its time server (the network
// clock no longer sets itself).
// Plain C++ (no libnx, no file access): host-tested (tests/dns_hosts).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dns_hosts
{

// The parental-control server the console synchronizes with ("api-%" on
// switchbrew's server list, "lp1" on production consoles).
constexpr const char* PCTL_HOST = "api-lp1.pctl.srv.nintendo.net";
// The server the console sets its network clock from ("aauth-%", its /v1/time;
// Atmosphère issue #2148): blocked, the clock no longer sets itself.
constexpr const char* TIME_HOST = "aauth-lp1.ndas.srv.nintendo.net";

// A host name against one of a hosts file's names: "*" any run of characters
// (none included), "%" the environment ("lp1"), letters in either case.
bool matches(const std::string& pattern, const std::string& host);

// Whether the hosts file `text` sends `host` elsewhere: a line "address
// name name …" naming it ("#" starts a comment).
bool redirects(const std::string& text, const std::string& host);

// The first line of `text` that sends `host` elsewhere, its comment and the
// spaces around it left out; "" when none does.
std::string redirecting_line(const std::string& text, const std::string& host);

// Whether system_settings.ini turns DNS.mitm off ([atmosphere]
// enable_dns_mitm = u8!0x0). On by default.
bool mitm_disabled(const std::string& ini);

// The emuMMC's id from emummc.ini ("id = 0x0002"), -1 when not given.
int emummc_id(const std::string& ini);

// The hosts files Atmosphère tries, in order, relative to the SD card's root,
// for `nand` ("sysmmc", "emummc", anything else: none, no Atmosphère seen):
// it reads the first one there.
std::vector<std::string> candidates(const std::string& nand, int emummc_id);

// What was read on the SD card: DNS.mitm turned off, and the hosts file
// Atmosphère reads ("" for none, its defaults only) with its text.
struct Seen
{
    bool mitm_off = false;
    std::string file, text;
};

// Where Atmosphère writes, at each start, the hosts file it loaded and every
// redirection it keeps, relative to the SD card's root.
constexpr const char* STARTUP_LOG = "atmosphere/logs/dns_mitm_startup.log";
// The last lines of that log the report keeps.
constexpr size_t STARTUP_LOG_LINES = 40;

// One line for the session log: "DNS.mitm on, hosts file
// atmosphere/hosts/default.txt: parental-control server redirected, time
// server reachable".
std::string summary(const Seen& seen);

// The diagnostic report's section: the same, the line that redirects each
// server, and the end of Atmosphère's start-up log (`startup_log`, "" when
// `has_startup_log` is false: not on the SD card).
std::string report(const Seen& seen, bool has_startup_log, const std::string& startup_log);

}   // namespace dns_hosts
