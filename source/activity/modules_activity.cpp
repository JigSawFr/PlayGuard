// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/modules_activity.hpp"

#include <fmt/format.h>
#include <memory>

#include <borealis/cadran/ui/cadran_frame.hpp>
#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_list.hpp>

#include "action/agent_update.hpp"
#include "action/bundled_modules.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/rescue.hpp"
#include "action/sync_flow.hpp"
#include "app.hpp"
#include "tab/tab_base.hpp"
#include "ui/ui.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"

using namespace brls::literals;

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
    history_flow::record_event("module", "", brls::getStr(std::string("playguard/modules/done/") + what, m.title));
}

// The module's state: in full (its tab), or short enough to share one line
// with the title of a cell that opens this screen; its colour in `color`.
std::string state_text(modules::Id id, bool brief, NVGcolor* color)
{
    const modules::Module& m = modules::get(id);
    const modules::State s = modules::state(paths::sd_root(), m);
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), m);
    std::string text;
    NVGcolor c = cadran::kit::color("cadran/text2");
    if (!s.installed) {
        text = b.present ? "playguard/modules/state/not_installed"_i18n : "playguard/modules/state/not_bundled"_i18n;
    } else if (modules::offer(s, b) == modules::Offer::Update) {
        text = brief ? "playguard/modules/short/update"_i18n : "playguard/modules/state/update"_i18n;
        c = cadran::kit::color("cadran/amber");
    } else {
        text = brief ? "playguard/modules/short/installed"_i18n : "playguard/modules/state/installed"_i18n;
        c = cadran::kit::color("cadran/teal");
    }
    if (!brief && m.resident && s.installed)
        text += " · " + (module_running(m.tid, nullptr) ? "playguard/modules/state/running"_i18n
                                                        : "playguard/modules/state/stopped"_i18n);
    if (color) *color = c;
    return text;
}

// One module's tab: what it does, then its cells. Tabs are deleted when
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

    BRLS_BIND(brls::Header,      header, "md_header");
    BRLS_BIND(brls::Label,       about,  "md_about");
    BRLS_BIND(brls::DetailCell,  status, "md_status");
    BRLS_BIND(brls::DetailCell,  action, "md_action");
    BRLS_BIND(brls::BooleanCell, boot,   "md_boot");
    BRLS_BIND(brls::DetailCell,  run,    "md_run");
    BRLS_BIND(brls::DetailCell,  remove, "md_remove");
};

// The overview: what the modules are, and each one's state (Ⓐ opens its tab).
class ModulesOverview : public TabBase
{
  public:
    ModulesOverview();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    BRLS_BIND(brls::Label,      intro,  "md_intro");
    BRLS_BIND(brls::DetailCell, rescue, "md_rescue");
    BRLS_BIND(brls::DetailCell, agent,  "md_agent");
};

// Sidebar positions (the separator counts).
constexpr int TAB_RESCUE   = 2;
constexpr int TAB_AGENT    = 3;

ModulesOverview::ModulesOverview() : TabBase("xml/tab/modules_overview.xml"), list(this)
{
    intro->setSingleLine(false);
    rescue->registerClickAction([this](brls::View*) {
        ui::go_to_tab(this, TAB_RESCUE);
        return true;
    });
    agent->registerClickAction([this](brls::View*) {
        ui::go_to_tab(this, TAB_AGENT);
        return true;
    });
    this->list.apply({ { "md_rescue", "unlock" }, { "md_agent", "server" } });
}

void ModulesOverview::refresh()
{
    NVGcolor color;
    rescue->setDetailText(ModulesActivity::summary(modules::Id::Rescue, &color));
    rescue->setDetailTextColor(color);
    agent->setDetailText(ModulesActivity::summary(modules::Id::Agent, &color));
    agent->setDetailTextColor(color);
    this->list.sync();
}

ModuleTab::ModuleTab(modules::Id which) : TabBase("xml/tab/module.xml"), id(which), list(this)
{
    const modules::Module& m = modules::get(which);
    header->setTitle(brls::getStr(key(m, "title")));
    about->setSingleLine(false);
    about->setText(brls::getStr(key(m, "about")));

    status->registerClickAction([which](brls::View*) {
        const modules::Module& mod = modules::get(which);
        const modules::State s = modules::state(paths::sd_root(), mod);
        const modules::Bundle b = modules::bundled(bundled_modules::dir(), mod);
        std::string text = brls::getStr(key(mod, "title")) + "\n";
        text += "\n" + brls::getStr("playguard/modules/details/where", modules::dir(paths::sd_root(), mod));
        if (s.installed)
            text += "\n" + brls::getStr("playguard/modules/details/installed",
                                        s.version.empty() ? "playguard/modules/details/by_hand"_i18n : s.version,
                                        short_sha(s.sha256));
        text += "\n" + (b.present ? brls::getStr("playguard/modules/details/bundled", b.version, short_sha(b.sha256))
                                  : "playguard/modules/details/not_bundled"_i18n);
        ui::info(text);
        return true;
    });
    action->registerClickAction([this](brls::View*) {
        this->install();
        return true;
    });
    boot->init("playguard/modules/at_boot"_i18n, false, [this](bool on) {
        const modules::Module& mod = modules::get(this->id);
        std::string err;
        if (!allowed() || !modules::set_at_boot(paths::sd_root(), mod, on, &err)) {
            if (!err.empty()) ui::error("playguard/modules/error"_i18n + " — " + err);
        } else {
            record(mod, on ? "boot_on" : "boot_off");
        }
        this->refresh();
    });
    run->registerClickAction([this](brls::View*) {
        const modules::Module& mod = modules::get(this->id);
        const bool running = module_running(mod.tid, nullptr);
        if (!allowed()) return true;
        const modules::Id mid = this->id;
        const std::function<void()> refresh_tab = this->refresher();
        const auto act = [mid, running, refresh_tab]() {
            const modules::Module& target = modules::get(mid);
            const Result rc = running ? module_terminate(target.tid) : module_launch(target.tid);
            ui::notify_result(rc, running ? "playguard/modules/stopped"_i18n : "playguard/modules/started"_i18n,
                              "playguard/modules/error"_i18n);
            if (R_SUCCEEDED(rc)) record(target, running ? "stopped" : "started");
            // The link follows: the agent's, or PlayGuard's own.
            if (running || R_FAILED(rc)) sync_flow::reload();
            else sync_flow::agent_started();
            refresh_tab();
        };
        if (running) sync_flow::agent_stopping(act);
        else act();
        return true;
    });
    remove->registerClickAction([this](brls::View*) {
        this->uninstall();
        return true;
    });
    this->list.apply({ { "md_status", "info" }, { "md_action", "download" }, { "md_boot", "bolt" },
                       { "md_run", "refresh" }, { "md_remove", "trash" } });
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
    const std::string body = brls::getStr(key(m, update ? "update_body" : "install_body"), b.version);
    const modules::Id mid = this->id;
    const std::function<void()> refresh_tab = this->refresher();
    ui::confirm(body, update ? "playguard/modules/update"_i18n : "playguard/modules/install"_i18n, [mid, b, update, refresh_tab]() {
        const modules::Module& target = modules::get(mid);
        if (!allowed()) return;
        if (target.resident) {
            // The agent: stopped cleanly, swapped, started and seen answering,
            // or the previous one put back (action/agent_update).
            if (agent_update::running()) {
                ui::notify("playguard/modules/agent/busy"_i18n);
                return;
            }
            ui::notify(update ? "playguard/modules/agent/updating"_i18n : "playguard/modules/agent/installing"_i18n);
            agent_update::run(bundled_modules::dir(), [refresh_tab](const agent_update::Outcome& o) {
                agent_update::tell(o);
                refresh_tab();
            });
            return;
        }
        // The recovery module only acts at boot: nothing to stop or start.
        const std::string sd = paths::sd_root();
        std::string err;
        const bool ok = modules::install(sd, target, b, false, &err);
        if (!ok) {
            ui::error("playguard/modules/error"_i18n + " — " + err);
        } else {
            modules::confirm(sd, target);
            // Which PlayGuard it comes with, as the release's zip says: the
            // bell's "out of date" entry goes.
            if (!modules::write_rescue_note(sd, app::version(), rescue::revision(), &err))
                brls::Logger::warning("modules: playguard-rescue.txt not written: {}", err);
            rescue::note_version();
            record(target, update ? "updated" : "installed");
            ui::notify(brls::getStr(key(target, update ? "updated" : "installed")));
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
    ui::confirm_danger(brls::getStr(key(m, "remove_body")), "playguard/modules/remove"_i18n, [mid, refresh_tab]() {
        const modules::Module& target = modules::get(mid);
        if (!allowed()) return;
        const bool running = target.resident && module_running(target.tid, nullptr);
        const auto drop = [mid, running, refresh_tab]() {
            const modules::Module& gone = modules::get(mid);
            if (running) {
                module_terminate(gone.tid);
                sync_flow::reload();
            }
            std::string err;
            if (!modules::uninstall(paths::sd_root(), gone, &err)) {
                ui::error("playguard/modules/error"_i18n + " — " + err);
            } else {
                record(gone, "removed");
                if (gone.id == modules::Id::Rescue) rescue::note_version();
                ui::notify("playguard/modules/removed"_i18n);
            }
            refresh_tab();
        };
        if (running) sync_flow::agent_stopping(drop);
        else drop();
    });
}

void ModuleTab::refresh()
{
    const modules::Module& m = modules::get(id);
    const modules::State s = modules::state(paths::sd_root(), m);
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), m);
    const modules::Offer offer = modules::offer(s, b);
    const bool running = m.resident && s.installed && module_running(m.tid, nullptr);

    NVGcolor color;
    status->setDetailText(state_text(id, false, &color));
    status->setDetailTextColor(color);

    action->setText(offer == modules::Offer::Update ? "playguard/modules/update"_i18n : "playguard/modules/install"_i18n);
    action->setDetailText(b.present ? b.version : "");
    ui::set_visible(action, offer != modules::Offer::None);
    boot->setOn(s.at_boot, false);
    ui::set_visible(boot, s.installed);
    // The recovery module only acts at boot: no Start / Stop.
    run->setText(running ? "playguard/modules/stop"_i18n : "playguard/modules/start"_i18n);
    ui::set_visible(run, m.resident && s.installed);
    ui::set_visible(remove, s.installed);
    this->list.sync();
}

// The rail, always open, its icons before the names: Cadran's own
// (cadran::style_rail) is the main screen's, the one that folds, and there is
// one. Its names shrink to fit, as there.
void dress_rail(brls::View* tab_frame)
{
    static const char* const ICONS[] = { "list", "", "unlock", "server" };
    auto* sidebar = dynamic_cast<brls::Sidebar*>(tab_frame->getView("brls/tab_frame/sidebar"));
    brls::SidebarItem* first = sidebar ? sidebar->getItem(0) : nullptr;
    brls::Box* list = first ? first->getParent() : nullptr;
    if (!list) return;
    list->setPadding(6, 8, 10, 16);
    size_t position = 0;
    for (brls::View* v : list->getChildren()) {
        const char* name = position < sizeof(ICONS) / sizeof(ICONS[0]) ? ICONS[position] : "";
        position++;
        auto* item = dynamic_cast<brls::SidebarItem*>(v);
        if (!item) continue;   // a separator
        item->setHeight(52);
        item->setPaddingLeft(12);
        item->setPaddingRight(12);
        item->setAlignItems(brls::AlignItems::CENTER);
        // The active tab says so in teal; no accent bar beside it.
        if (auto* accent = item->getView("brls/sidebar/item_accent")) {
            accent->setWidth(0);
            accent->setMargins(0, 0, 0, 0);
            accent->setVisibility(brls::Visibility::GONE);
        }
        if (auto* label = dynamic_cast<brls::Label*>(item->getView("brls/sidebar/item_label"))) {
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

std::string ModulesActivity::summary(modules::Id id, NVGcolor* color)
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
    tabs->addTab("playguard/modules/agent/tab"_i18n, []() { return new ModuleTab(modules::Id::Agent); });
    // Dressed here, not by the screens' skin (Cadran's), which would take
    // the sidebar for the list: the header, and the rail.
    cadran::style_header(dynamic_cast<brls::AppletFrame*>(this->getContentView()), nullptr, "", false, false);
    dress_rail(tabs.getView());

    if (start != Tab::Overview) {
        const int position = start == Tab::Rescue ? TAB_RESCUE : TAB_AGENT;
        brls::sync([this, position]() {
            auto stack = brls::Application::getActivitiesStack();
            if (!stack.empty() && stack.back() == this) tabs->focusTab(position);
        });
    }
}
