// Host tests for source/ui/cadran_text.cpp, translated by the real catalogs
// (resources/i18n/<lang>/playguard.json, read from the repository root `make
// test` runs in) through ui/tr.hpp's stand-in for borealis: a restriction's
// line and its state in each script's separator, a week's identical lines as
// one (and a short table left alone), and a failure's title, reason, code and
// report, in English and French, with or without each part.
#include "check.h"
#include <fstream>
#include <map>
#include <borealis/extern/nlohmann/json.hpp>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ui/cadran_text.hpp"
#include "ui/tr.hpp"

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

using Rows = std::vector<std::pair<std::string, std::string>>;

static void test_split_state()
{
    using cadran_text::split_state;
    CHECK(split_state(s_catalog["playguard/cadran/restrictions/comm_blocked"]) ==
          std::make_pair(std::string("Communication"), std::string("restricted")));
    CHECK(split_state("Games blocked \u00B7 10").second == "10");
    CHECK(split_state("\u4EA4\u6D41\u30FB\u5236\u9650") ==   // 交流・制限
          std::make_pair(std::string("\u4EA4\u6D41"), std::string("\u5236\u9650")));
    CHECK(split_state("A \u00B7 B \u00B7 C").first == "A \u00B7 B");
    CHECK(split_state("No separator") == std::make_pair(std::string("No separator"), std::string()));
    CHECK(split_state("") == std::make_pair(std::string(), std::string()));
}

static void test_group_rows()
{
    using cadran_text::group_rows;
    const Rows week = { { "Monday", "2 h" }, { "Tuesday", "2 h" }, { "Wednesday", "2 h" }, { "Thursday", "2 h" },
                        { "Friday", "2 h" }, { "Saturday", "3 h" }, { "Sunday", "3 h" } };
    const Rows grouped = group_rows(week);
    CHECK(grouped.size() == 2);
    CHECK(grouped[0] == std::make_pair(std::string("Monday \u2013 Friday"), std::string("2 h")));
    CHECK(grouped[1] == std::make_pair(std::string("Saturday \u2013 Sunday"), std::string("3 h")));
    // Every day different: as they are; a value back again later: not joined.
    const Rows mixed = { { "a", "1" }, { "b", "2" }, { "c", "1" }, { "d", "2" }, { "e", "1" } };
    CHECK(group_rows(mixed) == mixed);
    const Rows tail = { { "a", "1" }, { "b", "2" }, { "c", "2" }, { "d", "3" }, { "e", "3" } };
    CHECK(group_rows(tail).size() == 3 && group_rows(tail)[1].first == "b \u2013 c");
    // A short table (a few settings) is never grouped.
    const Rows short_table = { { "a", "1" }, { "b", "1" }, { "c", "1" }, { "d", "1" } };
    CHECK(group_rows(short_table) == short_table);
    CHECK(group_rows({}).empty());
}

static void test_failure()
{
    using cadran_text::failure;
    const std::string next = "\n\n" + s_catalog["playguard/error/next_step"];
    const std::string code = brls::getStr("playguard/error/code_hint", "0x0000C88E", "refused by the service");
    cadran_text::Failure f = failure("Could not unlock parental controls \u2014 " + code + next);
    CHECK(f.title == "Could not unlock parental controls");
    CHECK(f.reason == "Refused by the service");
    CHECK(f.code == brls::getStr("playguard/error/code", "0x0000C88E"));
    CHECK(f.report);

    // The code alone: no reason left; no report asked.
    f = failure("Could not save \u2014 " + brls::getStr("playguard/error/code", "0x00001234"));
    CHECK(f.title == "Could not save" && f.reason.empty() && !f.code.empty() && !f.report);

    // A sentence with no dash nor code: all of it is the reason.
    f = failure("The SD card is full.");
    CHECK(f.title.empty() && f.reason == "The SD card is full." && f.code.empty() && !f.report);

    // A dash too far into the text is part of the sentence, not a title.
    const std::string long_start(130, 'x');
    f = failure(long_start + " \u2014 more");
    CHECK(f.title.empty() && f.reason == long_start + " \u2014 more");

    // "0x" that is not a code after "error": the text as it is.
    f = failure("Value 0x12345678 out of range");
    CHECK(f.code.empty() && f.reason == "Value 0x12345678 out of range");
}

// French: its own words for "error" and the next step.
static void test_french()
{
    REQUIRE(load("fr"));
    const std::string next = "\n\n" + s_catalog["playguard/error/next_step"];
    const std::string code = brls::getStr("playguard/error/code_hint", "0x0000C88E", "refus\u00E9");
    const cadran_text::Failure f = cadran_text::failure("Impossible de d\u00E9verrouiller \u2014 " + code + next);
    CHECK(f.title == "Impossible de d\u00E9verrouiller" && f.report);
    CHECK(f.code == brls::getStr("playguard/error/code", "0x0000C88E"));
    CHECK(cadran_text::split_state(s_catalog["playguard/cadran/restrictions/sns_allowed"]).second == "autoris\u00E9s");
}

int main()
{
    REQUIRE(load("en-US"));
    test_split_state();
    test_group_rows();
    test_failure();
    test_french();
    for (const std::string& k : s_missing) std::fprintf(stderr, "no such key: %s\n", k.c_str());
    CHECK(s_missing.empty());
    return CHECK_DONE("cadran_text tests passed");
}
