// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/help_tab.hpp"

#include <fmt/format.h>

#include "action/cleanup_flow.hpp"
#include "action/dev_build_flow.hpp"
#include "action/github_login_flow.hpp"
#include "action/fw_gate.hpp"
#include "action/log_prune_flow.hpp"
#include "action/pairing_state.hpp"
#include "action/pt_block_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/frame_stats_flow.hpp"
#include "action/pt_log_flow.hpp"
#include "action/upload_flow.hpp"
#include "activity/diagnostic_activity.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "app.hpp"
#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/diagnostics.hpp"
#include "util/github_auth.hpp"
#include "util/frame_stats.hpp"
#include "util/pt_log.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace
{
// The limits of the reports and crash files kept (config logs_keep*).
std::string logs_keep_text(int n)
{
    return n == 0 ? "playguard/tools/logs_keep_all"_i18n : brls::getStr("playguard/tools/logs_keep_n", n);
}
std::string logs_days_text(int n)
{
    return n == 0 ? "playguard/tools/logs_days_never"_i18n : brls::getStr("playguard/tools/logs_days_n", n);
}
std::string logs_mb_text(int n)
{
    return n == 0 ? "playguard/common/no_limit"_i18n : brls::getStr("playguard/dev/size_mb", n);
}

// A picker over `values` for `field`; the limits apply at once.
template <size_t N>
void pick_limit(const std::string& title, const int (&values)[N], int& field, std::string (*text)(int),
                std::function<void()> done)
{
    std::vector<std::string> labels;
    int selected = 0;
    for (size_t i = 0; i < N; i++) {
        labels.push_back(text(values[i]));
        if (values[i] == field) selected = (int)i;
    }
    ui::pick(title, labels, selected, [&values, &field, done](int i) {
        field = values[i];
        ui::save_config();
        log_prune_flow::run();
        done();
    });
}
}   // namespace

HelpTab::HelpTab()
    : TabBase("xml/tab/help.xml", false)
{
    export_note->setSingleLine(false);

    first_steps->setDetailText(ui::opens());
    first_steps->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new OnboardingActivity());
        return true;
    });
    export_note->setText(brls::getStr("playguard/tools/export_note", paths::logs_dir()));

    export_cell->registerClickAction([](brls::View*) {
        std::string err;
        std::string path = diagnostic::save(diagnostic::current_report(), &err);
        if (path.empty()) ui::error("playguard/toast/diag_err"_i18n + ": " + err);
        else ui::notify(brls::getStr("playguard/toast/diag_saved", path));
        return true;
    });

    // Only reads and the network: also offered in read-only mode.
    upload_note->setSingleLine(false);
    upload->registerClickAction([](brls::View*) {
        upload_flow::choose();
        return true;
    });
    // The account a report goes with: signing out takes it off the console.
    // Shown once signed in (Send a report online signs in without developer
    // mode), and to a developer, whose builds need it.
    github->registerClickAction([this](brls::View*) {
        github_login_flow::open([this]() { this->refresh(); });
        return true;
    });

    // Only the SD card: also offered in read-only mode.
    logs_note->setSingleLine(false);
    logs_note->setText(brls::getStr("playguard/tools/logs_note", paths::logs_dir()));
    logs_keep->registerClickAction([this](brls::View*) {
        pick_limit("playguard/tools/logs_keep"_i18n, config::LOGS_KEEP, config::get().logs_keep, logs_keep_text,
                   [this]() { this->refresh(); });
        return true;
    });
    logs_keep_days->registerClickAction([this](brls::View*) {
        pick_limit("playguard/tools/logs_keep_days"_i18n, config::LOGS_KEEP_DAYS, config::get().logs_keep_days,
                   logs_days_text, [this]() { this->refresh(); });
        return true;
    });
    logs_keep_mb->registerClickAction([this](brls::View*) {
        pick_limit("playguard/tools/logs_keep_mb"_i18n, config::LOGS_KEEP_MB, config::get().logs_keep_mb, logs_mb_text,
                   [this]() { this->refresh(); });
        return true;
    });

    // Developer tools.
    // Turning a developer switch off asks first: the switch goes back on
    // until it is confirmed. Turning one on does not ask.
    dev_mode->init("playguard/dev/mode"_i18n, app::dev_mode(), [this](bool on) {
        if (on) {
            app::set_dev_mode(true, true);
            ui::on_mode_changed();
            return;
        }
        this->dev_mode->setOn(true, false);
        ui::confirm("playguard/dev/mode_off_body"_i18n, "playguard/dev/mode_off_confirm"_i18n, []() {
            app::set_dev_mode(false, true);
            ui::on_mode_changed();
        });
    });
    dev_read_only->init("playguard/dev/read_only"_i18n, app::read_only(), [this](bool on) {
        if (on) {
            app::set_read_only(true);
            ui::on_mode_changed();
            return;
        }
        this->dev_read_only->setOn(true, false);
        const auto allow = []() {
            app::set_read_only(false);
            ui::on_mode_changed();
            pt_flow::relock_if_interrupted();   // kept while read-only
        };
        // Untested firmware: same warning as "Continue at my own risk".
        if (fw_gate::needed())
            ui::confirm_danger(brls::getStr("playguard/fw_gate/risk_body", fw_gate::firmware()),
                               "playguard/fw_gate/risk_confirm"_i18n, allow);
        else
            ui::confirm("playguard/dev/read_only_off_body"_i18n, "playguard/dev/read_only_off_confirm"_i18n, allow);
    });
    dev_pt_log->init("playguard/dev/pt_log"_i18n, config::get().pt_log, [](bool on) {
        config::get().pt_log = on;
        ui::save_config();
        pt_log_flow::apply();
        if (on) ui::notify(brls::getStr("playguard/dev/pt_log_on", pt_log::path()));
    });
    dev_frame_stats->init("playguard/dev/frame_stats"_i18n, config::get().frame_stats, [this](bool on) {
        config::get().frame_stats = on;
        ui::save_config();
        frame_stats_flow::apply();
        if (on) ui::notify(brls::getStr("playguard/dev/frame_stats_on", frame_stats::path()));
        this->update_frame_stats();
    });
    dev_frame_stats_last->setSingleLine(false);
    // Cadran's background glow, off to weigh it with the frame statistics.
    dev_glow->init("playguard/dev/glow"_i18n, config::get().cadran_glow, [](bool on) {
        config::get().cadran_glow = on;
        ui::save_config();
    });
    this->stats_timer.setPeriod(1000);
    this->stats_timer.setCallback([this]() { this->update_frame_stats(); });
    dev_clear_cache->registerClickAction([](brls::View*) {
        cleanup_flow::clear_cache();
        return true;
    });
    dev_clear_logs->registerClickAction([](brls::View*) {
        cleanup_flow::clear_logs();
        return true;
    });
    dev_report->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new DiagnosticActivity());
        return true;
    });
    dev_pt_block->registerClickAction([](brls::View*) {
        pt_block_flow::open();
        return true;
    });
    dev_build->registerClickAction([](brls::View*) {
        dev_build_flow::open();
        return true;
    });
    dev_gate->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new FirmwareGateActivity());
        return true;
    });
    dev_forget->registerClickAction([](brls::View*) {
        ui::confirm("playguard/dev/forget_body"_i18n, "playguard/dev/forget_confirm"_i18n, []() {
            fw_gate::forget();
            ui::notify("playguard/dev/forgotten"_i18n);
        });
        return true;
    });

    // The Cadran look, with an icon per line (the screen's skin would dress
    // it without).
    if (cadran::enabled())
        cadran::List(this, true).apply({ { "hp_first_steps", "sparkle" }, { "hp_export", "file" }, { "hp_upload", "upload" },
                                         { "hp_github", "user" }, { "hp_logs_keep", "folder" }, { "hp_logs_keep_days", "calendar" },
                                         { "hp_logs_keep_mb", "save" }, { "hp_dev_mode", "code" }, { "hp_dev_read_only", "eye" },
                                         { "hp_dev_pt_log", "list" }, { "hp_dev_frame_stats", "activity" }, { "hp_dev_glow", "sparkle" },
                                         { "hp_dev_clear_cache", "trash" }, { "hp_dev_clear_logs", "trash" },
                                         { "hp_dev_report", "file" }, { "hp_dev_pt_block", "timer" }, { "hp_dev_gate", "shield" },
                                         { "hp_dev_forget", "refresh" }, { "hp_dev_build", "download" } });
}

void HelpTab::refresh()
{
    const auto& cfg = config::get();
    logs_keep->setDetailText(logs_keep_text(cfg.logs_keep));
    logs_keep_days->setDetailText(logs_days_text(cfg.logs_keep_days));
    logs_keep_mb->setDetailText(logs_mb_text(cfg.logs_keep_mb));
    const bool ro  = app::read_only();
    const bool dev = app::dev_mode();
    dev_mode->setOn(dev, false);
    dev_read_only->setOn(ro, false);
    dev_pt_log->setOn(cfg.pt_log, false);
    dev_frame_stats->setOn(cfg.frame_stats, false);
    dev_glow->setOn(cfg.cadran_glow, false);
    this->update_frame_stats();
    const bool signed_in = !github_auth::token().empty();
    // The block comparison reads what the phone app changes: only while it
    // is linked and can sync (the status read only for the developer tools).
    bool phone_syncs = false;
    if (dev) {
        PctlStatus s;
        pctl_status_fetch(&s);
        phone_syncs = pairing_state::syncs(s);
    }
    ui::set_visible_all({ { github.getView(), dev || signed_in },
                          { dev_header.getView(), dev },
                          { dev_mode.getView(), dev },
                          { dev_read_only.getView(), dev },
                          { dev_pt_log.getView(), dev },
                          { dev_pt_log_note.getView(), dev },
                          { dev_frame_stats.getView(), dev },
                          { dev_frame_stats_note.getView(), dev },
                          { dev_frame_stats_last.getView(), dev && frame_stats_flow::running() },
                          { dev_glow.getView(), dev && cadran::enabled() },
                          { dev_clear_cache.getView(), dev },
                          { dev_clear_logs.getView(), dev },
                          { dev_report.getView(), dev },
                          { dev_pt_block.getView(), dev && phone_syncs },
                          { dev_build.getView(), dev },
                          { dev_gate.getView(), dev && fw_gate::needed() },
                          { dev_forget.getView(), dev } });
    github->setDetailText(signed_in ? "playguard/github/signed_in_short"_i18n : "playguard/github/not_signed_in"_i18n);
}

// The last second's line under the switch, while the statistics run (hidden
// otherwise); refreshed every second while this tab is on screen.
void HelpTab::update_frame_stats()
{
    const bool on = app::dev_mode() && frame_stats_flow::running();
    if (on && !this->stats_timer.isRunning()) this->stats_timer.start();
    else if (!on && this->stats_timer.isRunning()) this->stats_timer.stop();
    const std::string last = on ? frame_stats_flow::last() : std::string();
    dev_frame_stats_last->setText(last.empty() ? "playguard/dev/frame_stats_wait"_i18n : last);
    ui::set_visible(dev_frame_stats_last.getView(), on);
}
