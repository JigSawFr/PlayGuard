// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/agent_update.hpp"

#include <aurore.hpp>
#include <fmt/format.h>
#include <memory>

#include "action/history_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/sync_flow.hpp"
#include "action/update_flow.hpp"
#include "ui/ui.hpp"
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
constexpr long CHECK_EVERY_MS = 500;
constexpr int  CHECK_TRIES = 20;        // 10 s

struct Run
{
    modules::Bundle bundle;
    bool            was_running = false;
    modules::Step   step = modules::Step::Failed;
    int             tries = 0;
    Outcome         out;
    std::function<void(const Outcome&)> done;
};

bool s_running = false;

const modules::Module& agent()
{
    return modules::get(modules::Id::Agent);
}

void record(const char* what)
{
    history_flow::record_event("module", "", aurore::getStr(std::string("playguard/modules/done/") + what, agent().title));
}

void finish(std::shared_ptr<Run> r)
{
    s_running = false;
    r->out.end = r->step;
    aurore::Logger::info("agent update: {}{}{}", modules::step_name(r->step), r->out.why.empty() ? "" : " — ",
                       r->out.why);
    switch (r->step) {
        case modules::Step::Done: record(r->out.update ? "updated" : "installed"); break;
        case modules::Step::RolledBack:
            if (r->out.update) record("rolled_back");
            break;
        default: record("failed"); break;
    }
    // The agent's session again (or PlayGuard's own when it is gone).
    sync_flow::hold(false);
    sync_flow::reload();
    if (r->done) r->done(r->out);
}

void go(std::shared_ptr<Run> r);

void advance(std::shared_ptr<Run> r, bool ok)
{
    aurore::Logger::info("agent update: {} {}", modules::step_name(r->step), ok ? "ok" : "failed");
    r->step = modules::next_step(r->step, ok, r->was_running);
    go(r);
}

// The new agent answers: pg:agent registered and PlayGuard's Hello accepted.
void check(std::shared_ptr<Run> r)
{
    AgentHelloReply reply{};
    Result rc = 0;
    if (agent_client::available() && agent_client::open(&reply, &rc)) {
        const bool ok = reply.accepted;
        agent_client::close();
        reply.version[sizeof(reply.version) - 1] = '\0';
        if (ok) aurore::Logger::info("agent update: the agent {} answers", reply.version);
        else r->out.why = aurore::getStr("playguard/modules/agent/other_protocol", (int)reply.protocol);
        advance(r, ok);
        return;
    }
    if (++r->tries >= CHECK_TRIES) {
        r->out.why = "playguard/modules/agent/no_answer"_i18n;
        advance(r, false);
        return;
    }
    aurore::delay(CHECK_EVERY_MS, [r]() { check(r); });
}

void go(std::shared_ptr<Run> r)
{
    using S = modules::Step;
    const modules::Module& m = agent();
    const std::string sd = paths::sd_root();
    std::string err;
    switch (r->step) {
        case S::Shutdown:
            // "offline" from the agent itself, then the next step.
            sync_flow::agent_stopping([r]() { advance(r, true); });
            return;
        case S::Stop:
        case S::StopNew: {
            const Result rc = module_terminate(m.tid);
            if (r->step == S::Stop && R_FAILED(rc)) r->out.why = ui::rc_text(rc);
            advance(r, R_SUCCEEDED(rc));
            return;
        }
        case S::Swap: {
            std::string warning;
            const bool ok = modules::install(sd, m, r->bundle, true, &err, &warning);
            if (!ok) r->out.why = err;
            if (!warning.empty()) {
                aurore::Logger::warning("agent update: the agent is in place, {}", warning);
                aurore::sync([]() { ui::notify("playguard/modules/meta_err"_i18n, ui::Toast::Refused); });
            }
            advance(r, ok);
            return;
        }
        case S::Start:
        case S::Restart: {
            const Result rc = module_launch(m.tid);
            if (R_FAILED(rc) && (r->step == S::Start || r->out.why.empty())) r->out.why = ui::rc_text(rc);
            advance(r, R_SUCCEEDED(rc));
            return;
        }
        case S::Check:
            r->tries = 0;
            check(r);
            return;
        case S::Restore: {
            // A first install that does not answer is removed; an update
            // gets the previous agent back.
            const bool ok = r->out.update ? modules::rollback(sd, m, &err) : modules::uninstall(sd, m, &err);
            if (!ok) r->out.why += (r->out.why.empty() ? "" : " — ") + err;
            advance(r, ok);
            return;
        }
        case S::Confirm:
            modules::confirm(sd, m);
            advance(r, true);
            return;
        case S::Done:
        case S::RolledBack:
        case S::Failed: finish(r); return;
    }
}
}   // namespace

bool running()
{
    return s_running;
}

void run(const std::string& bundle_dir, std::function<void(const Outcome&)> done)
{
    const modules::Module& m = agent();
    auto r = std::make_shared<Run>();
    r->bundle = modules::bundled(bundle_dir, m);
    r->done = std::move(done);
    if (s_running || !r->bundle.present) {
        r->out.why = s_running ? "playguard/modules/agent/busy"_i18n : "playguard/modules/details/not_bundled"_i18n;
        if (r->done) r->done(r->out);
        return;
    }
    s_running = true;
    r->out.update = modules::state(paths::sd_root(), m).installed;
    r->was_running = r->out.update && module_running(m.tid, nullptr);
    r->step = modules::first_step(r->was_running);
    aurore::Logger::info("agent update: to {} ({}), {}", r->bundle.version, r->bundle.sha256.substr(0, 12),
                       r->was_running ? "running" : "not running");
    // The link leaves the agent alone until the end.
    sync_flow::hold(true);
    go(r);
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
    if (stack.empty() || stack.back() != main || s_running) {
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
    dialog->addButton("playguard/common/later"_i18n, [sha]() {
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
