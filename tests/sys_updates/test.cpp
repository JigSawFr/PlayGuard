// Host tests for source/util/sys_updates.cpp: release tags as versions, the
// tag of a release page's URL, hekate's version in update.bin, and the text
// config.json keeps.
#include "check.h"

#include <string>

#include "util/sys_updates.hpp"

using namespace sys_updates;

static void test_version_of_tag()
{
    CHECK(version_of_tag("1.12.0") == "1.12.0");
    CHECK(version_of_tag("v6.5.4") == "6.5.4");
    CHECK(version_of_tag("V6.5") == "6.5");
    CHECK(version_of_tag("23.0.1") == "23.0.1");
    CHECK(version_of_tag("").empty());
    CHECK(version_of_tag("v").empty());
    CHECK(version_of_tag("6").empty());          // one number: not a version
    CHECK(version_of_tag("1.2.3.4").empty());
    CHECK(version_of_tag("1.2.").empty());
    CHECK(version_of_tag(".1.2").empty());
    CHECK(version_of_tag("1..2").empty());
    CHECK(version_of_tag("1.2.0-rc1").empty());
    CHECK(version_of_tag("latest").empty());
}

static void test_tag_of_url()
{
    CHECK(tag_of_url("https://github.com/CTCaer/hekate/releases/tag/v6.5.4") == "v6.5.4");
    CHECK(tag_of_url("https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.0?x=1") == "1.12.0");
    CHECK(tag_of_url("https://github.com/THZoria/NX_Firmware/releases/tag/23.0.1/") == "23.0.1");
    CHECK(tag_of_url("https://github.com/CTCaer/hekate/releases").empty());
    CHECK(tag_of_url("").empty());
}

static void test_hekate_version()
{
    // ipl_ver as hekate lays it out: the magic, then '0' + major, minor, hotfix, then the release byte.
    std::string bin(0x118, '\0');
    bin += "ICTC";
    bin += std::string{ '0' + 6, '0' + 5, '0' + 4, 0 };
    bin += std::string(64, '\x7f');
    CHECK(hekate_version(bin) == "6.5.4");
    std::string ten(0x40, '\0');
    ten += "ICTC";
    ten += std::string{ '0' + 6, '0' + 10, '0' + 0, 0 };   // a minor of 10: ':' as a byte
    CHECK(hekate_version(ten) == "6.10.0");
    CHECK(hekate_version("").empty());
    CHECK(hekate_version(std::string(2048, 'x')).empty());
    // The magic beyond the first kilobyte: not hekate's header.
    std::string far(1100, '\0');
    far += "ICTC654";
    CHECK(hekate_version(far).empty());
    // Bytes that are not digits after the magic: looked further.
    std::string noise = "ICTC\xff\xff\xff" + std::string(8, '\0') + "ICTC" + std::string{ '0' + 6, '0' + 2, '0' + 2 };
    CHECK(hekate_version(noise) == "6.2.2");
    // Cut right after the magic.
    CHECK(hekate_version("ICTC65").empty());
}

static void test_text()
{
    Latest l;
    l.atmosphere = "1.12.0";
    l.hekate     = "6.5.4";
    l.firmware   = "23.0.1";
    const std::string t = to_text(l);
    CHECK(t == "atmosphere=1.12.0;hekate=6.5.4;firmware=23.0.1");
    const Latest back = from_text(t);
    CHECK(back.atmosphere == "1.12.0" && back.hekate == "6.5.4" && back.firmware == "23.0.1");
    // Missing ones are left out, unknown keys and bad versions ignored.
    Latest some;
    some.hekate = "6.5.4";
    CHECK(to_text(some) == "hekate=6.5.4");
    const Latest odd = from_text("firmware=junk;hekate=v6.5.4;other=1.0.0;atmosphere");
    CHECK(odd.firmware.empty() && odd.hekate == "6.5.4" && odd.atmosphere.empty());
    CHECK(to_text(Latest{}).empty());
    const Latest none = from_text("");
    CHECK(none.atmosphere.empty() && none.hekate.empty() && none.firmware.empty());
}

int main()
{
    test_version_of_tag();
    test_tag_of_url();
    test_hekate_version();
    test_text();
    return CHECK_DONE("sys_updates tests passed");
}
