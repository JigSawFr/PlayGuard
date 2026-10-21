// Host tests for source/util/dns_hosts.cpp: Atmosphère's hosts-file names
// ("*", "%", case), a hosts file sending the parental-control server elsewhere
// or not (comments, several names a line, the telemetry defaults alone),
// system_settings.ini turning DNS.mitm off, the emuMMC's id, the files
// tried for each system, and what the session log and the diagnostic report
// say of it.
#include "check.h"

#include "util/dns_hosts.hpp"

static void test_matches()
{
    using dns_hosts::matches;
    CHECK(matches("api-lp1.pctl.srv.nintendo.net", dns_hosts::PCTL_HOST));
    CHECK(matches("api-%.pctl.srv.nintendo.net", dns_hosts::PCTL_HOST));
    CHECK(matches("*nintendo*", dns_hosts::PCTL_HOST));
    CHECK(matches("*.nintendo.net", dns_hosts::PCTL_HOST));
    CHECK(matches("*", dns_hosts::PCTL_HOST));
    CHECK(matches("API-LP1.PCTL.SRV.NINTENDO.NET", dns_hosts::PCTL_HOST));
    CHECK(matches("*pctl*", dns_hosts::PCTL_HOST));
    CHECK(!matches("receive-%.dg.srv.nintendo.net", dns_hosts::PCTL_HOST));
    CHECK(!matches("*.nintendo.com", dns_hosts::PCTL_HOST));
    CHECK(!matches("api-%.pctl.srv.nintendo.net.example", dns_hosts::PCTL_HOST));
    CHECK(!matches("", dns_hosts::PCTL_HOST));
    // The time server, by the same names.
    CHECK(matches("*nintendo.net", dns_hosts::TIME_HOST));
    CHECK(matches("aauth-%.ndas.srv.nintendo.net", dns_hosts::TIME_HOST));
    CHECK(!matches("api-%.pctl.srv.nintendo.net", dns_hosts::TIME_HOST));
    // "%" is the environment, not a wildcard.
    CHECK(!matches("api-%.pctl.srv.nintendo.net", "api-dd1.pctl.srv.nintendo.net"));
}

static void test_redirects()
{
    using dns_hosts::redirects;
    const char* blocking = "# 90DNS-like\n127.0.0.1 *nintendo.com\n127.0.0.1 *nintendo.net *nintendo.jp\n";
    CHECK(redirects(blocking, dns_hosts::PCTL_HOST));
    CHECK(redirects("95.216.149.205 *nintendo*", dns_hosts::PCTL_HOST));
    CHECK(redirects("\t127.0.0.1\tapi-%.pctl.srv.nintendo.net\r\n", dns_hosts::PCTL_HOST));
    // Atmosphère's defaults (telemetry only), a comment, an address alone.
    CHECK(!redirects("127.0.0.1 receive-%.dg.srv.nintendo.net receive-%.er.srv.nintendo.net\n", dns_hosts::PCTL_HOST));
    CHECK(!redirects("# 127.0.0.1 *nintendo*\n", dns_hosts::PCTL_HOST));
    CHECK(!redirects("127.0.0.1 # *nintendo*\n127.0.0.1\n", dns_hosts::PCTL_HOST));
    CHECK(!redirects("", dns_hosts::PCTL_HOST));
}

static void test_settings()
{
    using dns_hosts::mitm_disabled;
    CHECK(!mitm_disabled(""));
    CHECK(!mitm_disabled("[atmosphere]\nenable_dns_mitm = u8!0x1\n"));
    CHECK(mitm_disabled("[atmosphere]\nenable_dns_mitm = u8!0x0\n"));
    CHECK(mitm_disabled("[eupld]\nupload_enabled = u8!0x0\n[atmosphere]\n  enable_dns_mitm=u8!0x0 ; off\n"));
    // Another section, or commented out: still on.
    CHECK(!mitm_disabled("[usb]\nenable_dns_mitm = u8!0x0\n"));
    CHECK(!mitm_disabled("[atmosphere]\n; enable_dns_mitm = u8!0x0\n"));

    using dns_hosts::emummc_id;
    CHECK(emummc_id("[emummc]\nenabled=1\nsector=0x2\nid=0x0002\n") == 2);
    CHECK(emummc_id("[emummc]\nid = 0x001F\n") == 0x1F);
    CHECK(emummc_id("[emummc]\nenabled=1\n") == -1);
    CHECK(emummc_id("[emummc]\nid=zz\n") == -1);
}

static void test_candidates()
{
    using dns_hosts::candidates;
    const auto emu = candidates("emummc", 2);
    REQUIRE(emu.size() == 3);
    CHECK(emu[0] == "atmosphere/hosts/emummc_0002.txt");
    CHECK(emu[1] == "atmosphere/hosts/emummc.txt");
    CHECK(emu[2] == "atmosphere/hosts/default.txt");
    CHECK(candidates("emummc", -1).size() == 2);
    const auto sys = candidates("sysmmc", -1);
    REQUIRE(sys.size() == 2);
    CHECK(sys[0] == "atmosphere/hosts/sysmmc.txt");
    CHECK(sys[1] == "atmosphere/hosts/default.txt");
    CHECK(candidates("unknown", 0).empty());
}

static void test_redirecting_line()
{
    using dns_hosts::redirecting_line;
    CHECK(redirecting_line("127.0.0.1 receive-%.dg.srv.nintendo.net\n127.0.0.1 *nintendo.net  # all\n",
                           dns_hosts::PCTL_HOST) == "127.0.0.1 *nintendo.net");
    CHECK(redirecting_line("127.0.0.1 receive-%.dg.srv.nintendo.net\n", dns_hosts::PCTL_HOST).empty());
}

static bool has(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

static void test_summary()
{
    dns_hosts::Seen seen;
    seen.file = "atmosphere/hosts/default.txt";
    seen.text = "127.0.0.1 *pctl*\n";
    CHECK(dns_hosts::summary(seen) ==
          "DNS.mitm on, hosts file atmosphere/hosts/default.txt: parental-control server redirected, time server reachable");
    seen.mitm_off = true;   // the file is then not Atmosphère's
    CHECK(dns_hosts::summary(seen) ==
          "DNS.mitm off, hosts file not read (DNS.mitm off): parental-control server reachable, time server reachable");
    CHECK(has(dns_hosts::summary(dns_hosts::Seen()), "hosts file none (Atmosphère's defaults only)"));
}

static void test_report()
{
    dns_hosts::Seen seen;
    seen.file = "atmosphere/hosts/emummc.txt";
    seen.text = "95.216.149.205 *nintendo* # 90DNS\n";
    std::string out = dns_hosts::report(seen, false, "");
    CHECK(has(out, "=== atmosphere dns.mitm ===\n"));
    CHECK(has(out, "not a network test"));
    CHECK(has(out, "hosts file  : atmosphere/hosts/emummc.txt\n"));
    CHECK(has(out, "api-lp1.pctl.srv.nintendo.net redirected (\"95.216.149.205 *nintendo*\")\n"));
    CHECK(has(out, "aauth-lp1.ndas.srv.nintendo.net redirected"));
    CHECK(has(out, "dns_mitm_startup.log (none)\n"));

    // The start-up log: its last lines, indented, "\r" left out.
    std::string log;
    for (int i = 1; i <= 45; i++) log += "line " + std::to_string(i) + "\r\n";
    out = dns_hosts::report(dns_hosts::Seen(), true, log);
    CHECK(has(out, "reachable"));
    CHECK(has(out, "  (earlier lines left out)\n  line 6\n"));
    CHECK(!has(out, "line 5\n"));
    CHECK(has(out, "  line 45\n\n"));
    CHECK(!has(out, "\r"));
    out = dns_hosts::report(dns_hosts::Seen(), true, "one\ntwo");
    CHECK(has(out, "dns_mitm_startup.log\n  one\n  two\n"));
    CHECK(!has(out, "left out"));
}

int main()
{
    test_matches();
    test_redirects();
    test_settings();
    test_candidates();
    test_redirecting_line();
    test_summary();
    test_report();
    return CHECK_DONE("dns_hosts names, hosts files, settings, emuMMC id, file order and report assertions passed");
}
