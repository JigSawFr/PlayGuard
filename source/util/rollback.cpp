// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/rollback.hpp"

#include <cstdio>

#include <borealis/extern/nlohmann/json.hpp>

#include "util/paths.hpp"

namespace rollback
{

namespace
{
using nlohmann::json;

bool read_json(const std::string& file, json* out)
{
    std::string text;
    if (!paths::read_file(file, text)) return false;
    try {
        *out = json::parse(text);
        return out->is_object();
    } catch (const std::exception&) {
        return false;
    }
}

bool exists(const std::string& file)
{
    FILE* f = std::fopen(file.c_str(), "rb");
    if (f) std::fclose(f);
    return f != nullptr;
}
}   // namespace

std::string previous_file(const std::string& nro)
{
    return nro + ".previous";
}

bool read_label(const std::string& json_file, std::string* label)
{
    json j;
    if (!read_json(json_file, &j) || !j.contains("label") || !j["label"].is_string()) return false;
    *label = j["label"].get<std::string>();
    return !label->empty();
}

bool write_label(const std::string& json_file, const std::string& label)
{
    std::string err;
    return paths::atomic_write(json_file, json{ { "label", label } }.dump() + "\n", &err);
}

bool read_check(const std::string& json_file, Check* out)
{
    json j;
    if (!read_json(json_file, &j)) return false;
    Check c;
    if (j.contains("label") && j["label"].is_string()) c.label = j["label"].get<std::string>();
    if (j.contains("starts") && j["starts"].is_number_integer()) c.starts = j["starts"].get<int>();
    if (c.starts < 0) c.starts = 0;
    *out = c;
    return true;
}

bool write_check(const std::string& json_file, const Check& check)
{
    std::string err;
    return paths::atomic_write(json_file, json{ { "label", check.label }, { "starts", check.starts } }.dump() + "\n", &err);
}

Offer offer_at_start(bool checked, int starts_before, bool held, bool have_previous)
{
    if (!have_previous) return Offer::None;
    if (checked && starts_before >= 1) return Offer::Crashed;
    return held ? Offer::Asked : Offer::None;
}

bool swap(const std::string& nro, std::string* error)
{
    const std::string prev = previous_file(nro), tmp = nro + ".swap";
    auto fail = [error](const std::string& why) {
        if (error) *error = why;
        return false;
    };
    if (!exists(prev)) return fail("no previous build");
    std::remove(tmp.c_str());   // left by an earlier attempt
    const bool had_nro = std::rename(nro.c_str(), tmp.c_str()) == 0;
    if (!had_nro && exists(nro)) return fail("cannot move " + nro + " aside");
    if (std::rename(prev.c_str(), nro.c_str()) != 0) {
        if (had_nro) std::rename(tmp.c_str(), nro.c_str());
        return fail("cannot put the previous build in place");
    }
    // The build that ran becomes the previous one (none when there was none).
    if (had_nro && std::rename(tmp.c_str(), prev.c_str()) != 0) std::remove(tmp.c_str());
    return true;
}

}   // namespace rollback
