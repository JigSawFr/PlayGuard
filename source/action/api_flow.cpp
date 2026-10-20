// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/api_flow.hpp"

#include <borealis.hpp>
#include <cstdio>
#include <ctime>
#include <functional>
#include <memory>
#include <sys/stat.h>

#include "action/clock_flow.hpp"
#include "action/pin_lock.hpp"
#include "activity/api_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "core/time_ops.h"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"

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

std::string signed_seconds(int64_t s)
{
    return brls::getStr("playguard/clock/seconds", (s > 0 ? "+" : "") + std::to_string(s));
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
        done(a, brls::getStr("playguard/api/result/clock", signed_seconds(lm.offset_s)));
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
        done(a, brls::getStr("playguard/api/result/clock", signed_seconds(m.offset_s)));
    }, timeout);
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
        if (!pin_lock::ask()) {
            done(fail(problem(403, "pin_refused", "the PIN was not entered")), "");
            return;
        }
        clock_flow::write(m, [m, done](const TimeApply& a, const std::string&) {
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
                        { "server", m.server } };
            done(ok, "playguard/api/result/clock_set"_i18n);
        });
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
        log_call(list, e);
    }
    if (s_request.calls.empty()) {
        LogEntry e;
        e.at     = at;
        e.caller = s_request.caller;
        e.route  = "?";
        e.status = s_problem.status;
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
        run->screen->done(status == 200, s_request.return_to);
        return;
    }
    const Call& c = s_request.calls[i];
    run->screen->set_line(i, brls::getStr("playguard/api/working"));
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
    const bool found = from_argv(argc, argv, &launch, &warning);
    if (!warning.empty()) brls::Logger::warning("api: {}", warning);
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
    std::vector<std::string> lines;
    for (const auto& c : s_request.calls) lines.push_back(route_text(c));
    return new ApiActivity(s_request.caller, lines, [screen_lines = lines.size()](ApiActivity* screen) {
        // The same id again: its response again, nothing done twice.
        if (!s_problem && !s_request.id.empty()) {
            std::string text;
            const auto list = paths::read_file(handled_file(), text) ? parse_handled(text) : std::vector<Handled>{};
            if (const Handled* h = find_handled(list, s_request.id)) {
                brls::Logger::info("api: {} was already handled at {}: its response again", s_request.id, h->at);
                const int status = finish({}, &h->response);
                for (size_t i = 0; i < screen_lines; i++) screen->set_line(i, "playguard/api/result/replayed"_i18n);
                screen->done(status == 200, s_request.return_to);
                return;
            }
        }
        if (s_problem) {
            const int status = finish({ fail(s_problem) }, nullptr);
            screen->set_message(error_title(s_problem));
            screen->done(status == 200, s_request.return_to);
            return;
        }
        auto run    = std::make_shared<Run>();
        run->screen = screen;
        step(run, 0);
    });
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

std::string group_name(const std::string& group)
{
    return brls::getStr("playguard/api/groups/" + group);
}

std::string level_text(Level l)
{
    return brls::getStr(std::string("playguard/api/levels/") + level_name(l));
}

}   // namespace api_flow
