// Host tests for source/util/sys_updates.cpp: release tags as versions, the
// tag of a release page's URL, the newest firmware in switchbrew's
// System_Versions table, hekate's version in update.bin, and the text
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
    CHECK(version_of_tag("v1.6.3.0") == "1.6.3.0");   // sys-patch's
    CHECK(version_of_tag("1.2.3.4.5").empty());
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
    CHECK(tag_of_url("https://github.com/impeeza/sys-patch/releases/tag/v1.6.3.0/") == "v1.6.3.0");
    CHECK(tag_of_url("https://github.com/CTCaer/hekate/releases").empty());
    CHECK(tag_of_url("").empty());
}

// An excerpt of switchbrew's System_Versions page as its wikitext comes
// (action=raw): a header row, then a row per release, a cell per line.
// Made up for the test (dates and builds), in its layout.
static const char* VERSIONS_PAGE = R"({| class="wikitable"
|-
! System version 
! Release date
! Package1 build date
! [[NCA_Format|SDK versions]]
|-
| [[1.0.0]]
| Before console release
| 2016-11-21 18:30:08
| 
|-
| [[9.2.0]]
| March 3, 2020 (UTC)
| 2019-10-21 11:38:48
| 9.3.0
|-
| [[19.0.2]] (CHN-only)
| January 20, 2026 (UTC)
| 
| 19.3.0.0
|-
| [[22.5.0]]
| June 16, 2026 (UTC)
| 2026-01-23 11:18:04
| 22.2.0.0-22.2.2.0
|-
| [[23.0.1]]
| October 1, 2026 (UTC)
| 2026-07-30 13:54:23
| 23.3.0.0
|-
| [[23.0.0]]
| September 10, 2026 (UTC)
| 2026-07-30 13:54:23
| 23.2.2.0-23.3.0.0
|}
)";

static void test_firmware_of_versions()
{
    CHECK(firmware_of_versions(VERSIONS_PAGE) == "23.0.1");   // the highest, not the last row
    // Numbers compared as numbers: 10.0.0 above 9.2.0, 23.0.10 above 23.0.9.
    CHECK(firmware_of_versions("| [[9.2.0]]\n| [[10.0.0]]\n| [[2.3.0]]\n") == "10.0.0");
    CHECK(firmware_of_versions("| [[23.0.10]]\n| [[23.0.9]]\n") == "23.0.10");
    // A note after the version, a link with its own text, CRLF lines, indented.
    CHECK(firmware_of_versions("| [[24.0.0]] (CHN-only)\n| [[23.0.1]]\n") == "24.0.0");
    CHECK(firmware_of_versions("| [[24.1.0|24.1.0]]\r\n| [[23.0.1]]\r\n") == "24.1.0");
    CHECK(firmware_of_versions("  | [[24.0.1]]\n") == "24.0.1");
    // Not a version cell: the header's link, an SDK range, a page link, a
    // version of two or four numbers, other text in the brackets.
    CHECK(firmware_of_versions("! [[NCA_Format|SDK versions]]\n| 23.3.0.0\n| [[23.0]]\n| [[23.3.0.0]]\n"
                               "| [[Main Page]]\n| [[23.0.1-rc]]\n| [[v23.0.1]]\n| [[23..1]]\n")
              .empty());
    CHECK(firmware_of_versions("text [[30.0.0]]\n| [[23.0.1]]\n") == "23.0.1");   // only a cell's start
    // Nothing there: an empty page, an HTML error page, a cut row.
    CHECK(firmware_of_versions("").empty());
    CHECK(firmware_of_versions("<html><body>Too many requests</body></html>").empty());
    CHECK(firmware_of_versions("| [[23.0.1").empty());
    CHECK(firmware_of_versions("| [[").empty());
    CHECK(firmware_of_versions("| [").empty());
    // Absurdly long numbers are refused rather than overflowing.
    CHECK(firmware_of_versions("| [[123456789012345678901.0.0]]\n| [[23.0.1]]\n") == "23.0.1");
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
    l.syspatch   = "1.6.3.0";
    const std::string t = to_text(l);
    CHECK(t == "atmosphere=1.12.0;hekate=6.5.4;firmware=23.0.1;syspatch=1.6.3.0");
    const Latest back = from_text(t);
    CHECK(back.atmosphere == "1.12.0" && back.hekate == "6.5.4" && back.firmware == "23.0.1" &&
          back.syspatch == "1.6.3.0");
    // Missing ones are left out, unknown keys and bad versions ignored.
    Latest some;
    some.hekate = "6.5.4";
    CHECK(to_text(some) == "hekate=6.5.4");
    const Latest odd = from_text("firmware=junk;hekate=v6.5.4;other=1.0.0;atmosphere");
    CHECK(odd.firmware.empty() && odd.hekate == "6.5.4" && odd.atmosphere.empty());
    CHECK(to_text(Latest{}).empty());
    const Latest none = from_text("");
    CHECK(none.atmosphere.empty() && none.hekate.empty() && none.firmware.empty() && none.syspatch.empty());
    // A sys_latest kept before sys-patch was checked: it is just unknown.
    CHECK(from_text("atmosphere=1.12.0;hekate=6.5.4;firmware=23.0.1").syspatch.empty());
}

static void test_syspatch_version()
{
    CHECK(syspatch_version("v1.6.3.0") == "1.6.3.0");
    CHECK(syspatch_version("v1.5.9") == "1.5.9");
    CHECK(syspatch_version("v1.6.3.0-b962f75") == "1.6.3.0");   // with its commit
    CHECK(syspatch_version("1.5.8") == "1.5.8");
    CHECK(syspatch_version("").empty());
    CHECK(syspatch_version("unknown").empty());
    CHECK(syspatch_version("-1.5.8").empty());
}

int main()
{
    test_version_of_tag();
    test_tag_of_url();
    test_firmware_of_versions();
    test_hekate_version();
    test_text();
    test_syspatch_version();
    return CHECK_DONE("sys_updates tests passed");
}
