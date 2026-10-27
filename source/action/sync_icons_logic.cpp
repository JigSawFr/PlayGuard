// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sync_icons_logic.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>

#include "sync/sync_icons.h"

namespace sync_icons_logic
{

namespace
{
std::string hex16(uint64_t v)
{
    char id[17];
    std::snprintf(id, sizeof(id), "%016llX", (unsigned long long)v);
    return id;
}

// 16 hex digits, either case; false for anything else.
bool parse_id(const std::string& s, uint64_t* out)
{
    if (s.size() != 16) return false;
    uint64_t v = 0;
    for (const char c : s) {
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else return false;
        v = (v << 4) | (uint64_t)d;
    }
    *out = v;
    return true;
}
}   // namespace

std::vector<uint64_t> wanted(std::vector<Played> games, const std::vector<uint64_t>& installed, size_t max)
{
    games.erase(std::remove_if(games.begin(), games.end(),
                               [](const Played& g) { return !g.app_id || (!g.week_s && !g.last_played); }),
                games.end());
    // This week's first (the last time played may be unknown: the all-time
    // statistics not read), then the most recent; the week's play time
    // between two equally recent ones.
    std::stable_sort(games.begin(), games.end(), [](const Played& a, const Played& b) {
        if ((a.week_s != 0) != (b.week_s != 0)) return a.week_s != 0;
        if (a.last_played != b.last_played) return a.last_played > b.last_played;
        return a.week_s > b.week_s;
    });
    std::vector<uint64_t> out;
    std::set<uint64_t> seen;
    for (const Played& g : games) {
        if (out.size() == max) break;
        if (seen.insert(g.app_id).second) out.push_back(g.app_id);
    }
    for (const uint64_t id : installed) {
        if (out.size() == max) break;
        if (id && seen.insert(id).second) out.push_back(id);
    }
    return out;
}

std::string target_of(const std::string& host, uint16_t port, const std::string& base)
{
    char out[SYNC_ICONS_TARGET_LEN + 1];
    sync_icons_target(host.c_str(), port, base.c_str(), out);   // the agent's too
    return out;
}

Plan plan(const std::vector<uint64_t>& want, const Sent& sent, const std::string& target)
{
    Plan p;
    const bool same = sent.target == target;
    std::set<uint64_t> wanted_set(want.begin(), want.end());
    for (const uint64_t id : want)
        if (!same || !sent.icons.count(id)) p.send.push_back(id);
    if (same)
        for (const auto& s : sent.icons)
            if (!wanted_set.count(s.first)) p.clear.push_back(s.first);
    return p;
}

void record(Sent& sent, const std::string& target, uint64_t app_id, uint32_t bytes, bool cleared)
{
    if (sent.target != target) {
        sent.target = target;
        sent.icons.clear();
    }
    if (cleared) sent.icons.erase(app_id);
    else sent.icons[app_id] = bytes;
}

bool merge(Sent& sent, const Sent& agent, const std::string& target)
{
    if (agent.target != target) return false;
    bool added = false;
    for (const auto& a : agent.icons) {
        if (sent.target == target && sent.icons.count(a.first)) continue;
        record(sent, target, a.first, a.second, false);
        added = true;
    }
    return added;
}

std::string text(const Sent& sent)
{
    std::string out = "# Written by PlayGuard: the game icons it published (docs/sync-protocol.md).\n";
    out += "target=" + sent.target + "\n";
    for (const auto& s : sent.icons) out += hex16(s.first) + "=" + std::to_string(s.second) + "\n";
    return out;
}

Sent parse(const std::string& text)
{
    Sent out;
    for (size_t at = 0; at < text.size();) {
        size_t end = text.find('\n', at);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(at, end - at);
        at = end + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t eq = line.find('=');
        if (line.empty() || line[0] == '#' || eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq), value = line.substr(eq + 1);
        uint64_t id = 0;
        if (key == "target") {
            out.target = value;
        } else if (parse_id(key, &id) && !value.empty() && value.size() <= 9 &&
                   value.find_first_not_of("0123456789") == std::string::npos) {
            out.icons[id] = (uint32_t)std::strtoul(value.c_str(), nullptr, 10);
        }
    }
    return out;
}

std::string topic(uint64_t app_id)
{
    return "icon/" + hex16(app_id);
}

}   // namespace sync_icons_logic
