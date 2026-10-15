// GameCheckActivity — Tools › Check the installed games: the games the HOME
// menu cannot start or draw (core/gamecheck.h), each with what the HOME menu
// shows. A on one: the cause and what to do, then the actions that apply
// (accept the installed version again, remove the update or an add-on, check
// the data, remove the game; save data always kept). The games are read off
// the main thread when the screen opens and again after every change.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <memory>

#include "util/pctl_ops_c.hpp"

class GameCheckActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/game_check.xml");

    void onContentAvailable() override;

  private:
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    bool scanning  = false;
    bool verifying = false;   // one integrity check at a time
    bool focused   = false;   // the first game got the focus once

    void scan();
    void show(const GameCheck& check);
    void open(const GameFacts& game, GameIssue issue, u32 current_hos);
    void choose(const GameFacts& game, u32 actions);
    void remove(const GameFacts& game);
    void remove_content(const GameFacts& game, const GameContent& content);
    void reset_launch(const GameFacts& game);
    void verify(const GameFacts& game);

    BRLS_BIND(brls::Label,  note,   "gc_note");
    BRLS_BIND(brls::Label,  status, "gc_status");
    BRLS_BIND(brls::Header, header, "gc_header");
    BRLS_BIND(brls::Box,    list,   "gc_list");
    BRLS_BIND(brls::Label,  hint,   "gc_hint");
};
