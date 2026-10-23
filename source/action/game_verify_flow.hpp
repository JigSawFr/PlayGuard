// game_verify_flow — the dialog of the integrity check of Console › Check the
// installed games (GameCheckActivity::verify): the game under way (icon, name
// on one line), its place in a batch, a bar, the share done with how much and
// the time left, and Cancel (B too), which stops the batch after what was
// checked; and what the check's thread shares with it. Which games, and what
// to do with what the check found, stay in the activity.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <atomic>
#include <borealis.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "action/game_check_text.hpp"
#include "core/gamecheck.h"
#include "ui/cadran_modal.hpp"
#include "view/cover_strip.hpp"

namespace game_verify_flow
{

// The dialog and what its thread shares with it.
struct State
{
    std::atomic<bool> cancel{ false };
    bool open = true;   // the dialog is on screen (main thread only)
    brls::Dialog* dialog = nullptr;
    // Its heading "Checking the data", with the game's place in a batch;
    // its title the game's name.
    cadran::Progress body;
    CoverStrip* strip = nullptr;   // a batch's covers leafed through, above (its own icon then hidden)
    std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
};

// What the check of one game of a batch gave.
using Verified = game_check_text::Verified;

// The dialog, built but not open yet (main thread); for a batch (`ids`, in
// the order checked, more than one), the covers leafed through above.
std::shared_ptr<State> make_dialog(const std::vector<u64>& ids = {});

// The game under way, at `at` of `total`, in the dialog (main thread).
void show_game(State* s, const std::vector<unsigned char>& icon, const std::string& name, size_t at, size_t total);

// The console's integrity check of one game (any thread), the dialog
// following it; NXM_RC_CANCELLED when Cancel stopped it.
Result verify(const std::shared_ptr<State>& state, u64 app_id, bool* corrupt, Result* found);

// Closes the dialog if still open, then `then` (main thread).
void close(const std::shared_ptr<State>& state, std::function<void()> then);

}   // namespace game_verify_flow
