// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/migrations.hpp"

#include <borealis/extern/nlohmann/json.hpp>

#include "util/json_util.hpp"
#include "util/paths.hpp"
#include "util/update.hpp"

namespace migrations
{

using nlohmann::json;

bool read(const std::string& file, State* out)
{
    *out = State{};
    std::string text;
    if (!paths::read_file(file, text)) return false;
    State s;
    const bool ok = json_util::parse_with(text, [&s](const json& j) {
        if (!j.is_object()) return false;
        const auto level = j.find("level");
        if (level == j.end() || !level->is_number_integer() || level->get<long long>() < 0 ||
            level->get<long long>() > 0x7FFF)
            return false;
        s.level   = (int)level->get<long long>();
        s.version = json_util::string_at(j, "version");
        return true;
    });
    if (ok) *out = s;
    return ok;
}

bool write(const std::string& file, const State& state)
{
    const size_t slash = file.find_last_of('/');
    if (slash != std::string::npos) paths::ensure_dir(file.substr(0, slash));
    const json j = { { "schema", SCHEMA }, { "level", state.level }, { "version", state.version } };
    return paths::atomic_write(file, j.dump() + "\n");
}

size_t run(const std::string& file, const std::vector<Step>& steps, const std::string& version,
           const std::function<void(const std::string&)>& log)
{
    State was;
    read(file, &was);
    State s = was;
    if (s.level > Latest) {
        // A later build's layout: this one writes its own, the steps above
        // it run again when the later build is back.
        if (log)
            log("data last used by PlayGuard " + (s.version.empty() ? std::string("?") : s.version) + " (migration " +
                std::to_string(s.level) + "), level back to " + std::to_string(Latest));
        s.level = Latest;
    }
    size_t ran = 0;
    for (const Step& st : steps) {
        if (st.level <= s.level) continue;
        const size_t moved = st.run ? st.run() : 0;
        if (log)
            log("migration " + std::to_string(st.level) + " (" + st.name + "): " + std::to_string(moved) +
                " entries moved");
        s.level = st.level;
        ran++;
    }
    s.version = version;
    if (s.level != was.level || s.version != was.version) write(file, s);
    return ran;
}

bool earlier_build_next(const std::string& file)
{
    State s;
    if (!read(file, &s)) return !paths::exists_or_pending(file) || write(file, State{});
    if (s.level == None) return true;
    s.level = None;
    return write(file, s);
}

bool is_earlier(const std::string& version, const std::string& running)
{
    return version.empty() || update::compare_versions(version, running) <= 0;
}

}   // namespace migrations
