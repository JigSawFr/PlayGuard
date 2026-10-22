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
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/date.hpp"
#include "util/forwarders.hpp"
#include "util/gate_tally.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace forwarder_flow
{

namespace detail
{

// For the forwarders made before they carried their rules (Made::carried
// false): the hours they never open, which the gate still reads from the SD
// card (GATE_CLOSED_NAME: only stricter). Free hours are no longer read
// from it (GATE_RULES_NAME, removed): the SD card could add some.
void write_rules()
{
    std::string closed_text;
    for (const forwarders::Made& m : forwarders::list()) {
        if (!m.pin || m.carried) continue;
        char line[0x400];
        const GateHours c = { (uint8_t)m.closed_days, (uint16_t)m.closed_from, (uint16_t)m.closed_to };
        if (m.closed_days && gate_rule_format(&c, m.nro.c_str(), line, sizeof(line))) closed_text += line;
    }
    std::remove((paths::data_dir() + "/" + GATE_RULES_NAME).c_str());
    const std::string path = paths::data_dir() + "/" + GATE_CLOSED_NAME;
    if (closed_text.empty()) {
        std::remove(path.c_str());
        return;
    }
    std::string err;
    if (!paths::ensure_dir(paths::data_dir()) || !paths::atomic_write(path, closed_text, &err))
        brls::Logger::warning("forwarders: {} not written ({})", GATE_CLOSED_NAME, err);
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

std::string tally_path()
{
    return paths::nand_dir() + "/gate_tally.json";
}

gate_tally::Tally read_tally()
{
    std::string text;
    return paths::read_file(tally_path(), text) ? gate_tally::parse(text) : gate_tally::Tally();
}

}   // namespace detail

using namespace detail;

namespace
{
// A protected forwarder's rule changed (`change` on what PlayGuard
// remembers): the PIN first unless `ask` is false (putting it back as it
// was a moment ago), then made again, its rules in it; the change history's
// `kind` and `detail`, the toast's `toast`.
void change_rule(const forwarders_logic::Row& row, const std::function<void(forwarders::Made&)>& change, bool ask,
                 const char* kind, const std::function<std::string(const forwarders::Made&)>& detail,
                 const std::function<std::string(const forwarders::Made&)>& toast, std::function<void(bool)> done)
{
    forwarders::Made m = made_of(row);
    if (!m.pin || !forwarders::find(row.id, nullptr)) {
        if (done) done(false);
        return;
    }
    if (ui::refuse_read_only() || (ask && !pin_lock::confirm()) || R_FAILED(core_change_allowed())) {
        if (done) done(false);
        return;
    }
    change(m);
    // Made again: the gate reads its rules from the forwarder itself.
    if (!install(m, false, nullptr, "", true)) {
        if (done) done(false);
        return;
    }
    history_flow::record_event(kind, "", detail(m));
    ui::notify(toast(m), ui::Toast::Done);
    if (done) done(true);
}
}   // namespace

void set_hours(const forwarders_logic::Row& row, const Hours& h, std::function<void(bool)> done, bool ask)
{
    const Hours hours = h.days ? h : Hours{};
    change_rule(
        row,
        [hours](forwarders::Made& m) {
            m.free_days = hours.days;
            m.free_from = hours.days ? hours.from : 0;
            m.free_to = hours.days ? hours.to : 0;
            m.free2_days = hours.days ? hours.days2 : 0;
            m.free2_from = m.free2_days ? hours.from2 : 0;
            m.free2_to = m.free2_days ? hours.to2 : 0;
        },
        ask, "gate_hours",
        [hours](const forwarders::Made& m) {
            return history_label(m.name, m.id) + (hours.days ? " · " + hours_label(hours) : std::string());
        },
        [hours](const forwarders::Made& m) {
            return hours.days ? brls::getStr("playguard/forwarders/hours_set", m.name, hours_label(hours))
                              : brls::getStr("playguard/forwarders/hours_none", m.name);
        },
        std::move(done));
}

void set_closed(const forwarders_logic::Row& row, int days, int from, int to, std::function<void(bool)> done, bool ask)
{
    change_rule(
        row,
        [days, from, to](forwarders::Made& m) {
            m.closed_days = days;
            m.closed_from = days ? from : 0;
            m.closed_to = days ? to : 0;
        },
        ask, "gate_closed_hours",
        [days, from, to](const forwarders::Made& m) {
            return history_label(m.name, m.id) + (days ? " · " + hours_label(days, from, to) : std::string());
        },
        [days, from, to](const forwarders::Made& m) {
            return days ? brls::getStr("playguard/forwarders/closed_set", m.name, hours_label(days, from, to))
                        : brls::getStr("playguard/forwarders/closed_none", m.name);
        },
        std::move(done));
}

void set_limit(const forwarders_logic::Row& row, bool on, std::function<void(bool)> done, bool ask)
{
    change_rule(
        row, [on](forwarders::Made& m) { m.limit = on; }, ask, "gate_limit_set",
        [on](const forwarders::Made& m) {
            return history_label(m.name, m.id) + " · " +
                   (on ? "playguard/forwarders/limit_on"_i18n : "playguard/forwarders/limit_off"_i18n);
        },
        [on](const forwarders::Made& m) {
            return brls::getStr(on ? "playguard/forwarders/limit_set" : "playguard/forwarders/limit_unset", m.name);
        },
        std::move(done));
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
    // The forwarders that carry it: made again with it.
    std::vector<Job> jobs;
    for (forwarders::Made& m : forwarders::list())
        if (m.pin && m.carried) jobs.push_back({ m, m.nro == self_nro(), nullptr });
    install_batch(std::move(jobs), [rule, done](int made, int total) {
        const int failed = total - made;
        if (failed) brls::Logger::warning("forwarders: {} not made again with the new pause", failed);
        history_flow::record_event("gate_pause", "", pause_label(rule.tries, rule.minutes));
        ui::notify(brls::getStr("playguard/forwarders/pause_set", pause_label(rule.tries, rule.minutes)), ui::Toast::Done);
        if (done) done(failed == 0);
    });
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

std::string hours_label(const Hours& h)
{
    std::string out = hours_label(h.days, h.from, h.to);
    if (h.days && h.days2) out += ", " + hours_label(h.days2, h.from2, h.to2);
    return out;
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
    // Each try the gate logged since: into the change history, then gone
    // (only then: a crash before the history is written keeps them for the
    // next start).
    const std::string path = paths::data_dir() + "/" + GATE_LOG_NAME;
    std::string text;
    if (!paths::read_file(path, text)) return {};
    std::vector<forwarders::Made> made = forwarders::list();
    // Counted by day, for the week's tries (util/gate_tally.hpp).
    gate_tally::Tally tally = read_tally();
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
                               : r == GateResult_Limit  ? "gate_limit"
                                                        : "gate_refused";
            std::string w(when, wn);
            if (w == "?") w.clear();
            if (w.size() >= 10) gate_tally::add(tally, w.substr(0, 10), path_s, r);
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
    gate_tally::trim(tally, ui::today_date());
    std::string err;
    if (!paths::ensure_dir(paths::nand_dir()) || !paths::atomic_write(tally_path(), gate_tally::text(tally), &err))
        brls::Logger::warning("forwarders: the week's tries not kept ({})", err);
    std::remove(path.c_str());
    return forwarders_logic::repeated_refusals(refused);
}

void show_week()
{
    const std::string today = ui::today_date();
    const std::vector<gate_tally::Week> week = gate_tally::week(read_tally(), today);
    std::vector<ui::CheckLine> lines;
    const std::vector<forwarders::Made> made = forwarders::list();
    const std::vector<std::string> day_names = {
        "playguard/forwarders/day_sun"_i18n, "playguard/forwarders/day_mon"_i18n, "playguard/forwarders/day_tue"_i18n,
        "playguard/forwarders/day_wed"_i18n, "playguard/forwarders/day_thu"_i18n, "playguard/forwarders/day_fri"_i18n,
        "playguard/forwarders/day_sat"_i18n,
    };
    for (const gate_tally::Week& w : week) {
        std::string name = forwarders_logic::file_title(w.nro);
        for (const forwarders::Made& m : made)
            if (m.nro == w.nro) name = m.name;
        lines.push_back({ ui::Check::Header, name });
        lines.push_back({ w.total.opened ? ui::Check::Ok : ui::Check::Info,
                          brls::getStr("playguard/forwarders/week_opened", w.total.opened) });
        // The refusals, and on which days.
        std::string by_day;
        for (size_t i = 0; i < w.days.size(); i++) {
            if (!w.days[i].refused) continue;
            const int wd = date::weekday(date::add_days(today, (int)i - 6));
            by_day += (by_day.empty() ? "" : ", ") + (wd >= 0 ? day_names[(size_t)wd] : std::string("?")) + " " +
                      std::to_string(w.days[i].refused);
        }
        lines.push_back({ w.total.refused ? ui::Check::Warn : ui::Check::Ok,
                          w.total.refused ? brls::getStr("playguard/forwarders/week_refused", w.total.refused, by_day)
                                          : "playguard/forwarders/week_none_refused"_i18n });
        if (w.total.blocked)
            lines.push_back({ ui::Check::Info, brls::getStr("playguard/forwarders/week_blocked", w.total.blocked) });
    }
    int now = 0, before = 0;
    gate_tally::refusals(read_tally(), today, &now, &before);
    if (week.empty()) lines.push_back({ ui::Check::Info, "playguard/forwarders/week_empty"_i18n });
    auto* dialog = ui::box_dialog(ui::checklist_body("playguard/forwarders/week_title"_i18n, lines,
                                                       week.empty() ? "" : brls::getStr("playguard/forwarders/week_note", now, before),
                                                       "playguard/forwarders/week_sub"_i18n));
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    dialog->open();
}

bool has_week()
{
    return !gate_tally::week(read_tally(), ui::today_date()).empty();
}

std::string week_climb()
{
    int now = 0, before = 0;
    gate_tally::refusals(read_tally(), ui::today_date(), &now, &before);
    return gate_tally::rising(now, before) ? brls::getStr("playguard/forwarders/climb_detail", now, before) : "";
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
        tidy_gates();
    }
    return take_log();
}

std::vector<forwarders_logic::Row> to_remake()
{
    std::vector<forwarders_logic::Row> out;
    for (const auto& r : made_rows(false)) {
        forwarders::Made m;
        if (r.pin && r.state != forwarders_logic::State::NotInstalled && forwarders::find(r.id, &m) && !m.carried)
            out.push_back(r);
    }
    return out;
}

void remake(std::function<void(bool)> done)
{
    const std::vector<forwarders_logic::Row> todo = to_remake();
    if (todo.empty()) {
        if (done) done(false);
        return;
    }
    std::string names;
    for (const auto& r : todo) names += (names.empty() ? "" : ", ") + r.name;
    ui::confirm(brls::getStr("playguard/forwarders/remake_confirm", names), "playguard/forwarders/remake"_i18n,
                [todo, done]() {
                    install_many(todo, true, nullptr, "playguard/forwarders/remade_n", done);
                },
                [done]() {
                    if (done) done(false);
                });
}

}   // namespace forwarder_flow
