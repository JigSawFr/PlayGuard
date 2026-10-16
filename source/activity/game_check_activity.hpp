// GameCheckActivity — Tools › Check the installed games: the games the HOME
// menu cannot start or draw (core/gamecheck.h), each with what the HOME menu
// shows, and how many of the installed games have a problem, in a section per
// way out: repaired here, installed again, the firmware or the SD card
// (gamecheck_category), grouped by problem in each. A on one: its
// icon, why and what to do, the fix as the first button when one action is
// the fix, then the other actions (accept the installed version again, check
// the data, remove the update or an add-on, remove the game; save data always
// kept). The last check shows at once (util/game_cache.hpp); then, off the
// main thread, the console's entries are listed and only the games whose
// entry changed are checked again (every game once a day, on another
// firmware, or on Ⓧ), each shown as soon as it is checked. After a change
// its game is checked again, and after a repair a toast says whether it left
// the list.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <memory>
#include <string>
#include <vector>

#include "util/game_cache.hpp"

#include "util/pctl_ops_c.hpp"

class GameCheckActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/game_check.xml");

    void onContentAvailable() override;

    // The last check's file (util/game_cache.hpp), also read and written by
    // action/game_notice at start-up.
    static std::string cache_file();

  private:
    // A repair waiting for the next read to tell whether it worked.
    // A section of the list: its title (with its count), what to do, its games.
    struct Group
    {
        brls::Box*    box   = nullptr;
        brls::Header* title = nullptr;
        brls::Label*  note  = nullptr;
        brls::Box*    list  = nullptr;
        brls::Button* save       = nullptr;   // Reinstall only: save the list
        brls::Button* remove_all = nullptr;   // Reinstall only: remove them all
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
    std::string timing;       // where the last check spent its time (developer mode)
    Pending pending;
    Group groups[GC_CAT_COUNT];
    std::vector<GameFacts> reinstall;   // the games of "Reinstall", as shown
    bool removing = false;              // remove_all() under way

    void save();
    // Lists the console's entries, then checks what changed (`all`: every
    // game), and `also` (0: none) whatever its entry says.
    void refresh(bool all, u64 also);
    void check(std::vector<GameRecord> records, bool full);
    void status_text(const std::string& text);
    void status_text(const std::string& text, NVGcolor color);
    std::string age_text() const;
    void show();
    // Whether `game`, checked again, changes the list (not its status line).
    bool changes_list(const GameFacts& game) const;
    void open(const GameFacts& game, GameIssue issue, u32 current_hos);
    void choose(const GameFacts& game, u32 actions);
    void run(const GameFacts& game, u32 action, const GameContent& content);
    void remove(const GameFacts& game);
    // The games of "Reinstall": their list saved (the JSON's path, "" and
    // why on failure), and all of them removed after one question.
    std::string save_list(std::string* error) const;
    void remove_all();
    void remove_next(std::vector<GameFacts> games, size_t at, u32 removed, std::string path);
    void remove_content(const GameFacts& game, const GameContent& content);
    void reset_launch(const GameFacts& game);
    void verify(const GameFacts& game);

    BRLS_BIND(brls::Label,  note,   "gc_note");
    BRLS_BIND(brls::Label,  status, "gc_status");
    BRLS_BIND(brls::Box,    list,   "gc_list");
    BRLS_BIND(brls::Label,  hint,   "gc_hint");
};
