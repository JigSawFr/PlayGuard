// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/preferences_tab.hpp"

#include <vector>

#include <aurore/cadran/ui/cadran_frame.hpp>

#include "action/notices.hpp"
#include "action/sys_update_flow.hpp"
#include "action/update_flow.hpp"
#include "activity/onboarding_activity.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace aurore::literals;

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
    return aurore::getStr("playguard/tabs/" + tab);
}
}   // namespace

PreferencesTab::PreferencesTab()
    : TabBase("xml/tab/preferences.xml", false), list(this)
{
    language->registerClickAction([this](aurore::View*) {
        ui::pick_language(this->refresher());
        return true;
    });

    theme->registerClickAction([this](aurore::View*) {
        ui::pick_theme(this->refresher());
        return true;
    });
    // The colours: PlayGuard's, Aurore's presets or the user's theme files
    // (docs/themes.md), applied at once.
    palette->registerClickAction([this](aurore::View*) {
        ui::pick_palette(this->refresher());
        return true;
    });
    text_size->registerClickAction([this](aurore::View*) {
        ui::pick_text_size(this->refresher());
        return true;
    });
    effects->registerClickAction([this](aurore::View*) {
        std::vector<std::string> labels;
        for (const char* e : config::EFFECTS) labels.push_back(aurore::getStr(std::string("playguard/prefs/effects_values/") + e));
        // Automatic as Text size's (full on the TV), full, and still.
        const std::vector<std::string> icons = { "tv", "sparkle", "pause" };
        static_assert(sizeof(config::EFFECTS) / sizeof(config::EFFECTS[0]) == 3, "an icon for each choice");
        ui::pick("playguard/prefs/effects"_i18n, labels, index_of(config::EFFECTS, config::get().look.effects), [this](int i) {
            config::get().look.effects = config::EFFECTS[i];   // the next frame draws by it
            ui::save_config();
            this->refresh();
        }, true, {}, icons);
        return true;
    });

    // The main screen's sidebar: open, on its icons, or folding by itself.
    rail->registerClickAction([this](aurore::View*) {
        ui::pick_rail(this->refresher());
        return true;
    });

    start_tab->registerClickAction([this](aurore::View*) {
        std::vector<std::string> labels;
        for (const char* t : config::START_TABS) labels.push_back(tab_name(t));
        // Each tab's icon, as the sidebar draws it (ui/cadran_frame.cpp).
        const std::vector<std::string> icons = { "today", "timer",   "activity", "shield", "lock",
                                                 "history", "chip", "settings", "info" };
        static_assert(sizeof(config::START_TABS) / sizeof(config::START_TABS[0]) == 9, "an icon for each tab");
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

    // How PlayGuard updates (checking now is in About, by the version).
    update_daily->init("playguard/tools/update_daily"_i18n, config::get().update_daily, [](bool on) {
        config::get().update_daily = on;
        ui::save_config();
    });
    update_via->registerClickAction([this](aurore::View*) {
        update_flow::choose_method(this->refresher());
        return true;
    });
    // The guide again (also at the top of About › Help & diagnostics).
    first_steps->setDetailText(ui::opens());
    first_steps->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new OnboardingActivity());
        return true;
    });

    this->list.tiles({ { "pf_language", "language" }, { "pf_theme", "contrast" }, { "pf_text_size", "eye" },
                       { "pf_start_tab", "home" } }, false);
    this->list.apply({ { "pf_palette", "contrast" }, { "pf_effects", "sparkle" }, { "pf_rail", "rail-auto" }, { "pf_intro", "bolt" }, { "pf_notices_system", "chip" }, { "pf_notices_summary", "calendar" },
                       { "pf_notices_reminders", "bell" }, { "pf_notices_ack_remind", "alert" },
                       { "pf_support_reminder", "heart" }, { "pf_update_daily", "calendar" }, { "pf_update_via", "server" },
                       { "pf_first_steps", "sparkle" } });
}

void PreferencesTab::refresh()
{
    const auto& cfg = config::get();
    language->setDetailText(ui::language_name(true));
    theme->setDetailText(ui::theme_name(true));
    palette->setDetailText(ui::palette_name());
    text_size->setDetailText(ui::text_size_name(true));
    effects->setDetailText(aurore::getStr("playguard/prefs/effects_values/" + std::string(config::EFFECTS[index_of(config::EFFECTS, cfg.look.effects)])));
    rail->setDetailText(ui::rail_name());
    start_tab->setDetailText(tab_name(config::START_TABS[index_of(config::START_TABS, cfg.start_tab)]));
    intro->setOn(cfg.intro, false);
    notices_system->setOn(cfg.notices_system, false);
    notices_summary->setOn(cfg.notices_summary, false);
    notices_reminders->setOn(cfg.notices_reminders, false);
    notices_ack_remind->setOn(cfg.notices_ack_remind, false);
    support_reminder->setOn(cfg.support_reminder, false);
    update_daily->setOn(cfg.update_daily, false);
    update_via->setDetailText(update_flow::method_text());
    this->list.sync();
    cadran::set_pill("");
}

aurore::View* PreferencesTab::create()
{
    return new PreferencesTab();
}
