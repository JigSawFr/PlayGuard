// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/dns_hosts.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace dns_hosts
{

namespace
{
std::string lower(std::string text)
{
    for (char& c : text) c = (char)std::tolower((unsigned char)c);
    return text;
}

// "*" over any run (backtracking on the last star), the rest letter by letter.
bool glob(const std::string& p, const std::string& s)
{
    size_t pi = 0, si = 0, star = std::string::npos, mark = 0;
    while (si < s.size()) {
        if (pi < p.size() && p[pi] == '*') {
            star = pi++;
            mark = si;
        } else if (pi < p.size() && p[pi] == s[si]) {
            pi++;
            si++;
        } else if (star != std::string::npos) {
            pi = star + 1;
            si = ++mark;
        } else {
            return false;
        }
    }
    while (pi < p.size() && p[pi] == '*') pi++;
    return pi == p.size();
}

std::string trim(const std::string& text)
{
    const size_t first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) return "";
    return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
}
}   // namespace

bool matches(const std::string& pattern, const std::string& host)
{
    std::string p;
    for (char c : lower(pattern)) {
        if (c == '%') p += "lp1";
        else p += c;
    }
    return !p.empty() && glob(p, lower(host));
}

std::string redirecting_line(const std::string& text, const std::string& host)
{
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line)) {
        const size_t hash = line.find('#');
        if (hash != std::string::npos) line.erase(hash);
        std::istringstream words(line);
        std::string address, name;
        if (!(words >> address)) continue;
        while (words >> name)
            if (matches(name, host)) return trim(line);
    }
    return "";
}

bool redirects(const std::string& text, const std::string& host)
{
    return !redirecting_line(text, host).empty();
}

bool mitm_disabled(const std::string& ini)
{
    std::istringstream lines(ini);
    std::string line, section;
    bool off = false;
    while (std::getline(lines, line)) {
        const size_t semi = line.find(';');
        if (semi != std::string::npos) line.erase(semi);
        line = trim(line);
        if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
            section = lower(trim(line.substr(1, line.size() - 2)));
            continue;
        }
        const size_t eq = line.find('=');
        if (section != "atmosphere" || eq == std::string::npos) continue;
        if (lower(trim(line.substr(0, eq))) != "enable_dns_mitm") continue;
        // "u8!0x0": the value after the type.
        std::string value = lower(trim(line.substr(eq + 1)));
        const size_t bang = value.find('!');
        if (bang != std::string::npos) value = value.substr(bang + 1);
        off = std::strtoul(value.c_str(), nullptr, 0) == 0;   // the last line wins
    }
    return off;
}

int emummc_id(const std::string& ini)
{
    std::istringstream lines(ini);
    std::string line;
    while (std::getline(lines, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos || lower(trim(line.substr(0, eq))) != "id") continue;
        const std::string value = trim(line.substr(eq + 1));
        if (value.empty()) return -1;
        char* end = nullptr;
        const unsigned long id = std::strtoul(value.c_str(), &end, 0);
        return end && *end == '\0' && id <= 0xFFFF ? (int)id : -1;
    }
    return -1;
}

std::vector<std::string> candidates(const std::string& nand, int emummc_id)
{
    std::vector<std::string> files;
    if (nand == "emummc") {
        if (emummc_id >= 0) {
            char name[64];
            std::snprintf(name, sizeof(name), "atmosphere/hosts/emummc_%04x.txt", (unsigned)emummc_id);
            files.push_back(name);
        }
        files.push_back("atmosphere/hosts/emummc.txt");
    } else if (nand == "sysmmc") {
        files.push_back("atmosphere/hosts/sysmmc.txt");
    } else {
        return files;
    }
    files.push_back("atmosphere/hosts/default.txt");
    return files;
}

namespace
{
// "reachable" or "redirected", with the line that does it when `with_line`.
std::string verdict(const Seen& seen, const char* host, bool with_line)
{
    const std::string line = seen.mitm_off || seen.file.empty() ? "" : redirecting_line(seen.text, host);
    if (line.empty()) return "reachable";
    return with_line ? "redirected (\"" + line + "\")" : "redirected";
}

std::string hosts_file(const Seen& seen)
{
    if (seen.mitm_off) return "not read (DNS.mitm off)";
    return seen.file.empty() ? "none (Atmosphère's defaults only)" : seen.file;
}
}   // namespace

std::string summary(const Seen& seen)
{
    return std::string("DNS.mitm ") + (seen.mitm_off ? "off" : "on") + ", hosts file " + hosts_file(seen) +
           ": parental-control server " + verdict(seen, PCTL_HOST, false) + ", time server " +
           verdict(seen, TIME_HOST, false);
}

std::string report(const Seen& seen, bool has_startup_log, const std::string& startup_log)
{
    std::string out = "=== atmosphere dns.mitm ===\n";
    out += "read from   : the SD card's files, not a network test\n";
    out += std::string("dns.mitm    : ") + (seen.mitm_off ? "off (system_settings.ini)" : "on") + "\n";
    out += "hosts file  : " + hosts_file(seen) + "\n";
    out += std::string("pctl server : ") + PCTL_HOST + " " + verdict(seen, PCTL_HOST, true) + "\n";
    out += std::string("time server : ") + TIME_HOST + " " + verdict(seen, TIME_HOST, true) + "\n";
    out += std::string("startup log : ") + STARTUP_LOG + (has_startup_log ? "" : " (none)") + "\n";
    if (has_startup_log) {
        // Its last lines, indented.
        std::vector<std::string> kept;
        std::istringstream lines(startup_log);
        std::string line;
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            kept.push_back(line);
        }
        const size_t first = kept.size() > STARTUP_LOG_LINES ? kept.size() - STARTUP_LOG_LINES : 0;
        if (first > 0) out += "  (earlier lines left out)\n";
        for (size_t i = first; i < kept.size(); i++) out += "  " + kept[i] + "\n";
    }
    return out + "\n";
}

}   // namespace dns_hosts
