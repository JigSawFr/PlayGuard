// forwarders_pickers — Console › Forwarders' pickers of a protected
// forwarder's rules (ForwardersActivity): its free hours or the hours it
// never opens, a span of them, the pause after refused PINs. Each picker
// stops once `alive` (the screen's) has expired.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <memory>
#include <string>

#include "action/forwarder_flow.hpp"
#include "action/forwarders_logic.hpp"

namespace forwarders_pickers
{

// A span: its start, then its end, under `title`; `done(from, to)` in
// minutes (an end before the start runs past midnight).
void span(const std::string& title, int from_hour, int to_hour, std::weak_ptr<bool> alive,
          std::function<void(int, int)> done);

// Hours when a protected one starts without the PIN: the days, then the
// span (the weekdays' then the weekend's, when apart). `closed`: the hours
// it never opens instead. `set(hours)` with what was picked (none: never).
void hours(const forwarders_logic::Row& row, bool closed, std::weak_ptr<bool> alive,
           std::function<void(const forwarder_flow::Hours&)> set);

// The pause after refused PINs: its rule, set (forwarder_flow::set_pause_rule),
// then `done` on the UI thread.
void pause(std::weak_ptr<bool> alive, std::function<void()> done);

// How long the opening screen stays once a protected one starts
// (forwarder_flow::set_launch_wait), then `done` on the UI thread.
void wait(std::weak_ptr<bool> alive, std::function<void()> done);

}   // namespace forwarders_pickers
