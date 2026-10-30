// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/table_export_parts.hpp"

#include <charconv>

#include <aurore/extern/nlohmann/json.hpp>

namespace table_export
{
namespace detail
{

std::string render_json(const Table& t)
{
    nlohmann::ordered_json j;
    j["title"]    = t.title;
    j["subtitle"] = t.subtitle;
    nlohmann::ordered_json rows = nlohmann::ordered_json::array();
    for (const auto& row : t.rows) {
        nlohmann::ordered_json o = nlohmann::ordered_json::object();
        for (size_t i = 0; i < t.columns.size(); i++) {
            const std::string v = cell(row, i);
            if (!t.columns[i].numeric) o[t.columns[i].key] = v;
            else if (is_integer(v)) {
                long long n = 0;
                std::from_chars(v.data(), v.data() + v.size(), n);   // 18 digits at most: no overflow
                o[t.columns[i].key] = n;
            } else
                o[t.columns[i].key] = nullptr;
        }
        rows.push_back(o);
    }
    j["rows"] = rows;
    return j.dump(2, ' ', false, nlohmann::ordered_json::error_handler_t::replace) + "\n";
}

}   // namespace detail
}   // namespace table_export
