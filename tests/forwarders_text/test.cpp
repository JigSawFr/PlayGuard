// Host tests for source/action/forwarders_text.cpp, translated by the real
// catalogs (en-US, then French for the day names) through ui/tr.hpp's
// stand-in for borealis: a forwarder's state alone, its state with its rules
// (in a word in the list, every one in Details; paused first; to make
// again; a protected menu seen working), the list's text and icons, a
// rule's days and hours (two spans). Every key asked for exists.
#include <borealis/extern/nlohmann/json.hpp>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <string>

#include <borealis/cadran/ui/tr.hpp>

#include "check.h"

#include "action/forwarders_text.hpp"

using namespace forwarders_text;
using forwarders_logic::Row;
using forwarders_logic::State;

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
    // PlayGuard's catalog, and Aurore's Cadran's (extern/borealis).
    std::ifstream in("resources/i18n/" + lang + "/playguard.json");
    std::ifstream cadran("extern/borealis/resources/i18n/" + lang + "/cadran.json");
    if (!in || !cadran) return false;
    flatten(nlohmann::json::parse(in), "playguard/");
    flatten(nlohmann::json::parse(cadran), "cadran/");
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

static std::string tr(const char* key)
{
    return s_catalog["playguard/forwarders/" + std::string(key)];
}

static Row protected_row()
{
    Row r;
    r.name = "Menu";
    r.nro = "sdmc:/switch/ftpd/ftpd.nro";
    r.pin = true;
    r.carried = true;
    return r;
}

static std::string icons(const std::vector<Mark>& marks)
{
    std::string out;
    for (const Mark& m : marks) out += std::string(out.empty() ? "" : ",") + m.icon + (std::string(m.color_key) == "cadran/amber" ? "!" : "");
    return out;
}

static void test_hours()
{
    CHECK(day_names().size() == 7 && day_names()[0] == "Sun" && day_names()[6] == "Sat");
    CHECK(hours_label(0x41, 540, 1200) == "Sat–Sun 09:00–20:00");
    CHECK(hours_label(0x3E, 960, 1200, 0x41, 540, 1260) == "Mon–Fri 16:00–20:00, Sat–Sun 09:00–21:00");
    CHECK(hours_label(0x3E, 960, 1200, 0, 540, 1260) == "Mon–Fri 16:00–20:00");   // no second span
    CHECK(hours_label(0, 0, 0, 0x41, 540, 1260) == hours_label(0, 0, 0));          // a second one alone: not shown
}

static void test_state()
{
    Row r;
    const std::pair<State, const char*> states[] = {
        { State::Ready, "state_ready" },          { State::NroMissing, "state_nro_missing" },
        { State::GateMissing, "state_gate_missing" }, { State::NotInstalled, "state_not_installed" },
        { State::Elsewhere, "state_elsewhere" },
    };
    for (const auto& [state, key] : states) {
        r.state = state;
        CHECK(state_only(r) == tr(key) && !tr(key).empty());
        CHECK(state_text(r, "", true) == tr(key) && list_text(r, "") == tr(key));   // not protected: its state only
        CHECK(list_marks(r).empty());
    }

    // Protected, no rule.
    r = protected_row();
    CHECK(state_text(r, "") == "On the HOME menu · PIN");
    CHECK(list_text(r, "").empty() && icons(list_marks(r)) == "key");
    // One rule: said in full in the list too.
    r.free_days = 0x41;
    r.free_from = 540;
    r.free_to = 1200;
    CHECK(state_text(r, "") == "On the HOME menu · PIN, not asked Sat–Sun 09:00–20:00");
    CHECK(icons(list_marks(r)) == "key,unlock");
    // More than one: a word in the list, each in Details.
    r.closed_days = 0x3E;
    r.closed_from = 1320;
    r.closed_to = 420;
    r.limit = true;
    CHECK(state_text(r, "") == "On the HOME menu · PIN, with its own rules");
    CHECK(state_text(r, "", true) == "On the HOME menu · PIN, not asked Sat–Sun 09:00–20:00 · closed Mon–Fri 22:00–07:00"
                                     " · closed once today's time is used up");
    CHECK(icons(list_marks(r)) == "key,unlock,stop,timer");
    // Made before it carried its rules.
    r.carried = false;
    CHECK(state_text(r, "") == "On the HOME menu · PIN, with its own rules · to make again");
    CHECK(list_text(r, "") == "to make again");
    // Paused: that first and alone.
    r.paused_until = 1;
    CHECK(state_text(r, "07:00", true) == "On the HOME menu · paused until 07:00");
    CHECK(list_text(r, "07:00") == "paused until 07:00");
    CHECK(icons(list_marks(r)) == "key,pause!");
    // Not ready: its state in the list.
    r.paused_until = 0;
    r.carried = true;
    r.state = State::NroMissing;
    CHECK(list_text(r, "") == "Its .nro is gone");
    r.carried = false;
    CHECK(list_text(r, "") == "Its .nro is gone · to make again");

    // A protected menu seen working through its gate; another homebrew never.
    r = protected_row();
    r.verified = true;
    CHECK(state_text(r, "") == "On the HOME menu · PIN");
    r.nro = "sdmc:/hbmenu.nro";
    CHECK(state_text(r, "") == "On the HOME menu · PIN · seen working");
}

static void test_french()
{
    REQUIRE(load("fr"));
    CHECK(day_names()[1] == s_catalog["playguard/forwarders/day_mon"] && day_names()[1] != "Mon");
    CHECK(hours_label(0x02, 600, 660) == s_catalog["playguard/forwarders/day_mon"] + " 10:00–11:00");
}

int main()
{
    REQUIRE(load("en-US"));
    test_hours();
    test_state();
    test_french();
    for (const std::string& k : s_missing) std::fprintf(stderr, "no such key: %s\n", k.c_str());
    CHECK(s_missing.empty());
    return CHECK_DONE("forwarders_text tests passed");
}
