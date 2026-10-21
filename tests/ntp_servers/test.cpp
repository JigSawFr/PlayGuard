// Host tests for source/util/ntp_servers.cpp: the server picked for each
// console region (an unknown region, or one that cannot be read, giving
// pool.ntp.org), the built-in list (region ids and hosts unique, every host a
// plain host name config.json can keep, each region's default in it), and
// the two cross-check servers (never the primary, always two), and each
// server's emoji in the picker (a flag, a globe, none for a big provider; a
// file in resources/img/emoji/ for every built-in one, read from the
// repository root as `make test` runs).
#include "check.h"
#include <cstring>
#include <fstream>
#include <set>
#include <string>

#include "core/platform.h"
#include "util/ntp_servers.hpp"

// The console region platform_region() reports (core/platform.c reads
// SetRegion); `s_region_ok` false: it cannot be read.
static bool s_region_ok = true;
static int  s_region    = 2;

extern "C" bool platform_region(int* region)
{
    REQUIRE(region != nullptr);
    if (!s_region_ok) return false;
    *region = s_region;
    return true;
}

static std::string server_for(int region, bool ok = true)
{
    s_region_ok = ok;
    s_region    = region;
    return ntp::default_server_for_console();
}

static void test_default_server()
{
    CHECK(server_for(0) == "jp.pool.ntp.org");
    CHECK(server_for(1) == "north-america.pool.ntp.org");
    CHECK(server_for(2) == "europe.pool.ntp.org");
    CHECK(server_for(3) == "oceania.pool.ntp.org");
    CHECK(server_for(4) == "asia.pool.ntp.org");
    CHECK(server_for(5) == "cn.pool.ntp.org");
    // A region added by a later firmware, a nonsense value, none read.
    CHECK(server_for(6) == "pool.ntp.org");
    CHECK(server_for(-1) == "pool.ntp.org");
    CHECK(server_for(2, false) == "pool.ntp.org");
}

// A host name as config.json keeps one (config::MAX_HOST, 253) and a resolver
// takes it: lowercase labels of letters, digits and '-', none empty, none
// starting or ending with '-'.
static bool is_host(const std::string& h)
{
    if (h.empty() || h.size() > 253) return false;
    size_t start = 0;
    while (start <= h.size()) {
        size_t dot = h.find('.', start);
        if (dot == std::string::npos) dot = h.size();
        const std::string label = h.substr(start, dot - start);
        if (label.empty() || label.size() > 63 || label.front() == '-' || label.back() == '-') return false;
        for (char c : label)
            if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) return false;
        start = dot + 1;
    }
    return true;
}

static void test_regions()
{
    CHECK(is_host("pool.ntp.org") && is_host("ntp1.tencent.com"));
    CHECK(!is_host("") && !is_host("pool..org") && !is_host(".pool.org") && !is_host("pool.org."));
    CHECK(!is_host("-a.org") && !is_host("Pool.org") && !is_host("a b.org"));

    const auto& list = ntp::regions();
    CHECK(!list.empty());
    if (!CHECK(std::strcmp(list.front().id, "global") == 0)) return;   // the network clock screen's first choice
    CHECK(list.front().hosts.front() == std::string("pool.ntp.org"));

    std::set<std::string> ids, hosts;
    for (const auto& r : list) {
        CHECK(r.id != nullptr && *r.id);
        CHECK(ids.insert(r.id).second);   // an i18n key each: playguard/clock/region/<id>
        CHECK(!r.hosts.empty());
        for (const char* h : r.hosts) {
            CHECK(h != nullptr && is_host(h));
            CHECK(hosts.insert(h).second);   // listed once, in one region
        }
    }
    // What a console picks by itself is one of the servers it can pick by hand.
    for (int region = -1; region <= 6; region++) CHECK(hosts.count(server_for(region)) == 1);
    for (const auto& h : ntp::cross_check_servers("")) CHECK(hosts.count(h) == 1);
}

static void test_cross_check()
{
    using V = std::vector<std::string>;
    CHECK(ntp::cross_check_servers("europe.pool.ntp.org") == V({ "time.cloudflare.com", "pool.ntp.org" }));
    CHECK(ntp::cross_check_servers("") == V({ "time.cloudflare.com", "pool.ntp.org" }));
    CHECK(ntp::cross_check_servers("time.cloudflare.com") == V({ "pool.ntp.org", "time.google.com" }));
    CHECK(ntp::cross_check_servers("pool.ntp.org") == V({ "time.cloudflare.com", "time.google.com" }));
    CHECK(ntp::cross_check_servers("time.google.com") == V({ "time.cloudflare.com", "pool.ntp.org" }));
    // Compared as written: another spelling is another server.
    CHECK(ntp::cross_check_servers("POOL.NTP.ORG") == V({ "time.cloudflare.com", "pool.ntp.org" }));
}

static void test_host_emoji()
{
    CHECK(ntp::host_emoji("fr.pool.ntp.org") == "1f1eb-1f1f7");
    CHECK(ntp::host_emoji("uk.pool.ntp.org") == "1f1ec-1f1e7");   // Great Britain's flag
    CHECK(ntp::host_emoji("pool.ntp.org") == "1f310");
    CHECK(ntp::host_emoji("2.pool.ntp.org") == "1f310");
    CHECK(ntp::host_emoji("europe.pool.ntp.org") == "1f1ea-1f1fa");
    CHECK(ntp::host_emoji("south-america.pool.ntp.org") == "1f30e");
    CHECK(ntp::host_emoji("oceania.pool.ntp.org") == "1f30f");
    CHECK(ntp::host_emoji("africa.pool.ntp.org") == "1f30d");
    CHECK(ntp::host_emoji("ntp.nict.jp") == "1f1ef-1f1f5");
    CHECK(ntp::host_emoji("ntp.ntsc.ac.cn") == "1f1e8-1f1f3");
    CHECK(ntp::host_emoji("ntp.aliyun.com") == "1f1e8-1f1f3");
    // A big provider's, a name without a country, nothing.
    CHECK(ntp::host_emoji("time.cloudflare.com").empty());
    CHECK(ntp::host_emoji("time.nist.gov").empty());
    CHECK(ntp::host_emoji("ntp-server").empty());
    CHECK(ntp::host_emoji("192.168.1.1").empty());
    CHECK(ntp::host_emoji("").empty());
    // Each built-in server's emoji has its picture.
    for (const auto& r : ntp::regions())
        for (const char* h : r.hosts) {
            const std::string hex = ntp::host_emoji(h);
            if (hex.empty()) continue;
            std::ifstream in("resources/img/emoji/" + hex + ".png");
            CHECK(in.good());
        }
}

int main()
{
    test_default_server();
    test_regions();
    test_cross_check();
    test_host_emoji();
    return CHECK_DONE("ntp_servers tests passed");
}
