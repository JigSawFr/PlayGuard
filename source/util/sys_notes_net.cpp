// The network half of sys_notes.hpp (the rest stays plain C++ for the host tests).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#ifndef __SWITCH__
#include "sim/sim_net.hpp"
#endif
#include "util/github_auth.hpp"
#include "util/http.hpp"
#include "util/sys_notes.hpp"

namespace sys_notes
{

namespace
{
// A release list with each release's notes: a few KB apiece, plus GitHub's
// fields about the authors and the files.
constexpr size_t MAX_LIST = 2 * 1024 * 1024;
// A switchbrew page: its tables of changed services can be long.
constexpr size_t MAX_PAGE = 512 * 1024;

// GETs `url` (or, on desktop, the file `name` of the simulated folder).
bool get(const std::string& url, const std::string& name, const http::Headers& headers, size_t max, std::string* body,
         std::string* error)
{
#ifndef __SWITCH__
    (void)url;
    (void)headers;
    (void)max;
    return sim_net::get(sim_net::value("PLAYGUARD_SIM_SYS_NOTES"), name, body, error);
#else
    (void)name;
    return http::get(url, body, error, max, 20, headers);
#endif
}
}   // namespace

bool fetch(Which which, const std::string& installed, bool pages, List* out, std::string* error)
{
    out->releases.clear();
    std::string body, err;
    if (which != Which::Firmware) {
        // Signed in to GitHub (developer tools), its token: a higher rate limit.
        const std::string url =
            github_auth::api_url(std::string("repos/") + repo(which) + "/releases?per_page=" + std::to_string(MAX_RELEASES));
        if (!get(url, std::string(key(which)) + ".json", github_auth::api_headers(github_auth::token()), MAX_LIST, &body,
                 &err)) {
            if (error) *error = std::string(repo(which)) + ": " + err;
            return false;
        }
        if (!parse_github(body, which, &out->releases)) {
            if (error) *error = std::string(repo(which)) + ": not a release list";
            return false;
        }
        return true;
    }
    if (!get("https://switchbrew.org/w/index.php?title=System_Versions&action=raw", "System_Versions.wiki", {}, MAX_PAGE,
             &body, &err)) {
        if (error) *error = "switchbrew: " + err;
        return false;
    }
    for (const Version& v : parse_versions(body)) {
        Release r;
        r.version = v.version;
        r.date = v.date;
        r.page = firmware_page(v.version);
        out->releases.push_back(r);
    }
    if (out->releases.empty()) {
        if (error) *error = "switchbrew: no versions in its table";
        return false;
    }
    if (!pages) return true;
    size_t hidden = 0;
    int installed_at = -1;
    for (size_t i : tabs(*out, installed, &hidden, &installed_at)) {
        Release& r = out->releases[i];
        std::string page;
        if (get("https://switchbrew.org/w/index.php?title=" + r.version + "&action=raw", r.version + ".wiki", {}, MAX_PAGE,
                &page, &err))
            parse_firmware_page(page, &r);
        else if (error && error->empty())
            *error = "switchbrew " + r.version + ": " + err;
    }
    return true;
}

}   // namespace sys_notes
