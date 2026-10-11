// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/data_move.hpp"

#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "util/paths.hpp"

namespace data_move
{

namespace
{
bool stat_of(const std::string& path, struct stat* st)
{
    return stat(path.c_str(), st) == 0;
}

// `from` to `to`. A folder whose name is taken in `to` is merged into it,
// then removed when nothing of it is left behind.
size_t move(const std::string& from, const std::string& to)
{
    struct stat src, dst;
    if (!stat_of(from, &src)) return 0;
    if (!stat_of(to, &dst)) return std::rename(from.c_str(), to.c_str()) == 0 ? 1 : 0;
    if (!S_ISDIR(src.st_mode) || !S_ISDIR(dst.st_mode)) return 0;   // a file already there wins
    std::vector<std::string> names;
    if (DIR* d = opendir(from.c_str())) {
        while (dirent* e = readdir(d)) {
            const std::string n = e->d_name;
            if (n != "." && n != "..") names.push_back(n);
        }
        closedir(d);
    }
    size_t moved = 0;
    for (const std::string& n : names) moved += move(from + "/" + n, to + "/" + n);
    rmdir(from.c_str());   // fails, and stays, when something was not moved
    return moved;
}
}   // namespace

std::string old_dir()
{
    return "/switch/playguard";
}

size_t run(const std::string& from, const std::string& to)
{
    static const char* const FILES[] = { "config.json", "history.json", "watch.json", "github_token",
                                         "rescue_report.txt" };
    static const char* const SUFFIXES[] = { "", ".tmp", ".bad", ".bad.1" };
    static const char* const OTHERS[] = { "RESCUE", "RESCUE.txt", "RESCUE.done",
                                          "profiles", "backups", "exports", "cache", "logs" };
    if (from == to) return 0;
    struct stat st;
    if (!stat_of(from, &st) || !S_ISDIR(st.st_mode)) return 0;
    std::vector<std::string> names;
    for (const char* f : FILES)
        for (const char* s : SUFFIXES) names.push_back(std::string(f) + s);
    for (const char* o : OTHERS) names.push_back(o);
    size_t moved = 0;
    for (const std::string& n : names) {
        if (!stat_of(from + "/" + n, &st)) continue;
        if (!paths::ensure_dir(to)) return moved;
        moved += move(from + "/" + n, to + "/" + n);
    }
    return moved;
}

}   // namespace data_move
