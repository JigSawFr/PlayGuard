// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/boreale_hub_tab.hpp"

#include <aurore/cadran/ui/cadran_frame.hpp>

#include "action/agent_update.hpp"
#include "activity/agent_activity.hpp"
#include "activity/modules_activity.hpp"
#include "activity/sync_activity.hpp"
#include "kit_text.hpp"
#include "ui/play_ui.hpp"
#include "util/launcher.hpp"
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
    // As Console's: the link as a tile (an action:
    // its words under the title), the two modules side by side as status
    // tiles with their state's check or warning.
    this->list.tiles({ { "bh_remote", "server" } }, false);
    this->list.tiles({ { "bh_rescue", "unlock" }, { "bh_agent", "bolt" } }, true);
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
    this->refresh_homebrews();
    this->list.sync();
    cadran::set_pill("");
}

aurore::View* BorealeHubTab::create()
{
    return new BorealeHubTab();
}

aurore::DetailCell* BorealeHubTab::homebrew_cell(const boreale_apps_logic::App& app)
{
    auto* cell = new aurore::DetailCell();
    cell->setText(app.name);
    std::string detail = app.version.empty() ? "" : aurore::getStr("playguard/cadran/about/version", app.version);
    if (app.self) {
        detail += (detail.empty() ? "" : " · ") + "playguard/boreale/this_app"_i18n;
        cell->setDetailText(detail);
        cell->setFocusable(false);
        return cell;
    }
    cell->setDetailText(ui::opens(detail));
    const std::string name = app.name, nro = app.nro, module = app.module;
    cell->registerClickAction([name, nro, module](aurore::View*) {
        if (!nro.empty() && launcher::can_launch() && launcher::launch_nro(nro)) {
            aurore::Logger::info("boreale: opens {} ({})", module, nro);
            aurore::Application::quit();   // hbloader starts it as soon as PlayGuard exits
            return true;
        }
        ui::info(aurore::getStr("playguard/boreale/open_manual", name));
        return true;
    });
    return cell;
}

void BorealeHubTab::refresh_homebrews()
{
    const auto apps = boreale_apps_logic::installed(paths::sd_root());
    // Built again only when it changed (each time the screen comes back,
    // after a dialog too: the focus stays where it was).
    std::string now;
    for (const auto& app : apps) now += app.module + "\n" + app.name + "\n" + app.version + "\n" + app.nro + "\n";
    if (now == this->seen) return;
    this->seen = now;

    // New cells in a new box, the focus moved to it, then the old one
    // removed: Aurore would keep a pointer to a deleted focused cell.
    auto* box = new aurore::Box(aurore::Axis::COLUMN);
    box->setAlignItems(aurore::AlignItems::STRETCH);
    aurore::View* first = nullptr;
    std::vector<std::string> ids;
    for (const auto& app : apps) {
        aurore::DetailCell* c = this->homebrew_cell(app);
        ids.push_back("bo_" + app.module);
        c->setId(ids.back());
        box->addView(c);
        if (!first && c->isFocusable()) first = c;
    }
    // In a card, each with its sign: PlayGuard's and PlayShelf's own, a
    // gamepad for the others.
    std::vector<std::pair<const char*, const char*>> icons;
    for (size_t i = 0; i < apps.size(); i++)
        icons.push_back({ ids[i].c_str(), apps[i].module == "playguard" ? "shield"
                                          : apps[i].module == "playshelf" ? "shelf" : "gamepad" });
    cadran::List(box, box).apply(icons);
    aurore::View* old = this->homebrews->getChildren().empty() ? nullptr : this->homebrews->getChildren()[0];
    this->homebrews->addView(box);
    if (old) {
        bool inside = false;
        for (aurore::View* v = aurore::Application::getCurrentFocus(); v; v = v->hasParent() ? v->getParent() : nullptr)
            inside |= v == old;
        if (inside && first) aurore::Application::giveFocus(first);
        this->homebrews->removeView(old);
    }
}
