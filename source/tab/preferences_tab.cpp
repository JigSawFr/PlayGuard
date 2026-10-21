// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/preferences_tab.hpp"

#include <vector>

#include "action/backup_flow.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "action/sys_update_flow.hpp"
#include "activity/history_activity.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace
{
template <size_t N>
int index_of(const char* const (&list)[N], const std::string& value)
{
    for (size_t i = 0; i < N; i++)
        if (value == list[i]) return (int)i;
    return 0;
}

std::string tab_name(const std::string& tab)
{
    return brls::getStr("playguard/tabs/" + tab);
}

std::string keep_text(int keep)
{
    return keep == 0 ? "playguard/tools/backup_keep_all"_i18n : brls::getStr("playguard/tools/backup_keep_n", keep);
}
}   // namespace

PreferencesTab::PreferencesTab()
    : TabBase("xml/tab/preferences.xml", false), list(this)
{
    language->registerClickAction([this](brls::View*) {
        ui::pick_language([this]() { this->refresh(); });
        return true;
    });

    theme->registerClickAction([this](brls::View*) {
        ui::pick_theme([this]() { this->refresh(); });
        return true;
    });
    text_size->registerClickAction([this](brls::View*) {
        ui::pick_text_size([this]() { this->refresh(); });
        return true;
    });
    effects->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (const char* e : config::EFFECTS) labels.push_back(brls::getStr(std::string("playguard/prefs/effects_values/") + e));
        ui::pick("playguard/prefs/effects"_i18n, labels, index_of(config::EFFECTS, config::get().effects), [this](int i) {
            config::get().effects = config::EFFECTS[i];   // the next frame draws by it
            ui::save_config();
            this->refresh();
        });
        return true;
    });

    start_tab->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (const char* t : config::START_TABS) labels.push_back(tab_name(t));
        // Each tab's icon, as the sidebar draws it (ui/cadran_frame.cpp).
        const std::vector<std::string> icons = { "today", "timer", "activity", "shield",
                                                 "lock",  "chip",  "settings", "info" };
        static_assert(sizeof(config::START_TABS) / sizeof(config::START_TABS[0]) == 8, "an icon for each tab");
        ui::pick_grid("playguard/tools/start_tab"_i18n, labels, icons, index_of(config::START_TABS, config::get().start_tab), [this](int i) {
            config::get().start_tab = config::START_TABS[i];
            ui::save_config();
            this->refresh();
        });
        return true;
    });

    intro->init("playguard/prefs/intro"_i18n, config::get().intro, [](bool on) {
        config::get().intro = on;
        ui::save_config();
    });
    // The notification bell's kinds of entries (action/notices).
    notices_system->init("playguard/prefs/notices_system"_i18n, config::get().notices_system, [](bool on) {
        config::get().notices_system = on;
        ui::save_config();
        notices::apply_preferences();
        sys_update_flow::refresh_entries();
    });
    notices_summary->init("playguard/prefs/notices_summary"_i18n, config::get().notices_summary, [](bool on) {
        config::get().notices_summary = on;
        ui::save_config();
        notices::apply_preferences();
    });
    notices_reminders->init("playguard/prefs/notices_reminders"_i18n, config::get().notices_reminders, [](bool on) {
        config::get().notices_reminders = on;
        ui::save_config();
        notices::apply_preferences();
    });
    notices_ack_remind->init("playguard/prefs/notices_ack_remind"_i18n, config::get().notices_ack_remind, [](bool on) {
        config::get().notices_ack_remind = on;
        ui::save_config();
        notices::apply_preferences();
    });
    // Support PlayGuard once a month at start-up (action/support_flow).
    support_reminder->init("playguard/prefs/support_reminder"_i18n, config::get().support_reminder, [](bool on) {
        config::get().support_reminder = on;
        ui::save_config();
    });

    // What PlayGuard changed, and the settings backups.
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

    this->list.tiles({ { "pf_language", "language" }, { "pf_theme", "contrast" }, { "pf_text_size", "eye" },
                       { "pf_start_tab", "home" } }, false);
    this->list.tiles({ { "hs_backup_save", "save" }, { "hs_backup_restore", "restore" } }, false);
    this->list.apply({ { "pf_effects", "sparkle" }, { "pf_intro", "bolt" }, { "pf_notices_system", "chip" }, { "pf_notices_summary", "calendar" },
                       { "pf_notices_reminders", "bell" }, { "pf_notices_ack_remind", "alert" },
                       { "pf_support_reminder", "heart" }, { "hs_history", "history" }, { "hs_backup_keep", "folder" } });
}

void PreferencesTab::refresh()
{
    const auto& cfg = config::get();
    language->setDetailText(ui::language_name());
    theme->setDetailText(ui::theme_name());
    text_size->setDetailText(ui::text_size_name());
    effects->setDetailText(brls::getStr("playguard/prefs/effects_values/" + std::string(config::EFFECTS[index_of(config::EFFECTS, cfg.effects)])));
    start_tab->setDetailText(tab_name(config::START_TABS[index_of(config::START_TABS, cfg.start_tab)]));
    intro->setOn(cfg.intro, false);
    notices_system->setOn(cfg.notices_system, false);
    notices_summary->setOn(cfg.notices_summary, false);
    notices_reminders->setOn(cfg.notices_reminders, false);
    notices_ack_remind->setOn(cfg.notices_ack_remind, false);
    support_reminder->setOn(cfg.support_reminder, false);
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

brls::View* PreferencesTab::create()
{
    return new PreferencesTab();
}
