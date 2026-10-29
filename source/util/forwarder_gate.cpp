// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_gate.hpp"

#include "core/gate.h"

namespace forwarder_gate
{

namespace
{

std::string quoted(const std::string& path)
{
    return path.contains(' ') ? "\"" + path + "\"" : path;
}

}   // namespace

std::string argv_for(const std::string& self, const std::string& argv)
{
    return quoted(self) + " " + GATE_FLAG + " " + argv;
}

std::string argv_for(const std::string& self, const GateRules& rules, const std::string& argv)
{
    char tokens[512];
    if (gate_rules_format(&rules, tokens, sizeof(tokens)) == 0) return argv_for(self, argv);
    return quoted(self) + " " + GATE_FLAG + tokens + " " + argv;
}

GateRules rules_of(const forwarders::Made& m, const GatePauseRule& pause)
{
    GateRules r = {};
    r.carried = true;
    const auto add = [](GateHours* spans, unsigned& n, int days, int from, int to) {
        if (days <= 0 || n >= GATE_SPANS) return;
        spans[n++] = { (uint8_t)days, (uint16_t)from, (uint16_t)to };
    };
    add(r.free, r.free_n, m.free_days, m.free_from, m.free_to);
    if (m.free_days) add(r.free, r.free_n, m.free2_days, m.free2_from, m.free2_to);
    add(r.closed, r.closed_n, m.closed_days, m.closed_from, m.closed_to);
    r.has_pause = true;
    r.pause_tries = pause.minutes ? pause.tries : 0;
    r.pause_minutes = r.pause_tries ? pause.minutes : 0;
    r.limit = m.limit;
    return r;
}

bool parse(const std::string& raw, std::string& nro, std::string& argv)
{
    nro.clear();
    argv.clear();
    char path[0x301];
    const char* rest = nullptr;
    if (!gate_argv_parse(raw.c_str(), path, sizeof(path), &rest)) return false;
    nro = path;
    argv = rest;
    return true;
}

}   // namespace forwarder_gate
