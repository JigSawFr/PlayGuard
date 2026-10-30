// The network half of sys_updates.hpp (the rest stays plain C++ for the host tests).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <cstdlib>
#include <cstring>

#include "core/sim_env.h"
#ifndef __SWITCH__
#include "sim/sim_net.hpp"
#endif
#include "util/http.hpp"
#include "util/paths.hpp"
#include "util/sys_updates.hpp"

namespace sys_updates
{

#ifdef __SWITCH__
namespace
{
// The version releases/latest redirects to; "" (and *error) when it does not.
std::string latest_of(const char* repo, std::string* error)
{
    const auto target = http::redirect(std::string("https://github.com/") + repo + "/releases/latest");
    if (!target) {
        if (error && error->empty()) *error = std::string(repo) + ": " + target.error().error;
        return "";
    }
    return version_of_tag(tag_of_url(*target));
}

// switchbrew's System_Versions table, as wikitext: about 6 KB.
constexpr const char* FIRMWARE_PAGE = "https://switchbrew.org/w/index.php?title=System_Versions&action=raw";
constexpr size_t      FIRMWARE_MAX  = 256 * 1024;

// The newest firmware switchbrew lists; "" (and *error) when its page
// cannot be read.
std::string latest_firmware(std::string* error)
{
    const auto page = http::get(FIRMWARE_PAGE, FIRMWARE_MAX, 20);
    if (!page) {
        if (error && error->empty()) *error = "switchbrew: " + page.error().error;
        return "";
    }
    return firmware_of_versions(page->body);
}
}   // namespace
#endif

Latest fetch(std::string* error)
{
    Latest out;
#ifndef __SWITCH__
    // Desktop simulation: "1.12.0:6.5.4:23.0.1" (Atmosphère : hekate : firmware),
    // then ":1.6.3.0" for sys-patch.
    // Without it, nothing: a smoke test never depends on the network.
    const char* sim = sim_env("PLAYGUARD_SIM_SYS_LATEST");
    if (!sim || std::strcmp(sim, "offline") == 0) {
        sim_net::offline(error);
        return out;
    }
    const std::string s = sim;
    const size_t a = s.find(':'), b = a == std::string::npos ? a : s.find(':', a + 1);
    if (b == std::string::npos) {
        if (error) *error = "simulated: malformed";
        return out;
    }
    out.atmosphere = version_of_tag(s.substr(0, a));
    out.hekate     = version_of_tag(s.substr(a + 1, b - a - 1));
    const size_t c = s.find(':', b + 1);
    out.firmware   = version_of_tag(s.substr(b + 1, c == std::string::npos ? c : c - b - 1));
    if (c != std::string::npos) out.syspatch = version_of_tag(s.substr(c + 1));
    return out;
#else
    out.atmosphere = latest_of("Atmosphere-NX/Atmosphere", error);
    out.hekate     = latest_of("CTCaer/hekate", error);
    out.firmware   = latest_firmware(error);
    out.syspatch   = latest_of("impeeza/sys-patch", error);
    return out;
#endif
}

namespace
{
[[maybe_unused]] constexpr const char* HA_REPO = "JigSawFr/playguard-ha";
[[maybe_unused]] constexpr size_t CHANGELOG_MAX = 256 * 1024;
}   // namespace

std::string fetch_integration(std::string* error)
{
#ifndef __SWITCH__
    // Desktop simulation: the release's tag ("v0.2.0"); without it, offline.
    const char* sim = sim_net::value("PLAYGUARD_SIM_HA_LATEST");
    if (!sim || std::strcmp(sim, "offline") == 0) {
        sim_net::offline(error);
        return "";
    }
    return version_of_tag(sim);
#else
    return latest_of(HA_REPO, error);
#endif
}

bool fetch_integration_changelog(const std::string& version, std::string* md, std::string* error)
{
#ifndef __SWITCH__
    (void)version;
    // Desktop simulation: a file holding the changelog.
    const char* sim = sim_net::value("PLAYGUARD_SIM_HA_CHANGELOG");
    if (!sim || !paths::read_file(sim, *md)) {
        if (error) *error = "simulated: no changelog";
        return false;
    }
    return true;
#else
    const std::string url = std::string("https://raw.githubusercontent.com/") + HA_REPO + "/v" + version + "/CHANGELOG.md";
    auto got = http::get(url, CHANGELOG_MAX, 30);
    if (!got) {
        if (error) *error = got.error().error;
        return false;
    }
    *md = std::move(got->body);
    return true;
#endif
}

}   // namespace sys_updates
