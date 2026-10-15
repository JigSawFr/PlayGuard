// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/safe_text.hpp"

#include <cctype>
#include <cstdint>

namespace safe_text
{

namespace
{
// The code point of the valid UTF-8 sequence at `s[i]` and its length; 0
// when there is none there (the byte is to be left out).
size_t decode(const std::string& s, size_t i, uint32_t* cp)
{
    const unsigned char c = (unsigned char)s[i];
    if (c < 0x80) {
        *cp = c;
        return 1;
    }
    size_t n;
    uint32_t v, min;
    if ((c & 0xE0) == 0xC0) n = 2, v = c & 0x1F, min = 0x80;
    else if ((c & 0xF0) == 0xE0) n = 3, v = c & 0x0F, min = 0x800;
    else if ((c & 0xF8) == 0xF0) n = 4, v = c & 0x07, min = 0x10000;
    else return 0;
    if (i + n > s.size()) return 0;
    for (size_t k = 1; k < n; k++) {
        const unsigned char d = (unsigned char)s[i + k];
        if ((d & 0xC0) != 0x80) return 0;
        v = (v << 6) | (d & 0x3F);
    }
    if (v < min || v > 0x10FFFF || (v >= 0xD800 && v <= 0xDFFF)) return 0;
    *cp = v;
    return n;
}

bool hidden(uint32_t cp)
{
    return (cp >= 0x202A && cp <= 0x202E) || (cp >= 0x2066 && cp <= 0x2069) || cp == 0x200E || cp == 0x200F ||
           cp == 0x061C || cp == 0x200B || cp == 0x200C || cp == 0x2060 || cp == 0xFEFF;
}
}   // namespace

std::string clean(const std::string& text, size_t max_bytes, bool multiline)
{
    std::string out;
    out.reserve(text.size() < max_bytes ? text.size() : max_bytes);
    for (size_t i = 0; i < text.size();) {
        uint32_t cp = 0;
        const size_t n = decode(text, i, &cp);
        if (!n) {
            i++;
            continue;
        }
        std::string piece;
        if (cp == '\n' || cp == 0x2028 || cp == 0x2029) piece = multiline ? "\n" : " ";
        else if (cp == '\t') piece = " ";
        else if (cp < 0x20 || cp == 0x7F || (cp >= 0x80 && cp <= 0x9F) || hidden(cp)) piece = "";
        else piece = text.substr(i, n);
        i += n;
        if (out.size() + piece.size() > max_bytes) break;
        out += piece;
    }
    return out;
}

bool is_version(const std::string& text)
{
    if (text.empty() || text.size() > 40 || !std::isalnum((unsigned char)text[0])) return false;
    for (char c : text)
        if (!std::isalnum((unsigned char)c) && c != '.' && c != '+' && c != '-' && c != '_') return false;
    return true;
}

}   // namespace safe_text
