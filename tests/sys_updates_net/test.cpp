// Host tests for source/util/sys_updates_net.cpp, built twice from this file:
// - sys_updates_net (-D__SWITCH__, the console's path): fetch() asks each
//   repository's releases/latest page where it redirects (a fake
//   http::redirect below), in order, and reads the version off the tag at the
//   end of that URL; a page that does not redirect leaves its version empty
//   and the first failure in *error, the others still asked; a redirect
//   somewhere else than a release tag, or to a tag that is not a version,
//   leaves it empty too.
// - sys_updates_sim (the desktop build): PLAYGUARD_SIM_SYS_LATEST
//   ("1.12.0:6.5.4:23.0.1") in place of the network, nothing without it or
//   with "offline", a malformed value refused.
#include "check.h"
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "util/http.hpp"
#include "util/sys_updates.hpp"

// The fake network: the URL each releases/latest page redirects to, or
// absent (http::redirect fails with "no redirect").
static std::map<std::string, std::string> s_redirects;
static std::vector<std::string>          s_asked;

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
}   // namespace http

#ifdef __SWITCH__
static const std::string ATMOSPHERE = "https://github.com/Atmosphere-NX/Atmosphere/releases/latest";
static const std::string HEKATE     = "https://github.com/CTCaer/hekate/releases/latest";
static const std::string FIRMWARE   = "https://github.com/THZoria/NX_Firmware/releases/latest";

static void test_fetch()
{
    s_redirects = {
        { ATMOSPHERE, "https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.0" },
        { HEKATE, "https://github.com/CTCaer/hekate/releases/tag/v6.5.4" },
        { FIRMWARE, "https://github.com/THZoria/NX_Firmware/releases/tag/23.0.1?x=1" },
    };
    s_asked.clear();
    std::string error;
    sys_updates::Latest l = sys_updates::fetch(&error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate == "6.5.4" && l.firmware == "23.0.1");
    CHECK(error.empty());
    CHECK(s_asked == std::vector<std::string>({ ATMOSPHERE, HEKATE, FIRMWARE }));
    CHECK(sys_updates::fetch(nullptr).hekate == "6.5.4");   // no error wanted

    // hekate's page does not redirect: its version only is missing, and the
    // error names its repository; the firmware is still asked.
    s_redirects.erase(HEKATE);
    s_asked.clear();
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere == "1.12.0" && l.hekate.empty() && l.firmware == "23.0.1");
    CHECK(error == "CTCaer/hekate: no redirect");
    CHECK(s_asked.size() == 3);

    // Every page failing: the first failure is the one kept.
    s_redirects.clear();
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty());
    CHECK(error == "Atmosphere-NX/Atmosphere: no redirect");
    // An error already there is not replaced.
    error = "earlier";
    sys_updates::fetch(&error);
    CHECK(error == "earlier");

    // A redirect elsewhere (a login page, the releases list) or to a tag
    // that is not a version: nothing, and no error (the page did answer).
    s_redirects = {
        { ATMOSPHERE, "https://github.com/login?return_to=/releases" },
        { HEKATE, "https://github.com/CTCaer/hekate/releases" },
        { FIRMWARE, "https://github.com/THZoria/NX_Firmware/releases/tag/nightly" },
    };
    error.clear();
    l = sys_updates::fetch(&error);
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty());
    CHECK(error.empty());
    s_redirects = { { ATMOSPHERE, "https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.12.0-prerelease" },
                    { HEKATE, "https://github.com/CTCaer/hekate/releases/tag/v6.5.4/" },
                    { FIRMWARE, "https://github.com/THZoria/NX_Firmware/releases/tag/" } };
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
    CHECK(l.atmosphere == "1.12.0" && l.hekate == "6.5.4" && l.firmware == "23.0.1");
    CHECK(error.empty());

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
    CHECK(l.atmosphere.empty() && l.hekate.empty() && l.firmware.empty());   // "23.0.1:extra" is no version
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
