// Host tests for the header-only helpers of source/util: utf8.hpp (the strict
// and the loose reader, the writer), emoji.hpp (what counts, the icon a text
// opens on, dropping them), text.hpp (the two trims, ASCII lower case, a
// prefix, a URL without its scheme) and json_util.hpp (fields read by type,
// a text that is no JSON).
#include "check.h"
#include <cstdio>

#include "util/emoji.hpp"
#include "util/json_util.hpp"
#include "util/text.hpp"
#include "util/utf8.hpp"

static void test_utf8()
{
    uint32_t cp = 0;
    CHECK(utf8::decode("A", 0, &cp) == 1 && cp == 'A');
    CHECK(utf8::decode("\xC3\xA9", 0, &cp) == 2 && cp == 0xE9);                    // é
    CHECK(utf8::decode("\xE2\x80\x94", 0, &cp) == 3 && cp == 0x2014);              // —
    CHECK(utf8::decode("\xF0\x9F\x9A\x80", 0, &cp) == 4 && cp == 0x1F680);         // 🚀
    CHECK(utf8::decode("\x80", 0, &cp) == 0);                                       // stray continuation
    CHECK(utf8::decode("\xC0\x80", 0, &cp) == 0 && utf8::decode("\xE0\x80\x80", 0, &cp) == 0);   // overlong
    CHECK(utf8::decode("\xED\xA0\x80", 0, &cp) == 0);                               // surrogate
    CHECK(utf8::decode("\xF4\x90\x80\x80", 0, &cp) == 0);                           // beyond U+10FFFF
    CHECK(utf8::decode("\xE2\x80", 0, &cp) == 0 && utf8::decode("\xE2\x41\x41", 0, &cp) == 0);   // cut, broken
    CHECK(utf8::decode("x\xC3\xA9", 1, &cp) == 2 && cp == 0xE9);

    CHECK(utf8::decode_loose("\xF0\x9F\x9A\x80", 0, &cp) == 4 && cp == 0x1F680);
    CHECK(utf8::decode_loose("\x80", 0, &cp) == 1 && cp == 0x80);                   // its own code point
    CHECK(utf8::decode_loose("\xFF", 0, &cp) == 1 && cp == 0xFF);
    CHECK(utf8::decode_loose("\xE2\x80", 0, &cp) == 3);                             // cut: what is there

    std::string out;
    for (uint32_t c : { 0x41u, 0xE9u, 0x2014u, 0x1F680u, 0x10FFFFu }) utf8::append(out, c);
    CHECK(out == "A\xC3\xA9\xE2\x80\x94\xF0\x9F\x9A\x80\xF4\x8F\xBF\xBF");
    size_t i = 0, n = 0;
    while (i < out.size()) {
        const size_t len = utf8::decode(out, i, &cp);
        if (!CHECK(len)) break;
        i += len;
        n++;
    }
    CHECK(n == 5);
}

static void test_emoji()
{
    CHECK(emoji::is_emoji(0x1F680) && emoji::is_emoji(0x2615) && emoji::is_emoji(0x23F3) && emoji::is_emoji(0x2B50));
    CHECK(emoji::is_emoji(0x200D) && emoji::is_emoji(0xFE0F) && emoji::is_emoji(0x20E3));   // what goes with one
    CHECK(!emoji::is_emoji('A') && !emoji::is_emoji(0xE9) && !emoji::is_emoji(0x2014) && !emoji::is_emoji(0x2192));
    CHECK(emoji::is_modifier(0x20E3) && !emoji::is_modifier(0x1F680));

    CHECK(emoji::icon_at("\xF0\x9F\x9A\x80 Features") == "1f680");
    CHECK(emoji::icon_at("**\xF0\x9F\x9A\x80**", 2) == "1f680");
    CHECK(emoji::icon_at("Features").empty() && emoji::icon_at("").empty() && emoji::icon_at("ab", 5).empty());
    CHECK(emoji::icon_at("\xEF\xB8\x8F x").empty() && emoji::icon_at("\xE2\x83\xA3").empty());   // a modifier alone

    // 1️⃣ (1, U+FE0F, U+20E3): the digit stays, the keycap marks go.
    CHECK(emoji::drop("1\xEF\xB8\x8F\xE2\x83\xA3 first") == "1 first");
    CHECK(emoji::drop("\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x92\xBB dev") == " dev");   // a ZWJ sequence
    CHECK(emoji::drop("caf\xC3\xA9 \xE2\x80\x94 ok") == "caf\xC3\xA9 \xE2\x80\x94 ok");
}

static void test_text()
{
    CHECK(text::trim_space(" \t\r\n x y \v\f\n") == "x y");
    CHECK(text::trim_space("   ").empty() && text::trim_space("").empty() && text::trim_space("a") == "a");
    CHECK(text::trim_blank(" \t x y \t ") == "x y");
    CHECK(text::trim_blank("x\n") == "x\n" && text::trim_blank("\r x") == "\r x");   // line ends stay
    CHECK(text::trim_blank(" \t ").empty());
    CHECK(text::lower_ascii("Hello WORLD \xC3\x89") == "hello world \xC3\x89");
    CHECK(text::starts_with("## Title", "## ") && !text::starts_with("#", "## ") && text::starts_with("x", ""));
    CHECK(text::short_url("https://ko-fi.com/jigsawfr") == "ko-fi.com/jigsawfr");
    CHECK(text::short_url("github.com/x") == "github.com/x" && text::short_url("").empty());
}

static void test_json()
{
    nlohmann::json j;
    CHECK(json_util::parse(R"({"s": "v", "b": true, "f": false, "n": 42, "x": 1.5, "t": "true"})", &j));
    CHECK(json_util::string_at(j, "s") == "v" && json_util::string_at(j, "n").empty() && json_util::string_at(j, "-").empty());
    CHECK(json_util::flag_at(j, "b") && !json_util::flag_at(j, "f") && !json_util::flag_at(j, "t") && !json_util::flag_at(j, "-"));
    int64_t v = -1;
    CHECK(json_util::integer_at(j, "n", &v) && v == 42);
    v = -1;
    CHECK(!json_util::integer_at(j, "x", &v) && !json_util::integer_at(j, "s", &v) && v == -1);

    // Not an object: no field, no exception.
    CHECK(json_util::parse("[1, 2]", &j) && json_util::string_at(j, "s").empty() && !json_util::flag_at(j, "b"));
    CHECK(json_util::parse("\"text\"", &j) && !json_util::integer_at(j, "n", &v));
    CHECK(!json_util::parse("{\"s\": ", &j) && !json_util::parse("", &j) && !json_util::parse("nope", &j));
}

int main()
{
    test_utf8();
    test_emoji();
    test_text();
    test_json();
    CHECK_DONE("utf8, emoji, text and json_util assertions passed");
}
