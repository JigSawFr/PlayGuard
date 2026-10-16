// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/preferences_tab.hpp"

#include <vector>

#include "action/notices.hpp"
#include "action/sys_update_flow.hpp"
#include "action/update_flow.hpp"
#include "tab/help_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace
{
const char* THEMES[]    = { "system", "light", "dark" };

template <size_t N>
int index_of(const char* const (&list)[N], const std::string& value)
{
    for (size_t i = 0; i < N; i++)
        if (value == list[i]) return (int)i;
    return 0;
}
}   // namespace

PreferencesTab::PreferencesTab()
    : TabBase("xml/tab/preferences.xml")
{
    help->setDetailText(ui::opens());
    help->registerClickAction([](brls::View*) {
        ui::open_screen(new HelpTab(), "playguard/prefs/help"_i18n);
        return true;
    });

    language->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (const char* l : config::LANGUAGES) labels.push_back(brls::getStr(std::string("playguard/tools/languages/") + l));
        ui::pick("playguard/tools/language"_i18n, labels, index_of(config::LANGUAGES, config::get().language), [this](int i) {
            const bool changed = config::get().language != config::LANGUAGES[i];
            config::get().language = config::LANGUAGES[i];
            ui::save_config();
            this->refresh();
            if (changed) ui::offer_restart();
        });
        return true;
    });

    theme->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (const char* t : THEMES) labels.push_back(brls::getStr(std::string("playguard/tools/themes/") + t));
        ui::pick("playguard/tools/theme"_i18n, labels, index_of(THEMES, config::get().theme), [this](int i) {
            const bool changed = config::get().theme != THEMES[i];
            config::get().theme = THEMES[i];
            ui::save_config();
            this->refresh();
            if (changed) ui::offer_restart();
        });
        return true;
    });

    start_tab->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (const char* t : config::START_TABS) labels.push_back(brls::getStr(std::string("playguard/tabs/") + t));
        ui::pick("playguard/tools/start_tab"_i18n, labels, index_of(config::START_TABS, config::get().start_tab), [this](int i) {
            config::get().start_tab = config::START_TABS[i];
            ui::save_config();
            this->refresh();
        });
        return true;
    });

    support_reminder->init("playguard/prefs/support_reminder"_i18n, config::get().support_reminder, [](bool on) {
        config::get().support_reminder = on;
        ui::save_config();
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
    // Updates: PlayGuard's own (About keeps "Check for updates" by the version).
    update_daily->init("playguard/tools/update_daily"_i18n, config::get().update_daily, [](bool on) {
        config::get().update_daily = on;
        ui::save_config();
    });
    update_via->registerClickAction([this](brls::View*) {
        update_flow::choose_method([this]() { this->refresh(); });
        return true;
    });
}

void PreferencesTab::refresh()
{
    const auto& cfg = config::get();
    language->setDetailText(brls::getStr("playguard/tools/languages/" + std::string(config::LANGUAGES[index_of(config::LANGUAGES, cfg.language)])));
    theme->setDetailText(brls::getStr("playguard/tools/themes/" + std::string(THEMES[index_of(THEMES, cfg.theme)])));
    start_tab->setDetailText(brls::getStr(std::string("playguard/tabs/") + config::START_TABS[index_of(config::START_TABS, cfg.start_tab)]));
    intro->setOn(cfg.intro, false);
    support_reminder->setOn(cfg.support_reminder, false);
    notices_system->setOn(cfg.notices_system, false);
    notices_summary->setOn(cfg.notices_summary, false);
    notices_reminders->setOn(cfg.notices_reminders, false);
    update_daily->setOn(cfg.update_daily, false);
    update_via->setDetailText(update_flow::method_text());
}

brls::View* PreferencesTab::create()
{
    return new PreferencesTab();
}
