// core/sim_env.h: the PLAYGUARD_SIM_* variables, the environment's unless the
// web page changed one. A change is kept in a table of its own under a lock;
// the strings handed out are never freed, so a reader keeps a valid one
// whatever changes after it read.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "core/sim_env.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <string>

namespace
{

std::mutex s_lock;
// name -> the page's value (nullptr: removed). Allocated once, never freed:
// a static's destructor could run while a thread still reads it at exit.
std::map<std::string, const char*>& changes()
{
    static auto* m = new std::map<std::string, const char*>();
    return *m;
}

}   // namespace

extern "C" const char* sim_env(const char* name)
{
    if (!name)
        return nullptr;
    {
        std::lock_guard<std::mutex> guard(s_lock);
        auto it = changes().find(name);
        if (it != changes().end())
            return it->second;
    }
    return std::getenv(name);
}

extern "C" void sim_env_set(const char* name, const char* value)
{
    if (!name)
        return;
    const char* kept = value && *value ? strdup(value) : nullptr;
    std::lock_guard<std::mutex> guard(s_lock);
    changes()[name] = kept;
}
