// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/migrations.hpp"

#include <cstdlib>

#include "util/paths.hpp"

namespace migrations
{

int done(const std::string& file)
{
    std::string text;
    if (!paths::read_file(file, text)) return None;
    char* end = nullptr;
    const long n = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || n < 0) return None;
    while (*end == '\n' || *end == '\r' || *end == ' ') end++;
    return *end == '\0' ? (int)n : None;
}

size_t run(const std::string& file, const std::vector<Step>& steps,
           const std::function<void(const Step&, size_t)>& log)
{
    int level = done(file);
    size_t ran = 0;
    for (const Step& s : steps) {
        if (s.level <= level) continue;
        const size_t moved = s.run ? s.run() : 0;
        if (log) log(s, moved);
        level = s.level;
        ran++;
    }
    if (ran) {
        const size_t slash = file.find_last_of('/');
        if (slash != std::string::npos) paths::ensure_dir(file.substr(0, slash));
        paths::atomic_write(file, std::to_string(level) + "\n");
    }
    return ran;
}

}   // namespace migrations
