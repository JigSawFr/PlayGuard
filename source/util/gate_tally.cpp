// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/gate_tally.hpp"

#include <algorithm>

#include "util/date.hpp"
#include "util/json_util.hpp"

namespace gate_tally
{

Tally parse(const std::string& text)
{
    Tally t;
    nlohmann::json doc;
    if (!json_util::parse(text, &doc) || !doc.is_object()) return t;
    const nlohmann::json* days = json_util::object_at(doc, "days");
    if (!days) return t;
    for (auto d = days->begin(); d != days->end(); ++d) {
        if (!date::parse_valid(d.key()) || !d->is_object()) continue;
        for (auto f = d->begin(); f != d->end(); ++f) {
            if (!f->is_object() || !f.key().starts_with("sdmc:/")) continue;
            Count c;
            c.opened = (int)std::max<int64_t>(0, json_util::integer_or(*f, "opened"));
            c.refused = (int)std::max<int64_t>(0, json_util::integer_or(*f, "refused"));
            c.blocked = (int)std::max<int64_t>(0, json_util::integer_or(*f, "blocked"));
            if (c.opened || c.refused || c.blocked) t[d.key()][f.key()] = c;
        }
    }
    return t;
}

std::string text(const Tally& t)
{
    nlohmann::json days = nlohmann::json::object();
    for (const auto& [day, by] : t)
        for (const auto& [nro, c] : by)
            days[day][nro] = { { "opened", c.opened }, { "refused", c.refused }, { "blocked", c.blocked } };
    const nlohmann::json doc = { { "days", days } };
    return doc.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
}

void add(Tally& t, const std::string& day, const std::string& nro, GateResult r)
{
    if (!date::parse_valid(day) || !nro.starts_with("sdmc:/")) return;
    Count& c = t[day][nro];
    if (r == GateResult_Pin || r == GateResult_Free) c.opened++;
    else if (r == GateResult_Refused) c.refused++;
    else c.blocked++;
}

void trim(Tally& t, const std::string& today)
{
    const std::string first = date::add_days(today, -(KEEP_DAYS - 1));
    // "YYYY-MM-DD" sorts as the days do.
    std::erase_if(t, [&](const auto& day) {
        return !date::parse_valid(day.first) || first.empty() || day.first < first || day.first > today;
    });
}

std::vector<Week> week(const Tally& t, const std::string& today)
{
    std::map<std::string, Week> by;
    for (int i = 0; i < 7; i++) {
        const auto d = t.find(date::add_days(today, i - 6));
        if (d == t.end()) continue;
        for (const auto& [nro, c] : d->second) {
            Week& w = by[nro];
            w.nro = nro;
            w.days[(size_t)i] = c;
            w.total.opened += c.opened;
            w.total.refused += c.refused;
            w.total.blocked += c.blocked;
        }
    }
    std::vector<Week> out;
    for (auto& [nro, w] : by) out.push_back(w);
    std::stable_sort(out.begin(), out.end(), [](const Week& a, const Week& b) { return a.total.refused > b.total.refused; });
    return out;
}

void refusals(const Tally& t, const std::string& today, int* this_week, int* week_before)
{
    int now = 0, before = 0;
    for (int i = 0; i < 14; i++) {
        const auto d = t.find(date::add_days(today, -i));
        if (d == t.end()) continue;
        for (const auto& [nro, c] : d->second) (i < 7 ? now : before) += c.refused;
    }
    if (this_week) *this_week = now;
    if (week_before) *week_before = before;
}

bool rising(int this_week, int week_before)
{
    return this_week >= 5 && this_week > 2 * week_before;
}

}   // namespace gate_tally
