// libFuzzer target: source/action/api_logic.cpp reading what another
// homebrew hands PlayGuard (docs/api.md): the request file, the short form,
// and the two files kept between launches (handled.json, calls.json), all on
// the SD card. Beyond the sanitizers: every path a request keeps is one a
// caller may name, every call a known method with a path from "/",
// at most 8 calls, and a refused request says why with a 4xx or 5xx; the
// kept files read back the same once written.
#include <string>

#include "action/api_logic.hpp"
#include "fuzz.h"

using namespace api_logic;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string text(reinterpret_cast<const char*>(data), size);

    for (bool file : { true, false }) {
        Request r;
        Problem p;
        const bool ok = file ? parse_file(text, &r, &p) : parse_short(text, &r, &p);
        if (!ok) {
            FUZZ_ASSERT(p.status >= 400 && p.status < 600 && !p.code.empty());
            continue;
        }
        FUZZ_ASSERT(!r.calls.empty() && r.calls.size() <= MAX_CALLS);
        FUZZ_ASSERT(r.return_to.empty() || return_to_ok(r.return_to));
        FUZZ_ASSERT(r.response.empty() || response_path_ok(r.response));
        FUZZ_ASSERT(r.lang.empty() || lang_ok(r.lang));
        FUZZ_ASSERT(r.caller.size() <= 64);
        FUZZ_ASSERT(file ? id_ok(r.id) : r.id.empty());
        for (size_t i = 0; i < r.calls.size(); i++) {
            const Call& c = r.calls[i];
            FUZZ_ASSERT(c.method == "GET" || c.method == "POST" || c.method == "OPEN");
            FUZZ_ASSERT((c.mode == "open") == (c.method == "OPEN"));
            FUZZ_ASSERT(c.mode != "open" || i + 1 == r.calls.size());
            FUZZ_ASSERT(c.body.is_object() && (c.path.empty() || c.path[0] == '/'));
            Problem q;
            const Route* route = find(c, &q);
            FUZZ_ASSERT(route || q.status == 404);
            Problem t;
            const int ms = timeout_ms(c, &t);
            FUZZ_ASSERT(t ? ms == 0 && t.status == 422 : ms >= TIMEOUT_MIN_MS && ms <= TIMEOUT_MAX_MS);
        }
        std::vector<Answer> answers(r.calls.size());
        const nlohmann::json j = response(r, answers, "1.0.0");
        FUZZ_ASSERT(j.is_object() && j["api"] == API_VERSION);
    }

    const auto handled = parse_handled(text);
    FUZZ_ASSERT(handled.size() <= MAX_HANDLED);
    FUZZ_ASSERT(parse_handled(dump_handled(handled)).size() == handled.size());
    const auto log = parse_log(text);
    FUZZ_ASSERT(log.size() <= MAX_LOG);
    FUZZ_ASSERT(dump_log(parse_log(dump_log(log))) == dump_log(log));
    return 0;
}
