// forwarder_flow's part for the gate: its rules (the free hours, the hours
// it never opens), its pause after refused PINs, its log read into the
// change history.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarder_flow.hpp"

#include <borealis.hpp>
#include <cstdio>

#include "action/forwarder_flow_shared.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "core/gate.h"
#include "core/pctl_ops.h"
#include "ui/ui.hpp"
#include "util/forwarders.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace forwarder_flow
{

namespace detail
{

// The free hours of every protected forwarder, and the hours it never
// opens, for the gate (GATE_RULES_NAME, GATE_CLOSED_NAME).
void write_rules()
{
    std::string free_text, closed_text;
    for (const forwarders::Made& m : forwarders::list()) {
        if (!m.pin) continue;
        char line[0x400];
        const GateHours f = { (uint8_t)m.free_days, (uint16_t)m.free_from, (uint16_t)m.free_to };
        if (m.free_days && gate_rule_format(&f, m.nro.c_str(), line, sizeof(line))) free_text += line;
        const GateHours c = { (uint8_t)m.closed_days, (uint16_t)m.closed_from, (uint16_t)m.closed_to };
        if (m.closed_days && gate_rule_format(&c, m.nro.c_str(), line, sizeof(line))) closed_text += line;
    }
    for (const auto& [name, text] : { std::pair<const char*, std::string>{ GATE_RULES_NAME, free_text },
                                      { GATE_CLOSED_NAME, closed_text } }) {
        const std::string path = paths::data_dir() + "/" + name;
        if (text.empty()) {
            std::remove(path.c_str());
            continue;
        }
        std::string err;
        if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(path, text, &err))
            brls::Logger::warning("forwarders: {} not written ({})", name, err);
    }
}

std::string pause_path()
{
    return paths::data_dir() + "/" + GATE_PAUSE_NAME;
}

std::string read_pause()
{
    std::string text;
    if (!paths::read_file(pause_path(), text)) text.clear();
    return text;
}

}   // namespace detail

using namespace detail;

namespace
{
// Free hours (`closed` false) or the hours it never opens: saved, the
// gate's files written, in the change history.
void set_span(const forwarders_logic::Row& row, bool closed, int days, int from, int to, std::function<void(bool)> done,
              bool ask)
{
    forwarders::Made m = made_of(row);
    if (!m.pin || !forwarders::find(row.id, nullptr)) {
        if (done) done(false);
        return;
    }
    // The PIN first: these hours change when it starts (not to put them back
    // as they were a moment ago: ask false).
    if (ui::refuse_read_only() || (ask && !pin_lock::confirm()) || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    int& d = closed ? m.closed_days : m.free_days;
    int& f = closed ? m.closed_from : m.free_from;
    int& t = closed ? m.closed_to : m.free_to;
    d = days;
    f = days ? from : 0;
    t = days ? to : 0;
    std::string err;
    if (!forwarders::save(m, &err)) {
        const std::string hint = hbmenu_lock::write_hint(paths::nand_dir());
        ui::error("playguard/forwarders/hours_err"_i18n, hint.empty() ? err : err + "\n\n" + hint);
        if (done) done(false);
        return;
    }
    write_rules();
    history_flow::record_event(closed ? "gate_closed_hours" : "gate_hours", "",
                               history_label(m.name, m.id) + (days ? " · " + hours_label(days, from, to) : std::string()));
    if (closed)
        ui::notify(days ? brls::getStr("playguard/forwarders/closed_set", m.name, hours_label(days, from, to))
                        : brls::getStr("playguard/forwarders/closed_none", m.name),
                   ui::Toast::Done);
    else
        ui::notify(days ? brls::getStr("playguard/forwarders/hours_set", m.name, hours_label(days, from, to))
                        : brls::getStr("playguard/forwarders/hours_none", m.name),
                   ui::Toast::Done);
    if (done) done(true);
}
}   // namespace

void set_hours(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done, bool ask)
{
    set_span(row, false, days, from, to, std::move(done), ask);
}

void set_closed(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done, bool ask)
{
    set_span(row, true, days, from, to, std::move(done), ask);
}

GatePauseRule pause_rule()
{
    const std::string text = read_pause();
    return gate_pause_rule(text.data(), text.size());
}

void set_pause_rule(unsigned tries, unsigned minutes, std::function<void(bool)> done)
{
    if (ui::refuse_read_only() || !pin_lock::confirm() || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    const std::string text = read_pause();
    const GatePauseRule rule = { tries, tries ? minutes : 0 };
    std::string out(GATE_PAUSE_MAX, '\0');
    const size_t n = gate_pause_edit(text.data(), text.size(), &rule, nullptr, &out[0], out.size());
    std::string err;
    if (n == 0 || !paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(pause_path(), out.substr(0, n), &err)) {
        const std::string hint = hbmenu_lock::write_hint(paths::data_dir());
        ui::error("playguard/forwarders/pause_err"_i18n, hint.empty() ? err : err + "\n\n" + hint);
        if (done) done(false);
        return;
    }
    history_flow::record_event("gate_pause", "", pause_label(rule.tries, rule.minutes));
    ui::notify(brls::getStr("playguard/forwarders/pause_set", pause_label(rule.tries, rule.minutes)), ui::Toast::Done);
    if (done) done(true);
}

std::string until_label(uint64_t posix)
{
    // The time alone today, else the day too ("2026-10-21 07:00").
    const std::string full = ui::time_text(posix);
    const std::string today = ui::today_date();
    if (full.size() >= 16 && full.compare(0, 10, today) == 0) return full.substr(11, 5);
    return full.size() >= 16 ? full.substr(0, 16) : full;
}

std::string pause_label(unsigned tries, unsigned minutes)
{
    if (!tries) return "playguard/forwarders/pause_off"_i18n;
    return brls::getStr("playguard/forwarders/pause_rule", (int)tries, ui::fmt_played((uint16_t)minutes));
}

void lift_pause(const forwarders_logic::Row& row, std::function<void(bool)> done)
{
    // The PIN first: lifting it lets the PIN be tried again at once.
    if (ui::refuse_read_only() || !pin_lock::confirm()) {
        if (done) done(false);
        return;
    }
    const std::string text = read_pause();
    std::string out(GATE_PAUSE_MAX, '\0');
    const size_t n = gate_pause_edit(text.data(), text.size(), nullptr, row.nro.c_str(), &out[0], out.size());
    std::string err;
    if ((n == 0 && !text.empty() && std::remove(pause_path().c_str()) != 0) ||
        (n > 0 && !paths::atomic_write(pause_path(), out.substr(0, n), &err))) {
        ui::error("playguard/forwarders/pause_err"_i18n, err);
        if (done) done(false);
        return;
    }
    history_flow::record_event("gate_pause_lifted", "", history_label(row.name, row.id));
    ui::notify(brls::getStr("playguard/forwarders/pause_lifted", row.name), ui::Toast::Done);
    if (done) done(true);
}

std::string hours_label(int days, int from, int to)
{
    const std::vector<std::string> names = {
        "playguard/forwarders/day_sun"_i18n, "playguard/forwarders/day_mon"_i18n, "playguard/forwarders/day_tue"_i18n,
        "playguard/forwarders/day_wed"_i18n, "playguard/forwarders/day_thu"_i18n, "playguard/forwarders/day_fri"_i18n,
        "playguard/forwarders/day_sat"_i18n,
    };
    return forwarders_logic::hours_text(days, from, to, names);
}

std::vector<forwarders_logic::Refusals> take_log()
{
    // Each try the gate logged since: into the change history, then gone.
    const std::string path = paths::data_dir() + "/" + GATE_LOG_NAME;
    std::string text;
    if (!paths::read_file(path, text)) return {};
    std::remove(path.c_str());
    std::vector<forwarders::Made> made = forwarders::list();
    std::vector<std::pair<std::string, std::string>> refused;
    std::vector<size_t> seen_working;
    struct Tries
    {
        const char*              kind;
        std::string              when, detail;
        std::vector<std::string> times;
    };
    std::vector<Tries> tries;
    size_t at = 0;
    int n = 0;
    while (at < text.size() && n < 200) {
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
                    seen_working.push_back(i);
            }
            const char* kind = r == GateResult_Pin      ? "gate_open"
                               : r == GateResult_Free   ? "gate_free"
                               : r == GateResult_Paused ? "gate_paused"
                               : r == GateResult_Closed ? "gate_closed"
                                                        : "gate_refused";
            std::string w(when, wn);
            if (w == "?") w.clear();
            if (r == GateResult_Refused) refused.push_back({ name, w });
            const std::string detail = r == GateResult_Error ? name + " (error)" : name;
            // One entry per kind, forwarder and day: the times in it.
            bool joined = false;
            for (Tries& t : tries)
                if (t.kind == kind && t.detail == detail && t.when.substr(0, 10) == w.substr(0, 10) && !w.empty()) {
                    t.times.push_back(w.size() > 11 ? w.substr(11) : w);
                    joined = true;
                }
            if (!joined) tries.push_back({ kind, w, detail, { w.size() > 11 ? w.substr(11) : w } });
            n++;
        }
        at = end + 1;
    }
    for (const Tries& t : tries) {
        if (t.times.size() < 2) {
            history_flow::record_seen(t.kind, t.when, t.detail);
            continue;
        }
        std::string times;
        for (const std::string& h : t.times) times += (times.empty() ? "" : ", ") + h;
        history_flow::record_seen(t.kind, t.when,
                                  brls::getStr("playguard/forwarders/tries_grouped", t.detail, (int)t.times.size(), times));
    }
    for (const size_t i : seen_working) {
        if (made[i].verified) continue;
        made[i].verified = true;
        std::string err;
        if (!forwarders::save(made[i], &err)) brls::Logger::warning("forwarders: not remembered ({})", err);
    }
    return forwarders_logic::repeated_refusals(refused);
}

std::vector<forwarders_logic::Refusals> at_start()
{
    // The gate as this PlayGuard carries it, and the free hours, while a
    // forwarder is protected.
    bool any = false;
    for (const forwarders::Made& m : forwarders::list()) any = any || m.pin;
    if (any) {
        gate_nro();
        write_rules();
    }
    return take_log();
}

}   // namespace forwarder_flow
