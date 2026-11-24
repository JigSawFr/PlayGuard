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
    // As Console's: the link and the homebrews as one tile each (an action:
    // its words under the title), the two modules side by side as status
    // tiles with their state's check or warning.
    this->list.tiles({ { "bh_remote", "server" } }, false);
    this->list.tiles({ { "bh_rescue", "unlock" }, { "bh_agent", "bolt" } }, true);
    this->list.tiles({ { "bh_boreale", "list" } }, false);
    this->list.apply({});
}

void BorealeHubTab::refresh()
{
    remote->setDetailText(ModulesActivity::remote_state());
    // Each module's state, with a check when as it should be and a warning
    // when it needs an update.
    const auto tile = [this](aurore::DetailCell* cell, const char* tile_id, const std::string& text, const char* color) {
        cell->setDetailText(text);
        const std::string c = color ? color : "";
        this->list.state(tile_id, c == "cadran/teal" ? "cadran/teal_fill" : c == "cadran/amber" ? "cadran/amber_fill" : nullptr);
    };
    const char* color = nullptr;
    const std::string rescue_text = ModulesActivity::summary(modules::Id::Rescue, &color);
    tile(rescue, "bh_rescue", rescue_text, color);
    color = nullptr;
    const std::string agent_text = ModulesActivity::agent_summary(&color);
    tile(agent, "bh_agent", agent_text, color);
    boreale->setDetailText(BorealeTab::names());
    this->list.sync();
    cadran::set_pill("");
}

aurore::View* BorealeHubTab::create()
{
    return new BorealeHubTab();
}
