// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/data_cleanup.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>

namespace data_cleanup
{

namespace
{
// The files under `dir` and its subfolders, with their sizes. `keep(name)`
// leaves out a file of `dir` itself (not of a subfolder) by its name.
template <typename Keep>
void collect(const std::string& dir, Plan& plan, Keep keep, bool top)
{
    DIR* d = opendir(dir.c_str());
    if (!d) return;
    std::vector<std::string> subdirs;
    while (dirent* e = readdir(d)) {
        const std::string name = e->d_name;
        if (name == "." || name == "..") continue;
        const std::string path = dir + "/" + name;
        struct stat st;
        if (stat(path.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            subdirs.push_back(path);
        } else if (S_ISREG(st.st_mode) && !(top && keep(name))) {
            plan.files.push_back(path);
            plan.bytes += (uint64_t)st.st_size;
        }
    }
    closedir(d);
    for (const std::string& sub : subdirs) collect(sub, plan, keep, false);
}

Plan sorted(Plan plan)
{
    std::sort(plan.files.begin(), plan.files.end());
    return plan;
}
}   // namespace

Plan cache(const std::string& data_dir)
{
    Plan plan;
    collect(data_dir + "/cache", plan, [](const std::string&) { return false; }, true);
    return sorted(plan);
}

Plan logs(const std::string& data_dir)
{
    static const char* const KEEP[] = { "uploads.txt", "play_timer_block.json" };
    Plan plan;
    collect(data_dir + "/logs", plan, [](const std::string& name) {
        for (const char* k : KEEP)
            if (name == k || name == std::string(k) + ".tmp") return true;
        return false;
    }, true);
    return sorted(plan);
}

Outcome remove(const Plan& plan)
{
    Outcome out;
    for (const std::string& path : plan.files) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) continue;   // gone since the plan
        if (std::remove(path.c_str()) == 0) {
            out.removed++;
            out.bytes += (uint64_t)st.st_size;
        } else if (errno != ENOENT) {
            out.failed++;
        }
    }
    return out;
}

Unit size_unit(uint64_t bytes, uint64_t* value)
{
    constexpr uint64_t KB = 1024, MB = 1024 * 1024;
    if (bytes < MB) {
        *value = (bytes + KB - 1) / KB;   // rounded up: a small file is not "0 KB"
        return Unit::KB;
    }
    *value = (bytes + MB / 2) / MB;
    return Unit::MB;
}

}   // namespace data_cleanup
