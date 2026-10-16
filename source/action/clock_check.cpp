// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/clock_check.hpp"

#include <borealis.hpp>
#include <chrono>
#include <cstdlib>
#include <string>

#include "action/notices.hpp"
#include "tab/clock_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/ntp_client.hpp"
#include "util/ntp_servers.hpp"
#include "util/pctl_ops_c.hpp"

using namespace brls::literals;

namespace clock_check
{

void at_start()
{
    const auto& cfg = config::get();
    if (!cfg.clock_check_at_start) return;
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
        brls::sync([server, reply, clocks, server_now]() {
            // No network, no answer: nothing to say at start-up.
            if (!reply.ok || R_FAILED(clocks.network_rc)) {
                brls::Logger::info("clock check at start-up: {}", reply.ok ? "network clock unreadable" : reply.error);
                return;
            }
            const long long off = std::llabs(server_now - (long long)clocks.network_time);
            if (off < 60) {
                notices::clear("clock_off");
                return;
            }
            const std::string text = brls::getStr("playguard/clock/start_off", ui::fmt_play_time((uint64_t)off), server);
            ui::notify(text);
            // Kept in the bell after the toast, until put away.
            notices::set("clock_off", notices::Level::Info, "playguard/notices/clock"_i18n, text,
                         brls::getStr("playguard/notices/open_tab", "playguard/tabs/clock"_i18n),
                         []() { ClockTab::open_screen(); });
        });
    });
}

}   // namespace clock_check
