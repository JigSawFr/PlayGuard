// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/modules_activity.hpp"

#include <format>
#include <memory>

#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/agent_update.hpp"
#include "activity/agent_activity.hpp"
#include "action/bundled_modules.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/rescue.hpp"
#include "action/sync_flow.hpp"
#include "app.hpp"
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

}   // namespace

ModuleActivity::ModuleActivity(modules::Id which) : id(which)
{
}

void ModuleActivity::onResume()
{
    this->refresh();
    ui::keep_focus(this);   // after a dialog whose opener went (ui.hpp)
}

void ModuleActivity::onContentAvailable()
{
    const modules::Id which = this->id;
    about->setSingleLine(false);
    about->setText(aurore::getStr(key(modules::get(which), "about")));

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
    this->list = std::make_unique<cadran::List>(this->getContentView(), true);
    this->list->apply({ { "md_status", "info" }, { "md_action", "download" }, { "md_boot", "bolt" },
                        { "md_remove", "trash" } });
    this->refresh();
}

void ModuleActivity::install()
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
    const std::function<void()> refresh_tab = ui::guarded(this->alive, [this]() { this->refresh(); });
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

void ModuleActivity::uninstall()
{
    const modules::Module& m = modules::get(id);
    if (!modules::state(paths::sd_root(), m).installed) return;
    const modules::Id mid = this->id;
    const std::function<void()> refresh_tab = ui::guarded(this->alive, [this]() { this->refresh(); });
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

void ModuleActivity::refresh()
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
    if (this->list) this->list->sync();
}

std::string ModulesActivity::summary(modules::Id id, const char** color)
{
    return state_text(id, true, color);
}

std::string ModulesActivity::remote_state()
{
    const sync_flow::Status link = sync_flow::status();
    if (!link.enabled) return "playguard/sync/state_off"_i18n;
    const std::string text = link.running && link.link.state == SyncLink_Online ? "playguard/sync/state_online"_i18n
                                                                                  : "playguard/sync/state_offline"_i18n;
    // Held by the agent: said, so that the two sections of this screen read as one.
    return link.agent ? aurore::getStr("playguard/sync/state_agent", text) : text;
}

void ModulesActivity::refresh()
{
    // Each module as a tile: its state, with a check when it is as it should
    // be and a warning when it needs an update or a look.
    const auto tile = [this](aurore::DetailCell* cell, const char* id, const std::string& text, const char* color) {
        cell->setDetailText(text);
        const std::string c = color ? color : "";
        this->list->state(id, c == "cadran/teal" ? "cadran/teal_fill" : c == "cadran/amber" ? "cadran/amber_fill" : nullptr);
    };
    const char* color = nullptr;
    const std::string rescue_text = summary(modules::Id::Rescue, &color);
    if (this->list) tile(rescue, "md_rescue", rescue_text, color);
    std::string agent_text = AgentActivity::summary(&color);
    // The kit's line is long for a tile ("Not installed (not carried by this
    // build)"): the short one this screen already has for the other module.
    const modules::Module& agent_module = modules::get(modules::Id::Agent);
    if (!modules::state(paths::sd_root(), agent_module).installed &&
        !modules::bundled(bundled_modules::dir(), agent_module).present)
        agent_text = "playguard/modules/state/not_bundled"_i18n;
    if (this->list) tile(agent, "md_agent", agent_text, color);
    if (this->list) this->list->sync();
}

void ModulesActivity::onResume()
{
    this->refresh();   // back from a module's screen: its new state
    ui::keep_focus(this);   // after a dialog whose opener went (ui.hpp)
}

void ModulesActivity::onContentAvailable()
{
    intro->setSingleLine(false);
    rescue_note->setSingleLine(false);
    agent_note->setSingleLine(false);
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
    this->list = std::make_unique<cadran::List>(this->getContentView(), true);
    // The two modules side by side as tiles, what each one does under them.
    this->list->tiles({ { "md_rescue", "unlock" }, { "md_agent", "server" } }, true);
    this->list->apply({});
    this->refresh();
}
