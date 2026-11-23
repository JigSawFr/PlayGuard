// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/agent_latest.hpp"

#include <ctime>
#include <mutex>

#include "bl_latest.h"
#include "util/paths.hpp"

namespace agent_latest
{

Reading parse(const std::string& text)
{
    BlLatest l;
    bl_latest_parse(&l, text.data(), text.size());
    Reading r;
    if (!bl_latest_may_write(&l)) return r;   // a newer schema: as absent
    r.versions.atmosphere = l.atmosphere;
    r.versions.hekate     = l.hekate;
    r.versions.firmware   = l.firmware;
    r.versions.syspatch   = l.syspatch;
    r.integration         = l.integration;
    r.checked             = l.checked;
    r.clock_checked       = l.clock_checked;
    r.clock_network       = l.clock_network;
    r.clock_user          = l.clock_user;
    r.clock_server        = l.clock_server;
    return r;
}

static bool fresh(int64_t at, int64_t now, int64_t limit)
{
    return at > 0 && now >= at && now - at < limit;
}

bool versions_fresh(const Reading& r, int64_t now) { return fresh(r.checked, now, VERSIONS_FRESH_S); }
bool clock_fresh(const Reading& r, int64_t now) { return fresh(r.clock_checked, now, CLOCK_FRESH_S); }

Reading read()
{
    static std::mutex lock;
    static Reading kept;
    static std::time_t read_at = 0;
    std::lock_guard<std::mutex> guard(lock);
    const std::time_t now = std::time(nullptr);
    if (read_at != 0 && now >= read_at && now - read_at < 30) return kept;
    read_at = now;
    std::string path = paths::sd_root();
    if (!path.empty() && path.back() != '/') path += '/';
    std::string text;
    kept = (!path.empty() && paths::read_file(path + BL_LATEST_FILE, text)) ? parse(text) : Reading{};
    return kept;
}

}   // namespace agent_latest
