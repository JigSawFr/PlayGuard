// console_actions — the shared parental-control actions (see console_actions.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/console_actions.hpp"

#include <aurore.hpp>

#include "action/backup_flow.hpp"
#include "action/history_flow.hpp"
#include "action/pairing_state.hpp"
#include "action/session_pause.hpp"
#include "ui/ui.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace console_actions
{

namespace
{
// What the console answered, in the history (a refusal too) and on screen.
void record(const char* kind, Result rc, const std::string& source)
{
    if (R_SUCCEEDED(rc)) history_flow::record_event(kind, source);
    else history_flow::record_failure(kind, rc, {}, {}, source);
}
}   // namespace

void set_pin(const std::string& source, std::function<void()> after)
{
    if (ui::refuse_read_only()) return;
    // Blocks while the system PIN applet is shown; the session is released first.
    Result rc = pctl_set_pin();
    aurore::Logger::info("pctl_set_pin returned 0x{:08X}", (unsigned)rc);
    // Not recorded when it fails. Left with B: nothing changed, nothing to say.
    if (R_SUCCEEDED(rc)) history_flow::record_event("pin", source);
    if (rc != NXM_RC_CANCELLED)
        ui::notify_result(rc, "playguard/security/pin_ok"_i18n, "playguard/security/pin_err"_i18n);
    if (after) after();
}

void unlink(const std::string& source, std::function<void()> after)
{
    if (ui::refuse_read_only()) return;
    // The block comparison needs the link: last chance to help decode the
    // settings. With Atmosphère's DNS blocking its server, the app neither
    // overwrites anything nor helps decode: the question alone.
    const std::string question = "playguard/pairing/unlink_body"_i18n;
    const std::string body = pairing_state::dns_blocked() ? question.substr(0, question.find("\n\n"))
                                                          : question + "\n\n" + "playguard/pairing/decode_note"_i18n;
    ui::confirm_danger(body, "playguard/pairing/unlink_confirm"_i18n, [source, after]() {
        Result rc = pctl_delete_pairing();
        record("unlink", rc, source);
        ui::notify_result(rc, "playguard/pairing/unlinked"_i18n, "playguard/pairing/unlink_err"_i18n);
        if (after) after();
    });
}

void delete_all(const std::string& source, std::function<void()> after)
{
    if (ui::refuse_read_only()) return;
    ui::confirm_danger("playguard/security/delete_body"_i18n, "playguard/security/delete_confirm"_i18n, [source, after]() {
        // Second, separate confirmation with a different button: this cannot be undone.
        aurore::sync([source, after]() {
            ui::confirm_danger("playguard/security/delete_body2"_i18n, "playguard/security/delete_confirm2"_i18n, [source, after]() {
                // A backup first, so the settings can come back (History › Restore a backup).
                backup_flow::backup_then([source, after]() {
                    Result rc = pctl_delete_parental_controls();
                    record("delete", rc, source);
                    ui::notify_result(rc, "playguard/security/deleted"_i18n, "playguard/security/delete_err"_i18n);
                    if (after) after();
                });
            });
        });
    });
}

void relock(const std::string& source, std::function<void()> after)
{
    if (ui::refuse_read_only()) return;
    session_pause::stop();   // by hand: not paused again until the PIN is entered again
    Result rc = pctl_relock();
    record("relock", rc, source);
    ui::notify_result(rc, "playguard/toast/relocked"_i18n, "playguard/toast/relock_err"_i18n);
    if (after) after();
}

}   // namespace console_actions
