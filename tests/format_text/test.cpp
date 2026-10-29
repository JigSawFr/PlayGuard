// Host tests for source/ui/play_text.cpp and Aurore's Cadran format_text.cpp,
// translated by the real catalogs (resources/i18n/<lang>/playguard.json and
// Aurore's cadran.json, read from the repository root `make test` runs in)
// through ui/tr.hpp's stand-in for Aurore: limits ("No
// limit", "0 min (no play)", "45 min", "2 h", "2 h 05"), the week's limits in
// a few words, time played and left (rounded to the minute, at most a day),
// play time over any length ("< 1 min", "52 h 30", "278 h"), a daily
// average over days, months or years, the day names (in a title and inside
// a sentence, French for the difference), the restriction
// levels and ages, yes / no, "Unavailable", and the chevron of a line that
// opens a screen. Every key the helpers ask for exists in the catalog.
#include "check.h"
#include <cstdint>
#include <fstream>
#include <map>
#include <aurore/extern/nlohmann/json.hpp>
#include <set>
#include <string>

#include <aurore/cadran/ui/tr.hpp>

#include "ui/play_text.hpp"
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
    // PlayGuard's catalog, and Aurore's Cadran's (library/aurore).
    std::ifstream in("resources/i18n/" + lang + "/playguard.json");
    std::ifstream cadran("library/aurore/resources/i18n/" + lang + "/cadran.json");
    if (!in || !cadran) return false;
    flatten(nlohmann::json::parse(in), "playguard/");
    flatten(nlohmann::json::parse(cadran), "cadran/");
    return !s_catalog.empty();
}

namespace aurore
{
namespace internal
{
// As Aurore: a key the catalog lacks reads as itself.
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
}   // namespace aurore

static void test_minutes()
{
    CHECK(ui::fmt_limit(PT_DAY_NOLIMIT) == "No limit");
    CHECK(ui::fmt_limit(0) == "0 min (no play)");
    CHECK(ui::fmt_limit(1) == "1\u00A0min" && ui::fmt_limit(59) == "59\u00A0min");
    CHECK(ui::fmt_limit(60) == "1\u00A0h" && ui::fmt_limit(120) == "2\u00A0h" && ui::fmt_limit(1440) == "24\u00A0h");
    CHECK(ui::fmt_limit(125) == "2\u00A0h\u00A005" && ui::fmt_limit(150) == "2\u00A0h\u00A030" && ui::fmt_limit(61) == "1\u00A0h\u00A001");

    // Played: no "(no play)" for none.
    CHECK(ui::fmt_played(0) == "0\u00A0min");
    CHECK(ui::fmt_played(45) == "45\u00A0min" && ui::fmt_played(150) == "2\u00A0h\u00A030");

    // Left, from nanoseconds: rounded to the nearest minute, at most a day.
    const uint64_t MIN = 60000000000ULL;
    CHECK(ui::fmt_time_left(0) == "0\u00A0min");
    CHECK(ui::fmt_time_left(29 * 1000000000ULL) == "0\u00A0min");
    CHECK(ui::fmt_time_left(30 * 1000000000ULL) == "1\u00A0min");
    CHECK(ui::fmt_time_left(90 * MIN + 29 * 1000000000ULL) == "1\u00A0h\u00A030");
    CHECK(ui::fmt_time_left(90 * MIN + 30 * 1000000000ULL) == "1\u00A0h\u00A031");
    CHECK(ui::fmt_time_left(1440 * MIN) == "24\u00A0h" && ui::fmt_time_left(5000 * MIN) == "24\u00A0h");
    CHECK(ui::fmt_time_left(UINT64_MAX / 2) == "24\u00A0h");

    // Play time, from seconds: "< 1 min" under a minute (not for none), no cap.
    CHECK(ui::fmt_duration(0) == "0\u00A0min");
    CHECK(ui::fmt_duration(1) == "< 1 min" && ui::fmt_duration(59) == "< 1 min");
    CHECK(ui::fmt_duration(60) == "1\u00A0min" && ui::fmt_duration(89) == "1\u00A0min" && ui::fmt_duration(90) == "2\u00A0min");
    CHECK(ui::fmt_duration(3600) == "1\u00A0h" && ui::fmt_duration(3600 + 5 * 60) == "1\u00A0h\u00A005");
    CHECK(ui::fmt_duration(99 * 3600 + 59 * 60) == "99\u00A0h\u00A059");
    // From 100 h, whole hours, rounded.
    CHECK(ui::fmt_duration(152 * 3600 + 29 * 60) == "152\u00A0h" && ui::fmt_duration(152 * 3600 + 30 * 60) == "153\u00A0h");
    CHECK(ui::fmt_duration(277 * 3600 + 58 * 60) == "278\u00A0h");
    CHECK(ui::fmt_duration(100000ULL * 3600) == "100000\u00A0h");

    // A daily average over its days: days under two months, then months, then years.
    CHECK(ui::fmt_per_day_over(53 * 60, 45) == "53\u00A0min over 45 days");
    CHECK(ui::fmt_per_day_over(53 * 60, 59) == "53\u00A0min over 59 days");
    CHECK(ui::fmt_per_day_over(53 * 60, 60) == "53\u00A0min over 2 months");
    CHECK(ui::fmt_per_day_over(53 * 60, 548) == "53\u00A0min over 18 months");
    CHECK(ui::fmt_per_day_over(53 * 60, 729) == "53\u00A0min over 24 months");
    CHECK(ui::fmt_per_day_over(53 * 60, 730) == "53\u00A0min over 2 years");
    CHECK(ui::fmt_per_day_over(53 * 60, 2199) == "53\u00A0min over 6 years");
}

static void test_days_summary()
{
    const uint16_t N = PT_DAY_NOLIMIT;
    const uint16_t same[7] = { 120, 120, 120, 120, 120, 120, 120 };
    CHECK(ui::days_summary(same) == "2\u00A0h every day");
    const uint16_t zero[7] = { 0, 0, 0, 0, 0, 0, 0 };
    CHECK(ui::days_summary(zero) == "0 min (no play) every day");
    const uint16_t none[7] = { N, N, N, N, N, N, N };
    CHECK(ui::days_summary(none) == "No limit");
    const uint16_t range[7] = { 180, 60, 60, 90, 60, 60, 180 };
    CHECK(ui::days_summary(range) == "1\u00A0h to 3\u00A0h");
    // A day without a limit tops the range, in the sentence's case.
    const uint16_t open[7] = { N, 60, 60, 90, 60, 60, N };
    CHECK(ui::days_summary(open) == "1\u00A0h to no limit");
    const uint16_t one_open[7] = { 45, 45, 45, 45, 45, 45, N };
    CHECK(ui::days_summary(one_open) == "45\u00A0min to no limit");
    const uint16_t with_zero[7] = { 0, 30, 30, 30, 30, 30, 0 };
    CHECK(ui::days_summary(with_zero) == "0 min (no play) to 30\u00A0min");
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
    CHECK(ui::day_name(1) == s_catalog["cadran/days/1"]);
    CHECK(ui::day_name_in_text(1) == "lundi");
    CHECK(ui::fmt_limit(125) == "2\u00A0h\u00A005");
    CHECK(ui::fmt_per_day_over(53 * 60, 2199) == "53\u00A0min sur 6 ans");
    CHECK(ui::fmt_limit(PT_DAY_NOLIMIT) == s_catalog["playguard/common/no_limit"]);
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
