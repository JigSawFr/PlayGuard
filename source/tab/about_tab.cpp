// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/about_tab.hpp"

#include "action/sys_notes_flow.hpp"
#include "action/update_flow.hpp"
#include "app.hpp"
#include "tab/help_tab.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "view/funding.hpp"

using namespace brls::literals;

AboutTab::AboutTab()
    : TabBase("xml/tab/about.xml", false)
{
    support_note->setSingleLine(false);
    // Every version's notes, from the releases on GitHub, as for Atmosphère
    // and hekate on their rows in Console: the installed one, the newer and
    // the older ones. Seven presses of a hidden Ⓨ turn the developer tools on.
    version->registerAction("playguard/sys_notes/hint"_i18n, brls::BUTTON_A, [](brls::View*) {
        sys_notes_flow::open(sys_notes::Which::PlayGuard);
        return true;
    }, false, false, brls::SOUND_CLICK);
    version->registerAction("", brls::BUTTON_Y, [this](brls::View*) {
        this->count_version_press();
        return true;
    }, true, false, brls::SOUND_CLICK);

    update_cell->registerClickAction([](brls::View*) {
        update_flow::check_now();
        return true;
    });
    update_daily->init("playguard/tools/update_daily"_i18n, config::get().update_daily, [](bool on) {
        config::get().update_daily = on;
        ui::save_config();
    });
    update_via->registerClickAction([this](brls::View*) {
        update_flow::choose_method([this]() { this->refresh(); });
        return true;
    });

    help->setDetailText(ui::opens());
    help->registerClickAction([](brls::View*) {
        ui::open_screen(new HelpTab(), "playguard/prefs/help"_i18n);
        return true;
    });

    // The credits and the funding links do not change: set once.
    author->setDetailText("JigSawFr");
    upstream->setDetailText("playguard/about/credit_upstream_value"_i18n);
    fixes->setDetailText("anbingxi");
    ui_lib->setDetailText("borealis (Apache 2.0)");
    license->setDetailText("GPLv3");
    source->setDetailText(app::repo_url());
    funding->addView(funding::cards(180, true));
}

void AboutTab::count_version_press()
{
    if (app::dev_mode()) {
        ui::notify("playguard/dev/already_on"_i18n);
        return;
    }
    // Seven presses, at most 3 s apart.
    const brls::Time now = brls::getCPUTimeUsec();
    if (now - this->last_version_press > 3000000) this->version_presses = 0;
    this->last_version_press = now;
    const int left = 7 - ++this->version_presses;
    if (left > 0) {
        if (left == 1) ui::notify("playguard/dev/presses_left_one"_i18n);
        else if (left <= 3) ui::notify(brls::getStr("playguard/dev/presses_left", left));
        return;
    }
    this->version_presses = 0;
    app::set_dev_mode(true, true, "playguard/dev/enabled"_i18n,
                      "playguard/dev/enabled"_i18n + " " + "playguard/toast/config_err"_i18n);
    ui::on_mode_changed();
}

void AboutTab::refresh()
{
    const auto& cfg = config::get();
    update_cell->setDetailText(cfg.update_checked.empty() ? "" : brls::getStr("playguard/tools/update_last", cfg.update_checked));
    update_daily->setOn(cfg.update_daily, false);
    update_via->setDetailText(update_flow::method_text());
    SysInfo si;
    sysinfo_get(&si);
    std::string flags;
    if (app::read_only()) flags += " · " + "playguard/tools/flag_read_only"_i18n;
    if (app::dev_mode()) flags += " · " + "playguard/tools/flag_dev"_i18n;
    // Developer mode: the commit too, to tell development builds apart.
    const std::string commit = app::dev_mode() && !app::commit().empty() ? " (" + app::commit() + ")" : "";
    // With the newer release the last update check found, as in Console.
    bool newer = false;
    version->setDetailText(ui::opens(sys_notes_flow::status_text(sys_notes::Which::PlayGuard, &newer) + commit + flags));
    version->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    mode->setDetailText(si.applet_mode ? "playguard/tools/mode_applet"_i18n : "playguard/tools/mode_app"_i18n);
    data->setDetailText(paths::data_dir());
}

brls::View* AboutTab::create()
{
    return new AboutTab();
}
