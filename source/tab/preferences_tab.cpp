// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/preferences_tab.hpp"

#include <vector>

#include "action/notices.hpp"
#include "action/sys_update_flow.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

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

// A tab as the rail or the sidebar names it: the Overview is Cadran's Today.
std::string tab_name(const std::string& tab)
{
    if (tab == "dashboard" && cadran::enabled()) return "playguard/cadran/tabs/today"_i18n;
    return brls::getStr("playguard/tabs/" + tab);
}
}   // namespace

PreferencesTab::PreferencesTab()
    : TabBase("xml/tab/preferences.xml", false)
{
    language->registerClickAction([this](brls::View*) {
        ui::pick_language([this]() { this->refresh(); });
        return true;
    });

    appearance->registerClickAction([this](brls::View*) {
        ui::pick_appearance([this]() { this->refresh(); });
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
        ui::pick("playguard/tools/start_tab"_i18n, labels, index_of(config::START_TABS, config::get().start_tab), [this](int i) {
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
}

void PreferencesTab::refresh()
{
    const auto& cfg = config::get();
    language->setDetailText(ui::language_name());
    theme->setDetailText(ui::theme_name());
    appearance->setDetailText(ui::appearance_name());
    text_size->setDetailText(ui::text_size_name());
    effects->setDetailText(brls::getStr("playguard/prefs/effects_values/" + std::string(config::EFFECTS[index_of(config::EFFECTS, cfg.effects)])));
    start_tab->setDetailText(tab_name(config::START_TABS[index_of(config::START_TABS, cfg.start_tab)]));
    intro->setOn(cfg.intro, false);
    notices_system->setOn(cfg.notices_system, false);
    notices_summary->setOn(cfg.notices_summary, false);
    notices_reminders->setOn(cfg.notices_reminders, false);
    notices_ack_remind->setOn(cfg.notices_ack_remind, false);
    support_reminder->setOn(cfg.support_reminder, false);
}

brls::View* PreferencesTab::create()
{
    return new PreferencesTab();
}
