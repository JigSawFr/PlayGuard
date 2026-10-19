// The web simulator's http (web/README.md): no libcurl in a browser, and a
// page cannot reach GitHub or switchbrew without their CORS headers, so
// every request is answered as a console without a connection. The
// PLAYGUARD_SIM_* folders the *_net.cpp files read still answer first.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/http.hpp"

#include "sim/sim_net.hpp"

namespace http
{

bool get(const std::string&, std::string*, std::string* error, size_t, long, const Headers&, long*)
{
    return sim_net::offline(error);
}

bool post(const std::string&, const std::string&, const char*, std::string*, std::string* error, long*, size_t, long,
          const Headers&)
{
    return sim_net::offline(error);
}

bool redirect(const std::string&, std::string*, std::string* error, const Headers&, long*)
{
    return sim_net::offline(error);
}

bool download(const std::string&, const std::string&, std::string* error, size_t, long,
              std::function<void(uint64_t, uint64_t)>, uint64_t, const std::atomic<bool>*)
{
    return sim_net::offline(error);
}

void abort_all() {}

void cleanup() {}

}   // namespace http
