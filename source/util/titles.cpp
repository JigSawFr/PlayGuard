// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/titles.hpp"
#include "util/title_id.hpp"

#include <cstdio>

namespace titles
{

namespace
{
// A name on one line: line breaks and control characters as spaces.
std::string one_line(const std::string& s)
{
    std::string out = s;
    for (char& c : out)
        if ((unsigned char)c < 0x20) c = ' ';
    return out;
}
}   // namespace

SyncTitleKind kind(const Record& r, uint64_t id)
{
    const auto it = r.find(id);
    return it != r.end() ? it->second.kind : sync_title_kind_of_id(id);
}

std::string name(const Record& r, uint64_t id)
{
    const auto it = r.find(id);
    return it != r.end() ? it->second.name : "";
}

bool note(Record& r, uint64_t id, SyncTitleKind k, const std::string& title_name)
{
    if (!id) return false;
    const std::string clean = one_line(title_name);
    auto it = r.find(id);
    if (it == r.end()) {
        // A game by its ID needs no line; nor does a system program (its ID says).
        if (k != SyncTitle_Homebrew && k == sync_title_kind_of_id(id)) return false;
        if (r.size() >= MAX) return false;
        r[id] = Title{ k, clean };
        return true;
    }
    bool changed = false;
    if (it->second.kind != k) {
        it->second.kind = k;
        changed = true;
    }
    if (!clean.empty() && it->second.name != clean) {
        it->second.name = clean;
        changed = true;
    }
    return changed;
}

std::string text(const Record& r)
{
    std::string out = "# Written by PlayGuard: the titles that are not games, and their names.\n";
    for (const auto& t : r) out += title_id::text(t.first) + "=" + sync_title_kind_name(t.second.kind) + ":" + t.second.name + "\n";
    return out;
}

std::string agent_text(const Record& r)
{
    std::string out = "# Written by PlayGuard: the titles that are not games (docs/sync-protocol.md).\n";
    for (const auto& t : r) out += title_id::text(t.first) + "=" + sync_title_kind_name(t.second.kind) + "\n";
    return out;
}

Record parse(const std::string& in)
{
    Record r;
    SyncTitles kinds;
    sync_titles_parse(&kinds, in.data(), in.size());
    for (size_t i = 0; i < kinds.n; i++) r[kinds.ids[i]] = Title{ (SyncTitleKind)kinds.kinds[i], "" };
    // The names: after "<16 hex>=<kind>:".
    for (size_t at = 0; at < in.size();) {
        size_t end = in.find('\n', at);
        if (end == std::string::npos) end = in.size();
        std::string line = in.substr(at, end - at);
        at = end + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t colon = line.find(':');
        if (line.size() < 18 || line[16] != '=' || colon == std::string::npos) continue;
        uint64_t id = 0;
        if (std::sscanf(line.c_str(), "%16llx", (unsigned long long*)&id) != 1) continue;
        auto it = r.find(id);
        if (it != r.end()) it->second.name = line.substr(colon + 1);
    }
    return r;
}

}   // namespace titles
