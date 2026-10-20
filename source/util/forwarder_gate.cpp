// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_gate.hpp"

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
    return quoted(self) + " " + FLAG + " " + argv;
}

bool parse(const std::string& raw, std::string& nro, std::string& argv)
{
    nro.clear();
    argv.clear();
    // PlayGuard's path, then the flag: the rest is the homebrew's argv.
    size_t at = 0;
    if (!raw.empty() && raw[0] == '"') {
        at = raw.find('"', 1);
        if (at == std::string::npos) return false;
        at++;
    } else {
        at = raw.find(' ');
        if (at == std::string::npos) return false;
    }
    const std::string flag = std::string(" ") + FLAG + " ";
    if (raw.compare(at, flag.size(), flag) != 0) return false;
    argv = raw.substr(at + flag.size());
    // Its path: its first word, or what its quotes hold.
    if (!argv.empty() && argv[0] == '"') {
        const size_t end = argv.find('"', 1);
        if (end != std::string::npos) nro = argv.substr(1, end - 1);
    } else {
        nro = argv.substr(0, argv.find(' '));
    }
    if (nro.compare(0, 6, "sdmc:/") != 0) {
        nro.clear();
        argv.clear();
        return false;
    }
    return true;
}

}   // namespace forwarder_gate
