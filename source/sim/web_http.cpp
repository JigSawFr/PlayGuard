// The web simulator's http (web/README.md): no libcurl in a browser, and a
// page cannot reach GitHub or switchbrew without their CORS headers, so
// every request is answered as a console without a connection. The
// PLAYGUARD_SIM_* folders the *_net.cpp files read still answer first.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/http.hpp"

#include "sim/sim_net.hpp"

namespace http
{

namespace
{
std::unexpected<Failure> offline()
{
    Failure f;
    sim_net::offline(&f.error);
    return std::unexpected(std::move(f));
}
}   // namespace

Result<Reply> get(const std::string&, size_t, long, const Headers&)
{
    return offline();
}

Result<Reply> post(const std::string&, const std::string&, const char*, size_t, long, const Headers&)
{
    return offline();
}

Result<std::string> redirect(const std::string&, const Headers&)
{
    return offline();
}

Result<void> download(const std::string&, const std::string&, size_t, long, std::function<void(uint64_t, uint64_t)>,
                      uint64_t, const std::atomic<bool>*)
{
    return offline();
}

void abort_all() {}

void cleanup() {}

}   // namespace http
