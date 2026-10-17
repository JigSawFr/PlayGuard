// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/about_tab.hpp"

#include "action/update_flow.hpp"
#include "activity/changelog_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "view/funding.hpp"
#include "view/release_notes.hpp"

using namespace brls::literals;

namespace
{
}   // namespace

AboutTab::AboutTab()
    : TabBase("xml/tab/about.xml", false)
{
    changelog_note->setSingleLine(false);
    support_note->setSingleLine(false);
    version->registerClickAction([this](brls::View*) {
        this->count_version_press();
        return true;
    });

    update_cell->registerClickAction([](brls::View*) {
        update_flow::check_now();
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
    this->fill_changelog();
}

void AboutTab::fill_changelog()
{
    // This version's highlights, the full list a press away; the full list
    // itself when there are none. The earlier versions are on the releases page.
    std::string date;
    const auto lines      = release_notes::current(&date);
    const auto highlights = release_notes::highlights();
    changelog_header->setTitle(brls::getStr("playguard/about/section_changelog", app::version()));
    if (!date.empty()) changelog_header->setSubtitle(date);
    const std::string releases = std::string(app::repo_url()) + "/releases";
    changelog_note->setText(lines.empty() && highlights.empty() ? brls::getStr("playguard/about/changelog_none", releases)
                                                                : brls::getStr("playguard/about/changelog_note", releases));
    const bool short_form = !highlights.empty() && !lines.empty();
    if (highlights.empty()) release_notes::fill(changelog, lines);
    else release_notes::fill_highlights(changelog, highlights);
    changelog_full->setVisibility(short_form ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    changelog_full->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ChangelogActivity());
        return true;
    });
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

    SysInfo si;
    sysinfo_get(&si);
    std::string flags;
    if (app::read_only()) flags += " · " + "playguard/tools/flag_read_only"_i18n;
    if (app::dev_mode()) flags += " · " + "playguard/tools/flag_dev"_i18n;
    // Developer mode: the commit too, to tell development builds apart.
    const std::string commit = app::dev_mode() && !app::commit().empty() ? " (" + app::commit() + ")" : "";
    version->setDetailText(app::version() + commit + flags);
    mode->setDetailText(si.applet_mode ? "playguard/tools/mode_applet"_i18n : "playguard/tools/mode_app"_i18n);
    data->setDetailText(paths::data_dir());
}

brls::View* AboutTab::create()
{
    return new AboutTab();
}
