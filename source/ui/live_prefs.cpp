// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/live_prefs.hpp"

#include <borealis.hpp>
#include <functional>
#include <utility>

#include "activity/main_activity.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/text.hpp"

namespace live_prefs
{
namespace
{
brls::ThemeVariant s_system_theme = brls::ThemeVariant::DARK;
std::string s_system_locale;   // "" when PlayGuard started in another language
std::string s_locale;          // the strings' language now

// The tab shown, as a sidebar position (ui::tab).
int shown_tab()
{
    static const std::pair<const char*, int> tabs[] = {
        { "tab/dashboard", ui::tab::dashboard },   { "tab/play_timer", ui::tab::play_timer },
        { "tab/activity", ui::tab::activity },     { "tab/restrictions", ui::tab::restrictions },
        { "tab/security", ui::tab::security },     { "tab/console", ui::tab::console },
        { "tab/history", ui::tab::history },       { "tab/preferences", ui::tab::preferences },
        { "tab/about", ui::tab::about },
    };
    const std::string screen = TabBase::shown_screen();
    for (const auto& [name, position] : tabs)
        if (screen == name) return position;
    return ui::tab::preferences;
}

// Every screen above the loading one closed, one at a time; then `next`.
void close_all(std::function<void()> next)
{
    if (!brls::Application::popActivity(brls::TransitionAnimation::NONE, [next]() { close_all(next); })) next();
}

// Every screen made again, the main one on the tab that was shown: the
// screens above it (a sub-screen the header's setting was changed from) go.
void rebuild()
{
    const int tab = shown_tab();
    brls::sync([tab]() {
        close_all([tab]() {
            auto* main = new MainActivity();
            brls::Application::pushActivity(main, brls::TransitionAnimation::NONE);
            main->open_tab(tab);
        });
    });
}

bool chinese(const std::string& locale)
{
    return text::starts_with(locale, "zh");
}
}   // namespace

void remember_system()
{
    s_system_theme = brls::Application::getPlatform()->getThemeVariant();
    // The platform's locale is the preferences' when one was set (main.cpp's
    // apply_locale): the console's own only when none was.
    s_locale = brls::Application::getLocale();
    if (config::get().language == "system") s_system_locale = s_locale;
}

void apply_theme()
{
    const std::string& theme = config::get().theme;
    brls::Application::getPlatform()->setThemeVariant(theme == "dark"    ? brls::ThemeVariant::DARK
                                                      : theme == "light" ? brls::ThemeVariant::LIGHT
                                                                         : s_system_theme);
}

std::string locale()
{
    return s_locale;
}

bool theme_changed()
{
    apply_theme();
    rebuild();
    return true;
}

bool language_changed()
{
    const std::string& chosen = config::get().language;
    const std::string next = chosen == "system" ? s_system_locale : chosen;
    // Chinese, either way: its font and its characters' forms are chosen once
    // (ui::use_latin_font, borealis' Label).
    if (next.empty() || chinese(next) || chinese(s_locale)) return false;
    s_locale = next;
    brls::loadTranslations(next);
    rebuild();
    return true;
}
}   // namespace live_prefs
