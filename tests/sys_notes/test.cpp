// Host tests for source/util/sys_notes.cpp: GitHub release lists
// (Atmosphère's and hekate's notes, trimmed), the firmware each release
// says it supports, switchbrew's version table and pages, the tabs a screen
// shows, what a firmware update needs, and the cache file.
#include "check.h"

#include <string>

#include "util/markdown.hpp"
#include "util/sys_notes.hpp"

using namespace sys_notes;

// Shortened from the real notes (GitHub sends CRLF line ends).
static const char* AMS_1121 =
    "**1.12.1** is Atmosphère's ninety-fourth official release.\r\n\r\n"
    "**Please be sure to update fusee when upgrading to 1.12.1**.\r\n\r\n"
    "The following was changed since the last release:\r\n"
    "* Support was added for the new c-ares resolver in `dns.mitm`.\r\n"
    "  * Firmware 23.0.0 introduced a new set of commands to the `sfdnsres` service.\r\n"
    "* General system stability improvements to enhance the user's experience.\r\n\r\n"
    "And the following was changed in 1.12.0:\r\n"
    "* Support was added for 23.0.0.\r\n"
    "* A new ConfigItem was added to the secure monitor for the first time since firmware 5.0.0.\r\n\r\n"
    "For information on the featureset supported by 1.12, please see [the official release notes](https://x).";
static const char* AMS_1120 =
    "**1.12.0** is Atmosphère's ninety-third official release.\r\n\r\n"
    "The following was changed since the last release:\r\n\r\n"
    "* Support was added for 23.0.0.\r\n"
    "* General system stability improvements to enhance the user's experience.";
static const char* AMS_1112 =
    "The following was changed since the last release:\r\n"
    "* Basic support was added for 22.5.0.\r\n\r\n"
    "And the following was changed in 1.11.0:\r\n"
    "* Support was added for 22.0.0.";
static const char* HEKATE_654 =
    "![hekate-nyx-banner](https://user-images.githubusercontent.com/3665130/60391760.png)\r\n\r\n"
    "**Supports booting ALL current CFWs, Android, Linux and payload tools.**\r\n\r\n"
    "### Latest HOS supported: 23.0.1\r\n\r\n\r\n"
    "hekate v6.5.4 changes\r\n-------------\r\n- **HOS 23.0.1 support**\r\n\r\n\r\n"
    "FAQ\r\n-------------\r\n\r\n    Click to expand FAQ notes\r\n\r\n"
    "- **Which Linux/Android distributions are supported in Partition Manager?**\r\n"
    " For L4T Linux you can use the official Ubuntu Bionic 3.4.0 and newer.\r\n\r\n"
    "Notes\r\n-------------\r\n\r\nCheck [readme.md](./README.md) for more.\r\n\r\n"
    "# How to update:\r\n- Drag n drop `bootloader` folder into sd card root.\r\n\r\n\r\n# Download:";

static std::string release_json(const std::string& tag, const std::string& body, bool draft = false,
                                bool prerelease = false, const std::string& published = "2026-10-16T00:05:51Z")
{
    auto esc = [](const std::string& s) {
        std::string o;
        for (char c : s) {
            if (c == '"' || c == '\\') o += '\\';
            if (c == '\r') o += "\\r";
            else if (c == '\n') o += "\\n";
            else o += c;
        }
        return o;
    };
    return "{\"tag_name\":\"" + tag + "\",\"html_url\":\"https://github.com/o/r/releases/tag/" + tag +
           "\",\"draft\":" + (draft ? "true" : "false") + ",\"prerelease\":" + (prerelease ? "true" : "false") +
           ",\"published_at\":\"" + published + "\",\"body\":\"" + esc(body) + "\",\"author\":{\"login\":\"x\"}}";
}

static void test_parse_github()
{
    const std::string json = "[" + release_json("1.12.1", AMS_1121) + "," + release_json("1.12.0", AMS_1120, false, false, "2026-09-27T23:21:53Z") +
                             "," + release_json("1.13.0-pre", "x", false, true) + "," + release_json("1.11.9", "x", true) +
                             "," + release_json("nightly", "x") + "," + release_json("1.12.0", "duplicate") + "," +
                             release_json("1.11.2", AMS_1112) + "]";
    std::vector<Release> out;
    CHECK(parse_github(json, Which::Atmosphere, &out));
    if (!CHECK(out.size() == 3)) return;
    CHECK(out[0].version == "1.12.1" && out[1].version == "1.12.0" && out[2].version == "1.11.2");
    CHECK(out[0].date == "2026-10-16" && out[1].date == "2026-09-27");
    CHECK(out[0].page == "https://github.com/o/r/releases/tag/1.12.1");
    // Its own changes only: the previous release's are cut, CRs gone.
    CHECK(out[0].notes.find("And the following") == std::string::npos);
    CHECK(out[0].notes.find("c-ares") != std::string::npos);
    CHECK(out[0].notes.find('\r') == std::string::npos);
    CHECK(out[1].notes.find("Support was added for 23.0.0") != std::string::npos);
    // What each says it supports, from the whole notes ("Firmware 23.0.0
    // introduced", "since firmware 5.0.0" and "supported by 1.12" are not claims).
    CHECK(out[0].supports == "23.0.0");
    CHECK(out[1].supports == "23.0.0");
    CHECK(out[2].supports == "22.5.0");
    // At most `keep`.
    std::vector<Release> one;
    CHECK(parse_github(json, Which::Atmosphere, &one, 1) && one.size() == 1);
    // Not a release list.
    std::vector<Release> none;
    CHECK(!parse_github("{\"message\":\"API rate limit exceeded\"}", Which::Atmosphere, &none));
    CHECK(!parse_github("not json", Which::Atmosphere, &none));
    CHECK(parse_github("[]", Which::Atmosphere, &none) && none.empty());
    // A page elsewhere than github.com is not kept.
    std::vector<Release> odd;
    CHECK(parse_github("[{\"tag_name\":\"v6.5.4\",\"draft\":false,\"prerelease\":false,\"html_url\":\"https://evil.example/x\",\"body\":\"\"}]",
                       Which::Hekate, &odd));
    CHECK(odd.size() == 1 && odd[0].version == "6.5.4" && odd[0].page.empty() && odd[0].date.empty());
}

static void test_hekate_notes()
{
    const std::string notes = trim_notes(Which::Hekate, std::string(HEKATE_654));
    CHECK(notes.find("![") == std::string::npos);      // the banner
    CHECK(notes.find("FAQ") == std::string::npos);     // the FAQ heading ...
    CHECK(notes.find("Partition Manager") == std::string::npos);   // ... and what it held
    CHECK(notes.find("Download") == std::string::npos);
    CHECK(notes.find("hekate v6.5.4 changes") != std::string::npos);
    CHECK(notes.find("How to update") != std::string::npos);
    CHECK(notes.find("Check [readme.md]") != std::string::npos);
    CHECK(notes.compare(0, 10, "**Supports") == 0);   // no blank lines first
    CHECK(supported_firmware(HEKATE_654) == "23.0.1");
    // The Markdown reads: hekate's setext headings are headings.
    const auto blocks = markdown::parse(notes);
    bool heading = false;
    for (const auto& b : blocks) heading = heading || (b.kind == markdown::Block::Heading && b.text == "hekate v6.5.4 changes");
    CHECK(heading);
    // Atmosphère's FAQ (were there one) stays: only hekate's is cut.
    CHECK(trim_notes(Which::Atmosphere, "FAQ\n---\ntext").find("text") != std::string::npos);
}

static void test_supported_firmware()
{
    CHECK(supported_firmware("* Support was added for 23.0.0.") == "23.0.0");
    CHECK(supported_firmware("* Basic support was added for 22.5.0") == "22.5.0");
    CHECK(supported_firmware("- **HOS 23.0.1 support**") == "23.0.1");
    CHECK(supported_firmware("### Latest HOS supported: 23.0.1") == "23.0.1");
    CHECK(supported_firmware("Adds support for firmware 21.1.0 and 21.0.1") == "21.1.0");
    CHECK(supported_firmware("Support for **24.0.0** was added") == "24.0.0");
    // The newest of several.
    CHECK(supported_firmware("Support was added for 22.0.0.\nBasic support was added for 22.5.0.") == "22.5.0");
    // Not claims.
    CHECK(supported_firmware("Firmware 23.0.0 introduced new commands.").empty());
    CHECK(supported_firmware("featureset supported by 1.12").empty());
    CHECK(supported_firmware("support was added for the new resolver").empty());
    CHECK(supported_firmware("Support was added for 23.0").empty());
    CHECK(supported_firmware("Support was added for 1.2.3.4").empty());
    CHECK(supported_firmware("").empty());
}

static const char* VERSIONS =
    "{| class=\"wikitable\"\n|-\n! System version \n! Release date\n! Package1 build date\n|-\n"
    "| [[22.0.0]]\n| March 3, 2026 (UTC)\n| 2026-01-10 10:00:00\n|-\n"
    "| [[22.5.0]]\n| June 16, 2026 (UTC)\n| \n|-\n"
    "| [[23.0.0]]\n| September 10, 2026 (UTC)\n| \n|-\n"
    "| [[23.0.1]]\n| October 1, 2026 (UTC)\n| \n|-\n"
    "| [[1.0.0]]\n| Before console release\n| \n|-\n"
    "| [[Foo|bar]]\n| March 3, 2026\n|}\n";

static void test_versions()
{
    const std::vector<Version> v = parse_versions(VERSIONS);
    if (!CHECK(v.size() == 5)) return;
    CHECK(v[0].version == "23.0.1" && v[0].date == "2026-10-01");
    CHECK(v[1].version == "23.0.0" && v[1].date == "2026-09-10");
    CHECK(v[3].version == "22.0.0" && v[3].date == "2026-03-03");
    CHECK(v[4].version == "1.0.0" && v[4].date.empty());
    CHECK(parse_versions(VERSIONS, 2).size() == 2);
    CHECK(parse_versions("").empty());
    CHECK(wiki_date("October 1, 2026 (UTC)") == "2026-10-01");
    CHECK(wiki_date("March 27, 2017") == "2017-03-27");
    CHECK(wiki_date("Before console release").empty());
    CHECK(wiki_date("October 41, 2026").empty());
    CHECK(wiki_date("October 1 2026").empty());
    CHECK(wiki_date("").empty());
}

static const char* PAGE_2300 =
    "The Switch N/A system update was released on September 10, 2026 (UTC). This Switch update was released for the "
    "following regions: ALL.\n\nSecurity flaws fixed: yes.\n\n"
    "==Change-log==\n"
    "[https://en-americas-support.nintendo.com/app/answers/detail/a_id/22525 Official] ALL change-log:\n"
    "* Added \xe2\x80\x9cSettings\xe2\x80\x9d to Virtual Game Cards from the HOME Menu.\n"
    "** You can review and edit which of your systems can use virtual game cards.\n"
    "* '''General''' system stability improvements.{{Ref}}<ref>note</ref>\n"
    "* Note: Please see our [https://www.nintendo.com/security-advisories/en/index.html security notice page].\n\n"
    "==System Titles==\n* The following titles were updated:\n"
    "** Sysmodules: usb, htc.stub, settings, pctl, npns.\n"
    "** Applets: qlaunch, auth.\n\n[[NPDM]] changes:\n* pctl: KernelCap updated.\n";

static void test_firmware_page()
{
    Release r;
    CHECK(parse_firmware_page(PAGE_2300, &r));
    CHECK(r.date == "2026-09-10");
    CHECK(r.security);
    CHECK(r.pctl);
    CHECK(r.notes.find("Official") == std::string::npos);   // the source line
    CHECK(r.notes.find("- Added \xe2\x80\x9cSettings\xe2\x80\x9d") == 0);
    CHECK(r.notes.find("\n  - You can review") != std::string::npos);
    CHECK(r.notes.find("- **General** system stability improvements.\n") != std::string::npos);
    CHECK(r.notes.find("security notice page.") != std::string::npos);
    CHECK(r.notes.find("Sysmodules") == std::string::npos);   // the next section
    CHECK(r.notes.find("{{") == std::string::npos && r.notes.find("<ref") == std::string::npos);

    Release hotfix;
    CHECK(parse_firmware_page("The Switch 23.0.1 system update was released on October 1, 2026 (UTC).\n\n"
                              "Security flaws fixed: <fill this in manually>.\n\n==Change-log==\n"
                              "* Fixed an issue from system version 23.0.0.\n\n==System Titles==\n"
                              "** Sysmodules: sdb.\n",
                              &hotfix));
    CHECK(!hotfix.security && !hotfix.pctl && hotfix.date == "2026-10-01");
    CHECK(hotfix.notes == "- Fixed an issue from system version 23.0.0.");
    // "pctl" only as a whole name: "pctlx" is another.
    Release other;
    CHECK(parse_firmware_page("==Change-log==\n* x\n==System Titles==\n** Sysmodules: pctlx, apctl.\n", &other));
    CHECK(!other.pctl);
    Release none;
    CHECK(!parse_firmware_page("No change-log here.\n==System Titles==\n", &none));

    CHECK(wiki_to_markdown("[[Foo|bar]] and [[Baz]] [https://x.org] [https://y.org text]") == "bar and Baz  text");
    CHECK(wiki_to_markdown("*** deep\n# numbered") == "    - deep\n- numbered");
    CHECK(firmware_page("23.0.1") == "https://switchbrew.org/wiki/23.0.1");
}

static List list_of(std::initializer_list<std::pair<const char*, const char*>> versions)
{
    List l;
    for (const auto& v : versions) {
        Release r;
        r.version = v.first;
        r.supports = v.second;
        l.releases.push_back(r);
    }
    return l;
}

static void test_tabs()
{
    const List l = list_of({ { "23.0.1", "" }, { "23.0.0", "" }, { "22.5.0", "" }, { "22.1.0", "" }, { "22.0.1", "" },
                             { "22.0.0", "" }, { "21.2.0", "" }, { "21.0.0", "" } });
    size_t hidden = 0;
    int at = 0;
    size_t from = 0;
    // The older ones in the tabs left, the installed one, then the newer ones
    // from the closest.
    std::vector<size_t> t = tabs(l, "22.5.0", &hidden, &at, &from);
    CHECK(t.size() == 6 && t[0] == 5 && t[2] == 3 && t[3] == 2 && t[4] == 1 && t[5] == 0 && hidden == 0 && at == 2 &&
          from == 4);
    // Six newer: the newest five, one hidden, no room for older ones.
    t = tabs(l, "21.2.0", &hidden, &at, &from);
    CHECK(t.size() == 6 && t[0] == 6 && t[1] == 4 && t[5] == 0 && hidden == 1 && from == 1);
    // Up to date: the five closest older ones, then the installed one.
    t = tabs(l, "23.0.1", &hidden, &at, &from);
    CHECK(t.size() == 6 && t[0] == 5 && t[4] == 1 && t[5] == 0 && hidden == 0 && at == 0 && from == 6);
    // The oldest listed: no older ones.
    t = tabs(l, "21.0.0", &hidden, &at, &from);
    CHECK(t.size() == 6 && t[0] == 7 && from == 1 && hidden == 2);
    // Not listed (older than the list, or a version in between): no installed tab.
    t = tabs(l, "20.0.0", &hidden, &at);
    CHECK(at == -1 && t.size() == 5 && hidden == 3 && t[4] == 0);
    t = tabs(l, "22.0.2", &hidden, &at, &from);
    CHECK(at == -1 && t.size() == 6 && t[1] == 4 && t[2] == 3 && t[5] == 0 && from == 2);
    // Unknown: every listed one is "newer".
    t = tabs(l, "", &hidden, &at);
    CHECK(at == -1 && t.size() == 5);
    // Two-number versions (hekate's tags may have them).
    const List h = list_of({ { "6.5.4", "" }, { "6.5", "" } });
    t = tabs(h, "6.5.0", &hidden, &at, &from);
    CHECK(at == 1 && t.size() == 2 && t[0] == 1 && from == 1);
    CHECK(newer_count(l, "22.0.0") == 5);
    CHECK(newer_count(l, "23.0.1") == 0);
    CHECK(newer_count(l, "") == 0);
}

static void test_support()
{
    const List ams = list_of({ { "1.12.1", "23.0.0" }, { "1.12.0", "23.0.0" }, { "1.11.2", "22.5.0" },
                               { "1.11.1", "" }, { "1.11.0", "22.0.0" } });
    CHECK(supports_up_to(ams, "1.12.1") == "23.0.0");
    CHECK(supports_up_to(ams, "1.11.1") == "22.0.0");   // the notes before it
    CHECK(supports_up_to(ams, "1.10.0").empty());
    CHECK(covers("23.0.0", "23.0.1"));    // a hotfix
    CHECK(covers("23.0.0", "23.0.0"));
    CHECK(covers("23.0.1", "22.5.0"));
    CHECK(!covers("23.0.0", "23.1.0"));
    CHECK(!covers("22.5.0", "23.0.0"));
    CHECK(!covers("", "23.0.0"));
    CHECK(first_supporting(ams, "23.0.1") == "1.12.0");
    CHECK(first_supporting(ams, "22.5.0") == "1.11.2");
    CHECK(first_supporting(ams, "24.0.0").empty());

    std::string v;
    // Supported by the installed one.
    CHECK(need_for(ams, "1.12.0", "23.0.1", &v) == Need::Nothing && v.empty());
    // By a newer one: update it first.
    CHECK(need_for(ams, "1.11.2", "23.0.1", &v) == Need::Update && v == "1.12.0");
    CHECK(need_for(ams, "1.9.0", "23.0.1", &v) == Need::Update && v == "1.12.0");
    // By none yet.
    CHECK(need_for(ams, "1.12.1", "24.0.0", &v) == Need::Wait && v == "1.12.1");
    CHECK(need_for(ams, "1.11.0", "24.0.0", &v) == Need::Wait && v == "1.12.1");
    // Not installed (hekate off the SD card): only Wait can be said.
    CHECK(need_for(ams, "", "23.0.1", &v) == Need::Nothing);
    CHECK(need_for(ams, "", "24.0.0", &v) == Need::Wait);
    // Nothing named in any notes, or nothing listed: nothing said.
    CHECK(need_for(list_of({ { "6.5.4", "" } }), "6.5.3", "24.0.0", &v) == Need::Nothing);
    CHECK(need_for(List{}, "1.12.0", "24.0.0", &v) == Need::Nothing);
    CHECK(need_for(ams, "1.12.0", "", &v) == Need::Nothing);
}

// PlayGuard's release page, as the release workflow writes it (shortened).
static const char PLAYGUARD_110[] =
    "## [1.1.0](https://github.com/JigSawFr/PlayGuard/compare/v1.0.0...v1.1.0) \xc2\xb7 2026-10-14\n\n"
    "<p>\xe2\xac\x87\xef\xb8\x8f <a href=\"https://x/playguard.zip\"><b>Download playguard.zip</b></a> (2.4 MB)</p>\n\n"
    "<p>\xf0\x9f\x9a\x80 <b>4</b> features</p>\n\n<hr>\n\n"
    "### \xf0\x9f\x8c\x9f Highlights\n\nThis release lets you choose.\n\n"
    "* \xe2\x8f\xb0 **When time is up**:  \n  Choose what the console does.\n\n"
    "<!-- console:hide -->\n> [!IMPORTANT]\n> Summary written automatically.\n<!-- /console -->\n"
    "<!-- console: Summary written by a robot. -->\n\n<hr>\n\n"
    "<!-- console:hide -->\n### \xf0\x9f\x93\x9c What changed\n<!-- /console -->\n\n"
    "<a name=\"every-change\"></a>\n\n<details>\n<summary><b>Every change in 1.1.0</b> (9 changes)</summary>\n\n"
    "### \xf0\x9f\x9a\x80 Features\n\n"
    "* **about:** the release highlights with icons ([#122](https://github.com/JigSawFr/PlayGuard/issues/122)) "
    "([0b8bd5d](https://github.com/JigSawFr/PlayGuard/commit/0b8bd5d9468946608d197c9c70440a5cf6c76084))\n"
    "* a link to [the guide](https://github.com/JigSawFr/PlayGuard/blob/main/docs/guide.md) (kept)\n\n"
    "</details>\n\n## \xf0\x9f\x96\xbc\xef\xb8\x8f On screen\n\n| Screen | Before |\n|---|---|\n\n"
    "## \xf0\x9f\x93\xa6 Install\n\n1. **New install**: download it.\n\n"
    "> [!NOTE]\n> Requires custom firmware (**Atmosph\xc3\xa8re**). Checked on firmware **21.0.0 \xe2\x86\x92 22.5.0**, supported.\n";

static void test_playguard_notes()
{
    const std::string notes = trim_notes(Which::PlayGuard, PLAYGUARD_110);
    // Its heading, the download and count lines and the anchor go.
    CHECK(notes.find("1.1.0](") == std::string::npos);
    CHECK(notes.find("Download") == std::string::npos && notes.find("<p>") == std::string::npos);
    CHECK(notes.find("every-change") == std::string::npos);
    // The highlights and every change stay; the console's own lines read.
    CHECK(notes.find("Highlights") != std::string::npos && notes.find("When time is up") != std::string::npos);
    CHECK(notes.find("Summary written by a robot.") != std::string::npos);
    CHECK(notes.find("Summary written automatically") == std::string::npos);
    CHECK(notes.find("What changed") == std::string::npos);
    CHECK(notes.find("Every change in 1.1.0") != std::string::npos);
    // A change without its pull request and commit; another link kept.
    CHECK(notes.find("* **about:** the release highlights with icons\n") != std::string::npos);
    CHECK(notes.find("#122") == std::string::npos && notes.find("0b8bd5d") == std::string::npos);
    CHECK(notes.find("[the guide](") != std::string::npos && notes.find("(kept)") != std::string::npos);
    // What follows the notes: left out.
    CHECK(notes.find("On screen") == std::string::npos && notes.find("Install") == std::string::npos);
    CHECK(notes.find("</details>") != std::string::npos);

    // From a release list: no firmware it "supports" (its notes say what it was checked on).
    std::vector<Release> out;
    CHECK(parse_github("[" + release_json("v1.1.0", PLAYGUARD_110) + "]", Which::PlayGuard, &out));
    if (!CHECK(out.size() == 1)) return;
    CHECK(out[0].version == "1.1.0" && out[0].supports.empty() && out[0].notes == notes);
    CHECK(std::string(key(Which::PlayGuard)) == "playguard" && std::string(repo(Which::PlayGuard)) == "JigSawFr/PlayGuard");

    // Kept in the cache with the others.
    Cache c;
    c.playguard.releases = out;
    c.playguard.fetched = "2026-10-18";
    Cache back;
    CHECK(decode(encode(c), &back));
    CHECK(back.playguard.fetched == "2026-10-18" && back.playguard.releases.size() == 1);
    CHECK(back.playguard.releases[0].notes == notes && back.of(Which::PlayGuard).releases[0].version == "1.1.0");
    CHECK(back.atmosphere.releases.empty());
}

static void test_cache()
{
    Cache c;
    c.atmosphere = list_of({ { "1.12.1", "23.0.0" } });
    c.atmosphere.fetched = "2026-10-16";
    c.atmosphere.releases[0].notes = "* line\n\xe2\x80\x8bhidden";
    c.atmosphere.releases[0].page = "https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.1";
    c.atmosphere.releases[0].date = "2026-10-16";
    c.firmware = list_of({ { "23.0.1", "" } });
    c.firmware.releases[0].security = true;
    c.firmware.releases[0].pctl = true;
    c.firmware.releases[0].page = firmware_page("23.0.1");
    Cache back;
    CHECK(decode(encode(c), &back));
    CHECK(back.atmosphere.fetched == "2026-10-16" && back.atmosphere.releases.size() == 1);
    const Release& r = back.atmosphere.releases[0];
    CHECK(r.version == "1.12.1" && r.supports == "23.0.0" && r.date == "2026-10-16");
    CHECK(r.notes == "* line\nhidden");   // cleaned again (a zero-width space)
    CHECK(r.page == c.atmosphere.releases[0].page);
    CHECK(back.hekate.releases.empty());
    CHECK(back.firmware.releases.size() == 1 && back.firmware.releases[0].security && back.firmware.releases[0].pctl);
    CHECK(back.firmware.releases[0].page == "https://switchbrew.org/wiki/23.0.1");
    // An edited file: what is not valid is left out.
    Cache odd;
    CHECK(decode("{\"atmosphere\":{\"fetched\":\"yesterday\",\"releases\":[{\"version\":\"x\"},{\"version\":\"1.0.0\","
                 "\"page\":\"https://evil.example/\",\"supports\":\"soon\",\"security\":\"yes\"},7]},\"hekate\":[]}",
                 &odd));
    CHECK(odd.atmosphere.fetched.empty() && odd.atmosphere.releases.size() == 1);
    CHECK(odd.atmosphere.releases[0].page.empty() && odd.atmosphere.releases[0].supports.empty());
    CHECK(!odd.atmosphere.releases[0].security);
    Cache bad;
    CHECK(!decode("[]", &bad));
    CHECK(!decode("{", &bad));
    CHECK(decode("{}", &bad) && bad.firmware.releases.empty());

    // merge: fresh releases, the notes a firmware page gave kept.
    List kept = list_of({ { "23.0.0", "" }, { "22.5.0", "" } });
    kept.releases[0].notes = "- old notes";
    kept.releases[0].security = true;
    kept.fetched = "2026-10-01";
    List fresh = list_of({ { "23.0.1", "" }, { "23.0.0", "" }, { "22.5.0", "" } });
    fresh.releases[2].notes = "- fresh";
    fresh.fetched = "2026-10-16";
    merge(&kept, fresh);
    CHECK(kept.releases.size() == 3 && kept.fetched == "2026-10-16");
    CHECK(kept.releases[1].notes == "- old notes" && kept.releases[1].security);
    CHECK(kept.releases[2].notes == "- fresh");
    CHECK(kept.releases[0].notes.empty());
}

// sys-patch's: four numbers to a tag ("v1.6.3.0"), compared in full.
static void test_syspatch()
{
    CHECK(std::string(key(Which::SysPatch)) == "syspatch" && std::string(repo(Which::SysPatch)) == "impeeza/sys-patch");
    CHECK(std::string(product(Which::SysPatch)) == "sys-patch");
    std::vector<Release> out;
    const std::string json = "[" + release_json("v1.6.3.0", "Thanks for adding the am hbxfix for firmware 22.0.0") + "," +
                             release_json("v1.6.2.3", "erpt fix - latest ams only") + "," +
                             release_json("v1.6.2.0", "Am homebrew fix") + "," + release_json("v1.6.0", "22.0.0 es fix") +
                             "]";
    CHECK(parse_github(json, Which::SysPatch, &out));
    if (!CHECK(out.size() == 4)) return;
    CHECK(out[0].version == "1.6.3.0" && out[1].version == "1.6.2.3" && out[3].version == "1.6.0");
    CHECK(out[0].notes == "Thanks for adding the am hbxfix for firmware 22.0.0");
    List l;
    l.releases = out;
    // 1.6.2.0 runs: two newer (1.6.2.3, 1.6.3.0), the fourth number counted.
    CHECK(newer_count(l, "1.6.2.0") == 2 && newer_count(l, "1.6.3.0") == 0 && newer_count(l, "1.6.2") == 2);
    size_t hidden = 0;
    int at = -1;
    size_t newer_from = 0;
    const std::vector<size_t> t = tabs(l, "1.6.2.0", &hidden, &at, &newer_from);
    CHECK(at == 2 && t.size() == 4 && t[1] == 2 && newer_from == 2 && t[3] == 0 && hidden == 0);
    // Kept in the cache with the others.
    Cache c;
    c.syspatch = l;
    c.syspatch.fetched = "2026-10-21";
    Cache back;
    CHECK(decode(encode(c), &back));
    CHECK(back.syspatch.fetched == "2026-10-21" && back.of(Which::SysPatch).releases.size() == 4);
    CHECK(back.syspatch.releases[1].version == "1.6.2.3" && back.atmosphere.releases.empty());
}

int main()
{
    test_parse_github();
    test_hekate_notes();
    test_playguard_notes();
    test_supported_firmware();
    test_versions();
    test_firmware_page();
    test_tabs();
    test_support();
    test_cache();
    test_syspatch();
    return CHECK_DONE("sys_notes GitHub and switchbrew parsing, tabs, support and cache assertions passed");
}
