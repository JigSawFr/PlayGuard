// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/backup.hpp"

#include <algorithm>
#include <cstdio>
#include <aurore/extern/nlohmann/json.hpp>
#include <ctime>

#include "util/json_util.hpp"
#include "util/paths.hpp"

namespace backup
{

namespace
{
constexpr int FORMAT = 1;

// An integer in [lo, hi]; anything else (float, string, out of range) is refused.
bool int_in(const nlohmann::json& j, int lo, int hi, int& out)
{
    if (!j.is_number_integer()) return false;
    const long long v = j.get<long long>();
    if (v < lo || v > hi) return false;
    out = (int)v;
    return true;
}

bool boolean(const nlohmann::json& j, bool& out)
{
    if (!j.is_boolean()) return false;
    out = j.get<bool>();
    return true;
}
}   // namespace

std::string to_json(const Snapshot& s)
{
    nlohmann::json j;
    j["format"]   = FORMAT;
    j["app"]      = "PlayGuard";
    j["created"]  = s.created;
    j["firmware"] = s.firmware;
    nlohmann::json r = nlohmann::json::object();
    if (s.level) r["level"] = *s.level;
    if (s.custom)
        r["custom"] = { { "rating_age", s.custom->rating_age },
                        { "sns_post_restricted", s.custom->sns_restricted },
                        { "free_communication_restricted", s.custom->comm_restricted } };
    if (s.vr_restricted.has_value()) r["vr_restricted"] = *s.vr_restricted;
    if (s.rating_org) r["rating_organization"] = *s.rating_org;
    j["restrictions"] = r;
    // The play-timer part goes with its limits (the alarm flag and the raw
    // block were read with them).
    if (s.days) {
        nlohmann::json days = nlohmann::json::array();
        for (uint16_t d : *s.days) {
            if (d == 0xFFFF) days.push_back(nullptr);
            else days.push_back(d);
        }
        nlohmann::json pt = { { "days_from_sunday", days } };
        if (s.alarm_disabled.has_value()) pt["alarm_disabled"] = *s.alarm_disabled;
        if (!s.raw_block.empty()) pt["raw_0x44"] = s.raw_block;
        j["play_timer"] = pt;
    }
    if (s.forwarders) {
        nlohmann::json list;
        if (!json_util::parse(forwarders::list_text(s.forwarders->made), &list)) list = nlohmann::json::array();
        j["forwarders"] = { { "made", list }, { "hbmenu_locked", s.forwarders->hbmenu_locked } };
    }
    return j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
}

bool from_json(const std::string& text, Snapshot& out)
{
    Snapshot s;
    const bool read = json_util::parse_with(text, [&s](const nlohmann::json& j) {
        int format = 0;
        if (!j.is_object() || !j.contains("format") || !int_in(j.at("format"), FORMAT, FORMAT, format)) return false;
        s.created = json_util::string_at(j, "created");
        s.firmware = json_util::string_at(j, "firmware");

        if (j.contains("restrictions")) {
            const auto& r = j.at("restrictions");
            if (!r.is_object()) return false;
            if (r.contains("level")) {
                int level = 0;
                if (!int_in(r.at("level"), 0, MAX_LEVEL, level)) return false;
                s.level = (uint32_t)level;
            }
            if (r.contains("custom")) {
                const auto& c = r.at("custom");
                Custom      custom;
                int         age = 0;
                if (!c.is_object() || !c.contains("rating_age") || !c.contains("sns_post_restricted") ||
                    !c.contains("free_communication_restricted"))
                    return false;
                if (!int_in(c.at("rating_age"), 0, MAX_RATING_AGE, age) ||
                    !boolean(c.at("sns_post_restricted"), custom.sns_restricted) ||
                    !boolean(c.at("free_communication_restricted"), custom.comm_restricted))
                    return false;
                custom.rating_age = (uint8_t)age;
                s.custom          = custom;
            }
            if (r.contains("vr_restricted")) {
                bool vr = false;
                if (!boolean(r.at("vr_restricted"), vr)) return false;
                s.vr_restricted = vr;
            }
            if (r.contains("rating_organization")) {
                int org = 0;
                if (!int_in(r.at("rating_organization"), 0, MAX_RATING_ORG, org)) return false;
                s.rating_org = (uint32_t)org;
            }
        }

        if (j.contains("play_timer")) {
            const auto& pt = j.at("play_timer");
            if (!pt.is_object() || !pt.contains("days_from_sunday")) return false;
            const auto& days = pt.at("days_from_sunday");
            if (!days.is_array() || days.size() != 7) return false;
            std::array<uint16_t, 7> minutes{};
            for (size_t i = 0; i < 7; i++) {
                int v = 0;
                if (days[i].is_null()) v = DAY_NO_LIMIT;
                else if (!int_in(days[i], 0, MAX_DAY_MINUTES, v)) return false;
                minutes[i] = (uint16_t)v;
            }
            s.days = minutes;
            if (pt.contains("alarm_disabled")) {
                bool disabled = false;
                if (!boolean(pt.at("alarm_disabled"), disabled)) return false;
                s.alarm_disabled = disabled;
            }
            if (pt.contains("raw_0x44")) {
                // 34 u16 as 136 hex digits; anything else is not this block.
                const auto& raw = pt.at("raw_0x44");
                if (!raw.is_string()) return false;
                const std::string hex = raw.get<std::string>();
                if (hex.size() != 136 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) return false;
                s.raw_block = hex;
            }
        }
        // Optional: an older backup has none, and a damaged part is left
        // out (it writes nothing to the console).
        if (const nlohmann::json* f = json_util::object_at(j, "forwarders")) {
            const auto made = f->find("made");
            if (made != f->end() && made->is_array()) {
                s.forwarders = Forwarders{ forwarders::parse_list(made->dump()), json_util::flag_at(*f, "hbmenu_locked") };
            }
        }
        return true;
    });
    if (!read || s.empty()) return false;
    out = s;
    return true;
}

std::string save(const Snapshot& s, std::string* error)
{
    std::time_t now = std::time(nullptr);
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    char stamp[32] = "unknown";
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tmv);

    const std::string text = to_json(s);
    for (unsigned i = 0; i < 100; ++i) {
        std::string path = paths::backups_dir() + "/" + stamp;
        if (i > 0) path += "_" + std::to_string(i);
        path += ".json";
        std::string existing;
        if (paths::read_file(path, existing)) continue;
        return paths::atomic_write(path, text, error) ? path : "";
    }
    if (error) *error = "No free file name";
    return "";
}

std::vector<std::string> list()
{
    std::vector<std::string> names = paths::list_files(paths::backups_dir(), ".json");
    std::reverse(names.begin(), names.end());   // the stamp sorts oldest first
    return names;
}

bool load(const std::string& name, Snapshot& out)
{
    std::string text;
    return paths::read_file(paths::backups_dir() + "/" + name, text) && from_json(text, out);
}

void RestoreTally::add(bool known, bool equal, bool restored)
{
    if (known && equal) {
        same++;
        return;
    }
    differ++;
    if (restored) written++;
}

size_t prune(size_t keep, const std::string& keep_path)
{
    if (keep == 0) return 0;
    const std::vector<std::string> names = list();
    // `keep_path` is one of the kept, wherever its stamp sorts (made with the
    // clock in the past, it would be the "oldest").
    size_t kept = 0;
    for (const auto& n : names) kept += paths::backups_dir() + "/" + n == keep_path;
    size_t removed = 0;
    for (const auto& n : names) {
        const std::string path = paths::backups_dir() + "/" + n;
        if (path == keep_path) continue;
        if (kept < keep) {
            kept++;
            continue;
        }
        if (std::remove(path.c_str()) == 0) removed++;
    }
    return removed;
}

}   // namespace backup
