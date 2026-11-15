// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/agent_update.hpp"
#include "kit_text.hpp"

#include <aurore.hpp>
#include <format>
#include <memory>

#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/sync_flow.hpp"
#include "action/update_flow.hpp"
#include "ui/play_ui.hpp"
#include "util/agent_client.hpp"
#include "util/config.hpp"
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

// What the kit's flow asks of PlayGuard: the console's services, the link's
// hand-over (sync_flow), the change history, and its words.
sysmodule_flow::Host host()
{
    sysmodule_flow::Host h;
    h.sd_root = paths::sd_root();
    h.running = [](uint64_t tid) { return module_running(tid, nullptr); };
    h.launch = [](uint64_t tid) { const Result rc = module_launch(tid); return R_FAILED(rc) ? (uint32_t)rc : 0u; };
    h.terminate = [](uint64_t tid) { const Result rc = module_terminate(tid); return R_FAILED(rc) ? (uint32_t)rc : 0u; };
    h.rc_text = [](uint32_t rc) { return ui::rc_text((Result)rc); };
    // "offline" from the agent itself, then the next step.
    h.leave = [](std::function<void()> then) { sync_flow::agent_stopping(std::move(then)); };
    h.answers = answers;
    h.delay = [](long ms, std::function<void()> fn) { aurore::delay(ms, std::move(fn)); };
    h.hold = [](bool on) { sync_flow::hold(on); };
    h.link_back = []() { sync_flow::reload(); };   // the agent's session again (or PlayGuard's own when it is gone)
    h.record = record;
    h.log = [](const std::string& line) { aurore::Logger::info("agent {}", line); };
    h.warn = [](const std::string& why) {
        aurore::Logger::warning("agent update: the agent is in place, {}", why);
        aurore::sync([]() { ui::notify("playguard/modules/meta_err"_i18n, ui::Toast::Refused); });
    };
    h.busy_text = "playguard/modules/agent/busy"_i18n;
    h.not_bundled_text = "playguard/modules/details/not_bundled"_i18n;
    h.no_answer_text = "playguard/modules/agent/no_answer"_i18n;
    return h;
}
}   // namespace

bool running()
{
    return sysmodule_flow::running();
}

void run(const std::string& bundle_dir, std::function<void(const Outcome&)> done)
{
    sysmodule_flow::run(host(), agent(), modules::bundled(bundle_dir, agent()), std::move(done));
}

void tell(const Outcome& o)
{
    switch (o.end) {
        case modules::Step::Done:
            ui::notify(o.update ? "playguard/modules/agent/updated"_i18n : "playguard/modules/agent/installed"_i18n);
            break;
        case modules::Step::RolledBack:
            ui::error(aurore::getStr(o.update ? "playguard/modules/start_failed" : "playguard/modules/agent/install_undone",
                                   o.why));
            break;
        default: ui::error(aurore::getStr("playguard/modules/agent/update_failed", o.why)); break;
    }
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
    dialog->addButton("playguard/modules/update"_i18n, [bundle_dir]() {
        if (!pin_lock::allow_change()) {
            ui::notify(pin_lock::refusal_text());
            return;
        }
        ui::notify("playguard/modules/agent/updating"_i18n);
        run(bundle_dir, [](const Outcome& o) { tell(o); });
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
