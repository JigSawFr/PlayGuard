// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/sys_updates.hpp"

#include <cctype>
#include <vector>

#include "core/console_info.h"

namespace sys_updates
{

std::string version_of_tag(const std::string& tag)
{
    std::string t = tag;
    if (!t.empty() && (t[0] == 'v' || t[0] == 'V')) t.erase(0, 1);
    if (t.empty() || t.size() > 16) return "";
    int numbers = 0;
    bool digit = false;
    for (char c : t) {
        if (std::isdigit((unsigned char)c)) {
            if (!digit) numbers++;
            digit = true;
        } else if (c == '.' && digit) {
            digit = false;
        } else {
            return "";
        }
    }
    return digit && numbers >= 2 && numbers <= 4 ? t : "";
}

std::string tag_of_url(const std::string& url)
{
    const std::string mark = "/releases/tag/";
    const size_t at = url.find(mark);
    if (at == std::string::npos) return "";
    std::string tag = url.substr(at + mark.size());
    const size_t end = tag.find_first_of("/?#");
    if (end != std::string::npos) tag.erase(end);
    return tag;
}

std::string hekate_version(const std::string& bytes)
{
    char v[16];
    return console_hekate_version((const uint8_t*)bytes.data(), bytes.size(), v) ? v : "";
}

std::string syspatch_version(const std::string& logged)
{
    // Cut at a suffix: "v1.6.3.0-b962f75" (its commit) -> "v1.6.3.0".
    return version_of_tag(logged.substr(0, logged.find_first_of("-+ ")));
}

std::string to_text(const Latest& latest)
{
    std::string out;
    auto add = [&out](const char* key, const std::string& value) {
        if (value.empty()) return;
        out += (out.empty() ? "" : ";") + std::string(key) + "=" + value;
    };
    add("atmosphere", latest.atmosphere);
    add("hekate", latest.hekate);
    add("firmware", latest.firmware);
    add("syspatch", latest.syspatch);
    return out;
}

Latest from_text(const std::string& text)
{
    Latest out;
    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find(';', start);
        if (end == std::string::npos) end = text.size();
        const std::string part = text.substr(start, end - start);
        const size_t eq = part.find('=');
        if (eq != std::string::npos) {
            const std::string key = part.substr(0, eq), value = version_of_tag(part.substr(eq + 1));
            if (key == "atmosphere") out.atmosphere = value;
            else if (key == "hekate") out.hekate = value;
            else if (key == "firmware") out.firmware = value;
            else if (key == "syspatch") out.syspatch = value;
        }
        start = end + 1;
    }
    return out;
}

}   // namespace sys_updates
