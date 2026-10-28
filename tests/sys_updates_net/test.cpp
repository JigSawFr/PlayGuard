// Host tests for source/util/sys_updates_net.cpp, built twice from this file:
// - sys_updates_net (-D__SWITCH__, the console's path): fetch() asks each
//   repository's releases/latest page where it redirects (a fake
//   http::redirect below), in order, and reads the version off the tag at the
//   end of that URL; the firmware's is the newest in switchbrew's
//   System_Versions page (a fake http::get); a page that does not redirect,
//   or that cannot be read, leaves its version empty and the first failure in
//   *error, the others still asked; a redirect somewhere else than a release
//   tag, or to a tag that is not a version, or a page listing none, leaves it
//   empty too.
// - sys_updates_sim (the desktop build): PLAYGUARD_SIM_SYS_LATEST
//   ("1.12.0:6.5.4:23.0.1", sys-patch's after a fourth colon) in place of the network, nothing without it or
//   with "offline", a malformed value refused.
#include "check.h"
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "util/http.hpp"
#include "util/sys_updates.hpp"

// The fake network: the URL each releases/latest page redirects to, or
// absent (http::redirect fails with "no redirect"); the body of each page
// served, or absent (http::get fails with "not served").
static std::map<std::string, std::string> s_redirects;
static std::map<std::string, std::string> s_pages;
static std::vector<std::string>          s_asked;
static size_t                            s_max_asked;

namespace http
{
bool redirect(const std::string& url, std::string* target, std::string* error, const Headers&, long*)
{
    s_asked.push_back(url);
    const auto it = s_redirects.find(url);
    if (it == s_redirects.end()) {
        if (error) *error = "no redirect";
        return false;
    }
    *target = it->second;
    return true;
}
// switchbrew's System_Versions page; the integration's changelog
// (sys_updates::fetch_integration_changelog) is not asked for by these tests.
bool get(const std::string& url, std::string* body, std::string* error, size_t max_bytes, long, const Headers&, long*)
{
    s_asked.push_back(url);
    s_max_asked = max_bytes;
    const auto it = s_pages.find(url);
    if (it == s_pages.end()) {
        if (error) *error = "not served";
        return false;
    }
    *body = it->second;
    return true;
}
}   // namespace http

#ifdef __SWITCH__
static const std::string ATMOSPHERE = "https://github.com/Atmosphere-NX/Atmosphere/releases/latest";
static const std::string HEKATE     = "https://github.com/CTCaer/hekate/releases/latest";
static const std::string FIRMWARE   = "https://switchbrew.org/w/index.php?title=System_Versions&action=raw";
static const std::string SYSPATCH   = "https://github.com/impeeza/sys-patch/releases/latest";

// switchbrew's table, cut short (made-up dates), as action=raw serves it.
static const std::string VERSIONS = "{| class=\"wikitable\"\n|-\n! System version \n! Release date\n"
                                    "|-\n| [[22.5.0]]\n| June 16, 2026 (UTC)\n"
                                    "|-\n| [[23.0.1]]\n| October 1, 2026 (UTC)\n"
                                    "|-\n| [[23.0.0]]\n| September 10, 2026 (UTC)\n|}\n";

static void test_fetch()
{
    s_redirects = {
        { ATMOSPHERE, "https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.0" },
        { HEKATE, "https://github.com/CTCaer/hekate/releases/tag/v6.5.4" },
        { SYSPATCH, "https://github.com/impeeza/sys-patch/releases/tag/v1.6.3.0" },
    };
    s_pages = { { FIRMWARE, VERSIONS } };
    s_asked.clear();
    s_max_asked = 0;
    std::string error;
    sys_updates::Latest l = sys_updates::fetch(&error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate == "6.5.4" && l.firmware == "23.0.1" && l.syspatch == "1.6.3.0");
    CHECK(error.empty());
    CHECK(s_asked == std::vector<std::string>({ ATMOSPHERE, HEKATE, FIRMWARE, SYSPATCH }));
    CHECK(s_max_asked >= 64 * 1024 && s_max_asked <= 1024 * 1024);   // a bounded read, room to grow
    CHECK(sys_updates::fetch(nullptr).hekate == "6.5.4");   // no error wanted

    // hekate's page does not redirect: its version only is missing, and the
    // error names its repository; the firmware is still asked.
    s_redirects.erase(HEKATE);
    s_asked.clear();
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate.empty() && l.firmware == "23.0.1" && l.syspatch == "1.6.3.0");
    CHECK(error == "CTCaer/hekate: no redirect");
    CHECK(s_asked.size() == 4);

    // switchbrew's page cannot be read: the firmware only is missing, and
    // the error names switchbrew; sys-patch is still asked.
    s_redirects[HEKATE] = "https://github.com/CTCaer/hekate/releases/tag/v6.5.4";
    s_pages.clear();
    s_asked.clear();
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate == "6.5.4" && l.firmware.empty() && l.syspatch == "1.6.3.0");
    CHECK(error == "switchbrew: not served");
    CHECK(s_asked.size() == 4);

    // Every page failing: the first failure is the one kept.
    s_redirects.clear();
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty() && l.syspatch.empty());
    CHECK(error == "Atmosphere-NX/Atmosphere: no redirect");
    // An error already there is not replaced.
    error = "earlier";
    sys_updates::fetch(&error);
    CHECK(error == "earlier");

    // A redirect elsewhere (a login page, the releases list) or to a tag
    // that is not a version, a page that lists no version (an HTML page in
    // place of the wikitext): nothing, and no error (the page did answer).
    s_redirects = {
        { ATMOSPHERE, "https://github.com/login?return_to=/releases" },
        { HEKATE, "https://github.com/CTCaer/hekate/releases" },
        { SYSPATCH, "https://github.com/impeeza/sys-patch/releases/tag/v1.6.3.0.1" },
    };
    s_pages = { { FIRMWARE, "<!DOCTYPE html><html><body>Please wait</body></html>" } };
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty() && l.syspatch.empty());
    CHECK(error.empty());
    s_redirects = { { ATMOSPHERE, "https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.0-prerelease" },
                    { HEKATE, "https://github.com/CTCaer/hekate/releases/tag/v6.5.4/" } };
    s_pages     = { { FIRMWARE, "" } };
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere.empty() && l.hekate == "6.5.4" && l.firmware.empty());
}
#else
static sys_updates::Latest fetch_sim(const char* value, std::string* error)
{
    if (value) setenv("PLAYGUARD_SIM_SYS_LATEST", value, 1);
    else unsetenv("PLAYGUARD_SIM_SYS_LATEST");
    error->clear();
    return sys_updates::fetch(error);
}

static void test_fetch()
{
    std::string error;
    sys_updates::Latest l = fetch_sim("1.12.0:v6.5.4:23.0.1", &error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate == "6.5.4" && l.firmware == "23.0.1" && l.syspatch.empty());
    CHECK(error.empty());
    // sys-patch's after a fourth colon.
    l = fetch_sim("1.12.0:6.5.4:23.0.1:v1.6.3.0", &error);
    CHECK(l.firmware == "23.0.1" && l.syspatch == "1.6.3.0");

    // Nothing set, or "offline": no connection, and the network never asked.
    for (const char* none : { (const char*)nullptr, "offline" }) {
        l = fetch_sim(none, &error);
        CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty());
        CHECK(error == "simulated: no connection");
    }
    // Fewer than three parts.
    for (const char* bad : { "", "1.12.0", "1.12.0:6.5.4" }) {
        l = fetch_sim(bad, &error);
        CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty());
        CHECK(error == "simulated: malformed");
    }
    // Three parts, each read as a release tag: a bad one left empty.
    l = fetch_sim("junk::23.0.1:extra", &error);
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware == "23.0.1" && l.syspatch.empty());
    CHECK(error.empty());
    l = fetch_sim("1.12.0::23.0.1", &error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate.empty() && l.firmware == "23.0.1");
    CHECK(s_asked.empty());
    unsetenv("PLAYGUARD_SIM_SYS_LATEST");
}
#endif

int main()
{
    test_fetch();
#ifdef __SWITCH__
    return CHECK_DONE("sys_updates_net tests passed");
#else
    return CHECK_DONE("sys_updates_sim tests passed");
#endif
}
