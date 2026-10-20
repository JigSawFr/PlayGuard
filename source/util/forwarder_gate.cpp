// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_gate.hpp"

#include "core/gate.h"

namespace forwarder_gate
{

namespace
{

std::string quoted(const std::string& path)
{
    return path.find(' ') == std::string::npos ? path : "\"" + path + "\"";
}

}   // namespace

std::string argv_for(const std::string& self, const std::string& argv)
{
    return quoted(self) + " " + GATE_FLAG + " " + argv;
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
