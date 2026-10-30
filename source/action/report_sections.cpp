// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/report_sections.hpp"

#include <format>
#include <string>
#include <vector>

#include "action/fw_gate.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/nintendo_dns.hpp"
#include "action/sync_flow.hpp"
#include "core/sysinfo.h"
#include "util/diagnostics.hpp"
#include "util/pctl_ops_c.hpp"

namespace report_sections
{

void install()
{
    diagnostic::add_header_line([]() { return "fw choice   : " + fw_gate::summary() + "\n"; });
    diagnostic::add_header_line([]() {
        SysInfo si;
        sysinfo_get(&si);
        return std::format("forwarder   : {}\ncompat      : {}\n", si.forwarder ? "yes" : "no", (int)sysinfo_compat(&si));
    });
    diagnostic::add_section(&hbmenu_lock::report);
    diagnostic::add_section(&nintendo_dns::report);
    diagnostic::add_section([]() {
        std::vector<char> buf(4096);
        time_clock_dump(buf.data(), buf.size());
        std::string out = buf.data();
        buf.assign(24576, '\0');
        pctl_dump(buf.data(), buf.size());
        out += buf.data();
        if (std::string(buf.data()).size() >= buf.size() - 1) out += "\n(report truncated)\n";
        return out + sync_flow::report_section();
    });
}

}   // namespace report_sections
