// Host tests for source/action/api_logic.cpp: the launch arguments, the
// request (short form and file), the paths a caller may name, the routes and
// their permissions, the response and the relaunch arguments, and the files
// kept between launches (docs/api.md).
#include "check.h"

#include "action/api_logic.hpp"

using namespace api_logic;

static void test_argv()
{
    Launch l;
    std::string warn;
    const char* none[] = { "sdmc:/switch/playguard/playguard.nro" };
    CHECK(!from_argv(1, none, &l, &warn));

    const char* shortf[] = { "x.nro", "--api", "POST /v1/clock/sync" };
    CHECK(from_argv(3, shortf, &l, &warn));
    CHECK(l.short_form == "POST /v1/clock/sync" && l.file.empty() && warn.empty());

    const char* file[] = { "x.nro", "--api-request", "sdmc:/switch/a/r.json" };
    CHECK(from_argv(3, file, &l, &warn));
    CHECK(l.file == "sdmc:/switch/a/r.json" && l.short_form.empty());

    // The first one counts; the second is said.
    const char* both[] = { "x.nro", "--api", "GET /v1", "--api-request", "sdmc:/switch/a/r.json" };
    CHECK(from_argv(5, both, &l, &warn));
    CHECK(l.short_form == "GET /v1" && l.file.empty() && !warn.empty());

    const char* missing[] = { "x.nro", "--api" };
    CHECK(!from_argv(2, missing, &l, &warn));
    CHECK(!warn.empty());
}

static void test_short()
{
    Request r;
    Problem p;
    CHECK(parse_short("POST /v1/clock/sync", &r, &p));
    CHECK(r.short_form && r.calls.size() == 1 && r.id.empty() && r.return_to.empty());
    CHECK(r.calls[0].method == "POST" && r.calls[0].path == "/clock/sync" && r.calls[0].mode == "call");

    CHECK(parse_short("  GET /v1  ", &r, &p));
    CHECK(r.calls[0].path.empty());
    CHECK(parse_short("GET /v1/", &r, &p) && r.calls[0].path.empty());
    CHECK(parse_short("GET /clock", &r, &p) && r.calls[0].path == "/clock");   // "/v1" may be left out

    CHECK(parse_short("GET /v1/clock?cached=1&timeout_ms=2000", &r, &p));
    CHECK(r.calls[0].path == "/clock" && cached(r.calls[0]));
    CHECK(timeout_ms(r.calls[0], &p) == 2000 && !p);

    CHECK(parse_short("OPEN /v1/games/0100000000010000", &r, &p));
    CHECK(r.calls[0].mode == "open");

    p = {};
    CHECK(!parse_short("POST", &r, &p) && p.status == 400 && p.code == "bad_request");
    CHECK(!parse_short("DELETE /v1/clock", &r, &p) && p.status == 400);
    CHECK(!parse_short("post /v1/clock/sync", &r, &p) && p.status == 400);   // methods in capitals
    CHECK(!parse_short("GET clock", &r, &p) && p.status == 400);
    CHECK(!parse_short("GET /v2/clock", &r, &p) && p.code == "unsupported_api");
}

static void test_file()
{
    Request r;
    Problem p;
    CHECK(parse_file(R"({"api":"v1","id":"8f14e45f","method":"POST","path":"/clock/sync",
        "return_to":"sdmc:/switch/myapp/myapp.nro","response":"sdmc:/switch/myapp/r.json",
        "caller":"MyApp 1.2","lang":"fr","body":{"timeout_ms":3000},"dry_run":true})", &r, &p));
    CHECK(!r.short_form && !r.several && r.id == "8f14e45f");
    CHECK(r.return_to == "sdmc:/switch/myapp/myapp.nro" && r.response == "sdmc:/switch/myapp/r.json");
    CHECK(r.caller == "MyApp 1.2" && r.lang == "fr" && r.ignored.empty());
    CHECK(r.calls.size() == 1 && r.calls[0].dry_run && r.calls[0].path == "/clock/sync");
    CHECK(timeout_ms(r.calls[0], &p) == 3000);

    // Paths and names it may not have: set aside and said, the request still done.
    CHECK(parse_file(R"({"api":"v1","id":"a","method":"GET","path":"/v1",
        "return_to":"sdmc:/atmosphere/x.nro","response":"sdmc:/config/playguard/config.json","lang":"<b>"})", &r, &p));
    CHECK(r.return_to.empty() && r.response.empty() && r.lang.empty());
    CHECK(r.ignored.size() == 3);

    // The caller's name is cut, not refused, and not cut inside a character.
    CHECK(parse_file(std::string(R"({"api":"v1","id":"a","method":"GET","path":"/v1","caller":")") +
                         std::string(63, 'x') + "\xC3\xA9" + R"("})", &r, &p));
    CHECK(r.caller.size() == 63);

    p = {};
    CHECK(!parse_file("not json", &r, &p) && p.status == 400);
    CHECK(!parse_file("[]", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"id":"a","method":"GET","path":"/v1"})", &r, &p) && p.code == "bad_request");
    CHECK(!parse_file(R"({"api":"v2","id":"a","method":"GET","path":"/v1"})", &r, &p) && p.code == "unsupported_api");
    CHECK(!parse_file(R"({"api":"v1","method":"GET","path":"/v1"})", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"api":"v1","id":"a b","method":"GET","path":"/v1"})", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"api":"v1","id":"a","method":"GET","path":"/v1","body":[]})", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"api":"v1","id":"a","method":"GET","path":"/v1","dry_run":1})", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"api":"v1","id":"a","method":"GET","path":"/v1","mode":"open"})", &r, &p) && p.status == 400);
    CHECK(!parse_file(R"({"api":"v1","id":"a","method":"GET"})", &r, &p) && p.status == 400);
}

static void test_several()
{
    Request r;
    Problem p;
    CHECK(parse_file(R"({"api":"v1","id":"c4","stop_on_error":true,"requests":[
        {"method":"POST","path":"/clock/sync"},{"method":"GET","path":"/v1/clock","dry_run":true}]})", &r, &p));
    CHECK(r.several && r.stop_on_error && r.calls.size() == 2);
    CHECK(r.calls[1].dry_run && r.calls[1].path == "/clock");

    p = {};
    CHECK(!parse_file(R"({"api":"v1","id":"c4","requests":[]})", &r, &p) && p.status == 400);
    std::string nine = R"({"api":"v1","id":"c4","requests":[)";
    for (int i = 0; i < 9; i++) nine += std::string(i ? "," : "") + R"({"method":"GET","path":"/v1"})";
    CHECK(!parse_file(nine + "]}", &r, &p) && p.status == 400);
    // Only the last may open a screen.
    CHECK(!parse_file(R"({"api":"v1","id":"c4","requests":[{"method":"OPEN","path":"/clock"},{"method":"GET","path":"/v1"}]})", &r, &p));
    CHECK(parse_file(R"({"api":"v1","id":"c4","requests":[{"method":"GET","path":"/v1"},{"method":"OPEN","path":"/clock"}]})", &r, &p));
    // A call's keys at the top with a list: refused, not half read.
    CHECK(!parse_file(R"({"api":"v1","id":"c4","method":"GET","requests":[{"method":"GET","path":"/v1"}]})", &r, &p));
    CHECK(!parse_file(R"({"api":"v1","id":"c4","requests":[{"method":"GET"}]})", &r, &p));
    CHECK(p.detail.find("requests[0]") == 0);
}

static void test_paths()
{
    CHECK(return_to_ok("sdmc:/switch/myapp/myapp.nro"));
    CHECK(return_to_ok("sdmc:/switch/my app/my app.nro"));
    CHECK(!return_to_ok("sdmc:/switch/.nro"));
    CHECK(!return_to_ok("sdmc:/atmosphere/x.nro"));
    CHECK(!return_to_ok("/switch/myapp/myapp.nro"));
    CHECK(!return_to_ok("sdmc:/switch/../atmosphere/x.nro"));
    CHECK(!return_to_ok("sdmc:/switch/./x.nro"));
    CHECK(!return_to_ok("sdmc:/switch//x.nro"));
    CHECK(!return_to_ok("sdmc:/switch/x.nro.txt"));
    CHECK(!return_to_ok("sdmc:/switch/a\"b.nro"));
    CHECK(!return_to_ok("sdmc:/switch/a\\b.nro"));
    CHECK(!return_to_ok("sdmc:/switch/a\nb.nro"));
    CHECK(!return_to_ok("sdmc:/switch/" + std::string(600, 'a') + ".nro"));

    CHECK(response_path_ok("sdmc:/switch/myapp/r.json"));
    CHECK(response_path_ok("sdmc:/config/playguard/api/mine.json"));
    CHECK(!response_path_ok("sdmc:/config/playguard/config.json"));
    CHECK(!response_path_ok("sdmc:/config/playguard/nand/x/state.json"));
    CHECK(!response_path_ok("sdmc:/config/other/settings.json"));
    CHECK(!response_path_ok("sdmc:/config/playguard/api/../config.json"));
    CHECK(!response_path_ok("sdmc:/switch/myapp/r.txt"));

    CHECK(id_ok("a") && id_ok("A-z_09") && id_ok(std::string(64, 'x')));
    CHECK(!id_ok("") && !id_ok(std::string(65, 'x')) && !id_ok("a.b") && !id_ok("é"));

    CHECK(lang_ok("fr") && lang_ok("en-US") && lang_ok("zh-Hant"));
    CHECK(!lang_ok("f") && !lang_ok("-fr") && !lang_ok("fr_FR") && !lang_ok("abcdefghijk"));
}

static void test_routes()
{
    Request r;
    Problem p;
    CHECK(parse_short("GET /v1", &r, &p));
    const Route* disc = find(r.calls[0], &p);
    CHECK(disc && disc->group == nullptr);
    CHECK(parse_short("GET /v1/clock", &r, &p));
    const Route* clock = find(r.calls[0], &p);
    CHECK(clock && !clock->writes);
    CHECK(parse_short("POST /v1/clock/sync", &r, &p));
    const Route* sync = find(r.calls[0], &p);
    CHECK(sync && sync->writes && sync->pin);

    // Planned or unknown: 404, the route named.
    for (const char* t : { "GET /v1/status", "OPEN /v1/clock", "GET /v1/clock/sync", "POST /v1/clock" }) {
        p = {};
        CHECK(parse_short(t, &r, &p));
        CHECK(!find(r.calls[0], &p) && p.status == 404 && p.code == "no_route");
    }

    Settings s;
    CHECK(!permit(*disc, s, false));   // discovery answers while off
    CHECK(permit(*clock, s, false).code == "api_disabled");
    s.enabled = true;
    CHECK(permit(*clock, s, false).code == "permission_denied");
    s.levels["clock"] = Level::Read;
    CHECK(!permit(*clock, s, false));
    CHECK(!permit(*clock, s, true));   // reading in read-only mode
    const Problem w = permit(*sync, s, false);
    CHECK(w.status == 403 && w.code == "permission_denied" && w.detail.find("read_write") != std::string::npos);
    s.levels["clock"] = Level::ReadWrite;
    CHECK(!permit(*sync, s, false));
    CHECK(permit(*sync, s, true).status == 423);
    s.enabled = false;
    CHECK(permit(*sync, s, false).code == "api_disabled");

    CHECK(groups_in_build() == std::vector<std::string>{ "clock" });
}

static void test_levels()
{
    Level l;
    CHECK(parse_level("none", &l) && l == Level::None);
    CHECK(parse_level("read", &l) && l == Level::Read);
    CHECK(parse_level("read_write", &l) && l == Level::ReadWrite);
    CHECK(!parse_level("write", &l) && !parse_level("", &l));
    CHECK(std::string(level_name(Level::ReadWrite)) == "read_write");

    CHECK(levels_for("clock").size() == 3);
    CHECK(levels_for("status").size() == 2);                  // nothing to write
    CHECK(levels_for("playtime").size() == 2);                // nothing to read
    CHECK(levels_for("playtime")[1] == Level::ReadWrite);
    CHECK(levels_for("nope").size() == 1);

    CHECK(fit("status", Level::ReadWrite) == Level::Read);
    CHECK(fit("playtime", Level::Read) == Level::None);
    CHECK(fit("clock", Level::ReadWrite) == Level::ReadWrite);
    CHECK(fit("nope", Level::Read) == Level::None);
    CHECK(known_group("games") && !known_group("nope"));
}

static void test_timeout_and_clock()
{
    Call c;
    Problem p;
    CHECK(timeout_ms(c, &p) == TIMEOUT_DEFAULT_MS && !p);
    c.body = { { "timeout_ms", 999 } };
    CHECK(timeout_ms(c, &p) == 0 && p.status == 422);
    p = {};
    c.body = { { "timeout_ms", 30000 } };
    CHECK(timeout_ms(c, &p) == 30000 && !p);
    c.body = { { "timeout_ms", 2.5 } };
    CHECK(timeout_ms(c, &p) == 0 && p.code == "bad_body");
    p = {};
    c.body = { { "timeout_ms", 18446744073709551615ULL } };
    CHECK(timeout_ms(c, &p) == 0 && p.status == 422);
    p = {};
    c.body  = nlohmann::json::object();
    c.query = { { "timeout_ms", "abc" } };
    CHECK(timeout_ms(c, &p) == 0 && p.status == 422);

    CHECK(!clock_needs_write(0) && !clock_needs_write(2) && !clock_needs_write(-2));
    CHECK(clock_needs_write(3) && clock_needs_write(-183));
}

static void test_consequence()
{
    PlayToday p;
    CHECK(consequence(p).effect == Effect::Unknown);
    p.known = true;
    CHECK(consequence(p).effect == Effect::NoLimit);   // the timer off
    p.enabled   = true;
    p.limit_min = PlayToday::NO_LIMIT;
    CHECK(consequence(p).effect == Effect::NoLimit);
    p.limit_min = 0;
    CHECK(consequence(p).effect == Effect::NoPlayDay);
    p.limit_min = 180;
    CHECK(consequence(p).effect == Effect::Unknown);   // what is left unknown
    p.remaining_s = 80 * 60 + 30;
    Consequence c = consequence(p);
    CHECK(c.effect == Effect::Restarts && c.left_before_min == 80 && c.left_after_min == 180);
    CHECK(c.used_before > 0.55f && c.used_before < 0.56f);
    p.restricted = true;   // reached: nothing left, the whole limit after
    c = consequence(p);
    CHECK(c.left_before_min == 0 && c.left_after_min == 180 && c.used_before == 1.0f);
    p.restricted  = false;
    p.remaining_s = 500 * 60;   // more than the limit (extra time): capped
    CHECK(consequence(p).left_before_min == 180 && consequence(p).used_before == 0.0f);
}

static void test_trust()
{
    const std::string sha(64, 'a'), other(64, 'b');
    Trust t;
    t.nro    = "sdmc:/switch/myapp/myapp.nro";
    t.sha    = sha;
    t.name   = "MyApp";
    t.action = "clock_sync";
    t.until  = "2026-11-19";
    std::vector<Trust> list;
    grant(list, t, "2026-10-20");
    CHECK(trusted(list, t.nro, sha, "clock_sync", "2026-10-20"));
    CHECK(trusted(list, t.nro, sha, "clock_sync", "2026-11-19"));
    CHECK(!trusted(list, t.nro, sha, "clock_sync", "2026-11-20"));     // past its day
    CHECK(!trusted(list, t.nro, other, "clock_sync", "2026-10-20"));   // another file at that path
    CHECK(!trusted(list, "sdmc:/switch/x/x.nro", sha, "clock_sync", "2026-10-20"));
    CHECK(!trusted(list, t.nro, sha, "clock", "2026-10-20"));

    // A change worth one, once a day.
    const Trust* e = trusted(list, t.nro, sha, "clock_sync", "2026-10-20");
    CHECK(auto_sync_allowed(*e, 300, "2026-10-20") && auto_sync_allowed(*e, -61, "2026-10-20"));
    CHECK(!auto_sync_allowed(*e, 60, "2026-10-20") && !auto_sync_allowed(*e, 3, "2026-10-20"));
    used_today(list, t.nro, "clock_sync", "2026-10-20");
    CHECK(!auto_sync_allowed(*trusted(list, t.nro, sha, "clock_sync", "2026-10-20"), 300, "2026-10-20"));
    CHECK(auto_sync_allowed(*trusted(list, t.nro, sha, "clock_sync", "2026-10-21"), 300, "2026-10-21"));

    // Read back the same; a damaged entry dropped, not the others.
    const auto back = parse_trust(dump_trust(list));
    CHECK(back.size() == 1 && back[0].last_auto == "2026-10-20" && back[0].name == "MyApp");
    CHECK(parse_trust(R"({"trusted":[{"nro":"sdmc:/atmosphere/x.nro","sha":"aa","action":"clock_sync","until":"2026-11-19"}]})").empty());
    CHECK(parse_trust("{}").empty() && parse_trust("no").empty());

    // Granted again: replaced, not doubled; expired ones go.
    t.until = "2026-11-20";
    grant(list, t, "2026-10-21");
    CHECK(list.size() == 1 && list[0].until == "2026-11-20");
    Trust old = t;
    old.nro   = "sdmc:/switch/old/old.nro";
    old.until = "2026-10-01";
    list.push_back(old);
    Trust fresh = t;
    fresh.nro = "sdmc:/switch/new/new.nro";
    grant(list, fresh, "2026-10-21");
    CHECK(list.size() == 2);
    revoke(list, fresh.nro, "clock_sync");
    CHECK(list.size() == 1 && list[0].nro == t.nro);

    LogEntry l;
    l.route   = "POST /v1/clock/sync";
    l.status  = 200;
    l.trusted = true;
    std::vector<LogEntry> log;
    log_call(log, l);
    CHECK(parse_log(dump_log(log)).front().trusted);
}

static void test_response()
{
    Request r;
    Problem p;
    CHECK(parse_file(R"({"api":"v1","id":"8f","method":"GET","path":"/v1","return_to":"sdmc:/x.nro"})", &r, &p));
    Answer ok;
    ok.body = { { "offset_s", 0 } };
    nlohmann::json j = response(r, { ok }, "1.9.0");
    CHECK(j["api"] == "v1" && j["id"] == "8f" && j["status"] == 200 && j["body"]["offset_s"] == 0);
    CHECK(j["playguard"] == "1.9.0" && j["ignored"][0] == "return_to");

    const Answer e = error_answer(problem(503, "no_network", "No NTP server answered"), "Pas de connexion");
    CHECK(e.status == 503 && e.body["code"] == "no_network" && e.body["title"] == "Pas de connexion");
    CHECK(e.body["type"] == "https://playguard.link/api/errors/no_network");

    CHECK(parse_short("GET /v1", &r, &p));
    j = response(r, { e }, "1.9.0");
    CHECK(!j.contains("id") && j["status"] == 503 && !j.contains("ignored"));
    CHECK(overall_status(r, { e }) == 503);

    CHECK(parse_file(R"({"api":"v1","id":"c4","requests":[{"method":"GET","path":"/v1"},{"method":"GET","path":"/v1"}]})", &r, &p));
    j = response(r, { ok, e }, "1.9.0");
    CHECK(j["status"] == 200 && j["responses"].size() == 2 && j["responses"][1]["status"] == 503);
    CHECK(overall_status(r, { ok, e }) == 503);
    CHECK(overall_status(r, { ok, ok }) == 200);

    CHECK(relaunch_args("sdmc:/switch/a/a.nro", "sdmc:/switch/a/r.json", 200) ==
          "sdmc:/switch/a/a.nro --playguard-response sdmc:/switch/a/r.json --playguard-status 200");
    CHECK(relaunch_args("sdmc:/switch/my app/a.nro", "sdmc:/config/playguard/api/response.json", 403) ==
          "\"sdmc:/switch/my app/a.nro\" --playguard-response sdmc:/config/playguard/api/response.json --playguard-status 403");
}

static void test_discovery()
{
    Settings s;
    nlohmann::json d = discovery(s, false, "1.9.0");
    CHECK(d["enabled"] == false && d["routes"].empty() && d["groups"].empty() && d["api"][0] == "v1");
    s.enabled = true;
    s.levels["clock"] = Level::Read;
    d = discovery(s, true, "1.9.0");
    CHECK(d["read_only"] == true && d["groups"]["clock"] == "read");
    CHECK(d["routes"] == nlohmann::json::array({ "GET /v1/clock" }));
    s.levels["clock"] = Level::ReadWrite;
    d = discovery(s, false, "1.9.0");
    CHECK(d["routes"].size() == 2);
}

static void test_handled()
{
    std::vector<Handled> list;
    for (int i = 0; i < 40; i++) {
        Handled h;
        h.id       = "id" + std::to_string(i);
        h.response = { { "status", 200 } };
        remember(list, h);
    }
    CHECK(list.size() == MAX_HANDLED && list.front().id == "id8" && list.back().id == "id39");
    Handled again;
    again.id         = "id20";
    again.child_data = true;
    remember(list, again);
    CHECK(list.size() == MAX_HANDLED && list.back().id == "id20" && list.back().child_data);

    const auto back = parse_handled(dump_handled(list));
    CHECK(back.size() == list.size());
    const Handled* h = find_handled(back, "id39");
    CHECK(h && h->response["status"] == 200);
    CHECK(find_handled(back, "id20")->child_data);
    CHECK(!find_handled(back, "nope"));

    CHECK(parse_handled("").empty() && parse_handled("{}").empty());
    CHECK(parse_handled(R"({"handled":[{"id":"bad id"},{"id":"ok","response":[]},7]})").size() == 1);
}

static void test_log()
{
    std::vector<LogEntry> list;
    for (int i = 0; i < 25; i++) {
        LogEntry e;
        e.caller = "app" + std::to_string(i);
        e.route  = "GET /v1";
        e.status = 200;
        log_call(list, e);
    }
    CHECK(list.size() == MAX_LOG && list.front().caller == "app5");
    const auto back = parse_log(dump_log(list));
    CHECK(back.size() == MAX_LOG && back.back().caller == "app24" && back.back().status == 200);
    CHECK(parse_log(R"({"calls":[{"route":"x","status":"200"},{"route":"x","status":42},{"route":"x","status":403}]})").size() == 1);

    Call c;
    c.method = "POST";
    c.path   = "/clock/sync";
    CHECK(route_text(c) == "POST /v1/clock/sync");
    CHECK(route_key(c) == "clock_sync");
    CHECK(route_key("GET /v1") == "discovery" && route_key("GET /v1/clock") == "clock");
    CHECK(route_key("POST /v1/clock/sync") == "clock_sync");
    CHECK(route_key("GET /v1/status").empty() && route_key("?").empty() && route_key("").empty());

    // The error's code is kept, a strange one dropped.
    LogEntry e;
    e.route  = "POST /v1/clock/sync";
    e.status = 403;
    e.code   = "pin_refused";
    std::vector<LogEntry> one;
    log_call(one, e);
    CHECK(parse_log(dump_log(one)).front().code == "pin_refused");
    CHECK(parse_log(R"({"calls":[{"route":"x","status":403,"code":"<b>"}]})").front().code.empty());
}

int main()
{
    test_argv();
    test_short();
    test_file();
    test_several();
    test_paths();
    test_routes();
    test_levels();
    test_timeout_and_clock();
    test_consequence();
    test_trust();
    test_response();
    test_discovery();
    test_handled();
    test_log();
    return CHECK_DONE("api_logic launch arguments, requests, routes, permissions, responses and kept files assertions passed");
}
