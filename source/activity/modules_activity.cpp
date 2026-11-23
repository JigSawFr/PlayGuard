// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/modules_activity.hpp"

#include <format>
#include <iterator>
#include <memory>

#include <aurore/cadran/ui/cadran_frame.hpp>
#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/agent_update.hpp"
#include "activity/agent_activity.hpp"
#include "action/bundled_modules.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/rescue.hpp"
#include "app.hpp"
#include "tab/tab_base.hpp"
#include "ui/play_ui.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace
{
std::string key(const modules::Module& m, const char* what)
{
    return std::string("playguard/modules/") + m.name + "/" + what;
}

std::string short_sha(const std::string& sha)
{
    return sha.size() > 12 ? sha.substr(0, 12) : sha;
}

// The PIN, as for a console change; says why not when refused.
bool allowed()
{
    if (pin_lock::allow_change()) return true;
    ui::notify(pin_lock::refusal_text());
    return false;
}

void record(const modules::Module& m, const char* what)
{
    history_flow::record_event("module", "", aurore::getStr(std::string("playguard/modules/done/") + what, m.title));
}

// The module's state: in full (its tab), or short enough to share one line
// with the title of a cell that opens this screen; its colour in `color`.
std::string state_text(modules::Id id, bool brief, const char** color)
{
    const modules::Module& m = modules::get(id);
    const modules::State s = modules::state(paths::sd_root(), m);
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), m);
    std::string text;
    const char* c = "cadran/text2";
    if (!s.installed) {
        text = b.present ? "playguard/modules/state/not_installed"_i18n : "playguard/modules/state/not_bundled"_i18n;
    } else if (modules::offer(s, b) == modules::Offer::Update) {
        text = brief ? "playguard/modules/short/update"_i18n : "playguard/modules/state/update"_i18n;
        c = "cadran/amber";
    } else {
        text = brief ? "playguard/modules/short/installed"_i18n : "playguard/modules/state/installed"_i18n;
        c = "cadran/teal";
    }
    if (color) *color = c;
    return text;
}

// The recovery module's tab: what it does, then its cells. Tabs are deleted when
// another one is opened: what ends later refreshes through refresher()
// (TabBase's `alive`), never through `this`.
class ModuleTab : public TabBase
{
  public:
    explicit ModuleTab(modules::Id which);

  protected:
    void refresh() override;

  private:
    modules::Id id;
    cadran::List list;

    void install();
    void uninstall();

    AURORE_BIND(aurore::Header,      header, "md_header");
    AURORE_BIND(aurore::Label,       about,  "md_about");
    AURORE_BIND(aurore::DetailCell,  status, "md_status");
    AURORE_BIND(aurore::DetailCell,  action, "md_action");
    AURORE_BIND(aurore::BooleanCell, boot,   "md_boot");
    AURORE_BIND(aurore::DetailCell,  remove, "md_remove");
};

// The overview: what the modules are, and each one's state (Ⓐ opens the
// recovery module's tab, or the agent's own screen, boreale-kit's).
class ModulesOverview : public TabBase
{
  public:
    ModulesOverview();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    AURORE_BIND(aurore::Label,      intro,  "md_intro");
    AURORE_BIND(aurore::DetailCell, rescue, "md_rescue");
    AURORE_BIND(aurore::DetailCell, agent,  "md_agent");
};

// Sidebar positions (the separator counts).
constexpr int TAB_RESCUE   = 2;

ModulesOverview::ModulesOverview() : TabBase("xml/tab/modules_overview.xml"), list(this)
{
    intro->setSingleLine(false);
    rescue->registerClickAction([this](aurore::View*) {
        ui::go_to_tab(this, TAB_RESCUE);
        return true;
    });
    agent->registerClickAction([this](aurore::View*) {
        // The screen is the kit's, shared with the other apps; the PIN and the
        // history are PlayGuard's, through the flow's hooks.
        agent_update::install_hooks();
        aurore::Application::pushActivity(new AgentActivity());
        return true;
    });
    this->list.apply({ { "md_rescue", "unlock" }, { "md_agent", "server" } });
}

void ModulesOverview::refresh()
{
    const char* color = nullptr;
    rescue->setDetailText(ModulesActivity::summary(modules::Id::Rescue, &color));
    rescue->detail->setThemeColor("textColor", color);
    agent->setDetailText(AgentActivity::summary(&color));
    agent->detail->setThemeColor("textColor", color);
    this->list.sync();
}

ModuleTab::ModuleTab(modules::Id which) : TabBase("xml/tab/module.xml"), id(which), list(this)
{
    const modules::Module& m = modules::get(which);
    header->setTitle(aurore::getStr(key(m, "title")));
    about->setSingleLine(false);
    about->setText(aurore::getStr(key(m, "about")));

    status->registerClickAction([which](aurore::View*) {
        const modules::Module& mod = modules::get(which);
        const modules::State s = modules::state(paths::sd_root(), mod);
        const modules::Bundle b = modules::bundled(bundled_modules::dir(), mod);
        std::string text = aurore::getStr(key(mod, "title")) + "\n";
        text += "\n" + aurore::getStr("playguard/modules/details/where", modules::dir(paths::sd_root(), mod));
        if (s.installed)
            text += "\n" + aurore::getStr("playguard/modules/details/installed",
                                        s.version.empty() ? "playguard/modules/details/by_hand"_i18n : s.version,
                                        short_sha(s.sha256));
        text += "\n" + (b.present ? aurore::getStr("playguard/modules/details/bundled", b.version, short_sha(b.sha256))
                                  : "playguard/modules/details/not_bundled"_i18n);
        ui::info(text);
        return true;
    });
    action->registerClickAction([this](aurore::View*) {
        this->install();
        return true;
    });
    boot->init("playguard/modules/at_boot"_i18n, false, [this](bool on) {
        const modules::Module& mod = modules::get(this->id);
        if (allowed()) {
            if (const auto done = modules::set_at_boot(paths::sd_root(), mod, on); !done)
                ui::error("playguard/modules/error"_i18n + " — " + done.error());
            else
                record(mod, on ? "boot_on" : "boot_off");
        }
        this->refresh();
    });
    remove->registerClickAction([this](aurore::View*) {
        this->uninstall();
        return true;
    });
    this->list.apply({ { "md_status", "info" }, { "md_action", "download" }, { "md_boot", "bolt" },
                       { "md_remove", "trash" } });
}

void ModuleTab::install()
{
    const modules::Module& m = modules::get(id);
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), m);
    if (!b.present) {
        ui::info("playguard/modules/details/not_bundled"_i18n);
        return;
    }
    const modules::State s = modules::state(paths::sd_root(), m);
    const bool update = s.installed;
    const std::string body = aurore::getStr(key(m, update ? "update_body" : "install_body"), b.version);
    const modules::Id mid = this->id;
    const std::function<void()> refresh_tab = this->refresher();
    ui::confirm(body, update ? "playguard/modules/update"_i18n : "playguard/modules/install"_i18n, [mid, b, update, refresh_tab]() {
        const modules::Module& target = modules::get(mid);
        if (!allowed()) return;
        // The recovery module only acts at boot: nothing to stop or start.
        const std::string sd = paths::sd_root();
        const auto done = modules::install(sd, target, b, false);
        if (!done) {
            ui::error("playguard/modules/error"_i18n + " — " + done.error());
        } else {
            if (!done->warning.empty()) {
                aurore::Logger::warning("modules: {} installed, {}", target.name, done->warning);
                ui::notify("playguard/modules/meta_err"_i18n, ui::Toast::Refused);
            }
            modules::confirm(sd, target);
            // Which PlayGuard it comes with, as the release's zip says: the
            // bell's "out of date" entry goes.
            if (const auto noted = modules::write_rescue_note(sd, app::version(), rescue::revision()); !noted)
                aurore::Logger::warning("modules: playguard-rescue.txt not written: {}", noted.error());
            rescue::note_version();
            record(target, update ? "updated" : "installed");
            ui::notify(aurore::getStr(key(target, update ? "updated" : "installed")));
        }
        refresh_tab();
    });
}

void ModuleTab::uninstall()
{
    const modules::Module& m = modules::get(id);
    if (!modules::state(paths::sd_root(), m).installed) return;
    const modules::Id mid = this->id;
    const std::function<void()> refresh_tab = this->refresher();
    ui::confirm_danger(aurore::getStr(key(m, "remove_body")), "playguard/modules/remove"_i18n, [mid, refresh_tab]() {
        const modules::Module& target = modules::get(mid);
        if (!allowed()) return;
        if (const auto done = modules::uninstall(paths::sd_root(), target); !done) {
            ui::error("playguard/modules/error"_i18n + " — " + done.error());
        } else {
            record(target, "removed");
            if (target.id == modules::Id::Rescue) rescue::note_version();
            ui::notify("playguard/modules/removed"_i18n);
        }
        refresh_tab();
    });
}

void ModuleTab::refresh()
{
    const modules::Module& m = modules::get(id);
    const modules::State s = modules::state(paths::sd_root(), m);
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), m);
    const modules::Offer offer = modules::offer(s, b);

    const char* color = nullptr;
    status->setDetailText(state_text(id, false, &color));
    status->detail->setThemeColor("textColor", color);

    action->setText(offer == modules::Offer::Update ? "playguard/modules/update"_i18n : "playguard/modules/install"_i18n);
    action->setDetailText(b.present ? b.version : "");
    ui::set_visible(action, offer != modules::Offer::None);
    boot->setOn(s.at_boot, false);
    ui::set_visible(boot, s.installed);
    ui::set_visible(remove, s.installed);
    this->list.sync();
}

// The rail, always open, its icons before the names: Cadran's own
// (cadran::style_rail) is the main screen's, the one that folds, and there is
// one. Its names shrink to fit, as there.
void dress_rail(aurore::View* tab_frame)
{
    static const char* const ICONS[] = { "list", "", "unlock" };
    auto* sidebar = dynamic_cast<aurore::Sidebar*>(tab_frame->getView("aurore/tab_frame/sidebar"));
    aurore::SidebarItem* first = sidebar ? sidebar->getItem(0) : nullptr;
    aurore::Box* list = first ? first->getParent() : nullptr;
    if (!list) return;
    list->setPadding(6, 8, 10, 16);
    size_t position = 0;
    for (aurore::View* v : list->getChildren()) {
        const char* name = position < std::size(ICONS) ? ICONS[position] : "";
        position++;
        auto* item = dynamic_cast<aurore::SidebarItem*>(v);
        if (!item) continue;   // a separator
        item->setHeight(52);
        item->setPaddingLeft(12);
        item->setPaddingRight(12);
        item->setAlignItems(aurore::AlignItems::CENTER);
        // The active tab says so in teal; no accent bar beside it.
        if (auto* accent = item->getView("aurore/sidebar/item_accent")) {
            accent->setWidth(0);
            accent->setMargins(0, 0, 0, 0);
            accent->setVisibility(aurore::Visibility::GONE);
        }
        if (auto* label = dynamic_cast<aurore::Label*>(item->getView("aurore/sidebar/item_label"))) {
            label->setFontSize(17);
            label->setFontScaleMax(1.25f);
            label->setShrinkToFit(true);
            label->setSingleLine(true);
            label->setMargins(0, 0, 0, *name ? 10 : 4);
        }
        if (*name) item->addView(cadran::kit::icon(name, 22), 0);
    }
}
}   // namespace

ModulesActivity::ModulesActivity(Tab first) : start(first)
{
}

std::string ModulesActivity::summary(modules::Id id, const char** color)
{
    return state_text(id, true, color);
}

void ModulesActivity::onResume()
{
    ui::keep_focus(this);   // after a dialog whose opener went (ui.hpp)
}

void ModulesActivity::onContentAvailable()
{
    tabs->addTab("playguard/modules/overview"_i18n, []() { return new ModulesOverview(); });
    tabs->addSeparator();
    tabs->addTab("playguard/modules/rescue/tab"_i18n, []() { return new ModuleTab(modules::Id::Rescue); });
    // Dressed here, not by the screens' skin (Cadran's), which would take
    // the sidebar for the list: the header, and the rail.
    cadran::style_header(dynamic_cast<aurore::AppletFrame*>(this->getContentView()), nullptr, "", false, false);
    dress_rail(tabs.getView());

    if (start != Tab::Overview) {
        aurore::sync([this]() {
            auto stack = aurore::Application::getActivitiesStack();
            if (!stack.empty() && stack.back() == this) tabs->focusTab(TAB_RESCUE);
        });
    }
}
