// api_logic — the UI-free part of the launch API (docs/api.md): the launch
// arguments, the request (short form or file), the routes this build answers
// and the permission each needs, the response and its errors, the relaunch
// arguments, and the two small files kept between launches (the requests
// already handled, the last calls for Preferences › Other homebrews).
// Plain C++ for the host tests (tests/api_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <borealis/extern/nlohmann/json.hpp>

namespace api_logic
{

constexpr const char* API_VERSION = "v1";
constexpr size_t MAX_REQUEST_BYTES = 16 * 1024;
constexpr size_t MAX_CALLS         = 8;     // requests in one file
constexpr size_t MAX_HANDLED       = 32;    // handled.json
constexpr size_t MAX_LOG           = 20;    // the calls Preferences › Other homebrews lists
constexpr int64_t CLOCK_SLACK_S    = 2;     // a clock this close is left alone
constexpr int TIMEOUT_MIN_MS = 1000, TIMEOUT_MAX_MS = 30000, TIMEOUT_DEFAULT_MS = 5000;

// ---- Permissions ---------------------------------------------------------

enum class Level { None, Read, ReadWrite };

// "none", "read", "read_write".
const char* level_name(Level l);
bool        parse_level(const std::string& s, Level* out);

// The groups of routes, in the settings' order.
constexpr const char* GROUPS[] = { "clock", "status", "activity", "games", "playtime" };
bool known_group(const std::string& g);

struct Settings
{
    bool enabled = false;
    std::map<std::string, Level> levels;   // a group not there: None
    Level level(const std::string& group) const;
};

// What a group may be set to: None, plus Read when it has a route that only
// reads, plus ReadWrite when it has one that writes (docs/api.md's table,
// planned routes included, so a setting survives the build that adds them).
std::vector<Level> levels_for(const std::string& group);
// `l` moved to the nearest level the group offers (a group with nothing to
// write set to ReadWrite reads; play time set to Read gets None).
Level fit(const std::string& group, Level l);

// ---- Routes --------------------------------------------------------------

struct Route
{
    const char* method;   // "GET", "POST", "OPEN"
    const char* path;     // without "/v1": "", "/clock", "/clock/sync"
    const char* group;    // nullptr: answers whatever the settings (discovery)
    bool        writes;
    bool        pin;      // the PIN whatever the lock setting (or, for the
                          // clock sync, whenever it has something to write)
};

// The routes this build answers.
const std::vector<Route>& routes();
// Groups with at least one route in this build (the settings screen shows these).
std::vector<std::string> groups_in_build();

// ---- Requests ------------------------------------------------------------

struct Call
{
    std::string mode = "call";   // "call" or "open"
    std::string method;          // "GET", "POST", "OPEN"
    std::string path;            // as given, "/v1" removed: "/clock/sync"
    std::map<std::string, std::string> query;
    nlohmann::json body = nlohmann::json::object();
    bool dry_run = false;
};

struct Request
{
    bool        short_form = false;
    bool        several    = false;   // a "requests" list
    std::string id;                   // "" for the short form
    std::string return_to;            // "sdmc:/switch/…/x.nro" or ""
    std::string response;             // where to write the response, or "" (the default)
    std::string caller;
    std::string lang;
    bool        stop_on_error = false;
    std::vector<Call> calls;
    std::vector<std::string> ignored;  // what was set aside, for the response ("return_to")
};

// An error: `status` 0 is none. `code` is the stable name (docs/api.md),
// `detail` English for the logs and the developer.
struct Problem
{
    int status = 0;
    std::string code, detail;
    explicit operator bool() const { return status != 0; }
};
Problem problem(int status, const char* code, const std::string& detail = "");

// The launch arguments: `--api "<METHOD> <path>"` or `--api-request <file>`.
// False when neither is there. A second one, or one without its value: the
// first is taken, the rest logged by the caller (`*warning`).
struct Launch
{
    std::string short_form;   // "POST /v1/clock/sync"
    std::string file;         // "sdmc:/switch/myapp/pg_request.json"
};
bool from_argv(int argc, const char* const* argv, Launch* out, std::string* warning);

bool parse_short(const std::string& text, Request* out, Problem* p);
// The request file's text (already read whole; larger than
// MAX_REQUEST_BYTES is refused by the caller before).
bool parse_file(const std::string& text, Request* out, Problem* p);

// The checks on paths the caller gives, on the text alone (the flow checks
// that return_to exists).
bool id_ok(const std::string& id);
bool return_to_ok(const std::string& sdmc);
bool response_path_ok(const std::string& sdmc);
// A language tag as the console names them ("fr", "en-US", "zh-Hant").
bool lang_ok(const std::string& lang);

// The route `c` asks for, or nullptr with 404 no_route (a planned or
// unknown path).
const Route* find(const Call& c, Problem* p);
// May `r` run? 403 api_disabled, 403 permission_denied (detail: the group
// and the level it needs), 423 read_only for a route that writes.
Problem permit(const Route& r, const Settings& s, bool read_only);

// `timeout_ms` from the body or the query: TIMEOUT_DEFAULT_MS when absent,
// 422 bad_body when not an integer in range.
int timeout_ms(const Call& c, Problem* p);
// `?cached=1`.
bool cached(const Call& c);

// Whether a measured offset is worth a write (and the PIN): more than CLOCK_SLACK_S.
bool clock_needs_write(int64_t offset_s);

// ---- Responses -----------------------------------------------------------

struct Answer
{
    int status = 200;
    nlohmann::json body = nlohmann::json::object();
};
// An error's body (RFC 9457): type, code, title (`title` already translated),
// detail.
Answer error_answer(const Problem& p, const std::string& title);

// The whole response file: one answer, or one per call (several).
nlohmann::json response(const Request& r, const std::vector<Answer>& answers, const std::string& version);
// What --playguard-status says: the one status, or the first that is not
// 200 among several (200 when all are).
int overall_status(const Request& r, const std::vector<Answer>& answers);
// The arguments `return_to` is started with (argv[0] first).
std::string relaunch_args(const std::string& return_to, const std::string& response_path, int status);

// GET /v1's body.
nlohmann::json discovery(const Settings& s, bool read_only, const std::string& version);

// ---- Kept between launches ----------------------------------------------

// handled.json: the requests already done, newest last.
struct Handled
{
    std::string id;
    std::string at;              // "2026-10-20T18:42:07Z"
    nlohmann::json response;     // written again for the same id
    std::string response_path;   // where it was written
    bool child_data = false;     // a response with the child's play: deleted at the next start
};
std::vector<Handled> parse_handled(const std::string& text);
std::string          dump_handled(const std::vector<Handled>& list);
// Adds or replaces `h` (by id), keeping the newest MAX_HANDLED.
void                 remember(std::vector<Handled>& list, const Handled& h);
const Handled*       find_handled(const std::vector<Handled>& list, const std::string& id);

// calls.json: the last calls, newest last.
struct LogEntry
{
    std::string at, caller, route;   // route: "POST /v1/clock/sync"
    int status = 0;
    bool dry_run = false;
};
std::vector<LogEntry> parse_log(const std::string& text);
std::string           dump_log(const std::vector<LogEntry>& list);
void                  log_call(std::vector<LogEntry>& list, const LogEntry& e);

// "POST /v1/clock/sync" for the log and the screen.
std::string route_text(const Call& c);

}   // namespace api_logic
