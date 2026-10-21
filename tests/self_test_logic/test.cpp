// Host tests for source/action/self_test_logic.cpp: the report's heading,
// its sections in the order met, each result's mark, line ends kept out of
// a line, the tally.
#include "check.h"

#include "action/self_test_logic.hpp"

using namespace self_test_logic;

int main()
{
    const Header h = { "1.4.0", "23.0.1 · Atmosphère 1.12.0", "sysMMC", "2026-10-21 08:30" };
    const std::vector<Item> items = {
        { "Console", Result::Pass, "A PIN on the console" },
        { "Forwarders", Result::Fail, "sphaira: not on the HOME menu" },
        { "Console", Result::Warn, "Signature patches: unknown\n(no log)" },
        { "Forwarders", Result::Skip, "Rescue module: not installed" },
    };
    const std::string md = markdown(h, items);
    CHECK(md.rfind("## PlayGuard console self-test\n\nPlayGuard 1.4.0 · 23.0.1 · Atmosphère 1.12.0 · sysMMC · 2026-10-21 08:30\n", 0) == 0);
    const size_t console = md.find("### Console\n\n- [x] A PIN on the console\n- [ ] \xE2\x9A\xA0 Signature patches: unknown (no log)\n");
    const size_t fw = md.find("### Forwarders\n\n- [ ] \xE2\x9C\x97 sphaira: not on the HOME menu\n- [ ] \xE2\x80\x93 Rescue module: not installed\n");
    CHECK(console != std::string::npos && fw != std::string::npos && console < fw);
    CHECK(md.find("1 passed, 1 to know, 1 failed, 1 not checked.") != std::string::npos);
    const Tally t = tally(items);
    CHECK(t.pass == 1 && t.warn == 1 && t.fail == 1 && t.skip == 1);
    CHECK(markdown(h, {}).find("0 passed, 0 to know, 0 failed, 0 not checked.") != std::string::npos);
    return CHECK_DONE("self_test_logic report, sections, marks and tally assertions passed");
}
