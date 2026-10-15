// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/update.hpp"

#include <cstdio>

#include <borealis/extern/nlohmann/json.hpp>

namespace update
{

static bool parse_triplet(const std::string& text, int out[3], bool* suffixed)
{
    size_t i = (!text.empty() && (text[0] == 'v' || text[0] == 'V')) ? 1 : 0;
    for (int part = 0; part < 3; part++) {
        if (part > 0) {
            if (i >= text.size() || text[i] != '.') return false;
            i++;
        }
        if (i >= text.size() || text[i] < '0' || text[i] > '9') return false;
        long v = 0;
        while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
            v = v * 10 + (text[i] - '0');
            if (v > 0xFFFF) return false;
            i++;
        }
        out[part] = (int)v;
    }
    // Anything after the third number must be a suffix ("-dev", "+abc").
    if (i < text.size() && text[i] != '-' && text[i] != '+') return false;
    if (suffixed) *suffixed = i < text.size();
    return true;
}

bool parse_version(const std::string& text, int out[3])
{
    return parse_triplet(text, out, nullptr);
}

int compare_versions(const std::string& a, const std::string& b)
{
    int va[3], vb[3];
    bool sa = false, sb = false;
    const bool oka = parse_triplet(a, va, &sa);
    const bool okb = parse_triplet(b, vb, &sb);
    if (!oka || !okb) return (int)oka - (int)okb;
    for (int i = 0; i < 3; i++)
        if (va[i] != vb[i]) return va[i] < vb[i] ? -1 : 1;
    return (int)sb - (int)sa;   // "1.0.0-dev" < "1.0.0"
}

uint32_t parse_firmware(const std::string& text)
{
    int v[3];
    bool suffixed = false;
    if (!parse_triplet(text, v, &suffixed) || suffixed) return 0;
    if (v[0] > 255 || v[1] > 255 || v[2] > 255) return 0;
    return ((uint32_t)v[0] << 16) | ((uint32_t)v[1] << 8) | (uint32_t)v[2];
}

bool parse_compat(const std::string& json, Latest* out)
{
    try {
        const auto j = nlohmann::json::parse(json);
        if (!j.is_object() || !j.contains("version") || !j.contains("fw_tested_max")) return false;
        if (!j["version"].is_string() || !j["fw_tested_max"].is_string()) return false;
        Latest l;
        l.version       = j["version"].get<std::string>();
        l.fw_tested_max = parse_firmware(j["fw_tested_max"].get<std::string>());
        int v[3];
        if (!parse_version(l.version, v) || !l.fw_tested_max) return false;
        *out = l;
        return true;
    } catch (...) {
        return false;
    }
}

namespace
{
// Days since 1970-01-01 of a proleptic Gregorian date (H. Hinnant's algorithm).
long days_from_civil(int y, int m, int d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const int yoe = y - era * 400;
    const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (long)era * 146097 + doe - 719468;
}

void civil_from_days(long z, int* y, int* m, int* d)
{
    z += 719468;
    const long era = (z >= 0 ? z : z - 146096) / 146097;
    const long doe = z - era * 146097;
    const long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long mp = (5 * doy + 2) / 153;
    *d = (int)(doy - (153 * mp + 2) / 5 + 1);
    *m = (int)(mp < 10 ? mp + 3 : mp - 9);
    *y = (int)(yoe + era * 400 + (*m <= 2));
}

// "YYYY-MM-DD" -> days since 1970-01-01; false when malformed.
bool day_number(const std::string& date, long* out)
{
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (size_t i : { 0, 1, 2, 3, 5, 6, 8, 9 })
        if (date[i] < '0' || date[i] > '9') return false;
    const int y = std::stoi(date.substr(0, 4)), m = std::stoi(date.substr(5, 2)), d = std::stoi(date.substr(8, 2));
    if (m < 1 || m > 12 || d < 1 || d > 31) return false;
    const long n = days_from_civil(y, m, d);
    int y2, m2, d2;
    civil_from_days(n, &y2, &m2, &d2);
    if (m2 != m || d2 != d) return false;   // 2026-02-30
    *out = n;
    return true;
}
}   // namespace

std::string add_days(const std::string& date, int days)
{
    long n;
    if (!day_number(date, &n)) return "";
    int y, m, d;
    civil_from_days(n + days, &y, &m, &d);
    if (y < 0 || y > 9999) return "";
    char out[16];
    std::snprintf(out, sizeof(out), "%04d-%02d-%02d", y, m, d);
    return out;
}

bool offer_at_start(const std::string& latest, const std::string& running, const Snooze& snooze,
                    const std::string& today)
{
    if (latest.empty() || compare_versions(latest, running) <= 0) return false;
    if (snooze.version.empty() || compare_versions(snooze.version, latest) != 0) return true;
    long now, until;
    if (!day_number(today, &now) || !day_number(snooze.until, &until)) return true;
    const long left = until - now;
    return left <= 0 || left > SNOOZE_MAX_DAYS;
}

Verdict decide(uint32_t console_fw, const std::string& app_version, const Latest& latest)
{
    if (compare_versions(latest.version, app_version) <= 0) return Verdict::UpToDate;
    return console_fw && console_fw <= latest.fw_tested_max ? Verdict::UpdateSupports : Verdict::UpdateNoSupport;
}

}   // namespace update
