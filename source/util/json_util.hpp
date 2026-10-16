// json_util — reading the JSON files and answers PlayGuard is given without
// an exception: each field read as its type or not at all, a text that is no
// JSON as a false. Header-only, plain C++ (nlohmann/json, from borealis).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include <borealis/extern/nlohmann/json.hpp>

namespace json_util
{

// `text` parsed into `out`; false (and `out` discarded) when it is not JSON.
// Never throws.
inline bool parse(const std::string& text, nlohmann::json* out)
{
    *out = nlohmann::json::parse(text, nullptr, false);
    return !out->is_discarded();
}

// j[key] when `j` is an object holding a string there, else "".
inline std::string string_at(const nlohmann::json& j, const char* key)
{
    const auto it = j.find(key);
    return it != j.end() && it->is_string() ? it->get<std::string>() : std::string();
}

// j[key] when it is the boolean true; false otherwise.
inline bool flag_at(const nlohmann::json& j, const char* key)
{
    const auto it = j.find(key);
    return it != j.end() && it->is_boolean() && it->get<bool>();
}

// j[key] into `out` when it is an integer; false (and `out` untouched) otherwise.
inline bool integer_at(const nlohmann::json& j, const char* key, int64_t* out)
{
    const auto it = j.find(key);
    if (it == j.end() || !it->is_number_integer()) return false;
    *out = it->get<int64_t>();
    return true;
}

}   // namespace json_util
