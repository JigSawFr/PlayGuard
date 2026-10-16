// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/changelog.hpp"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <sstream>

#include "util/update.hpp"

namespace changelog
{
namespace
{
std::string trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

bool starts_with(const std::string& s, const char* prefix)
{
    return s.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

// Pictographs, dingbats and the joiners / variation selectors that go with
// them: the console's shared font draws none of them.
bool is_emoji(uint32_t cp)
{
    return cp >= 0x1F000 || (cp >= 0x2600 && cp <= 0x27BF) || (cp >= 0x2B00 && cp <= 0x2BFF) ||
           (cp >= 0x231A && cp <= 0x231B) || (cp >= 0x23E9 && cp <= 0x23FA) ||   // ⌚ ⏱ ⏳ …
           cp == 0x200D || (cp >= 0xFE00 && cp <= 0xFE0F);
}

// The UTF-8 sequence at `s[i]`: its length and code point.
size_t decode(const std::string& s, size_t i, uint32_t* cp)
{
    const unsigned char c = s[i];
    const size_t n = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
    *cp = n == 1 ? c : n == 2 ? c & 0x1F : n == 3 ? c & 0x0F : c & 0x07;
    for (size_t k = 1; k < n && i + k < s.size(); k++) *cp = (*cp << 6) | (s[i + k] & 0x3F);
    return n;
}

// The emoji `s` opens on (after spaces and "**"), as its first code point in
// hex: "1f680" for "🚀 Features". Empty when it opens on anything else.
std::string leading_icon(const std::string& s)
{
    size_t i = s.find_first_not_of(" *");
    if (i == std::string::npos) return "";
    uint32_t cp = 0;
    decode(s, i, &cp);
    if (!is_emoji(cp) || cp == 0x200D || (cp >= 0xFE00 && cp <= 0xFE0F)) return "";
    char hex[9];
    std::snprintf(hex, sizeof hex, "%x", (unsigned)cp);
    return hex;
}

std::string drop_emoji(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size();) {
        uint32_t     cp = 0;
        const size_t n  = decode(s, i, &cp);
        if (!is_emoji(cp)) out.append(s, i, n);
        i += n;
    }
    return out;
}

// "abc1234" (a commit) or "#12" (a pull request or an issue).
bool is_reference(const std::string& s)
{
    if (s.size() > 1 && s[0] == '#') {
        for (size_t i = 1; i < s.size(); i++)
            if (!std::isdigit((unsigned char)s[i])) return false;
        return true;
    }
    if (s.size() < 7 || s.size() > 40) return false;
    for (char c : s)
        if (!std::isxdigit((unsigned char)c)) return false;
    return true;
}

// "[text](url)" → "text"; " ([#12](url))" and " ([abc1234](url))" go.
std::string strip_links(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size();) {
        if (s[i] == '[') {
            const size_t close = s.find(']', i);
            if (close != std::string::npos && close + 1 < s.size() && s[close + 1] == '(') {
                const size_t end = s.find(')', close);
                if (end != std::string::npos) {
                    const std::string text = s.substr(i + 1, close - i - 1);
                    // A reference wrapped in parentheses: drop the whole group.
                    if (is_reference(text) && !out.empty() && out.back() == '(' && end + 1 < s.size() && s[end + 1] == ')') {
                        out.pop_back();
                        while (!out.empty() && out.back() == ' ') out.pop_back();
                        i = end + 2;
                    } else {
                        out += text;
                        i = end + 1;
                    }
                    continue;
                }
            }
        }
        out += s[i++];
    }
    return out;
}

std::string clean(const std::string& s)
{
    std::string out;
    for (char c : strip_links(drop_emoji(s)))
        if (c != '`') out += c;
    for (size_t p; (p = out.find("**")) != std::string::npos;) out.erase(p, 2);
    return trim(out);
}

// "1.1.0 (2026-10-10)" → "1.1.0 · 2026-10-10".
std::string release_title(std::string s)
{
    const size_t open = s.rfind(" (");
    if (open != std::string::npos && s.back() == ')')
        s = s.substr(0, open) + " · " + s.substr(open + 2, s.size() - open - 3);
    return s;
}

// "**Title**: text" or "**Title:** text" → "Title" and "text"; false when
// `item` does not open on a bold lead-in.
bool split_title(const std::string& item, std::string* title, std::string* text)
{
    const size_t open = item.find("**");   // after the emoji, if any
    if (open == std::string::npos || !trim(drop_emoji(item.substr(0, open))).empty()) return false;
    const size_t close = item.find("**", open + 2);
    if (close == std::string::npos) return false;
    std::string head = clean(item.substr(open + 2, close - open - 2));
    std::string rest = item.substr(close + 2);
    if (!head.empty() && head.back() == ':') head.pop_back();
    else if (starts_with(rest, ":")) rest.erase(0, 1);
    else return false;   // bold words inside a sentence
    *title = trim(head);
    *text  = clean(rest);
    return !title->empty();
}

}   // namespace

std::vector<Line> parse(const std::string& md, int max_releases)
{
    std::vector<Line> lines;
    std::istringstream in(md);
    std::string raw;
    int  releases = 0;
    bool in_item  = false;   // an indented line continues the last item
    bool stop     = false;   // past `max_releases`
    // One line of Markdown; `raw` keeps its indentation, `t` is trimmed.
    auto line = [&](const std::string& raw, const std::string& t) {
        if (t.empty()) {
            in_item = false;
            return;
        }
        if (t.front() == '<' && t.back() == '>') {
            in_item = false;   // an HTML line: the release page's <details>, its figures
            return;
        }
        if (starts_with(t, "## ")) {
            if (max_releases > 0 && ++releases > max_releases) {
                stop = true;
                return;
            }
            lines.push_back({ Line::Release, release_title(clean(t.substr(3))) });
            in_item = false;
        } else if (starts_with(t, "# ")) {
            in_item = false;   // the document's title
        } else if (starts_with(t, "### ")) {
            lines.push_back({ Line::Section, clean(t.substr(4)) });
            lines.back().icon = leading_icon(t.substr(4));
            in_item = false;
        } else if (starts_with(t, "* ") || starts_with(t, "- ")) {
            const size_t indent = raw.find_first_not_of(' ');
            Line item{ Line::Item, clean(t.substr(2)), (int)(indent / 2) };
            if (item.depth == 0) {
                split_title(t.substr(2), &item.title, &item.text);
                item.icon = leading_icon(t.substr(2));
            }
            lines.push_back(std::move(item));
            in_item = true;
        } else if (in_item && raw[0] == ' ') {
            // "* **Title**:" then its text on the next line (the release page's
            // two-line bullets): no leading space.
            std::string& text = lines.back().text;
            text += (text.empty() ? "" : " ") + clean(t);
        } else {
            lines.push_back({ Line::Text, clean(t) });
            in_item = false;
        }
        if (!lines.empty() && lines.back().text.empty() && lines.back().title.empty()) lines.pop_back();
    };

    // HTML comments, invisible on GitHub: skipped, except the console's own
    //   <!-- console: a line -->              that line, read here only;
    //   <!-- console:only  …  -->             the lines between, here only;
    //   <!-- console:hide --> … <!-- /console -->   the lines between, not here.
    enum { Plain, Comment, Hide, Only } block = Plain;
    while (!stop && std::getline(in, raw)) {
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();
        const std::string t = trim(raw);
        if (block == Comment) {
            if (t.find("-->") != std::string::npos) block = Plain;
            continue;
        }
        if (block == Hide) {
            if (t == "<!-- /console -->") block = Plain;
            continue;
        }
        if (block == Only && t == "-->") {
            block = Plain;
            in_item = false;
            continue;
        }
        if (block == Plain && starts_with(t, "<!--")) {
            in_item = false;
            const bool closed = t.size() >= 7 && t.compare(t.size() - 3, 3, "-->") == 0;
            if (t == "<!-- console:hide -->") {
                block = Hide;
            } else if (t == "<!-- console:only") {
                block = Only;
            } else if (closed && starts_with(t, "<!-- console:")) {
                const std::string inner = trim(t.substr(13, t.size() - 16));
                line(inner, inner);
                in_item = false;
            } else if (!closed) {
                block = Comment;
            }
            continue;
        }
        line(raw, t);
    }
    return lines;
}

std::vector<Line> newer_than(const std::string& md, const std::string& installed)
{
    std::vector<Line> lines;
    bool inside = false;
    for (Line& l : parse(md, 0)) {
        if (l.kind == Line::Release) {
            const std::string v = l.text.substr(0, l.text.find(" · "));
            int numbers[3];
            inside = update::parse_version(v, numbers) && update::compare_versions(v, installed) > 0;
        }
        if (inside) lines.push_back(std::move(l));
    }
    return lines;
}

std::vector<Line> release_notes(const std::string& md, const std::string& version, std::string* date)
{
    auto bare = [](const std::string& v) { return !v.empty() && (v[0] == 'v' || v[0] == 'V') ? v.substr(1) : v; };
    const std::string wanted = bare(version);
    std::vector<Line> notes;
    bool inside = false;
    for (Line& l : parse(md, 0)) {
        if (l.kind == Line::Release) {
            if (inside) break;
            const size_t dot = l.text.find(" · ");
            if (wanted.empty() || bare(l.text.substr(0, dot)) != wanted) continue;
            inside = true;
            if (date) *date = dot == std::string::npos ? "" : l.text.substr(dot + std::string(" · ").size());
        } else if (inside) {
            notes.push_back(std::move(l));
        }
    }
    return notes;
}

}   // namespace changelog
