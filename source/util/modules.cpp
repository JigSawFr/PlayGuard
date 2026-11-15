// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/modules.hpp"

#include <string>

#include "util/paths.hpp"

namespace modules
{

const std::vector<Module>& all()
{
    static const std::vector<Module> list = {
        { { "rescue", 0x4200000000505247ULL, false, "PlayGuard rescue" }, Id::Rescue },
        { { "agent", sysmodule::AGENT_TID, true, "PlayGuard agent" }, Id::Agent },
    };
    return list;
}

const Module& get(Id id)
{
    for (const auto& m : all())
        if (m.id == id) return m;
    return all().front();
}

Applied write_rescue_note(const std::string& sd_root, const std::string& version, const std::string& revision)
{
    if (revision.empty()) return {};
    std::string err;
    if (!paths::atomic_write(dir(sd_root, get(Id::Rescue)) + "/playguard-rescue.txt", version + " " + revision + "\n",
                             &err))
        return std::unexpected(err);
    return {};
}

}   // namespace modules
