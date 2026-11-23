// Host tests for source/util/agent_latest.cpp: latest.conf as PlayGuard reads
// it (the versions, the clocks, a key it does not know, a value that is not
// one, a newer schema read as absent, nothing at all) and how long a reading
// holds.
#include "check.h"

#include <string>

#include "util/agent_latest.hpp"

static void test_parse()
{
    const agent_latest::Reading r = agent_latest::parse(
        "schema=1\nchecked=1792000000\natmosphere=1.12.0\nhekate=v6.5.4\nfirmware=23.0.1\nsyspatch=1.6.3.0\n"
        "integration=0.9.0\nclock_checked=1792003600\nclock_network=-2\nclock_user=75\n"
        "clock_server=europe.pool.ntp.org\nfuture_key=1\n");
    CHECK(r.versions.atmosphere == "1.12.0" && r.versions.hekate == "6.5.4" && r.versions.firmware == "23.0.1" &&
          r.versions.syspatch == "1.6.3.0" && r.integration == "0.9.0");
    CHECK(r.checked == 1792000000 && r.clock_checked == 1792003600);
    CHECK(r.clock_network == -2 && r.clock_user == 75 && r.clock_server == "europe.pool.ntp.org");

    // A value that is not a version is unknown, not a guess.
    const agent_latest::Reading odd = agent_latest::parse("schema=1\nchecked=5\nhekate=junk\nfirmware=23.0.1\n");
    CHECK(odd.versions.hekate.empty() && odd.versions.firmware == "23.0.1");

    // Absent, empty, or of a newer schema: nothing known.
    for (const char* none : { "", "not a conf file", "schema=2\nchecked=1792000000\nhekate=6.5.4\n" }) {
        const agent_latest::Reading n = agent_latest::parse(none);
        CHECK(n.checked == 0 && n.clock_checked == 0 && n.versions.hekate.empty() && n.versions.firmware.empty());
    }
}

static void test_fresh()
{
    agent_latest::Reading r;
    const int64_t now = 1792000000;
    CHECK(!agent_latest::versions_fresh(r, now) && !agent_latest::clock_fresh(r, now));   // never
    r.checked = now - 12 * 3600;
    r.clock_checked = now - 3 * 3600;
    CHECK(agent_latest::versions_fresh(r, now) && agent_latest::clock_fresh(r, now));
    r.checked = now - 24 * 3600;
    r.clock_checked = now - 4 * 3600;
    CHECK(!agent_latest::versions_fresh(r, now) && !agent_latest::clock_fresh(r, now));   // the limit is past
    r.checked = now + 60;
    r.clock_checked = now + 60;
    CHECK(!agent_latest::versions_fresh(r, now) && !agent_latest::clock_fresh(r, now));   // a clock set back
}

int main()
{
    test_parse();
    test_fresh();
    return CHECK_DONE("agent_latest tests passed");
}
