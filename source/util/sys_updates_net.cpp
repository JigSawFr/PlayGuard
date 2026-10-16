// The network half of sys_updates.hpp (the rest stays plain C++ for the host tests).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <cstdlib>
#include <cstring>

#ifndef __SWITCH__
#include "sim/sim_net.hpp"
#endif
#include "util/http.hpp"
#include "util/sys_updates.hpp"

namespace sys_updates
{

namespace
{
// The version releases/latest redirects to; "" (and *error) when it does not.
[[maybe_unused]] std::string latest_of(const char* repo, std::string* error)
{
    std::string target, err;
    if (!http::redirect(std::string("https://github.com/") + repo + "/releases/latest", &target, &err)) {
        if (error && error->empty()) *error = std::string(repo) + ": " + err;
        return "";
    }
    return version_of_tag(tag_of_url(target));
}
}   // namespace

Latest fetch(std::string* error)
{
    Latest out;
#ifndef __SWITCH__
    // Desktop simulation: "1.12.0:6.5.4:23.0.1" (Atmosphère : hekate : firmware).
    // Without it, nothing: a smoke test never depends on the network.
    const char* sim = std::getenv("PLAYGUARD_SIM_SYS_LATEST");
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
    out.firmware   = version_of_tag(s.substr(b + 1));
    return out;
#else
    out.atmosphere = latest_of("Atmosphere-NX/Atmosphere", error);
    out.hekate     = latest_of("CTCaer/hekate", error);
    out.firmware   = latest_of("THZoria/NX_Firmware", error);
    return out;
#endif
}

}   // namespace sys_updates
