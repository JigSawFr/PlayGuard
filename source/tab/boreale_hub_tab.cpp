// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/boreale_hub_tab.hpp"

#include <aurore/cadran/ui/cadran_frame.hpp>

#include "action/agent_update.hpp"
#include "activity/agent_activity.hpp"
#include "activity/modules_activity.hpp"
#include "activity/sync_activity.hpp"
#include "kit_text.hpp"
#include "tab/boreale_tab.hpp"
#include "ui/play_ui.hpp"
#include "util/modules.hpp"

using namespace aurore::literals;

BorealeHubTab::BorealeHubTab()
    : TabBase("xml/tab/boreale_hub.xml"), list(this)
{
    remote->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new SyncActivity());
        return true;
    });
    rescue->registerClickAction([](aurore::View*) {
        aurore::Application::pushActivity(new ModuleActivity(modules::Id::Rescue));
        return true;
    });
    agent->registerClickAction([](aurore::View*) {
        // The screen is the kit's, shared with the other apps; the PIN and the
        // history are PlayGuard's, through the flow's hooks.
        agent_update::install_hooks();
        aurore::Application::pushActivity(new AgentActivity());
        return true;
    });
    boreale->registerClickAction([](aurore::View*) {
        ui::open_screen(new BorealeTab(), "playguard/about/boreale"_i18n);
        return true;
    });
    this->list.apply({ { "bh_remote", "server" }, { "bh_rescue", "unlock" }, { "bh_agent", "bolt" }, { "bh_boreale", "list" } });
}

void BorealeHubTab::refresh()
{
    remote->setDetailText(ui::opens(ModulesActivity::remote_state()));
    // Each module's state, with its colour: teal when as it should be, amber
    // when it needs an update.
    const char* color = nullptr;
    const std::string rescue_text = ModulesActivity::summary(modules::Id::Rescue, &color);
    rescue->setDetailText(ui::opens(rescue_text));
    rescue->detail->setThemeColor("textColor", color ? color : "cadran/text2");
    color = nullptr;
    const std::string agent_text = ModulesActivity::agent_summary(&color);
    agent->setDetailText(ui::opens(agent_text));
    agent->detail->setThemeColor("textColor", color ? color : "cadran/text2");
    boreale->setDetailText(ui::opens(BorealeTab::names()));
    this->list.sync();
    cadran::set_pill("");
}

aurore::View* BorealeHubTab::create()
{
    return new BorealeHubTab();
}
