// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/history_tab.hpp"

#include "action/backup_flow.hpp"
#include "action/history_flow.hpp"
#include "activity/history_activity.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace
{
std::string keep_text(int keep)
{
    return keep == 0 ? "playguard/tools/backup_keep_all"_i18n : brls::getStr("playguard/tools/backup_keep_n", keep);
}
}   // namespace

HistoryTab::HistoryTab()
    : TabBase("xml/tab/history.xml", false), list(this)
{
    backup_note->setSingleLine(false);

    history->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new HistoryActivity());
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
    this->list.tiles({ { "hs_backup_save", "save" }, { "hs_backup_restore", "restore" } }, false);
    this->list.apply({ { "hs_history", "history" }, { "hs_backup_keep", "folder" } });
}

void HistoryTab::refresh()
{
    const auto& cfg = config::get();
    backup_keep->setDetailText(keep_text(cfg.backup_keep));
    const size_t backups = backup_flow::count();
    backup_restore->setDetailText(backups ? brls::getStr("playguard/tools/backup_count", (int)backups) : "");
    const size_t changes = history_flow::count();
    history->setDetailText(ui::opens(changes == 1 ? "playguard/history/count_one"_i18n
                                         : changes ? brls::getStr("playguard/history/count", (int)changes) : ""));
    // Read-only: restoring would write; it stays in sight, greyed.
    ui::show_writable(backup_restore, !app::read_only());
    this->list.sync();
    cadran::set_pill("");
}

brls::View* HistoryTab::create()
{
    return new HistoryTab();
}
