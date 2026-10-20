// api_flow — the launch API (docs/api.md): another homebrew starts PlayGuard
// with a request (`--api "POST /v1/clock/sync"` or `--api-request <file>`);
// PlayGuard answers it on a screen of its own instead of the main screen,
// writes the JSON response, then starts the caller again (`return_to`) or
// exits. What is allowed is the parent's choice, in Preferences › Other
// homebrews; what needs the PIN asks for it, whatever the request says.
// The UI-free part (requests, routes, permissions, responses) is
// action/api_logic.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

#include "action/api_logic.hpp"

namespace brls
{
class Activity;
}

namespace api_flow
{

// In main(), once config::load() ran: reads the launch arguments and, for a
// request file, reads it and deletes it (a request acts once). Without a
// request, deletes the responses holding the child's play still on the card.
void at_start(int argc, const char* const* argv);

// The language the request asks for ("" none), for the screen and the
// errors' titles: applied before borealis loads its translations.
std::string lang();

// There is a request to answer: the first screen is activity(), not the
// main screen (choose_first).
bool pending();
brls::Activity* activity();

// The main screen, once at start-up: in the notification bell, what other
// homebrews changed in the last 7 days, and the calls refused for want of a
// permission when there were 3 or more (an app the parent may want to allow,
// or one trying again and again).
void note_calls();

// Preferences › Other homebrews, behind the PIN (from the bell, the request
// screen's Allow…).
void open_settings();

// Preferences › Other homebrews.
api_logic::Settings settings();
std::vector<api_logic::LogEntry> calls();
// Every group back to None (the switch stays as it is). Saves the settings.
void turn_all_off();
// "Network clock", "Status" …
std::string group_name(const std::string& group);
// "Not allowed", "Read", "Read and change".
std::string level_text(api_logic::Level l);

}   // namespace api_flow
