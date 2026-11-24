// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/boreale_hub_tab.hpp"

#include <aurore/cadran/ui/cadran_frame.hpp>

#include "action/sync_flow.hpp"
#include "activity/modules_activity.hpp"
#include "activity/sync_activity.hpp"
#include "kit_text.hpp"
#include "tab/boreale_tab.hpp"
#include "ui/play_ui.hpp"
#include "util/modules.hpp"
#include "util/paths.hpp"

using namespace aurore::literals;

BorealeHubTab::BorealeHubTab()
    : TabBase("xml/tab/boreale_hub.xml"), list(this)
{
    remote->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new SyncActivity());
        return true;
    });
    modules_cell->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new ModulesActivity());
        return true;
    });
    boreale->registerClickAction([](aurore::View*) {
        ui::open_screen(new BorealeTab(), "playguard/about/boreale"_i18n);
        return true;
    });
    this->list.apply({ { "bh_remote", "server" }, { "bh_modules", "tools" }, { "bh_boreale", "list" } });
}

void BorealeHubTab::refresh()
{
    // The remote link's state, and how many optional sysmodules are on the SD
    // card (whether each exefs.nsp is there: a stat, not its hash, which the
    // screen itself reads).
    const sync_flow::Status link = sync_flow::status();
    remote->setDetailText(ui::opens(!link.enabled ? "playguard/sync/state_off"_i18n
                          : link.running && link.link.state == SyncLink_Online ? "playguard/sync/state_online"_i18n
                                                                              : "playguard/sync/state_offline"_i18n));
    int installed = 0;
    for (const auto& m : modules::all())
        installed += paths::file_exists(modules::dir(paths::sd_root(), m) + "/exefs.nsp") ? 1 : 0;
    modules_cell->setDetailText(ui::opens(installed == 1 ? "playguard/modules/count_one"_i18n
                                : installed ? aurore::getStr("playguard/modules/count", installed)
                                            : "playguard/modules/state/none"_i18n));
    boreale->setDetailText(ui::opens(BorealeTab::names()));
    this->list.sync();
    cadran::set_pill("");
}

aurore::View* BorealeHubTab::create()
{
    return new BorealeHubTab();
}
