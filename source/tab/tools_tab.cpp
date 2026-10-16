// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/tools_tab.hpp"

#include <fmt/format.h>

#include "action/backup_flow.hpp"
#include "action/cleanup_flow.hpp"
#include "action/dev_build_flow.hpp"
#include "action/github_login_flow.hpp"
#include "action/fw_gate.hpp"
#include "action/history_flow.hpp"
#include "action/log_prune_flow.hpp"
#include "action/pt_block_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_log_flow.hpp"
#include "action/sys_notes_flow.hpp"
#include "action/sys_update_flow.hpp"
#include "action/upload_flow.hpp"
#include "activity/diagnostic_activity.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/game_check_activity.hpp"
#include "activity/history_activity.hpp"
#include "activity/onboarding_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/diagnostics.hpp"
#include "util/github_auth.hpp"
#include "util/patches.hpp"
#include "util/pt_log.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace
{
std::string keep_text(int keep)
{
    return keep == 0 ? "playguard/tools/backup_keep_all"_i18n : brls::getStr("playguard/tools/backup_keep_n", keep);
}

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

ToolsTab::ToolsTab()
    : TabBase("xml/tab/tools.xml")
{
    export_note->setSingleLine(false);
    backup_note->setSingleLine(false);
    storage_note->setSingleLine(false);
    serial_note->setSingleLine(false);
    patches_note->setSingleLine(false);

    first_steps->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new OnboardingActivity());
        return true;
    });
    history->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new HistoryActivity());
        return true;
    });

    // Ⓐ on the serial number shows / hides the masked digits.
    serial->registerAction("playguard/tools/serial_show"_i18n, brls::BUTTON_A, [this](brls::View*) {
        this->serial_revealed = !this->serial_revealed;
        this->refresh();
        return true;
    }, false, false, brls::SOUND_CLICK);
    export_note->setText(brls::getStr("playguard/tools/export_note", paths::logs_dir()));

    games->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new GameCheckActivity());
        return true;
    });

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

    // Saving a backup only writes to the SD card: also offered in read-only mode.
    backup_note->setText(brls::getStr("playguard/tools/backup_note", paths::backups_dir()));
    backup_save->registerClickAction([this](brls::View*) {
        backup_flow::save_now();
        this->refresh();
        return true;
    });
    backup_restore->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        backup_flow::choose_and_restore([this]() { this->refresh(); });
        return true;
    });

    backup_keep->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        int selected = 0;
        for (size_t i = 0; i < sizeof(config::BACKUP_KEEP) / sizeof(config::BACKUP_KEEP[0]); i++) {
            labels.push_back(keep_text(config::BACKUP_KEEP[i]));
            if (config::BACKUP_KEEP[i] == config::get().backup_keep) selected = (int)i;
        }
        ui::pick("playguard/tools/backup_keep"_i18n, labels, selected, [this](int i) {
            config::get().backup_keep = config::BACKUP_KEEP[i];
            ui::save_config();
            this->refresh();
        });
        return true;
    });

    // The release notes of each (installed and newer versions).
    const std::pair<brls::DetailCell*, sys_notes::Which> notes_cells[] = {
        { fw.getView(), sys_notes::Which::Firmware },
        { ams.getView(), sys_notes::Which::Atmosphere },
        { hekate.getView(), sys_notes::Which::Hekate },
    };
    for (const auto& n : notes_cells) {
        const sys_notes::Which which = n.second;
        n.first->registerAction("playguard/sys_notes/hint"_i18n, brls::BUTTON_A, [which](brls::View*) {
            sys_notes_flow::open(which);
            return true;
        });
    }
    sys_advice->setSingleLine(false);

    // On a firmware newer than the checked one, the firmware screen again.
    compat->registerClickAction([](brls::View*) {
        if (fw_gate::needed()) brls::Application::pushActivity(new FirmwareGateActivity());
        else ui::info("playguard/tools/compat_ok_info"_i18n);
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
    dev_github->registerClickAction([this](brls::View*) {
        github_login_flow::open([this]() { this->refresh(); });
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
}

void ToolsTab::refresh()
{
    const auto& cfg = config::get();
    backup_keep->setDetailText(keep_text(cfg.backup_keep));
    logs_keep->setDetailText(logs_keep_text(cfg.logs_keep));
    logs_keep_days->setDetailText(logs_days_text(cfg.logs_keep_days));
    logs_keep_mb->setDetailText(logs_mb_text(cfg.logs_keep_mb));
    const size_t backups = backup_flow::count();
    backup_restore->setDetailText(backups ? brls::getStr("playguard/tools/backup_count", (int)backups) : "");
    const size_t changes = history_flow::load().size();
    history->setDetailText(changes ? brls::getStr("playguard/history/count", (int)changes) : "");

    SysInfo si;
    sysinfo_get(&si);
    const bool ro  = app::read_only();
    const bool dev = app::dev_mode();
    dev_mode->setOn(dev, false);
    dev_read_only->setOn(ro, false);
    dev_pt_log->setOn(cfg.pt_log, false);
    // With the newer release the last update check found (sys_update_flow),
    // and how many releases behind when more than one.
    bool newer = false;
    fw->setDetailText(sys_notes_flow::status_text(sys_notes::Which::Firmware, &newer));
    fw->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    if (si.ams_valid) {
        ams->setDetailText(sys_notes_flow::status_text(sys_notes::Which::Atmosphere, &newer));
        ams->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    } else {
        ams->setDetailText("playguard/common/unavailable"_i18n);
    }
    // hekate: only when its bootloader/update.bin is on the SD card.
    const std::string hk = sys_update_flow::installed_hekate();
    hekate->setDetailText(sys_notes_flow::status_text(sys_notes::Which::Hekate, &newer));
    hekate->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    ui::set_visible(hekate.getView(), !hk.empty());
    // A newer firmware Atmosphère or hekate does not support yet, or only in
    // a newer release (their notes say).
    const std::string advice = sys_notes_flow::firmware_advice();
    sys_advice->setText(advice);
    NVGcolor c = ui::color_neutral();
    compat->setDetailText(ui::compat_text(si, &c));
    compat->setDetailTextColor(c);

    storage->setDetailText(ui::storage_text(si));
    const std::string other = ui::other_storage_note(si);
    storage_note->setText(other);
    c = ui::color_neutral();
    blank->setDetailText(ui::blank_text(si, &c));
    blank->setDetailTextColor(c);
    const bool maskable = si.serial_valid && si.serial[0] && !(si.blank_valid && si.blank);
    serial->setDetailText(ui::serial_text(si, this->serial_revealed));
    serial->setDetailTextColor(ui::serial_warning(si) ? ui::color_warn() : ui::color_neutral());
    serial->setActionAvailable(brls::BUTTON_A, maskable);
    serial->updateActionHint(brls::BUTTON_A, this->serial_revealed ? "playguard/tools/serial_hide"_i18n
                                                                   : "playguard/tools/serial_show"_i18n);
    brls::Application::getGlobalHintsUpdateEvent()->fire();   // redraw the footer hints

    const patches::Report& report = ui::patch_report();
    c = ui::color_neutral();
    game_patches->setDetailText(ui::patches_text(report, &c));
    game_patches->setDetailTextColor(c);
    bool warn = false;
    const std::string note = ui::patches_note(report, si, &warn);
    patches_note->setText(note);
    patches_note->setTextColor(warn ? ui::color_warn() : ui::color_note());
    ui::set_visible_all({ { storage_note.getView(), !other.empty() },
                          { sys_advice.getView(), !advice.empty() },
                          { serial_note.getView(), ui::serial_warning(si) },
                          { patches_note.getView(), !note.empty() },
                          { dev_header.getView(), dev },
                          { dev_mode.getView(), dev },
                          { dev_read_only.getView(), dev },
                          { dev_pt_log.getView(), dev },
                          { dev_pt_log_note.getView(), dev },
                          { dev_clear_cache.getView(), dev },
                          { dev_clear_logs.getView(), dev },
                          { dev_report.getView(), dev },
                          { dev_pt_block.getView(), dev },
                          { dev_github.getView(), dev },
                          { dev_build.getView(), dev },
                          { dev_gate.getView(), dev && fw_gate::needed() },
                          { dev_forget.getView(), dev } });
    dev_github->setDetailText(github_auth::token().empty() ? "playguard/github/not_signed_in"_i18n
                                                           : "playguard/github/signed_in_short"_i18n);
    // Read-only: restoring would write; it stays in sight, greyed.
    ui::show_writable(backup_restore, !ro);
}

brls::View* ToolsTab::create()
{
    return new ToolsTab();
}
