// text — the small string helpers the parsers share. The two trims differ on
// purpose, and each says what it removes: Markdown keeps a line end that
// matters to it, the other readers drop every kind of space. Header-only,
// plain C++ (ASCII only: no locale).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstring>
#include <string>

namespace text
{

// `s` without the white space (std::isspace in the C locale: space, \t, \n,
// \v, \f, \r) at either end.
inline std::string trim_space(const std::string& s)
{
    const char* ws = " \t\n\v\f\r";
    const size_t a = s.find_first_not_of(ws);
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(ws) - a + 1);
}

// `s` without the spaces and tabs at either end (what Markdown calls blank);
// line ends stay.
inline std::string trim_blank(const std::string& s)
{
    const size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t") - a + 1);
}

// A..Z as a..z, every other byte as it is.
inline std::string lower_ascii(std::string s)
{
    for (char& c : s)
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
    return s;
}

inline bool starts_with(const std::string& s, const char* prefix)
{
    return s.compare(0, std::strlen(prefix), prefix) == 0;
}

}   // namespace text
