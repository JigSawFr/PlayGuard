// ntp_servers — the built-in list of public NTP servers, grouped by region.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

namespace ntp
{

struct Region
{
    const char* id;          // also the i18n key suffix: playguard/clock/region/<id>
    std::vector<const char*> hosts;
};

const std::vector<Region>& regions();

// Pool for the console's region setting (Japan, Americas, Europe, Australia,
// Hong Kong/Taiwan/Korea, China), "pool.ntp.org" when unknown.
std::string default_server_for_console();

// The emoji a server is shown with, as its code points in hex (Twemoji's
// file names, resources/img/emoji/): its country's flag ("fr.pool.ntp.org",
// "ntp.nict.jp": "1f1eb-1f1f7", "1f1ef-1f1f5"), Europe's flag or the globe
// turned to its continent for a continent's pool, a globe for the worldwide
// pool; empty for a server of no country (a big provider's).
std::string host_emoji(const std::string& host);

// Two independent public servers used to cross-check a measurement.
std::vector<std::string> cross_check_servers(const std::string& primary);

}   // namespace ntp
