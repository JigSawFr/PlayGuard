// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/modules_activity.hpp"

#include <cstdlib>
#include <fmt/format.h>
#include <memory>

#include "action/agent_update.hpp"
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
    const modules::Bundle b = modules::bundled(ModulesActivity::bundle_dir(), m);
    std::string text;
    NVGcolor c = ui::color_neutral();
    if (!s.installed) {
        text = b.present ? "playguard/modules/state/not_installed"_i18n : "playguard/modules/state/not_bundled"_i18n;
    } else if (modules::offer(s, b) == modules::Offer::Update) {
        text = brief ? "playguard/modules/short/update"_i18n : "playguard/modules/state/update"_i18n;
        c = ui::color_warn();
    } else {
        text = brief ? "playguard/modules/short/installed"_i18n : "playguard/modules/state/installed"_i18n;
        c = ui::color_ok();
    }
    if (!brief && m.resident && s.installed)
        text += " · " + (module_running(m.tid, nullptr) ? "playguard/modules/state/running"_i18n
                                                        : "playguard/modules/state/stopped"_i18n);
    if (color) *color = c;
    return text;
}

// One module's tab: what it does, then its cells. Tabs are deleted when
// another one is opened: `alive` tells an agent update that ends later.
class ModuleTab : public TabBase
{
  public:
    explicit ModuleTab(modules::Id id);
    ~ModuleTab() override { *alive = false; }

  protected:
    void refresh() override;

  private:
    modules::Id id;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

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
    BRLS_BIND(brls::Label,      intro,  "md_intro");
    BRLS_BIND(brls::DetailCell, rescue, "md_rescue");
    BRLS_BIND(brls::DetailCell, agent,  "md_agent");
};

// Sidebar positions (the separator counts).
constexpr int TAB_OVERVIEW = 0;
constexpr int TAB_RESCUE   = 2;
constexpr int TAB_AGENT    = 3;

ModulesOverview::ModulesOverview() : TabBase("xml/tab/modules_overview.xml")
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
}

void ModulesOverview::refresh()
{
    NVGcolor color;
    rescue->setDetailText(ModulesActivity::summary(modules::Id::Rescue, &color));
    rescue->setDetailTextColor(color);
    agent->setDetailText(ModulesActivity::summary(modules::Id::Agent, &color));
    agent->setDetailTextColor(color);
}

ModuleTab::ModuleTab(modules::Id id) : TabBase("xml/tab/module.xml"), id(id)
{
    const modules::Module& m = modules::get(id);
    header->setTitle(brls::getStr(key(m, "title")));
    about->setSingleLine(false);
    about->setText(brls::getStr(key(m, "about")));

    status->registerClickAction([id](brls::View*) {
        const modules::Module& m = modules::get(id);
        const modules::State s = modules::state(paths::sd_root(), m);
        const modules::Bundle b = modules::bundled(ModulesActivity::bundle_dir(), m);
        std::string text = brls::getStr(key(m, "title")) + "\n";
        text += "\n" + brls::getStr("playguard/modules/details/where", modules::dir(paths::sd_root(), m));
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
        const modules::Module& m = modules::get(this->id);
        std::string err;
        if (!allowed() || !modules::set_at_boot(paths::sd_root(), m, on, &err)) {
            if (!err.empty()) ui::error("playguard/modules/error"_i18n + " — " + err);
        } else {
            record(m, on ? "boot_on" : "boot_off");
        }
        this->refresh();
    });
    run->registerClickAction([this](brls::View*) {
        const modules::Module& m = modules::get(this->id);
        const bool running = module_running(m.tid, nullptr);
        if (!allowed()) return true;
        std::shared_ptr<bool> alive = this->alive;
        const modules::Id id = this->id;
        const auto act = [this, alive, id, running]() {
            const modules::Module& m = modules::get(id);
            const Result rc = running ? module_terminate(m.tid) : module_launch(m.tid);
            ui::notify_result(rc, running ? "playguard/modules/stopped"_i18n : "playguard/modules/started"_i18n,
                              "playguard/modules/error"_i18n);
            if (R_SUCCEEDED(rc)) record(m, running ? "stopped" : "started");
            // The link follows: the agent's, or PlayGuard's own.
            if (running || R_FAILED(rc)) sync_flow::reload();
            else sync_flow::agent_started();
            if (*alive) this->refresh();
        };
        if (running) sync_flow::agent_stopping(act);
        else act();
        return true;
    });
    remove->registerClickAction([this](brls::View*) {
        this->uninstall();
        return true;
    });
}

void ModuleTab::install()
{
    const modules::Module& m = modules::get(id);
    const modules::Bundle b = modules::bundled(ModulesActivity::bundle_dir(), m);
    if (!b.present) {
        ui::info("playguard/modules/details/not_bundled"_i18n);
        return;
    }
    const modules::State s = modules::state(paths::sd_root(), m);
    const bool update = s.installed;
    const std::string body = brls::getStr(key(m, update ? "update_body" : "install_body"), b.version);
    std::shared_ptr<bool> alive = this->alive;
    const modules::Id id = this->id;
    ui::confirm(body, update ? "playguard/modules/update"_i18n : "playguard/modules/install"_i18n, [this, alive, id, b, update]() {
        const modules::Module& m = modules::get(id);
        if (!allowed()) return;
        if (m.resident) {
            // The agent: stopped cleanly, swapped, started and seen answering,
            // or the previous one put back (action/agent_update).
            if (agent_update::running()) {
                ui::notify("playguard/modules/agent/busy"_i18n);
                return;
            }
            ui::notify(update ? "playguard/modules/agent/updating"_i18n : "playguard/modules/agent/installing"_i18n);
            agent_update::run(ModulesActivity::bundle_dir(), [this, alive](const agent_update::Outcome& o) {
                agent_update::tell(o);
                if (*alive) this->refresh();
            });
            return;
        }
        // The recovery module only acts at boot: nothing to stop or start.
        const std::string sd = paths::sd_root();
        std::string err;
        const bool ok = modules::install(sd, m, b, false, &err);
        if (!ok) {
            ui::error("playguard/modules/error"_i18n + " — " + err);
        } else {
            modules::confirm(sd, m);
            // Which PlayGuard it comes with, as the release's zip says: the
            // bell's "out of date" entry goes.
            if (!modules::write_rescue_note(sd, app::version(), rescue::revision(), &err))
                brls::Logger::warning("modules: playguard-rescue.txt not written: {}", err);
            rescue::note_version();
            record(m, update ? "updated" : "installed");
            ui::notify(brls::getStr(key(m, update ? "updated" : "installed")));
        }
        if (*alive) this->refresh();
    });
}

void ModuleTab::uninstall()
{
    const modules::Module& m = modules::get(id);
    if (!modules::state(paths::sd_root(), m).installed) return;
    std::shared_ptr<bool> alive = this->alive;
    const modules::Id id = this->id;
    ui::confirm_danger(brls::getStr(key(m, "remove_body")), "playguard/modules/remove"_i18n, [this, alive, id]() {
        const modules::Module& m = modules::get(id);
        if (!allowed()) return;
        const bool running = m.resident && module_running(m.tid, nullptr);
        const auto remove = [this, alive, id, running]() {
            const modules::Module& m = modules::get(id);
            if (running) {
                module_terminate(m.tid);
                sync_flow::reload();
            }
            std::string err;
            if (!modules::uninstall(paths::sd_root(), m, &err)) {
                ui::error("playguard/modules/error"_i18n + " — " + err);
            } else {
                record(m, "removed");
                if (m.id == modules::Id::Rescue) rescue::note_version();
                ui::notify("playguard/modules/removed"_i18n);
            }
            if (*alive) this->refresh();
        };
        if (running) sync_flow::agent_stopping(remove);
        else remove();
    });
}

void ModuleTab::refresh()
{
    const modules::Module& m = modules::get(id);
    const modules::State s = modules::state(paths::sd_root(), m);
    const modules::Bundle b = modules::bundled(ModulesActivity::bundle_dir(), m);
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
}
}   // namespace

ModulesActivity::ModulesActivity(Tab start) : start(start)
{
}

std::string ModulesActivity::bundle_dir()
{
#ifndef __SWITCH__
    if (const char* sim = std::getenv("PLAYGUARD_SIM_BUNDLED")) return sim;
#endif
    return BRLS_ASSET("sysmodules");
}

std::string ModulesActivity::summary(modules::Id id, NVGcolor* color)
{
    return state_text(id, true, color);
}

void ModulesActivity::onContentAvailable()
{
    tabs->addTab("playguard/modules/overview"_i18n, []() { return new ModulesOverview(); });
    tabs->addSeparator();
    tabs->addTab("playguard/modules/rescue/tab"_i18n, []() { return new ModuleTab(modules::Id::Rescue); });
    tabs->addTab("playguard/modules/agent/title"_i18n, []() { return new ModuleTab(modules::Id::Agent); });

    if (start != Tab::Overview) {
        const int position = start == Tab::Rescue ? TAB_RESCUE : TAB_AGENT;
        brls::sync([this, position]() {
            auto stack = brls::Application::getActivitiesStack();
            if (!stack.empty() && stack.back() == this) tabs->focusTab(position);
        });
    }
}
