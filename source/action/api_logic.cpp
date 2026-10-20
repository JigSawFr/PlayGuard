// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/api_logic.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>

namespace api_logic
{

namespace
{
bool starts_with(const std::string& s, const char* prefix)
{
    return s.rfind(prefix, 0) == 0;
}

bool ends_with(const std::string& s, const char* suffix)
{
    const size_t n = std::char_traits<char>::length(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

std::string trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    const size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// At most `max` bytes, not cutting a UTF-8 character in two.
std::string cut_utf8(const std::string& s, size_t max)
{
    if (s.size() <= max) return s;
    size_t n = max;
    while (n > 0 && (static_cast<unsigned char>(s[n]) & 0xC0) == 0x80) n--;
    return s.substr(0, n);
}

// The planned routes too (docs/api.md): what each group can ever read or
// write, so a level set now is still right once the routes come.
struct GroupKinds
{
    const char* group;
    bool reads, writes;
};
constexpr GroupKinds KINDS[] = {
    { "clock", true, true },     { "status", true, false }, { "activity", true, false },
    { "games", true, false },    { "playtime", false, true },
};

const GroupKinds* kinds(const std::string& g)
{
    for (const auto& k : KINDS)
        if (g == k.group) return &k;
    return nullptr;
}

// An SD-card path a caller may name: "sdmc:/" and one of `roots`, then no
// "." or ".." segment, no backslash, no control character and no quote
// (it goes into the relaunch arguments), at most 512 bytes.
bool sd_path_ok(const std::string& p, std::initializer_list<const char*> roots, const char* suffix)
{
    if (p.size() > 512 || !ends_with(p, suffix)) return false;
    bool rooted = false;
    for (const char* r : roots) rooted |= starts_with(p, r) && p.size() > std::char_traits<char>::length(r) + std::char_traits<char>::length(suffix);
    if (!rooted) return false;
    for (unsigned char ch : p)
        if (ch < 0x20 || ch == 0x7F || ch == '\\' || ch == '"') return false;
    const std::string rest = p.substr(5);   // after "sdmc:"
    size_t i = 0;
    while (i <= rest.size()) {
        const size_t j = rest.find('/', i);
        const std::string seg = rest.substr(i, j == std::string::npos ? std::string::npos : j - i);
        if (seg == "." || seg == "..") return false;
        if (seg.empty() && i != 0) return false;   // "//"
        if (j == std::string::npos) break;
        i = j + 1;
    }
    return true;
}

// "/v1/clock/sync?x=1" into "/clock/sync" and its query; false for another
// API version ("/v2/…").
bool split_path(const std::string& full, std::string* path, std::map<std::string, std::string>* query)
{
    std::string p = full;
    const size_t q = p.find('?');
    if (q != std::string::npos) {
        std::string rest = p.substr(q + 1);
        p.resize(q);
        size_t i = 0;
        while (i <= rest.size()) {
            const size_t amp = rest.find('&', i);
            const std::string pair = rest.substr(i, amp == std::string::npos ? std::string::npos : amp - i);
            if (!pair.empty()) {
                const size_t eq = pair.find('=');
                if (eq == std::string::npos) (*query)[pair] = "";
                else (*query)[pair.substr(0, eq)] = pair.substr(eq + 1);
            }
            if (amp == std::string::npos) break;
            i = amp + 1;
        }
    }
    if (p == "/v1" || p == "/v1/") p = "";
    else if (starts_with(p, "/v1/")) p = p.substr(3);
    else if (p.size() >= 3 && p[0] == '/' && p[1] == 'v' && std::isdigit(static_cast<unsigned char>(p[2]))) return false;
    if (p == "/") p = "";
    while (p.size() > 1 && p.back() == '/') p.pop_back();
    *path = p;
    return true;
}

bool method_ok(const std::string& m)
{
    return m == "GET" || m == "POST" || m == "OPEN";
}

// One call from a JSON object (the file's top level, or an entry of "requests").
bool parse_call(const nlohmann::json& j, Call* c, Problem* p)
{
    auto m = j.find("method");
    auto path = j.find("path");
    if (m == j.end() || !m->is_string() || path == j.end() || !path->is_string()) {
        *p = problem(400, "bad_request", "\"method\" and \"path\" must be strings");
        return false;
    }
    c->method = m->get<std::string>();
    if (!method_ok(c->method)) {
        *p = problem(400, "bad_request", "\"method\" must be GET, POST or OPEN");
        return false;
    }
    const std::string full = path->get<std::string>();
    if (full.empty() || full[0] != '/') {
        *p = problem(400, "bad_request", "\"path\" must start with /");
        return false;
    }
    if (!split_path(full, &c->path, &c->query)) {
        *p = problem(400, "unsupported_api", "this PlayGuard answers v1 only");
        return false;
    }
    c->mode = c->method == "OPEN" ? "open" : "call";
    auto mode = j.find("mode");
    if (mode != j.end() && (!mode->is_string() || mode->get<std::string>() != c->mode)) {
        *p = problem(400, "bad_request", "\"mode\" is \"open\" with OPEN, \"call\" otherwise");
        return false;
    }
    auto body = j.find("body");
    if (body != j.end()) {
        if (!body->is_object()) {
            *p = problem(400, "bad_request", "\"body\" must be an object");
            return false;
        }
        c->body = *body;
    }
    auto dry = j.find("dry_run");
    if (dry != j.end()) {
        if (!dry->is_boolean()) {
            *p = problem(400, "bad_request", "\"dry_run\" must be true or false");
            return false;
        }
        c->dry_run = dry->get<bool>();
    }
    return true;
}

std::string string_at(const nlohmann::json& j, const char* key)
{
    auto it = j.find(key);
    return it != j.end() && it->is_string() ? it->get<std::string>() : std::string();
}
}   // namespace

// ---- Permissions ---------------------------------------------------------

const char* level_name(Level l)
{
    switch (l) {
        case Level::Read:      return "read";
        case Level::ReadWrite: return "read_write";
        default:               return "none";
    }
}

bool parse_level(const std::string& s, Level* out)
{
    if (s == "none") *out = Level::None;
    else if (s == "read") *out = Level::Read;
    else if (s == "read_write") *out = Level::ReadWrite;
    else return false;
    return true;
}

bool known_group(const std::string& g)
{
    return kinds(g) != nullptr;
}

Level Settings::level(const std::string& group) const
{
    auto it = levels.find(group);
    return it == levels.end() ? Level::None : it->second;
}

std::vector<Level> levels_for(const std::string& group)
{
    std::vector<Level> out = { Level::None };
    const GroupKinds* k = kinds(group);
    if (!k) return out;
    if (k->reads) out.push_back(Level::Read);
    if (k->writes) out.push_back(Level::ReadWrite);
    return out;
}

Level fit(const std::string& group, Level l)
{
    const GroupKinds* k = kinds(group);
    if (!k || l == Level::None) return Level::None;
    if (l == Level::ReadWrite && !k->writes) return k->reads ? Level::Read : Level::None;
    if (l == Level::Read && !k->reads) return Level::None;
    return l;
}

// ---- Routes --------------------------------------------------------------

const std::vector<Route>& routes()
{
    static const std::vector<Route> list = {
        { "GET", "", nullptr, false, false },
        { "GET", "/clock", "clock", false, false },
        { "POST", "/clock/sync", "clock", true, true },
    };
    return list;
}

std::vector<std::string> groups_in_build()
{
    std::vector<std::string> out;
    for (const char* g : GROUPS)
        for (const auto& r : routes())
            if (r.group && g == std::string(r.group)) {
                out.push_back(g);
                break;
            }
    return out;
}

// ---- Requests ------------------------------------------------------------

Problem problem(int status, const char* code, const std::string& detail)
{
    Problem p;
    p.status = status;
    p.code   = code;
    p.detail = detail;
    return p;
}

bool from_argv(int argc, const char* const* argv, Launch* out, std::string* warning)
{
    *out = Launch{};
    warning->clear();
    bool found = false;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i] ? argv[i] : "";
        const bool is_short = a == "--api", is_file = a == "--api-request";
        if (!is_short && !is_file) continue;
        if (i + 1 >= argc || !argv[i + 1]) {
            *warning = a + " without its value";
            continue;
        }
        const std::string value = argv[++i];
        if (found) {
            *warning = "a second " + a + " was ignored";
            continue;
        }
        found = true;
        if (is_short) out->short_form = value;
        else out->file = value;
    }
    return found;
}

bool parse_short(const std::string& text, Request* out, Problem* p)
{
    *out = Request{};
    out->short_form = true;
    const std::string t = trim(text);
    const size_t sp = t.find(' ');
    if (sp == std::string::npos) {
        *p = problem(400, "bad_request", "the short form is \"<METHOD> <path>\"");
        return false;
    }
    nlohmann::json j = { { "method", t.substr(0, sp) }, { "path", trim(t.substr(sp + 1)) } };
    Call c;
    if (!parse_call(j, &c, p)) return false;
    out->calls.push_back(c);
    return true;
}

bool parse_file(const std::string& text, Request* out, Problem* p)
{
    *out = Request{};
    const nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        *p = problem(400, "bad_request", "the request is not a JSON object");
        return false;
    }
    auto api = j.find("api");
    if (api == j.end() || !api->is_string()) {
        *p = problem(400, "bad_request", "\"api\" is missing");
        return false;
    }
    if (api->get<std::string>() != API_VERSION) {
        *p = problem(400, "unsupported_api", "this PlayGuard answers v1 only");
        return false;
    }
    auto id = j.find("id");
    if (id == j.end() || !id->is_string() || !id_ok(id->get<std::string>())) {
        *p = problem(400, "bad_request", "\"id\" must be 1 to 64 of A-Z a-z 0-9 _ -");
        return false;
    }
    out->id = id->get<std::string>();

    // What the caller names but may not have: set aside, said in the response.
    if (j.contains("return_to")) {
        const std::string r = string_at(j, "return_to");
        if (return_to_ok(r)) out->return_to = r;
        else out->ignored.push_back("return_to");
    }
    if (j.contains("response")) {
        const std::string r = string_at(j, "response");
        if (response_path_ok(r)) out->response = r;
        else out->ignored.push_back("response");
    }
    if (j.contains("lang")) {
        const std::string l = string_at(j, "lang");
        if (lang_ok(l)) out->lang = l;
        else out->ignored.push_back("lang");
    }
    out->caller = cut_utf8(string_at(j, "caller"), 64);
    auto stop = j.find("stop_on_error");
    if (stop != j.end()) {
        if (!stop->is_boolean()) {
            *p = problem(400, "bad_request", "\"stop_on_error\" must be true or false");
            return false;
        }
        out->stop_on_error = stop->get<bool>();
    }

    auto list = j.find("requests");
    if (list == j.end()) {
        Call c;
        if (!parse_call(j, &c, p)) return false;
        out->calls.push_back(c);
        return true;
    }
    for (const char* k : { "method", "path", "mode", "body", "dry_run" })
        if (j.contains(k)) {
            *p = problem(400, "bad_request", std::string("\"") + k + "\" goes in each entry of \"requests\"");
            return false;
        }
    if (!list->is_array() || list->empty() || list->size() > MAX_CALLS) {
        *p = problem(400, "bad_request", "\"requests\" must hold 1 to 8 requests");
        return false;
    }
    out->several = true;
    for (size_t i = 0; i < list->size(); i++) {
        const auto& e = (*list)[i];
        if (!e.is_object()) {
            *p = problem(400, "bad_request", "each entry of \"requests\" must be an object");
            return false;
        }
        Call c;
        if (!parse_call(e, &c, p)) {
            p->detail = "requests[" + std::to_string(i) + "]: " + p->detail;
            return false;
        }
        if (c.mode == "open" && i + 1 != list->size()) {
            *p = problem(400, "bad_request", "only the last request may be OPEN");
            return false;
        }
        out->calls.push_back(c);
    }
    return true;
}

bool id_ok(const std::string& id)
{
    if (id.empty() || id.size() > 64) return false;
    for (unsigned char ch : id)
        if (!std::isalnum(ch) && ch != '_' && ch != '-') return false;
    return true;
}

bool return_to_ok(const std::string& sdmc)
{
    return sd_path_ok(sdmc, { "sdmc:/switch/" }, ".nro");
}

bool response_path_ok(const std::string& sdmc)
{
    // Not PlayGuard's own files (config.json, state.json …): its api folder only.
    return sd_path_ok(sdmc, { "sdmc:/switch/", "sdmc:/config/playguard/api/" }, ".json");
}

bool lang_ok(const std::string& lang)
{
    if (lang.size() < 2 || lang.size() > 10 || !std::isalpha(static_cast<unsigned char>(lang[0]))) return false;
    for (unsigned char ch : lang)
        if (!std::isalpha(ch) && ch != '-') return false;
    return true;
}

const Route* find(const Call& c, Problem* p)
{
    for (const auto& r : routes())
        if (c.method == r.method && c.path == r.path) return &r;
    *p = problem(404, "no_route", "no " + route_text(c) + " in this build (GET /v1 lists them)");
    return nullptr;
}

Problem permit(const Route& r, const Settings& s, bool read_only)
{
    if (!r.group) return {};
    if (!s.enabled) return problem(403, "api_disabled", "Preferences > Other homebrews is off");
    const Level need = r.writes ? Level::ReadWrite : Level::Read;
    if (static_cast<int>(s.level(r.group)) < static_cast<int>(need))
        return problem(403, "permission_denied", std::string("the group \"") + r.group + "\" needs " + level_name(need) +
                                                     ", it is " + level_name(s.level(r.group)));
    if (r.writes && read_only) return problem(423, "read_only", "this firmware is open read only");
    return {};
}

int timeout_ms(const Call& c, Problem* p)
{
    long long v = TIMEOUT_DEFAULT_MS;
    auto b = c.body.find("timeout_ms");
    auto q = c.query.find("timeout_ms");
    if (b != c.body.end()) {
        if (!b->is_number_integer()) {
            *p = problem(422, "bad_body", "\"timeout_ms\" must be an integer");
            return 0;
        }
        v = b->is_number_unsigned() && b->get<unsigned long long>() > 1000000ULL ? 1000001LL : b->get<long long>();
    } else if (q != c.query.end()) {
        const std::string& s = q->second;
        if (s.empty() || s.size() > 7 || !std::all_of(s.begin(), s.end(), [](unsigned char ch) { return std::isdigit(ch); })) {
            *p = problem(422, "bad_body", "\"timeout_ms\" must be an integer");
            return 0;
        }
        v = std::stoll(s);
    }
    if (v < TIMEOUT_MIN_MS || v > TIMEOUT_MAX_MS) {
        *p = problem(422, "bad_body", "\"timeout_ms\" must be 1000 to 30000");
        return 0;
    }
    return static_cast<int>(v);
}

bool cached(const Call& c)
{
    auto q = c.query.find("cached");
    return q != c.query.end() && (q->second == "1" || q->second == "true");
}

bool clock_needs_write(int64_t offset_s)
{
    return offset_s > CLOCK_SLACK_S || offset_s < -CLOCK_SLACK_S;
}

// ---- Responses -----------------------------------------------------------

Answer error_answer(const Problem& p, const std::string& title)
{
    Answer a;
    a.status = p.status;
    a.body   = { { "type", "https://playguard.link/api/errors/" + p.code },
                 { "code", p.code },
                 { "title", title },
                 { "detail", p.detail } };
    return a;
}

nlohmann::json response(const Request& r, const std::vector<Answer>& answers, const std::string& version)
{
    nlohmann::json j;
    j["api"] = API_VERSION;
    if (!r.id.empty()) j["id"] = r.id;
    if (r.several) {
        j["status"] = 200;
        nlohmann::json list = nlohmann::json::array();
        for (const auto& a : answers) list.push_back({ { "status", a.status }, { "body", a.body } });
        j["responses"] = list;
    } else {
        const Answer a = answers.empty() ? Answer{} : answers.front();
        j["status"] = a.status;
        j["body"]   = a.body;
    }
    if (!r.ignored.empty()) j["ignored"] = r.ignored;
    j["playguard"] = version;
    return j;
}

int overall_status(const Request& r, const std::vector<Answer>& answers)
{
    if (!r.several) return answers.empty() ? 200 : answers.front().status;
    for (const auto& a : answers)
        if (a.status != 200) return a.status;
    return 200;
}

std::string relaunch_args(const std::string& return_to, const std::string& response_path, int status)
{
    const auto quoted = [](const std::string& s) { return s.find(' ') == std::string::npos ? s : "\"" + s + "\""; };
    return quoted(return_to) + " --playguard-response " + quoted(response_path) + " --playguard-status " +
           std::to_string(status);
}

nlohmann::json discovery(const Settings& s, bool read_only, const std::string& version)
{
    nlohmann::json j;
    j["playguard"] = version;
    j["api"]       = nlohmann::json::array({ API_VERSION });
    j["enabled"]   = s.enabled;
    j["read_only"] = read_only;
    nlohmann::json groups = nlohmann::json::object();
    nlohmann::json list   = nlohmann::json::array();
    if (s.enabled) {
        for (const auto& g : groups_in_build()) groups[g] = level_name(s.level(g));
        for (const auto& r : routes()) {
            if (!r.group) continue;
            const Level need = r.writes ? Level::ReadWrite : Level::Read;
            if (static_cast<int>(s.level(r.group)) >= static_cast<int>(need))
                list.push_back(std::string(r.method) + " /v1" + r.path);
        }
    }
    j["groups"] = groups;
    j["routes"] = list;
    return j;
}

// ---- Kept between launches ----------------------------------------------

std::vector<Handled> parse_handled(const std::string& text)
{
    std::vector<Handled> out;
    const nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return out;
    auto list = j.find("handled");
    if (list == j.end() || !list->is_array()) return out;
    for (const auto& e : *list) {
        if (!e.is_object()) continue;
        Handled h;
        h.id = string_at(e, "id");
        if (!id_ok(h.id)) continue;
        h.at            = string_at(e, "at");
        h.response_path = string_at(e, "response_path");
        auto r = e.find("response");
        if (r != e.end() && r->is_object()) h.response = *r;
        auto c = e.find("child_data");
        h.child_data = c != e.end() && c->is_boolean() && c->get<bool>();
        out.push_back(h);
    }
    if (out.size() > MAX_HANDLED) out.erase(out.begin(), out.end() - MAX_HANDLED);
    return out;
}

std::string dump_handled(const std::vector<Handled>& list)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& h : list)
        arr.push_back({ { "id", h.id }, { "at", h.at }, { "response", h.response },
                        { "response_path", h.response_path }, { "child_data", h.child_data } });
    nlohmann::json j = { { "schema", 1 }, { "handled", arr } };
    return j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
}

void remember(std::vector<Handled>& list, const Handled& h)
{
    list.erase(std::remove_if(list.begin(), list.end(), [&](const Handled& x) { return x.id == h.id; }), list.end());
    list.push_back(h);
    if (list.size() > MAX_HANDLED) list.erase(list.begin(), list.end() - MAX_HANDLED);
}

const Handled* find_handled(const std::vector<Handled>& list, const std::string& id)
{
    for (const auto& h : list)
        if (h.id == id) return &h;
    return nullptr;
}

std::vector<LogEntry> parse_log(const std::string& text)
{
    std::vector<LogEntry> out;
    const nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return out;
    auto list = j.find("calls");
    if (list == j.end() || !list->is_array()) return out;
    for (const auto& e : *list) {
        if (!e.is_object()) continue;
        LogEntry l;
        l.at     = cut_utf8(string_at(e, "at"), 32);
        l.caller = cut_utf8(string_at(e, "caller"), 64);
        l.route  = cut_utf8(string_at(e, "route"), 128);
        auto s = e.find("status");
        if (s == e.end() || !s->is_number_integer()) continue;
        const long long st = s->get<long long>();
        if (st < 100 || st > 599) continue;
        l.status = static_cast<int>(st);
        auto d = e.find("dry_run");
        l.dry_run = d != e.end() && d->is_boolean() && d->get<bool>();
        out.push_back(l);
    }
    if (out.size() > MAX_LOG) out.erase(out.begin(), out.end() - MAX_LOG);
    return out;
}

std::string dump_log(const std::vector<LogEntry>& list)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& l : list)
        arr.push_back({ { "at", l.at }, { "caller", l.caller }, { "route", l.route }, { "status", l.status },
                        { "dry_run", l.dry_run } });
    nlohmann::json j = { { "schema", 1 }, { "calls", arr } };
    return j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
}

void log_call(std::vector<LogEntry>& list, const LogEntry& e)
{
    list.push_back(e);
    if (list.size() > MAX_LOG) list.erase(list.begin(), list.end() - MAX_LOG);
}

std::string route_text(const Call& c)
{
    return c.method + " /v1" + c.path;
}

}   // namespace api_logic
