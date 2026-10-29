// GamesActivity — Activity › All games: every game played over the chosen
// period, most played first, as a list or a grid of covers (Ⓨ switches, kept
// in the config), with the selected game's figures always on top (its icon
// and name, the period's play time, all time, launches, last play). Ⓧ picks
// the period, Ⓐ opens the game's own screen. The data is play_data's, for the
// account the Activity tab shows; a read ending while the screen is up
// updates it in place. A large library shows its first games, then every
// game on "Show every game" (capped in applet mode, without icons there).
// Icons are read around the selected game only (action/game_icons).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <memory>
#include <vector>

#include <aurore/cadran/view/scroll_view.hpp>

#include "util/pctl_ops_c.hpp"
#include "view/game_cell.hpp"
#include "view/game_tile.hpp"

class GamesActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/games.xml");

    // `period` as the Activity tab's (0 today, 1 last 7 days, 2 all time);
    // `account`: an index in play_data::accounts(), -1 for every account.
    GamesActivity(int period, int account);
    ~GamesActivity() override;

    void onContentAvailable() override;
    void onResume() override;

  private:
    // A game on screen: its cell (list) or its cover (grid).
    struct Entry
    {
        u64 app_id;
        aurore::View* view;
        GameCell* cell;   // list
        GameTile* tile;   // grid
        bool iconed;
    };

    const PlayAccount* account() const;
    void rebuild();
    // Every cell or cover built again, the focus back on `focus_id` when it
    // is still listed.
    void build(u64 focus_id);
    // The games built: the first ROWS_SHOWN, every one after "Show every
    // game" (APPLET_ROWS_MAX in applet mode).
    size_t cap() const;
    aurore::Box* next_row();
    void add_entry(size_t i);
    void add_more();
    void fill(Entry& e, const GameStat& g) const;
    // The selected game's figures on top (nullptr: none).
    void show_selected(const GameStat* g);
    void select(size_t i);
    void show_status();
    void set_hints();
    // Reads the icons around entry `i`, then puts them on the entries.
    void load_icons(size_t i);
    void apply_icons();
    void pick_period();
    void switch_view();

    int period;
    int account_index;
    bool grid = true;
    bool show_all = false;
    std::shared_ptr<const PlayStats> data;
    std::vector<const GameStat*> rows;   // the games of the period, most played first (into `data`)
    std::vector<Entry> entries;          // on screen, in the same order
    aurore::View* more = nullptr;          // "Show every game", after them, when rows are left out
    aurore::Box* grid_row = nullptr;       // the grid's row being filled
    size_t selected = 0;
    int listener = 0;                    // play_data::listen id
    bool stale = false;                  // a read ended under a game's screen or a dialog
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    AURORE_BIND(aurore::Image,      icon,           "gs_icon");
    AURORE_BIND(aurore::Label,      name,           "gs_name");
    AURORE_BIND(aurore::Label,      period_caption, "gs_period_caption");
    AURORE_BIND(aurore::Label,      period_value,   "gs_period");
    AURORE_BIND(aurore::Label,      total,          "gs_total");
    AURORE_BIND(aurore::Label,      launches,       "gs_launches");
    AURORE_BIND(aurore::Label,      last,           "gs_last");
    AURORE_BIND(aurore::DetailCell, view_cell,      "gs_view");
    AURORE_BIND(aurore::DetailCell, period_cell,    "gs_period_cell");
    AURORE_BIND(aurore::Box,        content,        "gs_content");
    AURORE_BIND(aurore::Box,        progress,       "gs_progress");
    AURORE_BIND(aurore::Box,        list,           "gs_list");
    AURORE_BIND(aurore::Label,      capped,         "gs_capped");
    AURORE_BIND(aurore::Label,      status,         "gs_status");
};
