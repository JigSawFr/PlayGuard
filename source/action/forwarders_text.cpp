// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarders_text.hpp"

#include <borealis/cadran/ui/tr.hpp>

using namespace brls::literals;
using forwarders_logic::Row;
using forwarders_logic::State;

namespace forwarders_text
{

namespace
{
// Protected: the PIN, and when it is not asked.
std::string state_text_pin(const Row& r, const std::string& until, bool full)
{
    std::string out = state_only(r) + " · ";
    // Paused after refused PINs: that first, it is what the child sees now.
    if (r.paused_until) return out + brls::getStr("playguard/forwarders/state_paused", until);
    // Made before it carried its rules: its free hours wait for it to be made again.
    const std::string remake = r.carried ? "" : " · " + "playguard/forwarders/state_remake"_i18n;
    const int rules = (r.free_days ? 1 : 0) + (r.closed_days ? 1 : 0) + (r.limit ? 1 : 0);
    if (!full && rules > 1) return out + "playguard/forwarders/state_pin_rules"_i18n + remake;
    out += !r.free_days ? "playguard/forwarders/state_pin"_i18n
                        : brls::getStr("playguard/forwarders/state_pin_hours",
                                       hours_label(r.free_days, r.free_from, r.free_to, r.free2_days, r.free2_from, r.free2_to));
    if (r.closed_days)
        out += " · " + brls::getStr("playguard/forwarders/state_closed", hours_label(r.closed_days, r.closed_from, r.closed_to));
    if (r.limit) out += " · " + "playguard/forwarders/state_limit"_i18n;
    return out + remake;
}
}   // namespace

std::vector<std::string> day_names()
{
    return {
        "playguard/forwarders/day_sun"_i18n, "playguard/forwarders/day_mon"_i18n, "playguard/forwarders/day_tue"_i18n,
        "playguard/forwarders/day_wed"_i18n, "playguard/forwarders/day_thu"_i18n, "playguard/forwarders/day_fri"_i18n,
        "playguard/forwarders/day_sat"_i18n,
    };
}

std::string hours_label(int days, int from, int to)
{
    return forwarders_logic::hours_text(days, from, to, day_names());
}

std::string hours_label(int days, int from, int to, int days2, int from2, int to2)
{
    std::string out = hours_label(days, from, to);
    if (days && days2) out += ", " + hours_label(days2, from2, to2);
    return out;
}

std::string state_only(const Row& r)
{
    switch (r.state) {
        case State::Ready:        return "playguard/forwarders/state_ready"_i18n;
        case State::NroMissing:   return "playguard/forwarders/state_nro_missing"_i18n;
        case State::GateMissing:  return "playguard/forwarders/state_gate_missing"_i18n;
        case State::NotInstalled: return "playguard/forwarders/state_not_installed"_i18n;
        case State::Elsewhere:    return "playguard/forwarders/state_elsewhere"_i18n;
    }
    return "";
}

std::string state_text(const Row& r, const std::string& until, bool full)
{
    if (!r.pin) return state_only(r);
    if (r.verified && forwarders_logic::is_menu(r.nro))
        return state_text_pin(r, until, full) + " · " + "playguard/forwarders/state_verified"_i18n;
    return state_text_pin(r, until, full);
}

std::string list_text(const Row& r, const std::string& until)
{
    if (!r.pin) return state_only(r);
    std::string out = r.state == State::Ready ? "" : state_only(r);
    const auto add = [&out](const std::string& part) { out += (out.empty() ? "" : " · ") + part; };
    if (r.paused_until) add(brls::getStr("playguard/forwarders/state_paused", until));
    else if (!r.carried) add("playguard/forwarders/state_remake"_i18n);
    return out;
}

std::vector<Mark> list_marks(const Row& r)
{
    if (!r.pin) return {};
    std::vector<Mark> out = { { "key", "cadran/teal" } };
    if (r.paused_until) {
        out.push_back({ "pause", "cadran/amber" });
        return out;
    }
    if (r.free_days) out.push_back({ "unlock", "cadran/teal" });
    if (r.closed_days) out.push_back({ "stop", "cadran/teal" });
    if (r.limit) out.push_back({ "timer", "cadran/teal" });
    return out;
}

}   // namespace forwarders_text
