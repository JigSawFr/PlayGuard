// GameActivity — one game of the Activity tab on its own screen: icon and
// name, the last seven days as bars, today / 7 days / all time, launches,
// first and last play, its age rating (and whether the level in force blocks
// it), and the all-time play time of each user account (read in the
// background when the screen opens: a few requests for this game only), with
// the time no account holds.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <memory>
#include <vector>

#include "util/pctl_ops_c.hpp"
#include "view/play_days.hpp"

class GameActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/game.xml");

    // `stats` is the Activity tab's read (for the weekday names and whether
    // the 7-day figures are meaningful); `icon` may be empty. `every_account`:
    // `game` holds every account's figures (the time no account holds can be
    // told), not one account's.
    GameActivity(GameStat game, std::shared_ptr<const PlayStats> stats, std::vector<unsigned char> icon,
                 bool every_account);

    void onContentAvailable() override;

  private:
    // The accounts' figures, once read (an error `rc` when there are none).
    void show_accounts(const std::vector<AccountPlay>& by_account, Result rc);

    GameStat game;
    std::shared_ptr<const PlayStats> stats;
    std::vector<unsigned char> icon_bytes;
    bool every_account;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);   // guards the accounts' read

    AURORE_BIND(aurore::Image,      icon,     "ga_icon");
    AURORE_BIND(aurore::Label,      name,     "ga_name");
    AURORE_BIND(PlayDaysView,     days,     "ga_days");
    AURORE_BIND(aurore::DetailCell, today,    "ga_today");
    AURORE_BIND(aurore::DetailCell, week,     "ga_week");
    AURORE_BIND(aurore::DetailCell, total,    "ga_total");
    AURORE_BIND(aurore::DetailCell, launches, "ga_launches");
    AURORE_BIND(aurore::DetailCell, first,    "ga_first");
    AURORE_BIND(aurore::DetailCell, last,     "ga_last");
    AURORE_BIND(aurore::DetailCell, rating,   "ga_rating");
    AURORE_BIND(aurore::Header,     accounts_header, "ga_accounts_header");
    AURORE_BIND(aurore::Box,        accounts, "ga_accounts");
    AURORE_BIND(aurore::Label,      accounts_note, "ga_accounts_note");
};
