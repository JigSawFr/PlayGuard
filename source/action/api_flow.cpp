// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/api_flow.hpp"

#include <borealis.hpp>
#include <cstdio>
#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <sys/stat.h>

#include "action/clock_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/notices.hpp"
#include "activity/api_activity.hpp"
#include "activity/api_consent_activity.hpp"
#include "activity/api_settings_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "core/sim_env.h"
#include "core/time_ops.h"
#include "ui/format_text.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/date.hpp"
#include "util/nro.hpp"
#include "util/paths.hpp"
#include "util/sha256.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/text.hpp"

using namespace brls::literals;
using namespace api_logic;

namespace api_flow
{

namespace
{
constexpr const char* DEFAULT_RESPONSE = "sdmc:/config/playguard/api/response.json";

enum class State { None, Ready };
State       s_state = State::None;
Request     s_request;
Problem     s_problem;   // the whole request refused (400 …): no call is done
std::string s_lang;

std::string api_dir()       { return paths::data_dir() + "/api"; }
std::string handled_file()  { return api_dir() + "/handled.json"; }
std::string calls_file()    { return api_dir() + "/calls.json"; }
std::string trust_file()    { return api_dir() + "/trust.json"; }

// An "sdmc:/…" path as this build opens it: PlayGuard's own folder is
// paths::data_dir() (./playguard_data on the desktop), the rest of the card
// under paths::sd_root(). On the console without "sdmc:", the default
// device, as every other path of PlayGuard (paths::ensure_dir makes its
// folders from "/").
std::string local(const std::string& sdmc)
{
    const std::string own = "sdmc:/config/playguard/";
    if (sdmc.rfind(own, 0) == 0) return paths::data_dir() + "/" + sdmc.substr(own.size());
#ifdef __SWITCH__
    return sdmc.substr(5);
#else
    const std::string root = paths::sd_root();
    return root + (root.back() == '/' ? "" : "/") + sdmc.substr(6);
#endif
}

bool is_file(const std::string& path, off_t* size = nullptr)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) return false;
    if (size) *size = st.st_size;
    return true;
}

std::string iso_utc(uint64_t posix)
{
    const std::time_t t = (std::time_t)posix;
    std::tm tm{};
    char buf[32] = "";
    if (gmtime_r(&t, &tm)) std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

std::string now_iso()
{
    u64 posix = 0;
    LocalTime local_time;
    time_local_now(&posix, &local_time);
    return iso_utc(posix);
}

// The responses holding the child's play still on the card (a caller that
// did not delete its own): gone at the first start without a request.
void forget_child_data()
{
    std::string text;
    if (!paths::read_file(handled_file(), text)) return;
    auto list = parse_handled(text);
    bool changed = false;
    for (auto& h : list) {
        if (!h.child_data) continue;
        const std::string path = local(h.response_path.empty() ? DEFAULT_RESPONSE : h.response_path);
        if (is_file(path) && std::remove(path.c_str()) == 0)
            brls::Logger::info("api: removed {}, a response with the child's play", h.response_path);
        h.child_data = false;
        changed      = true;
    }
    if (changed && !paths::atomic_write(handled_file(), dump_handled(list)))
        brls::Logger::warning("api: {} not updated", handled_file());
}

// The request file: named by the caller, so only where a response could be
// (docs/api.md), read whole, then deleted before anything is done.
bool read_request(const std::string& sdmc)
{
    if (!response_path_ok(sdmc)) {
        s_problem = problem(400, "bad_request", "the request file must be a .json under sdmc:/switch/ or sdmc:/config/playguard/api/");
        return false;
    }
    const std::string path = local(sdmc);
    off_t size = 0;
    if (!is_file(path, &size)) {
        s_problem = problem(400, "bad_request", "no request file at " + sdmc);
        return false;
    }
    std::string text;
    const bool read = size <= (off_t)MAX_REQUEST_BYTES && paths::read_file(path, text);
    if (std::remove(path.c_str()) != 0) {
        s_problem = problem(500, "request_not_consumed", "the request file could not be deleted: it was not done");
        return false;
    }
    if (size > (off_t)MAX_REQUEST_BYTES) {
        s_problem = problem(400, "bad_request", "the request file is over 16 KiB");
        return false;
    }
    if (!read) {
        s_problem = problem(400, "bad_request", "the request file could not be read");
        return false;
    }
    return parse_file(text, &s_request, &s_problem);
}

// ---- Answering ------------------------------------------------------------

std::string error_title(const Problem& p)
{
    return brls::getStr("playguard/api/errors/" + p.code);
}

Answer fail(const Problem& p)
{
    return error_answer(p, error_title(p));
}

// "Set the network clock"; the route itself for one this build does not answer.
std::string call_name(const Call& c)
{
    const std::string key = route_key(c);
    return key.empty() ? route_text(c) : brls::getStr("playguard/api/routes/" + key);
}

// How far off the network clock is, as a parent says it: "5 min behind",
// "12 s ahead", "on time". `offset_s`: the server time minus the network
// clock (more than 0: the clock is behind).
std::string offset_text(int64_t offset_s)
{
    if (!clock_needs_write(offset_s)) return "playguard/api/on_time"_i18n;
    const uint64_t a = offset_s < 0 ? (uint64_t)(-(offset_s + 1)) + 1 : (uint64_t)offset_s;
    std::string span;
    if (a < 60) span = brls::getStr("playguard/api/span_s", std::to_string(a));
    else if (a < 3600) span = brls::getStr("playguard/api/span_min", std::to_string((a + 30) / 60));
    else if (a < 48 * 3600) {
        const uint64_t minutes = (a + 30) / 60;
        span = brls::getStr("playguard/api/span_h_min", std::to_string(minutes / 60), std::to_string(minutes % 60));
    }
    else span = brls::getStr("playguard/api/span_days", std::to_string(a / 86400));
    return brls::getStr(offset_s > 0 ? "playguard/api/behind" : "playguard/api/ahead", span);
}

using Done = std::function<void(const Answer& a, const std::string& line)>;

// GET /v1/clock: measured now, or the last measure with ?cached=1.
void get_clock(const Call& c, Done done)
{
    if (cached(c)) {
        clock_flow::LastMeasure lm;
        if (!clock_flow::last_measure(&lm)) {
            done(fail(problem(404, "no_measure", "PlayGuard never measured the network clock on this system")), "");
            return;
        }
        Answer a;
        a.body = { { "offset_s", lm.offset_s }, { "server", lm.server }, { "measured_at", iso_utc(lm.at_utc) },
                   { "cached", true } };
        done(a, brls::getStr("playguard/api/result/clock", offset_text(lm.offset_s)));
        return;
    }
    Problem p;
    const int timeout = timeout_ms(c, &p);
    if (p) {
        done(fail(p), "");
        return;
    }
    clock_flow::measure(clock_flow::current_server(), [done](const clock_flow::Measurement& m) {
        if (!m.ok) {
            done(fail(problem(503, "no_network", "no NTP server answered")), "");
            return;
        }
        if (!m.offset_known) {
            done(fail(problem(500, "failed", "the network clock could not be read")), "");
            return;
        }
        Answer a;
        a.body = { { "offset_s", m.offset_s }, { "server", m.server }, { "measured_at", iso_utc(m.unix_seconds) },
                   { "cached", false } };
        done(a, brls::getStr("playguard/api/result/clock", offset_text(m.offset_s)));
    }, timeout);
}

// Today's play timer, for what a change costs (the consent screen).
PlayToday play_today()
{
    PtState pt{};
    pctl_play_timer_query(&pt);
    PlayToday p;
    p.known = pt.fw_supported && pt.valid && pt.enabled_valid;
    if (!p.known) return p;
    const int today = ui::today_weekday();
    p.enabled     = pt.enabled;
    p.restricted  = pt.restricted_valid && pt.restricted;
    p.limit_min   = pt.day_min[today];
    p.remaining_s = pt.remaining_valid ? (int64_t)(pt.remaining_ns / 1000000000LL) : -1;
    return p;
}

// The caller's .nro, as it is on the card: what it says of itself, and its
// SHA-256 (an app the parent trusts is this file, not a name).
struct Caller
{
    bool read = false;
    nro::Info info;
    std::string sha;
};

Caller read_caller()
{
    Caller c;
    if (s_request.return_to.empty()) return c;
    const std::string path = local(s_request.return_to);
    if (!nro::read(path, c.info)) return c;
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return c;
    Sha256 h;
    std::vector<uint8_t> buf(64 * 1024);
    size_t n;
    while ((n = std::fread(buf.data(), 1, buf.size(), f)) > 0) h.update(buf.data(), n);
    const bool ok = !std::ferror(f);
    std::fclose(f);
    if (!ok) return c;
    c.sha  = h.hex();
    c.read = true;
    return c;
}

std::vector<Trust> load_trust()
{
    std::string text;
    return paths::read_file(trust_file(), text) ? parse_trust(text) : std::vector<Trust>{};
}

void save_trust(const std::vector<Trust>& list)
{
    if (!paths::ensure_dir(api_dir()) || !paths::atomic_write(trust_file(), dump_trust(list)))
        brls::Logger::warning("api: {} not written", trust_file());
}

// The write itself, then its answer. `trusted`: done without asking (an app
// the parent trusts), said in the answer, the log and the bell.
void write_sync(const clock_flow::Measurement& m, bool trusted, Done done)
{
    clock_flow::write(m, [m, trusted, done](const TimeApply& a, const std::string&) {
        if (a.refused_automatic) {
            done(fail(problem(409, "auto_correction_off", "System Settings > Synchronize clock via Internet is off")), "");
            return;
        }
        if (!a.write_attempted || R_FAILED(a.write_rc) || !a.verified) {
            const Result rc = !a.write_attempted ? a.open_rc : R_FAILED(a.write_rc) ? a.write_rc : a.verify_rc;
            char detail[96];
            std::snprintf(detail, sizeof(detail), "time:s refused the write (0x%08X)", (unsigned)rc);
            done(fail(problem(500, "failed", detail)), "");
            return;
        }
        Answer ok;
        ok.body = { { "offset_s_before", m.offset_known ? nlohmann::json(m.offset_s) : nlohmann::json(nullptr) },
                    { "offset_s_after", (int64_t)a.target - (int64_t)a.readback },
                    { "written", true },
                    { "trusted", trusted },
                    { "server", m.server } };
        done(ok, trusted ? "playguard/api/result/clock_set_trusted"_i18n : "playguard/api/result/clock_set"_i18n);
    });
}

// The parent said yes: the PIN, then (with "Allow for 30 days") the app
// trusted, then the write.
void approved_sync(const clock_flow::Measurement& m, const Caller& caller, bool trust, Done done)
{
    if (!pin_lock::ask()) {
        done(fail(problem(403, "pin_refused", "the PIN was not entered")), "");
        return;
    }
    if (trust && caller.read) {
        const std::string today = ui::today_date();
        Trust t;
        t.nro    = s_request.return_to;
        t.sha    = caller.sha;
        t.name   = caller.info.name.empty() ? s_request.caller : caller.info.name;
        t.action = "clock_sync";
        t.until  = date::add_days(today, TRUST_DAYS);
        auto list = load_trust();
        grant(list, t, today);
        save_trust(list);
        brls::Logger::info("api: {} trusted to set the clock until {}", t.nro, t.until);
    }
    write_sync(m, false, done);
}

// POST /v1/clock/sync: measured, left alone when right, else the PIN (the
// change starts the day's play time over) and the write.
void sync_clock(const Call& c, Done done)
{
    Problem p;
    const int timeout = timeout_ms(c, &p);
    if (p) {
        done(fail(p), "");
        return;
    }
    clock_flow::measure(clock_flow::current_server(), [done](const clock_flow::Measurement& m) {
        if (!m.ok) {
            done(fail(problem(503, "no_network", "no NTP server answered")), "");
            return;
        }
        if (m.offset_known && !clock_needs_write(m.offset_s)) {
            Answer a;
            a.body = { { "offset_s_before", m.offset_s }, { "offset_s_after", m.offset_s }, { "written", false },
                       { "server", m.server } };
            done(a, "playguard/api/result/clock_right"_i18n);
            return;
        }
        TimeSnapshot s;
        time_clock_snapshot(&s);
        if (R_SUCCEEDED(s.automatic_rc) && !s.automatic) {
            done(fail(problem(409, "auto_correction_off", "System Settings > Synchronize clock via Internet is off")), "");
            return;
        }
        const Caller caller = read_caller();
        // An app the parent trusts with this (its very file), the clock
        // really off and not already set by it today: no question.
        const std::string today = ui::today_date();
        auto list = load_trust();
        if (caller.read && m.offset_known)
            if (const Trust* t = trusted(list, s_request.return_to, caller.sha, "clock_sync", today);
                t && auto_sync_allowed(*t, m.offset_s, today)) {
                used_today(list, s_request.return_to, "clock_sync", today);
                save_trust(list);
                brls::Logger::info("api: {} is trusted to set the clock: done without asking", s_request.return_to);
                write_sync(m, true, done);
                return;
            }
        // Else the parent's yes before the PIN: who asks, what it changes,
        // what it costs.
        ApiConsentActivity::Consent consent;
        consent.declared  = s_request.caller;
        consent.nro_shown = s_request.return_to;
        consent.route     = "POST /v1/clock/sync";
        consent.can_trust = caller.read;
        if (caller.read) {
            consent.app_name = caller.info.name;
            consent.icon     = caller.info.icon;
            if (!caller.info.version.empty()) consent.app_meta = brls::getStr("playguard/api/consent/version", caller.info.version);
            if (!caller.info.author.empty())
                consent.app_meta += (consent.app_meta.empty() ? "" : " · ") +
                                    brls::getStr("playguard/api/consent/by", caller.info.author);
        }
        std::string who = consent.app_name.empty() ? s_request.caller : consent.app_name;
        consent.question = who.empty() ? "playguard/api/consent/question_unnamed"_i18n
                                       : brls::getStr("playguard/api/consent/question", who);
        if (m.offset_known) consent.text = brls::getStr("playguard/api/result/clock", offset_text(m.offset_s)) + " ";
        const Consequence effect = consequence(play_today());
        switch (effect.effect) {
            case Effect::Restarts:
                consent.text += brls::getStr("playguard/api/consent/restarts",
                                             ui::fmt_minutes((uint16_t)effect.left_before_min),
                                             ui::fmt_minutes((uint16_t)effect.left_after_min));
                break;
            case Effect::NoLimit: consent.text += "playguard/api/consent/no_limit"_i18n; break;
            case Effect::NoPlayDay: consent.text += "playguard/api/consent/no_play"_i18n; break;
            default: consent.text += "playguard/api/consent/unknown"_i18n; break;
        }
        brls::Application::pushActivity(new ApiConsentActivity(consent, [m, caller, done](bool yes, bool trust) {
            if (yes) approved_sync(m, caller, trust, done);
            else done(fail(problem(403, "declined", "the parent declined the change")), "");
        }), brls::TransitionAnimation::NONE);
    }, timeout);
}

void run_route(const Route& r, const Call& c, Done done)
{
    const std::string path = r.path;
    if (path.empty()) {
        Answer a;
        a.body = discovery(settings(), app::read_only(), app::version());
        done(a, "playguard/api/result/discovery"_i18n);
    } else if (path == "/clock") {
        get_clock(c, done);
    } else if (path == "/clock/sync") {
        sync_clock(c, done);
    } else {
        done(fail(problem(404, "no_route", "")), "");
    }
}

// One call: found, allowed, then done (or only checked, for a dry run).
void answer(const Call& c, Done done)
{
    Problem p;
    const Route* r = find(c, &p);
    if (!r) {
        done(fail(p), "");
        return;
    }
    p = permit(*r, settings(), app::read_only());
    if (p) {
        done(fail(p), "");
        return;
    }
    if (c.dry_run) {
        Answer a;
        a.body = { { "dry_run", true } };
        done(a, "playguard/api/result/dry_run"_i18n);
        return;
    }
    run_route(*r, c, done);
}

struct Run
{
    std::vector<Answer> answers;
    ApiActivity* screen = nullptr;
    bool failed = false;
};

void save_calls(const std::vector<Answer>& answers)
{
    std::string text;
    auto list = paths::read_file(calls_file(), text) ? parse_log(text) : std::vector<LogEntry>{};
    const std::string at = now_iso();
    for (size_t i = 0; i < s_request.calls.size(); i++) {
        LogEntry e;
        e.at      = at;
        e.caller  = s_request.caller;
        e.route   = route_text(s_request.calls[i]);
        e.status  = i < answers.size() ? answers[i].status : 0;
        e.dry_run = s_request.calls[i].dry_run;
        if (i < answers.size() && answers[i].status != 200) e.code = answers[i].body.value("code", std::string());
        if (i < answers.size()) e.trusted = answers[i].body.value("trusted", false);
        log_call(list, e);
    }
    if (s_request.calls.empty()) {
        LogEntry e;
        e.at     = at;
        e.caller = s_request.caller;
        e.route  = "?";
        e.status = s_problem.status;
        e.code   = s_problem.code;
        log_call(list, e);
    }
    if (!paths::ensure_dir(api_dir()) || !paths::atomic_write(calls_file(), dump_log(list)))
        brls::Logger::warning("api: {} not written", calls_file());
}

// The response written, the request remembered and logged, then the caller
// started again (or PlayGuard closed). Returns the status for the screen.
int finish(const std::vector<Answer>& answers, const nlohmann::json* replay)
{
    const nlohmann::json j = replay ? *replay : response(s_request, answers, app::version());
    const std::string sdmc = s_request.response.empty() ? DEFAULT_RESPONSE : s_request.response;
    std::string err;
    const std::string path = local(sdmc);
    const size_t slash = path.find_last_of('/');
    if (slash != std::string::npos) paths::ensure_dir(path.substr(0, slash));
    if (!paths::atomic_write(path, j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n", &err))
        brls::Logger::warning("api: the response was not written to {} ({})", sdmc, err);
    else
        brls::Logger::info("api: response written to {}", sdmc);

    // Kept once something was done: a refusal (the API off, no network)
    // answers again, and changes, when the caller asks again.
    bool done_something = false;
    for (size_t i = 0; i < answers.size() && i < s_request.calls.size(); i++)
        done_something |= answers[i].status == 200 && !s_request.calls[i].dry_run;
    if (!replay && !s_request.id.empty() && done_something && !s_problem) {
        std::string text;
        auto list = paths::read_file(handled_file(), text) ? parse_handled(text) : std::vector<Handled>{};
        Handled h;
        h.id            = s_request.id;
        h.at            = now_iso();
        h.response      = j;
        h.response_path = s_request.response;
        remember(list, h);
        if (!paths::ensure_dir(api_dir()) || !paths::atomic_write(handled_file(), dump_handled(list)))
            brls::Logger::warning("api: {} not written", handled_file());
    }
    if (!replay) save_calls(answers);

    const int status = replay ? j.value("status", 200) : s_problem ? s_problem.status : overall_status(s_request, answers);
    if (!s_request.return_to.empty()) {
        if (!is_file(local(s_request.return_to)))
            brls::Logger::warning("api: {} is not there, not started", s_request.return_to);
        else if (!platform_set_next_load_args(s_request.return_to.c_str(),
                                              relaunch_args(s_request.return_to, sdmc, status).c_str()))
            brls::Logger::warning("api: {} cannot be started from here", s_request.return_to);
        else
            brls::Logger::info("api: {} starts next", s_request.return_to);
    }
    return status;
}

// The calls one after the other (each may wait for the network or the PIN).
void step(std::shared_ptr<Run> run, size_t i)
{
    if (i >= s_request.calls.size()) {
        const int status = finish(run->answers, nullptr);
        (void)status;
        bool refused = false;
        for (const auto& a : run->answers) {
            const std::string code = a.body.value("code", std::string());
            refused |= code == "api_disabled" || code == "permission_denied";
        }
        run->screen->done(s_request.return_to, refused);
        return;
    }
    const Call& c = s_request.calls[i];
    // What it is doing, in words: the clock routes wait for time servers.
    const std::string key = route_key(c);
    run->screen->set_line(i, key == "clock" || key == "clock_sync"
                                 ? brls::getStr("playguard/api/measuring", clock_flow::current_server())
                                 : "playguard/api/working"_i18n);
    if (run->failed && s_request.stop_on_error) {
        const Answer a = fail(problem(424, "not_done", "an earlier request failed (stop_on_error)"));
        run->answers.push_back(a);
        run->screen->set_line(i, error_title(problem(424, "not_done")), false);
        brls::sync([run, i]() { step(run, i + 1); });
        return;
    }
    answer(c, [run, i](const Answer& a, const std::string& line) {
        run->answers.push_back(a);
        if (a.status != 200) run->failed = true;
        const std::string shown = a.status == 200 ? line : a.body.value("title", std::string());
        run->screen->set_line(i, shown, a.status == 200);
        brls::Logger::info("api: {} -> {}", route_text(s_request.calls[i]), a.status);
        brls::sync([run, i]() { step(run, i + 1); });
    });
}
}   // namespace

void at_start(int argc, const char* const* argv)
{
    Launch launch;
    std::string warning;
    bool found = from_argv(argc, argv, &launch, &warning);
    if (!warning.empty()) brls::Logger::warning("api: {}", warning);
#ifndef __SWITCH__
    // The simulators: PLAYGUARD_SIM_API is the short form (the web page's
    // ?api=), PLAYGUARD_SIM_API_PERMISSIONS ("clock:read_write,status:read")
    // turns the API on with those levels, for this run only.
    if (!found)
        if (const char* sim = sim_env("PLAYGUARD_SIM_API"); sim && *sim) {
            launch.short_form = sim;
            found             = true;
        }
    if (const char* perms = sim_env("PLAYGUARD_SIM_API_PERMISSIONS"); perms && *perms) {
        auto& cfg = config::get();
        cfg.api_enabled = true;
        std::string rest = perms;
        while (!rest.empty()) {
            const size_t comma = rest.find(',');
            const std::string pair = rest.substr(0, comma);
            rest = comma == std::string::npos ? "" : rest.substr(comma + 1);
            const size_t colon = pair.find(':');
            Level l;
            if (colon != std::string::npos && known_group(pair.substr(0, colon)) && parse_level(pair.substr(colon + 1), &l))
                cfg.api_permissions[pair.substr(0, colon)] = level_name(l);
        }
    }
#endif
    if (!found) {
        forget_child_data();
        return;
    }
    s_state = State::Ready;
    if (!launch.short_form.empty()) {
        brls::Logger::info("api: request \"{}\"", launch.short_form);
        parse_short(launch.short_form, &s_request, &s_problem);
    } else {
        brls::Logger::info("api: request file {}", launch.file);
        read_request(launch.file);
    }
    if (s_problem) brls::Logger::warning("api: request refused, {} {} ({})", s_problem.status, s_problem.code, s_problem.detail);
    s_lang = s_request.lang;
}

std::string lang()
{
    return s_lang;
}

bool pending()
{
    return s_state == State::Ready;
}

brls::Activity* activity()
{
    std::vector<ApiActivity::Row> rows;
    for (const auto& c : s_request.calls) rows.push_back({ call_name(c), route_text(c) });
    return new ApiActivity(s_request.caller, rows, [screen_lines = rows.size()](ApiActivity* screen) {
        // The same id again: its response again, nothing done twice.
        if (!s_problem && !s_request.id.empty()) {
            std::string text;
            const auto list = paths::read_file(handled_file(), text) ? parse_handled(text) : std::vector<Handled>{};
            if (const Handled* h = find_handled(list, s_request.id)) {
                brls::Logger::info("api: {} was already handled at {}: its response again", s_request.id, h->at);
                finish({}, &h->response);
                for (size_t i = 0; i < screen_lines; i++) screen->set_line(i, "playguard/api/result/replayed"_i18n, true);
                screen->done(s_request.return_to, false);
                return;
            }
        }
        if (s_problem) {
            finish({ fail(s_problem) }, nullptr);
            screen->set_message(error_title(s_problem));
            screen->done(s_request.return_to, false);
            return;
        }
        auto run    = std::make_shared<Run>();
        run->screen = screen;
        step(run, 0);
    });
}

void note_calls()
{
    u64 now = 0;
    LocalTime lt;
    time_local_now(&now, &lt);
    const std::string since = iso_utc(now > 7 * 86400 ? now - 7 * 86400 : 0);
    std::vector<std::pair<std::string, int>> changes;   // "who: what", how many times, first seen first
    std::map<std::string, int> refused;   // by caller
    int refused_total = 0;
    for (const auto& e : calls()) {
        if (e.at < since) continue;
        const std::string who = e.caller.empty() ? "playguard/api/unnamed"_i18n : e.caller;
        const std::string key = route_key(e.route);
        const std::string action = key.empty() ? e.route : brls::getStr("playguard/api/routes/" + key);
        if (e.status == 200 && !e.dry_run && e.route.rfind("POST ", 0) == 0) {
            const std::string line = brls::getStr(e.trusted ? "playguard/api/notice_line_trusted" : "playguard/api/notice_line",
                                                  who, action);
            bool seen = false;
            for (auto& c : changes)
                if (c.first == line) seen = ++c.second > 0;
            if (!seen) changes.push_back({ line, 1 });
        }
        if (e.code == "api_disabled" || e.code == "permission_denied") {
            refused[who]++;
            refused_total++;
        }
    }
    std::vector<std::string> change_lines;
    for (const auto& [line, n] : changes)
        change_lines.push_back(n == 1 ? line : brls::getStr("playguard/api/notice_line_many", line, n));
    if (changes.empty()) notices::clear("api_changes");
    else
        notices::set("api_changes", notices::Level::Info, "playguard/api/notice_changes"_i18n, text::join(change_lines, "\n"),
                     "playguard/api/notice_open"_i18n, []() { open_settings(); });
    if (refused_total < 3) {
        notices::clear("api_refused");
        return;
    }
    std::vector<std::string> lines;
    for (const auto& [who, n] : refused) lines.push_back(brls::getStr("playguard/api/notice_refused_line", who, n));
    notices::set("api_refused", notices::Level::Info, "playguard/api/notice_refused"_i18n, text::join(lines, "\n"),
                 "playguard/api/allow"_i18n, []() { open_settings(); });
}

void open_settings()
{
    if (!pin_lock::confirm()) {
        ui::notify(pin_lock::refusal_text(), ui::Toast::Refused);
        return;
    }
    brls::Application::pushActivity(new ApiSettingsActivity());
}

Settings settings()
{
    const auto& cfg = config::get();
    Settings s;
    s.enabled = cfg.api_enabled;
    for (const auto& [group, text] : cfg.api_permissions) {
        Level l;
        if (known_group(group) && parse_level(text, &l)) s.levels[group] = fit(group, l);
    }
    return s;
}

std::vector<LogEntry> calls()
{
    std::string text;
    return paths::read_file(calls_file(), text) ? parse_log(text) : std::vector<LogEntry>{};
}

void turn_all_off()
{
    auto& cfg = config::get();
    for (const char* g : GROUPS) cfg.api_permissions[g] = level_name(Level::None);
    ui::save_config();
}

std::vector<Trust> trusts()
{
    return load_trust();
}

void forget_trust(const Trust& t)
{
    auto list = load_trust();
    revoke(list, t.nro, t.action);
    save_trust(list);
}

bool clear_calls()
{
    const bool ok = std::remove(calls_file().c_str()) == 0 || !is_file(calls_file());
    if (ok) {
        notices::clear("api_changes");
        notices::clear("api_refused");
    }
    return ok;
}

std::string group_name(const std::string& group)
{
    return brls::getStr("playguard/api/groups/" + group);
}

std::string level_text(Level l)
{
    return brls::getStr(std::string("playguard/api/levels/") + level_name(l));
}

}   // namespace api_flow
