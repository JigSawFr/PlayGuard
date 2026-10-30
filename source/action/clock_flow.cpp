// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "action/clock_flow.hpp"

#include <atomic>
#include <format>
#include <aurore.hpp>
#include <memory>
#include <optional>
#include <system_error>
#include <thread>
#include <vector>

#include <aurore/cadran/ui/cadran_modal.hpp>

#include "action/clock_logic.hpp"
#include "action/history_flow.hpp"
#include "action/offline_clock_logic.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/diagnostics.hpp"
#include "util/ntp_client.hpp"
#include "util/ntp_servers.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace clock_flow
{

namespace
{
std::string signed_seconds(int64_t s)
{
    return aurore::getStr("playguard/clock/seconds", (s > 0 ? "+" : "") + std::to_string(s));
}

std::string ntp_error_text(const ntp::Reply& r)
{
    switch (r.kind) {
        case ntp::Error::BadHost:  return "playguard/clock/ntp_err/bad_host"_i18n;
        case ntp::Error::Lookup:   return "playguard/clock/ntp_err/lookup"_i18n;
        case ntp::Error::Timeout:  return "playguard/clock/ntp_err/timeout"_i18n;
        case ntp::Error::BadReply: return "playguard/clock/ntp_err/bad_reply"_i18n;
        default:                   return "playguard/clock/ntp_err/network"_i18n;
    }
}

int64_t elapsed_s(const Measurement& m)
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m.at).count();
}

// The sentence for the user after a write (or a refusal).
std::string result_message(const TimeApply& a)
{
    if (a.refused_automatic) return "playguard/clock/applied_refused"_i18n;
    if (!a.write_attempted) return "playguard/clock/applied_err"_i18n + " — " + ui::rc_text(a.open_rc);
    if (R_FAILED(a.write_rc)) return "playguard/clock/applied_err"_i18n + " — " + ui::rc_text(a.write_rc);
    if (!a.verified) return "playguard/clock/applied_err"_i18n + " — " + ui::rc_text(a.verify_rc);
    if (R_FAILED(a.after.accuracy_rc) || !a.after.accuracy) return "playguard/clock/applied_pending"_i18n;
    return "playguard/clock/applied_ok"_i18n;
}

// The history entry and the "after" report, then `message` for the user.
void written(const std::string& source, const TimeApply& a, const std::string& before, std::string message,
             const std::function<void(const std::string&)>& done)
{
    if (a.write_attempted && R_SUCCEEDED(a.write_rc)) history_flow::record_event("clock", "", ui::time_text(a.target));
    if (a.write_attempted && R_FAILED(a.write_rc)) history_flow::record_failure("clock", a.write_rc, {}, {}, "", ui::time_text(a.target));
    diagnostic::save(std::format(
        "=== Network clock change ===\nserver={}\ntarget_utc={}\nopen_rc=0x{:08X} write_attempted={} write_rc=0x{:08X}\n"
        "verify_attempted={} verify_rc=0x{:08X} verified={} readback={}\nrefused_automatic={}\n\n--- Before ---\n{}\n--- After ---\n{}",
        source, a.target, (unsigned)a.open_rc, a.write_attempted, (unsigned)a.write_rc,
        a.verify_attempted, (unsigned)a.verify_rc, a.verified, a.readback, a.refused_automatic,
        before, diagnostic::current_report()));
    done(message);
}

// After the confirmation: the write itself, with its reports.
void write_clock(const Measurement& m, const std::string& before, std::function<void(const std::string&)> done)
{
    if (seconds_left(m) <= 0) {
        done("playguard/clock/expired"_i18n);
        return;
    }
    TimeApply a;
    time_clock_apply(projected_now(m), &a);
    written(m.server, a, before, result_message(a), done);
}

// The same with no server: the network clock takes the console clock.
void copy_clock(const std::string& before, std::function<void(const std::string&)> done)
{
    TimeApply a;
    time_clock_set_to_user(&a);
    std::string message = result_message(a);
    // Set with "Synchronize" off: turning it on now no longer moves the console clock.
    if (a.verified && (R_FAILED(a.after.automatic_rc) || !a.after.automatic))
        message += "playguard/clock/copied_sync_on"_i18n;
    written("console clock (no server answered)", a, before, message, done);
}

// A "before" report first, so the change can be analysed, then `write`. When
// the SD card refuses it (full, read-only), ask rather than give up.
void with_before_report(std::function<void(const std::string& before)> write)
{
    const std::string before = diagnostic::current_report();
    std::string err;
    if (diagnostic::save("=== Before network clock change ===\n" + before, &err).empty()) {
        aurore::sync([before, err, write]() {
            ui::confirm(aurore::getStr("playguard/clock/report_err_ask", err), "playguard/clock/apply_confirm"_i18n,
                        [before, write]() { write(before); });
        });
        return;
    }
    write(before);
}

// A spinner and what is being asked; Cancel (or B) sets `cancelled`.
aurore::Dialog* progress_dialog(const std::string& text, std::shared_ptr<std::atomic<bool>> cancelled)
{
    auto* dialog = new aurore::Dialog(cadran::wait_body(text));
    ui::add_cancel(dialog, [cancelled]() { *cancelled = true; });
    return dialog;
}
}   // namespace

std::string current_server()
{
    const auto& cfg = config::get();
    return cfg.ntp_server.empty() ? ntp::default_server_for_console() : cfg.ntp_server;
}

void measure(const std::string& server, std::function<void(const Measurement&)> done)
{
    std::vector<std::string> hosts = { server };
    for (auto& h : ntp::cross_check_servers(server)) hosts.push_back(h);
    // In applet mode (opened from the album) Aurore opens only two network
    // sessions: a third query in parallel would wait for one anyway.
    SysInfo si;
    sysinfo_get(&si);
    const bool in_series = si.applet_mode;
    auto run = [hosts, done, in_series]() {
        // Query the servers in parallel so an unreachable one costs a single
        // timeout (one after the other in applet mode, or when no thread can
        // be had).
        std::vector<ntp::Reply> replies(hosts.size());
        // Each stops waiting when the app quits: it waits for this thread.
        std::vector<std::thread> workers;
        // Joined however this ends: an exception (from a query made here, or
        // the vector growing) with a thread still joinable would end the app.
        struct JoinAll
        {
            std::vector<std::thread>& threads;
            ~JoinAll()
            {
                for (auto& t : threads)
                    if (t.joinable()) t.join();
            }
        } join_all{ workers };
        for (size_t i = 0; i < hosts.size(); i++) {
            if (in_series) {
                if (ui::quitting()) break;
                replies[i] = ntp::fetch(hosts[i], 2500, 2, ui::quitting);
                continue;
            }
            try {
                workers.emplace_back([&replies, &hosts, i]() { replies[i] = ntp::fetch(hosts[i], 2500, 2, ui::quitting); });
            } catch (const std::system_error& e) {
                aurore::Logger::warning("NTP: no thread for {} ({}), querying it here", hosts[i], e.what());
                replies[i] = ntp::fetch(hosts[i], 2500, 2, ui::quitting);
            }
        }
        for (auto& w : workers) w.join();
        if (ui::quitting()) return;
        aurore::sync([hosts, replies, done]() {
            // Bring every sample to the same instant, then take the median.
            const auto ref = std::chrono::steady_clock::now();
            TimeSnapshot s;
            time_clock_snapshot(&s);
            const auto sum = clock_logic::summarize(replies, ref, R_SUCCEEDED(s.network_rc)
                                                    ? std::optional<uint64_t>(s.network_time) : std::nullopt);
            Measurement m;
            m.server = hosts.front();
            m.at     = ref;
            m.offset_known    = sum.offset_known;
            m.network_seconds = m.offset_known ? s.network_time : 0;
            for (size_t i = 0; i < hosts.size(); i++) {
                const auto& r = replies[i];
                const auto& h = sum.hosts[i];
                ServerLine line;
                line.host = hosts[i];
                line.ok   = r.ok;
                if (r.ok) {
                    line.unix_seconds = (uint64_t)h.estimate;
                    line.offset = m.offset_known ? signed_seconds(h.offset) : "?";
                    m.report += aurore::getStr("playguard/clock/result_line_ok", hosts[i], ui::time_text((uint64_t)h.estimate), line.offset);
                } else {
                    line.error = ntp_error_text(r);
                    m.report += aurore::getStr("playguard/clock/result_line_err", hosts[i], line.error);
                    aurore::Logger::info("NTP {}: {}", hosts[i], r.error);
                }
                m.lines.push_back(line);
                m.report += "\n";
            }
            if (!sum.ok) {
                m.report += "playguard/clock/result_none"_i18n;
                done(m);
                return;
            }
            m.ok           = true;
            m.unix_seconds = sum.median;
            m.spread       = sum.spread;
            if (m.offset_known) m.offset = sum.offset;
            const std::string diff = m.offset_known ? signed_seconds(m.offset) : "?";
            m.report += aurore::getStr("playguard/clock/result_summary", ui::time_text(m.unix_seconds), diff);
            if (m.spread > 5) m.warning = aurore::getStr("playguard/clock/result_spread", (int)m.spread);
            done(m);
        });
    };
    // Its spinner is on screen: not behind the play log or the game icons.
    ui::in_background("NTP", run);
}

std::string offset_text(int64_t offset)
{
    // Within a second: the servers' answers are rounded to the second.
    if (offset >= -1 && offset <= 1) return "playguard/clock/on_time"_i18n;
    const uint64_t a = offset < 0 ? (uint64_t)(-(offset + 1)) + 1 : (uint64_t)offset;
    std::string span;
    if (a >= 86400) span = aurore::getStr("playguard/clock/days", (unsigned long long)(a / 86400));
    else if (a >= 3600) span = aurore::getStr("cadran/common/hours_minutes", (int)(a / 3600), std::format("{:02}", (int)(a % 3600 / 60)));
    else if (a >= 60) span = aurore::getStr("playguard/common/min_sec", (int)(a / 60), (int)(a % 60));
    else span = aurore::getStr("playguard/common/sec", (int)a);
    // The servers ahead of the network clock: it is behind.
    return aurore::getStr(offset > 0 ? "playguard/clock/behind" : "playguard/clock/ahead", span);
}

int64_t seconds_left(const Measurement& m)
{
    if (!m.ok) return 0;
    const int64_t e = elapsed_s(m);
    return e < 0 ? 0 : LIFETIME_S - e;
}

uint64_t projected_now(const Measurement& m)
{
    const int64_t e = elapsed_s(m);
    return m.unix_seconds + (uint64_t)(e < 0 ? 0 : e);
}

void apply(const Measurement& m, std::function<void(const std::string& message)> done)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    // Setting the clock would be refused: say so now, not after the confirmation.
    if (R_SUCCEEDED(s.automatic_rc) && !s.automatic) {
        ui::refuse(NXM_RC_AUTOCORRECT_OFF);
        return;
    }
    if (!m.ok) {
        ui::info("playguard/clock/need_measure"_i18n);
        return;
    }
    if (seconds_left(m) <= 0) {
        ui::info("playguard/clock/expired"_i18n);
        return;
    }
    const uint64_t preview = projected_now(m);
    char utc[48];
    time_format_utc(preview, utc, sizeof(utc));
    std::string body = aurore::getStr("playguard/clock/confirm_apply", ui::time_text(preview), std::string(utc));
    if (m.spread > 5) body += aurore::getStr("playguard/clock/confirm_apply_spread", (int)m.spread);
    // Measured on a console (docs/parental-controls.md): the play timer
    // starts the day over when the clock changes. Say so before, not after.
    body += "playguard/clock/confirm_apply_resets"_i18n;

    auto finish = [done](const std::string& message) {
        if (done) done(message);
        ui::info(message);
    };
    ui::confirm(body, "playguard/clock/apply_confirm"_i18n, [m, finish]() {
        with_before_report([m, finish](const std::string& before) { write_clock(m, before, finish); });
    });
}

void apply_console_clock(std::function<void(const std::string& message)> done)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    switch (offline_clock_logic::offer(false, s)) {
        case offline_clock_logic::Offer::Unreadable:
            ui::error("playguard/clock/applied_err"_i18n, R_FAILED(s.service_rc) ? s.service_rc : s.user_rc);
            return;
        case offline_clock_logic::Offer::Implausible:
            ui::info(aurore::getStr("playguard/clock/copy_implausible", ui::time_text(s.user_time)));
            return;
        default:
            break;
    }
    // No server to check it against: the parent is the check. Say what will
    // be written, and what "Synchronize" does to the console clock.
    const auto notes = offline_clock_logic::notes(s);
    char utc[48];
    time_format_utc(s.user_time, utc, sizeof(utc));
    std::string body = aurore::getStr("playguard/clock/confirm_copy", ui::time_text(s.user_time), std::string(utc));
    body += "playguard/clock/confirm_copy_check"_i18n;
    if (notes.follows_network) body += "playguard/clock/confirm_copy_follows"_i18n;
    if (notes.turn_sync_on) body += "playguard/clock/confirm_copy_sync_after"_i18n;
    body += "playguard/clock/confirm_apply_resets"_i18n;

    auto finish = [done](const std::string& message) {
        if (done) done(message);
        ui::info(message);
    };
    ui::confirm(body, "playguard/clock/apply_confirm"_i18n, [finish]() {
        with_before_report([finish](const std::string& before) { copy_clock(before, finish); });
    });
}

namespace
{
// No server answered on the guided path: the console clock instead, when
// there is one worth copying.
void offer_console_clock(const Measurement& m, std::function<void()> done)
{
    TimeSnapshot s;
    time_clock_snapshot(&s);
    const auto offer = offline_clock_logic::offer(false, s);
    if (offer == offline_clock_logic::Offer::Implausible) {
        ui::info(m.report + "\n\n" + aurore::getStr("playguard/clock/copy_implausible", ui::time_text(s.user_time)));
        return;
    }
    if (offer != offline_clock_logic::Offer::Copy) {
        ui::info(m.report);
        return;
    }
    ui::confirm(m.report + "\n\n" + aurore::getStr("playguard/clock/offline_guided", ui::time_text(s.user_time)),
                "playguard/clock/guided_copy"_i18n, [done]() {
        aurore::sync([done]() { apply_console_clock([done](const std::string&) { if (done) done(); }); });
    });
}
}   // namespace

void guided(std::function<void()> done)
{
    if (ui::refuse_read_only()) return;
    const std::string server = current_server();
    ui::confirm(aurore::getStr("playguard/clock/guided_body", server), "playguard/clock/guided_measure"_i18n, [server, done]() {
        aurore::sync([server, done]() {
            auto cancelled = std::make_shared<std::atomic<bool>>(false);
            auto* progress = progress_dialog(aurore::getStr("playguard/clock/measuring", server), cancelled);
            progress->open();
            measure(server, [progress, cancelled, done](const Measurement& m) {
                if (*cancelled) return;   // the dialog is already closed
                *cancelled = true;
                progress->close([m, done]() {
                    aurore::sync([m, done]() {
                        if (!m.ok) {
                            offer_console_clock(m, done);
                            return;
                        }
                        apply(m, [done](const std::string&) { if (done) done(); });
                    });
                });
            });
        });
    });
}

}   // namespace clock_flow
