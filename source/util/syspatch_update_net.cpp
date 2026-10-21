// The network half of syspatch_update.hpp (the rest stays plain C++ for the host tests).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <cstdio>
#ifndef __SWITCH__
#include "sim/sim_net.hpp"
#endif

#include "util/dev_builds.hpp"
#include "util/github_auth.hpp"
#include "util/http.hpp"
#include "util/paths.hpp"
#include "util/syspatch_update.hpp"
#include "util/text.hpp"

namespace syspatch_update
{

namespace
{
#ifndef __SWITCH__
// PLAYGUARD_SIM_SYSPATCH: a folder whose release.json answers, its
// browser_download_url "https://<a file on this computer>"; "offline", or
// unset, no connection (the desktop build reaches no GitHub).
const char* sim()
{
    return sim_net::value("PLAYGUARD_SIM_SYSPATCH");
}
#endif
}   // namespace

bool fetch(Release* out, std::string* error)
{
    std::string body;
#ifndef __SWITCH__
    if (!sim_net::get(sim(), "release.json", &body, error)) return false;
#else
    // Without a token: a public repository, and the token is PlayGuard's.
    if (!http::get(github_auth::api_url(std::string("repos/") + REPO + "/releases/latest"), &body, error, 256 * 1024,
                   30, github_auth::api_headers("")))
        return false;
#endif
    if (parse_release(body, out)) return true;
    if (error) *error = "no sys-patch release with its zip on GitHub";
    return false;
}

bool download(const Release& r, const std::string& path, std::string* error)
{
    if (r.sha256.empty()) {
        if (error) *error = dev_builds::NO_DIGEST;
        return false;
    }
    bool ok;
#ifndef __SWITCH__
    std::string content;
    ok = sim() && text::starts_with(r.url, "https://") && paths::read_file(r.url.substr(8), content) &&
         paths::atomic_write(path, content, error);
    if (!ok && error && error->empty()) *error = "simulated: cannot read " + r.url;
#else
    ok = http::download(r.url, path, error, (size_t)r.size + 1, 120);
#endif
    if (ok) ok = dev_builds::verify_file(path, r.size, r.sha256, error);
    if (!ok) std::remove(path.c_str());
    return ok;
}

}   // namespace syspatch_update
