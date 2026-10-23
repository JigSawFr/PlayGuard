// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_rules_logic.hpp"

#include "action/forwarders_logic.hpp"
#include "core/gate.h"
#include "util/date.hpp"

namespace forwarder_rules_logic
{

void set_free(forwarders::Made& m, int days, int from, int to, int days2, int from2, int to2)
{
    m.free_days = days;
    m.free_from = days ? from : 0;
    m.free_to = days ? to : 0;
    m.free2_days = days ? days2 : 0;
    m.free2_from = m.free2_days ? from2 : 0;
    m.free2_to = m.free2_days ? to2 : 0;
}

void set_closed(forwarders::Made& m, int days, int from, int to)
{
    m.closed_days = days;
    m.closed_from = days ? from : 0;
    m.closed_to = days ? to : 0;
}

std::string closed_rules(const std::vector<forwarders::Made>& made)
{
    std::string closed_text;
    for (const forwarders::Made& m : made) {
        if (!m.pin || m.carried) continue;
        char line[0x400];
        const GateHours c = { (uint8_t)m.closed_days, (uint16_t)m.closed_from, (uint16_t)m.closed_to };
        if (m.closed_days && gate_rule_format(&c, m.nro.c_str(), line, sizeof(line))) closed_text += line;
    }
    return closed_text;
}

Log fold_log(const std::string& text, const std::vector<forwarders::Made>& made, gate_tally::Tally& tally)
{
    Log out;
    size_t at = 0;
    int n = 0;
    while (at < text.size() && n < LOG_MAX) {
        size_t end = text.find('\n', at);
        if (end == std::string::npos) end = text.size();
        const char *when, *nro;
        size_t wn, nn;
        GateResult r;
        if (gate_log_parse(text.data() + at, end - at, &when, &wn, &r, &nro, &nn)) {
            const std::string path_s(nro, nn);
            std::string name = forwarders_logic::file_title(path_s);
            for (size_t i = 0; i < made.size(); i++) {
                if (made[i].nro != path_s) continue;
                name = made[i].name;
                // The gate opened it: this way back works.
                if ((r == GateResult_Pin || r == GateResult_Free) && made[i].pin && !made[i].verified)
                    out.seen_working.push_back(i);
            }
            const char* kind = r == GateResult_Pin      ? "gate_open"
                               : r == GateResult_Free   ? "gate_free"
                               : r == GateResult_Paused ? "gate_paused"
                               : r == GateResult_Closed ? "gate_closed"
                               : r == GateResult_Limit  ? "gate_limit"
                                                        : "gate_refused";
            std::string w(when, wn);
            if (w == "?") w.clear();
            if (w.size() >= 10) gate_tally::add(tally, w.substr(0, 10), path_s, r);
            if (r == GateResult_Refused) out.refused.push_back({ name, w });
            const std::string detail = r == GateResult_Error ? name + " (error)" : name;
            // One entry per kind, forwarder and day: the times in it.
            bool joined = false;
            for (Seen& t : out.seen)
                if (t.kind == kind && t.detail == detail && t.when.substr(0, 10) == w.substr(0, 10) && !w.empty()) {
                    t.times.push_back(w.size() > 11 ? w.substr(11) : w);
                    joined = true;
                }
            if (!joined) out.seen.push_back({ kind, w, detail, { w.size() > 11 ? w.substr(11) : w } });
            n++;
        }
        at = end + 1;
    }
    return out;
}

std::string until_text(const std::string& full, const std::string& today)
{
    if (full.size() >= 16 && full.compare(0, 10, today) == 0) return full.substr(11, 5);
    return full.size() >= 16 ? full.substr(0, 16) : full;
}

std::string refused_by_day(const gate_tally::Week& w, const std::string& today, const std::vector<std::string>& day_names)
{
    std::string by_day;
    for (size_t i = 0; i < w.days.size(); i++) {
        if (!w.days[i].refused) continue;
        const int wd = date::weekday(date::add_days(today, (int)i - 6));
        by_day += (by_day.empty() ? "" : ", ") + (wd >= 0 ? day_names[(size_t)wd] : std::string("?")) + " " +
                  std::to_string(w.days[i].refused);
    }
    return by_day;
}

}   // namespace forwarder_rules_logic
