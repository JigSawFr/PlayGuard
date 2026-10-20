// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarders.hpp"

#include <borealis/extern/nlohmann/json.hpp>
#include <cstdio>

#include "util/json_util.hpp"
#include "util/paths.hpp"
#include "util/utf8.hpp"

namespace forwarders
{

namespace
{

std::string file_path()
{
    return paths::nand_dir() + "/forwarders.json";
}

bool write_all(const std::vector<Made>& all, std::string* error)
{
    nlohmann::json list = nlohmann::json::array();
    for (const Made& m : all) {
        nlohmann::json e = { { "id", hex_id(m.id) }, { "nro", m.nro }, { "argv", m.argv }, { "name", m.name },
                             { "author", m.author } };
        if (m.pin) {
            e["pin"] = true;
            e["gate"] = m.gate;
        }
        if (m.gone) e["gone"] = true;
        if (m.pin && m.verified) e["verified"] = true;
        if (m.pin && m.free_days)
            e["free"] = { { "days", m.free_days }, { "from", m.free_from }, { "to", m.free_to } };
        list.push_back(e);
    }
    const nlohmann::json doc = { { "forwarders", list } };
    if (!paths::ensure_dir(paths::nand_dir())) {
        if (error) *error = "cannot create " + paths::nand_dir();
        return false;
    }
    // Names come from outside (a .nro, the keyboard): replaced, never thrown on.
    return paths::atomic_write(file_path(), doc.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n",
                               error);
}

}   // namespace

std::vector<Made> list()
{
    std::vector<Made> out;
    std::string text;
    nlohmann::json doc;
    if (!paths::read_file(file_path(), text) || !json_util::parse(text, &doc) || !doc.is_object()) return out;
    const auto it = doc.find("forwarders");
    if (it == doc.end() || !it->is_array()) return out;
    for (const auto& e : *it) {
        if (!e.is_object()) continue;
        Made m;
        if (!parse_id(json_util::string_at(e, "id"), &m.id)) continue;
        m.nro = json_util::string_at(e, "nro");
        if (m.nro.empty()) continue;
        m.argv   = json_util::string_at(e, "argv");
        m.name   = clean_text(json_util::string_at(e, "name"), MAX_NAME);
        m.author = clean_text(json_util::string_at(e, "author"), MAX_AUTHOR);
        if (m.argv.empty()) m.argv = m.nro;
        m.gate = json_util::string_at(e, "gate");
        m.pin  = json_util::flag_at(e, "pin") && !m.gate.empty();
        m.gone = json_util::flag_at(e, "gone");
        m.verified = m.pin && json_util::flag_at(e, "verified");
        if (const nlohmann::json* f = json_util::object_at(e, "free")) {
            const int64_t days = json_util::integer_or(*f, "days"), from = json_util::integer_or(*f, "from"),
                          to = json_util::integer_or(*f, "to");
            // Kept only as the gate reads it (gate_rule_find): days of a week, minutes of a day, a span.
            if (m.pin && days > 0 && days <= 0x7F && from >= 0 && from <= 1440 && to >= 0 && to <= 1440 && from != to) {
                m.free_days = (int)days;
                m.free_from = (int)from;
                m.free_to = (int)to;
            }
        }
        bool seen = false;
        for (const Made& o : out) seen = seen || o.id == m.id;
        if (!seen) out.push_back(m);
    }
    return out;
}

bool save(const Made& m, std::string* error)
{
    std::vector<Made> all = list();
    bool replaced = false;
    for (Made& o : all) {
        if (o.id != m.id) continue;
        o = m;
        replaced = true;
    }
    if (!replaced) all.push_back(m);
    return write_all(all, error);
}

bool forget(uint64_t id, std::string* error)
{
    std::vector<Made> all = list();
    const size_t before = all.size();
    for (size_t i = all.size(); i-- > 0;)
        if (all[i].id == id) all.erase(all.begin() + (long)i);
    return all.size() == before || write_all(all, error);
}

bool find(uint64_t id, Made* out)
{
    for (const Made& m : list()) {
        if (m.id != id) continue;
        if (out) *out = m;
        return true;
    }
    return false;
}

std::string clean_text(const std::string& text, size_t max)
{
    std::string out;
    size_t chars = 0;
    for (size_t i = 0; i < text.size() && chars < max;) {
        uint32_t cp = 0;
        const size_t n = utf8::decode(text, i, &cp);
        if (n == 0) {
            i++;
            continue;
        }
        i += n;
        if (cp < 0x20 || (cp >= 0x7F && cp < 0xA0)) continue;
        if (cp == ' ' && (out.empty() || out.back() == ' ')) continue;
        utf8::append(out, cp);
        chars++;
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

std::string hex_id(uint64_t id)
{
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)id);
    return buf;
}

bool parse_id(const std::string& hex, uint64_t* id)
{
    if (hex.size() != 16) return false;
    uint64_t v = 0;
    for (const char c : hex) {
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return false;
        v = v << 4 | (uint64_t)d;
    }
    *id = v;
    return v != 0;
}

}   // namespace forwarders
