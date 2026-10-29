// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/table_export_parts.hpp"

namespace table_export
{
namespace detail
{

namespace
{
std::string csv_field(std::string s, bool numeric)
{
    // A text cell starting like a formula is shown as text by spreadsheets
    // (OWASP: = + - @, tab and carriage return). A numeric cell is written as
    // is only when it is an integer (as in JSON and XLSX), else it is text.
    const bool number = numeric && (s.empty() || is_integer(s));
    if (!number && !s.empty() && std::string_view("=+-@\t\r").contains(s[0])) s.insert(0, "'");
    if (s.find_first_of(",\"\r\n") == std::string::npos && (s.empty() || (s.front() != ' ' && s.back() != ' ')))
        return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += '"';
        out += c;
    }
    return out + "\"";
}
}   // namespace

std::string render_csv(const Table& t)
{
    std::string out = "\xEF\xBB\xBF";   // UTF-8 byte-order mark: spreadsheets then read accents right
    for (size_t i = 0; i < t.columns.size(); i++) out += (i ? "," : "") + csv_field(t.columns[i].label, false);
    out += "\r\n";
    for (const auto& row : t.rows) {
        for (size_t i = 0; i < t.columns.size(); i++)
            out += (i ? "," : "") + csv_field(cell(row, i), t.columns[i].numeric);
        out += "\r\n";
    }
    return out;
}

}   // namespace detail
}   // namespace table_export
