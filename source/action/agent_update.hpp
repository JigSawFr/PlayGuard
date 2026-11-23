// agent_update — what PlayGuard gives boreale-kit's agent flow
// (action/agent_manage_flow: the steps, the rollback, the wait for the new
// agent's answer, without blocking the screen): the link's hand-over, the
// change history, its way to make the agent leave and to see it answer. Also
// the offer at start-up, when the agent installed is not the one PlayGuard
// carries.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

namespace aurore
{
class Activity;
}

namespace agent_update
{

// What boreale-kit's agent screen and flow ask of PlayGuard (the PIN, the
// link's hand-over, the history, its own way to see the agent answer): set
// once, before the screen opens or the flow runs.
void install_hooks();

// Installs or updates the agent from the copy PlayGuard carries, says how it
// went, records it in the change history, then hands the link over again;
// `done` is called when it has ended. The flow asks the PIN.
void run(std::function<void()> done);
bool running();

// Once the main screen is up, with nothing in front of it (tried every 2 s
// for a minute): the agent installed is another build than the one
// PlayGuard carries (or speaks another protocol), and the parent did not
// answer "Later" to this one. Not while a newer PlayGuard waits
// (update_flow::pending_at_start()): it carries a newer agent.
void at_start(aurore::Activity* main, const std::string& bundle_dir);

}   // namespace agent_update
