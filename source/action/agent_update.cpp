// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/agent_update.hpp"
#include "kit_text.hpp"

#include <aurore.hpp>
#include <format>
#include <memory>

#include "action/agent_manage_flow.hpp"
#include "action/bundled_modules.hpp"
#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/sync_flow.hpp"
#include "action/update_flow.hpp"
#include "ui/play_ui.hpp"
#include "util/agent_client.hpp"
#include "util/config.hpp"
#include "util/modules.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

namespace agent_update
{

namespace
{
constexpr long START_DELAY_MS = 2500;   // after support_flow's offers (1.5 s)
constexpr long FREE_STEP_MS = 2000;
constexpr int  FREE_STEPS = 30;

const modules::Module& agent()
{
    return modules::get(modules::Id::Agent);
}

void record(const char* what)
{
    history_flow::record_event("module", "", aurore::getStr(std::string("playguard/modules/done/") + what, agent().title));
}

// Whether the new agent answers: pg:agent registered and PlayGuard's Hello accepted.
sysmodule_flow::Answer answers(std::string& why)
{
    AgentHelloReply reply{};
    Result rc = 0;
    if (!agent_client::available() || !agent_client::open(&reply, &rc)) return sysmodule_flow::Answer::Waiting;
    const bool ok = reply.accepted;
    agent_client::close();
    reply.version[sizeof(reply.version) - 1] = '\0';
    if (ok) {
        aurore::Logger::info("agent update: the agent {} answers", reply.version);
        return sysmodule_flow::Answer::Yes;
    }
    why = aurore::getStr("playguard/modules/agent/other_protocol", (int)reply.protocol);
    return sysmodule_flow::Answer::Refused;
}

// What boreale-kit's agent flow asks of PlayGuard: the link's hand-over
// (sync_flow), the change history, and the two ways PlayGuard has of its own:
// the agent asked to leave through its session (and the neutral control), and
// its Hello on pg:agent, which also tells the protocol PlayGuard speaks.
}   // namespace

void install_hooks()
{
    static bool done = false;
    if (done) return;
    done = true;
    auto& h = agent_manage_flow::hooks();
    h.bundle_dir = []() { return bundled_modules::dir(); };
    h.hold = [](bool on) { sync_flow::hold(on); };
    // The agent that was asked to leave is back already (the flow resumed it);
    // PlayGuard's own session again, or its own link when the agent is gone.
    h.link_back = []() { sync_flow::reload(); };
    h.record = record;
    h.leave = [](std::function<void()> then) { sync_flow::agent_stopping(std::move(then)); };
    h.answers = answers;
    // The PIN, as for a console change: the agent can change the console.
    h.allowed = []() {
        if (pin_lock::allow_change()) return true;
        ui::notify(pin_lock::refusal_text());
        return false;
    };
}

bool running()
{
    return agent_manage_flow::busy();
}

void run(std::function<void()> done)
{
    install_hooks();
    agent_manage_flow::install(std::move(done));
}

namespace
{
// The offer, once `main` is on top: tried every 2 s for a minute (What's new,
// a question, the update screen in front of it first), then left for the
// next start.
void offer_when_free(aurore::Activity* main, const std::string& bundle_dir, int tries_left)
{
    auto stack = aurore::Application::getActivitiesStack();
    if (stack.empty() || stack.back() != main || sysmodule_flow::running()) {
        if (tries_left > 0)
            aurore::delay(FREE_STEP_MS, [main, bundle_dir, tries_left]() { offer_when_free(main, bundle_dir, tries_left - 1); });
        return;
    }
    // A newer PlayGuard first (found meanwhile too): it carries a newer agent.
    if (update_flow::pending_at_start()) return;
    const modules::Module& m = agent();
    const modules::Bundle b = modules::bundled(bundle_dir, m);
    if (modules::offer(modules::state(paths::sd_root(), m), b) != modules::Offer::Update) return;
    if (config::get().agent_update_skipped == b.sha256) return;
    std::string body = aurore::getStr("playguard/modules/agent/offer_body", b.version);
    if (sync_flow::status().agent_refused) body += "\n\n" + "playguard/modules/agent/offer_refused"_i18n;
    const std::string sha = b.sha256;
    aurore::Dialog* dialog = ui::dialog(body);
    dialog->addButton(kit_text::str("common/later"), [sha]() {
        // Not at start again for this build (Console › Optional modules
        // still offers it).
        config::get().agent_update_skipped = sha;
        ui::save_config();
    });
    dialog->addButton("playguard/modules/update"_i18n, []() {
        run(nullptr);
    });
    dialog->setCancelable(true);
    dialog->open();
}
}   // namespace

void at_start(aurore::Activity* main, const std::string& bundle_dir)
{
    aurore::delay(START_DELAY_MS, [main, bundle_dir]() { offer_when_free(main, bundle_dir, FREE_STEPS); });
}

}   // namespace agent_update
