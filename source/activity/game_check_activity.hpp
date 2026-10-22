// GameCheckActivity — Console › Check the installed games: the games the HOME
// menu cannot start or draw (core/gamecheck.h), each with what the HOME menu
// shows, and how many of the installed games have a problem, in a section per
// way out: repaired here, to check first, installed again, the firmware or
// the SD card (gamecheck_category), grouped by problem in each. A on one: its
// icon, why and what to do, the fix as the first button when one action is
// the fix, then the other actions (accept the installed version again, check
// the data, remove the update or an add-on, remove the game; save data always
// kept). The last check shows at once (util/game_cache.hpp); then, off the
// main thread, the console's entries are listed and only the games whose
// entry changed are checked again (every game once a day, on another
// firmware, or on Ⓧ), each shown as soon as it is checked. After a change
// its game is checked again, and after a repair a toast says whether it left
// the list. What the integrity check found is kept with the game (corrupt:
// to install again; intact: out of the list) until another version is
// installed. Ⓨ selects games, for one action on them all: check their data
// one after the other, or remove them (the line that says so shows from the
// first game picked); "Check these games" does the same for the whole
// section to check. Ⓡ shows the games as a list or as covers (kept in the
// config), as Ⓨ does in Activity › All games. Above the games, a single
// short line: the result of the check.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "action/game_verify_flow.hpp"
#include "ui/cadran_kit.hpp"
#include "util/game_cache.hpp"

#include "util/pctl_ops_c.hpp"

class GameCheckActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/game_check.xml");

    void onContentAvailable() override;
    void onResume() override;

  private:
    // A repair waiting for the next read to tell whether it worked.
    // A section of the list: its title (with its count), what to do, its games.
    struct Group
    {
        brls::Box*    box   = nullptr;
        brls::Header* title = nullptr;
        brls::Label*  note  = nullptr;
        brls::Box*    list  = nullptr;      // the cells, or the rows of covers
        std::vector<brls::View*> tiles;     // as covers: in order, along the rows
        brls::Button* save       = nullptr;   // Reinstall only: save the list
        brls::Button* send       = nullptr;   // Reinstall only: send the list to a gist
        brls::Button* remove_all = nullptr;   // Reinstall only: remove them all
        brls::Button* verify_all = nullptr;   // To check only: check them all
        std::vector<GameFacts> games;         // as shown
    };

    struct Pending
    {
        u64 id = 0;
        std::string name;
        std::string done;   // the toast when the game is still in the list
    };

    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    game_cache::Cache cache;          // what is shown: the last check, updated game by game
    bool   checking  = false;         // listing or checking under way
    u32    check_done = 0, check_total = 0;
    Result list_rc   = 0;             // the console's entries could not be listed
    std::vector<u64> waiting;         // to check again once the check under way is over
    bool verifying = false;   // one integrity check at a time
    bool focused   = false;   // the first game got the focus once
    // The cell given the focus while the games are being checked, until the
    // user moves it: it follows the first game as the list grows above it.
    brls::View* auto_focus = nullptr;
    std::string timing;       // where the last check spent its time (developer mode)
    Pending pending;
    Group groups[GC_CAT_COUNT];
    bool removing = false;              // remove_games() under way
    std::set<u64> selected;             // the games picked with Ⓨ
    brls::DetailCell* selection = nullptr;   // above the sections, once a game is picked: A for what to do
    // In the sections' place when no game has a problem: how many look fine,
    // when they were checked, and Check again (the focus).
    cadran::kit::Empty all_clear;
    brls::Box* check_again = nullptr;
    bool grid = false;

    // A list or covers: every section's games built again, next frame, the
    // game with the focus keeping it.
    void switch_view();
    void set_view_hint();   // Ⓡ says what it switches to (registered once, in onContentAvailable)
    // The views of the games are deleted when rebuilt, and borealis keeps a
    // pointer to the one with the focus: the focus waits on the status line
    // meanwhile (focusable only then), and leaves it in show().
    void park_focus();
    u64 focused_game() const;            // the game whose cell or cover has the focus (0: none)
    brls::View* game_view(u64 id) const;   // its cell or cover (nullptr: not shown)
    // A section's games as covers: rebuilt when their number changes (the
    // up and down routes go with it), else updated in place.
    void show_tiles(Group& group, size_t count);
    // The icons of the covers shown, read off the main thread.
    void load_icons();

    // A section's box, title, note, actions and list, hidden until it has a game.
    void build_group(GameCategory category);
    void save();
    // Lists the console's entries, then checks what changed (`all`: every
    // game), and `also` (0: none) whatever its entry says.
    void refresh(bool all, u64 also);
    void check(std::vector<GameRecord> records, bool full);
    void status_text(const std::string& text);
    void status_text(const std::string& text, NVGcolor color);
    // The check under way: in the card when it shows, else on the status line.
    void checking_text(const std::string& text);
    void show();
    brls::View* first_game() const;
    // Whether `game`, checked again, changes the list (not its status line).
    bool changes_list(const GameFacts& game) const;
    void open(const GameFacts& game, GameIssue issue, u32 current_hos);
    void choose(const GameFacts& game, u32 actions);
    void run(const GameFacts& game, u32 action, const GameContent& content);
    // `asked`: the parent already said yes (a check's result offered it):
    // removed without asking again.
    void remove(const GameFacts& game, bool asked = false);
    // `games`' list saved off the UI thread, then `done` on it with the
    // JSON's path ("" and why on failure); and all of them removed after one
    // question (the list saved first).
    void save_list(const std::vector<GameFacts>& games,
                   std::function<void(const std::string& path, const std::string& error)> done) const;
    void remove_games(std::vector<GameFacts> games, bool asked = false);
    // remove_games()' dialog: the game under way, the bar, and Cancel.
    struct Removal;
    void remove_next(std::shared_ptr<Removal> removal, size_t at);
    // Short of some games, or stopped: how many went, the ones left, the list.
    static void removal_summary(const Removal& removal);
    // Ⓨ on a game; the actions on the games picked.
    void toggle(u64 id);
    void selection_menu();
    std::vector<GameFacts> selected_games() const;
    std::string selection_text() const;   // "3 games selected", or how to pick some
    void remove_content(const GameFacts& game, const GameContent& content);
    void reset_launch(const GameFacts& game);
    // The console's integrity check of `games`, one after the other in one
    // dialog; what it found kept with each game as it goes.
    void verify(std::vector<GameFacts> games);
    // What it found, once its dialog is closed; `self` only while `weak` lives.
    static void report_verified(GameCheckActivity* self, const std::weak_ptr<bool>& weak,
                                const std::vector<game_verify_flow::Verified>& results, size_t total);

    BRLS_BIND(brls::Label,  note,   "gc_note");
    BRLS_BIND(brls::Label,  status, "gc_status");
    BRLS_BIND(brls::Box,    list,   "gc_list");
    BRLS_BIND(brls::Label,  hint,   "gc_hint");
};
