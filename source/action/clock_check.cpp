// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/clock_check.hpp"

#include <aurore.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

#include "action/notices.hpp"
#include "tab/clock_tab.hpp"
#include "ui/play_ui.hpp"
#include "util/agent_latest.hpp"
#include "util/config.hpp"
#include "util/ntp_client.hpp"
#include "util/ntp_servers.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace clock_check
{

// The toast and the bell's entry when the network clock is a minute or more
// off `server`'s; the entry cleared when it is not.
static void report(long long off, const std::string& server)
{
    if (off < 60) {
        notices::clear("clock_off");
        return;
    }
    const std::string text = aurore::getStr("playguard/clock/start_off", ui::fmt_duration((uint64_t)off), server);
    ui::notify(text);
    // Kept in the bell after the toast, until put away.
    notices::set("clock_off", notices::Level::Info, "playguard/notices/clock"_i18n, text,
                 aurore::getStr("playguard/notices/open_tab", "playguard/tabs/clock"_i18n),
                 []() { ClockTab::open_screen(); });
}

void at_start()
{
    const auto& cfg = config::get();
    if (!cfg.clock_check_at_start) return;
    // The agent's comparison (latest.conf) when it is recent: no sample here.
    const agent_latest::Reading reading = agent_latest::read();
    if (agent_latest::clock_fresh(reading, (int64_t)std::time(nullptr))) {
        report(std::llabs((long long)reading.clock_network),
               reading.clock_server.empty() ? ntp::default_server_for_console() : reading.clock_server);
        return;
    }
    const std::string server = cfg.ntp_server.empty() ? ntp::default_server_for_console() : cfg.ntp_server;
    ui::in_background("clock check", [server]() {
        const ntp::Reply reply = ntp::fetch(server, 2500, 2, ui::quitting);
        if (ui::quitting()) return;
        TimeSnapshot clocks;
        time_clock_snapshot(&clocks);
        // The server's time at the moment the clocks were read.
        const auto since = std::chrono::steady_clock::now() - reply.received_at;
        const long long server_now =
            (long long)reply.unix_seconds + std::chrono::duration_cast<std::chrono::seconds>(since).count();
        aurore::sync([server, reply, clocks, server_now]() {
            // No network, no answer: nothing to say at start-up.
            if (!reply.ok || R_FAILED(clocks.network_rc)) {
                aurore::Logger::info("clock check at start-up: {}", reply.ok ? "network clock unreadable" : reply.error);
                return;
            }
            report(std::llabs(server_now - (long long)clocks.network_time), server);
        });
    });
}

}   // namespace clock_check
