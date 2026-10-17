// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sim/sim_net.hpp"

#include <cstdlib>
#include <cstring>

#include "util/paths.hpp"

namespace sim_net
{

const char* value(const char* name)
{
    const char* v = std::getenv(name);
    return v && *v ? v : nullptr;
}

bool offline(std::string* error)
{
    if (error) *error = "simulated: no connection";
    return false;
}

bool get(const char* folder, const std::string& name, std::string* body, std::string* error)
{
    if (!folder || std::strcmp(folder, "offline") == 0 || !paths::read_file(std::string(folder) + "/" + name, *body))
        return offline(error);
    return true;
}

}   // namespace sim_net
