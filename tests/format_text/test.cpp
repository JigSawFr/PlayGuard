// Host tests for source/ui/format_text.cpp, translated by the real catalogs
// (resources/i18n/<lang>/playguard.json, read from the repository root `make
// test` runs in) through ui/tr.hpp's stand-in for borealis: limits ("No
// limit", "0 min (no play)", "45 min", "2 h", "2 h 05"), the week's limits in
// a few words, time played and left (rounded to the minute, at most a day),
// play time over any length ("< 1 min", "152 h 30"), the day names (in a
// title and inside a sentence, French for the difference), the restriction
// levels and ages, yes / no, "Unavailable", and the chevron of a line that
// opens a screen. Every key the helpers ask for exists in the catalog.
#include "check.h"
#include <cstdint>
#include <fstream>
#include <map>
#include <borealis/extern/nlohmann/json.hpp>
#include <set>
#include <string>

#include "ui/format_text.hpp"
#include "ui/tr.hpp"
#include "util/pctl_ops_c.hpp"

static std::map<std::string, std::string> s_catalog;
static std::set<std::string>              s_missing;   // keys asked for that the catalog lacks

static void flatten(const nlohmann::json& j, const std::string& prefix)
{
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_object())
            flatten(*it, prefix + it.key() + "/");
        else if (it->is_string())
            s_catalog[prefix + it.key()] = it->get<std::string>();
    }
}

static bool load(const std::string& lang)
{
    s_catalog.clear();
    std::ifstream in("resources/i18n/" + lang + "/playguard.json");
    if (!in) return false;
    flatten(nlohmann::json::parse(in), "playguard/");
    return !s_catalog.empty();
}

namespace brls
{
namespace internal
{
// As borealis: a key the catalog lacks reads as itself.
std::string getRawStr(std::string name)
{
    const auto it = s_catalog.find(name);
    if (it != s_catalog.end()) return it->second;
    s_missing.insert(name);
    return name;
}
}   // namespace internal

inline namespace literals
{
std::string operator""_i18n(const char* str, size_t len)
{
    return internal::getRawStr(std::string(str, len));
}
}   // namespace literals
}   // namespace brls

static void test_minutes()
{
    CHECK(ui::fmt_minutes(PT_DAY_NOLIMIT) == "No limit");
    CHECK(ui::fmt_minutes(0) == "0 min (no play)");
    CHECK(ui::fmt_minutes(1) == "1 min" && ui::fmt_minutes(59) == "59 min");
    CHECK(ui::fmt_minutes(60) == "1 h" && ui::fmt_minutes(120) == "2 h" && ui::fmt_minutes(1440) == "24 h");
    CHECK(ui::fmt_minutes(125) == "2 h 05" && ui::fmt_minutes(150) == "2 h 30" && ui::fmt_minutes(61) == "1 h 01");

    // Played: no "(no play)" for none.
    CHECK(ui::fmt_played(0) == "0 min");
    CHECK(ui::fmt_played(45) == "45 min" && ui::fmt_played(150) == "2 h 30");

    // Left, from nanoseconds: rounded to the nearest minute, at most a day.
    const uint64_t MIN = 60000000000ULL;
    CHECK(ui::fmt_duration_ns(0) == "0 min");
    CHECK(ui::fmt_duration_ns(29 * 1000000000ULL) == "0 min");
    CHECK(ui::fmt_duration_ns(30 * 1000000000ULL) == "1 min");
    CHECK(ui::fmt_duration_ns(90 * MIN + 29 * 1000000000ULL) == "1 h 30");
    CHECK(ui::fmt_duration_ns(90 * MIN + 30 * 1000000000ULL) == "1 h 31");
    CHECK(ui::fmt_duration_ns(1440 * MIN) == "24 h" && ui::fmt_duration_ns(5000 * MIN) == "24 h");
    CHECK(ui::fmt_duration_ns(UINT64_MAX / 2) == "24 h");

    // Play time, from seconds: "< 1 min" under a minute (not for none), no cap.
    CHECK(ui::fmt_play_time(0) == "0 min");
    CHECK(ui::fmt_play_time(1) == "< 1 min" && ui::fmt_play_time(59) == "< 1 min");
    CHECK(ui::fmt_play_time(60) == "1 min" && ui::fmt_play_time(89) == "1 min" && ui::fmt_play_time(90) == "2 min");
    CHECK(ui::fmt_play_time(3600) == "1 h" && ui::fmt_play_time(3600 + 5 * 60) == "1 h 05");
    CHECK(ui::fmt_play_time(152 * 3600 + 30 * 60) == "152 h 30");
    CHECK(ui::fmt_play_time(100000ULL * 3600) == "100000 h");
}

static void test_days_summary()
{
    const uint16_t N = PT_DAY_NOLIMIT;
    const uint16_t same[7] = { 120, 120, 120, 120, 120, 120, 120 };
    CHECK(ui::days_summary(same) == "2 h every day");
    const uint16_t zero[7] = { 0, 0, 0, 0, 0, 0, 0 };
    CHECK(ui::days_summary(zero) == "0 min (no play) every day");
    const uint16_t none[7] = { N, N, N, N, N, N, N };
    CHECK(ui::days_summary(none) == "No limit");
    const uint16_t range[7] = { 180, 60, 60, 90, 60, 60, 180 };
    CHECK(ui::days_summary(range) == "1 h to 3 h");
    // A day without a limit tops the range, in the sentence's case.
    const uint16_t open[7] = { N, 60, 60, 90, 60, 60, N };
    CHECK(ui::days_summary(open) == "1 h to no limit");
    const uint16_t one_open[7] = { 45, 45, 45, 45, 45, 45, N };
    CHECK(ui::days_summary(one_open) == "45 min to no limit");
    const uint16_t with_zero[7] = { 0, 30, 30, 30, 30, 30, 0 };
    CHECK(ui::days_summary(with_zero) == "0 min (no play) to 30 min");
}

static void test_names()
{
    CHECK(ui::day_name(0) == "Sunday" && ui::day_name(1) == "Monday" && ui::day_name(6) == "Saturday");
    CHECK(ui::day_name(-1) == "?" && ui::day_name(7) == "?");
    CHECK(ui::day_name_in_text(3) == "Wednesday" && ui::day_name_in_text(7) == "?" && ui::day_name_in_text(-1) == "?");

    CHECK(ui::level_name(0) == "None" && ui::level_name(1) == "Custom" && ui::level_name(4) == "Teen");
    CHECK(ui::level_name(5) == "?" && ui::level_name(UINT32_MAX) == "?");
    CHECK(ui::age_text(0) == "No restriction" && ui::age_text(12) == "Up to 12 years");

    CHECK(ui::yes_no(true) == "Yes" && ui::yes_no(false) == "No");
    CHECK(ui::bool_text(true, true, "on", "off") == "on" && ui::bool_text(true, false, "on", "off") == "off");
    CHECK(ui::bool_text(false, true, "on", "off") == "Unavailable");

    CHECK(ui::opens() == "›" && ui::opens("") == "›" && ui::opens("12 changes") == "12 changes  ›");
}

// Another language: its own words, its own cases ("lundi" inside a sentence).
static void test_french()
{
    REQUIRE(load("fr"));
    CHECK(ui::day_name(1) == s_catalog["playguard/days/1"]);
    CHECK(ui::day_name_in_text(1) == "lundi");
    CHECK(ui::fmt_minutes(125) == "2 h 05");
    CHECK(ui::fmt_minutes(PT_DAY_NOLIMIT) == s_catalog["playguard/common/no_limit"]);
}

int main()
{
    REQUIRE(load("en-US"));
    test_minutes();
    test_days_summary();
    test_names();
    test_french();
    for (const std::string& k : s_missing) std::fprintf(stderr, "no such key: %s\n", k.c_str());
    CHECK(s_missing.empty());
    return CHECK_DONE("format_text tests passed");
}
