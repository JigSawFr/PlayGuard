// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/nand.hpp"

#include <cctype>

#include "util/patches.hpp"

namespace nand
{

namespace
{
std::string lower(std::string s)
{
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// Letters, digits, "-" and "_"; anything else "_"; at most 32 characters.
std::string clean(const std::string& s)
{
    std::string out;
    for (char c : lower(s)) {
        if (out.size() >= 32) break;
        out += std::isalnum((unsigned char)c) || c == '-' || c == '_' ? c : '_';
    }
    return out;
}
}   // namespace

std::string emummc_where(const std::string& ini_text)
{
    const patches::Ini ini = patches::parse_ini(ini_text);
    const auto section = ini.find("emummc");
    if (section == ini.end()) return "";
    const auto& kv = section->second;
    auto path = kv.find("path");
    if (path != kv.end()) {
        std::string p = path->second;
        while (!p.empty() && (p.front() == '/' || p.front() == '\\')) p.erase(0, 1);
        while (!p.empty() && (p.back() == '/' || p.back() == '\\')) p.pop_back();
        if (lower(p.substr(0, 7)) == "emummc/" || lower(p.substr(0, 7)) == "emummc\\") p.erase(0, 7);
        if (!p.empty()) return clean(p);
    }
    auto sector = kv.find("sector");
    if (sector != kv.end()) {
        std::string s = lower(sector->second);
        if (s.compare(0, 2, "0x") == 0) s.erase(0, 2);
        while (s.size() > 1 && s[0] == '0') s.erase(0, 1);
        if (!s.empty() && s != "0") return "sector-" + clean(s);
    }
    return "";
}

std::string key(Storage storage, const std::string& ini_text)
{
    if (storage == Storage::SysMMC) return "sysmmc";
    if (storage == Storage::Unknown) return "unknown";
    const std::string where = emummc_where(ini_text);
    return where.empty() ? "emummc" : "emummc-" + where;
}

}   // namespace nand
