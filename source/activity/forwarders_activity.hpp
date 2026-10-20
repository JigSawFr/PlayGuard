// ForwardersActivity — Console › Forwarders: the homebrew on the HOME menu as
// games (NRO forwarders, core/forwarder.h), each with its icon. A on one:
// its details, rename it, change its publisher, install it again, delete
// it; one made by another app can be seen and deleted. Below: PlayGuard
// itself on the HOME menu in one press, or another homebrew of /switch.
// Above the list, whether the console can start them (its signature
// patches), and a card: the homebrew menu locked or open, its ways back,
// what opens it without the PIN, the rescue module. The list is rebuilt
// after every change. Last, the homebrew menu locked (action/hbmenu_lock.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <memory>
#include <vector>

#include "action/forwarders_logic.hpp"

class GameCell;

class ForwardersActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/forwarders.xml");

    void onContentAvailable() override;

  private:
    std::vector<forwarders_logic::Row> list_rows;
    std::vector<std::pair<uint64_t, GameCell*>> cells;   // for their icons
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    // The list again: the focus on `keep`, else on the forwarder `focus_id`.
    void rebuild(uint64_t focus_id = 0, brls::View* keep = nullptr);
    // A on the card: what it says, each a press away.
    void fixes();
    // Deleted, then ZR a few seconds to put it back.
    void remove_with_undo(const forwarders_logic::Row& row);
    // ZR for a few seconds runs `undo` (as the change history's).
    void offer_undo(std::function<void()> undo);
    void withdraw_undo();
    brls::ActionIdentifier undo_action = 0;
    size_t                 undo_timer = 0;
    void show_patches();
    void show_self();
    // The lock's cell and the card at the top.
    void show_lock();
    void load_icons();
    void actions(const forwarders_logic::Row& row);
    void details(const forwarders_logic::Row& row);
    void rename(const forwarders_logic::Row& row);
    void change_author(const forwarders_logic::Row& row);
    void pick_homebrew();
    // Hours when a protected one starts without the PIN: the days, then the span.
    // `closed`: the hours it never opens instead.
    void pick_hours(const forwarders_logic::Row& row, bool closed);
    // Sets them, then ZR a few seconds to put the ones before back.
    void hours(const forwarders_logic::Row& row, bool closed, int days, int from, int to);
    // Its icon: its .nro's, or a screenshot of the album.
    void pick_icon(const forwarders_logic::Row& row);
    // The pause after refused PINs: its rule.
    void pick_pause();
    // After a change: the list again, the focus on `id` when it is there.
    std::function<void(bool)> then_rebuild(uint64_t id = 0);

    BRLS_BIND(brls::Label,      note,      "fw_note");
    BRLS_BIND(brls::Label,      warning,   "fw_patches");
    BRLS_BIND(brls::Box,        summary,   "fw_summary");
    BRLS_BIND(brls::Box,        list,      "fw_list");
    BRLS_BIND(brls::Label,      empty,     "fw_empty");
    BRLS_BIND(brls::DetailCell, self_cell, "fw_self");
    BRLS_BIND(brls::DetailCell, other,     "fw_other");
    BRLS_BIND(brls::Label,      game_note, "fw_game_note");
    BRLS_BIND(brls::Header,     lock_header, "fw_lock_header");
    BRLS_BIND(brls::DetailCell, lock_cell, "fw_lock");
    BRLS_BIND(brls::Label,      lock_note, "fw_lock_note");
    BRLS_BIND(brls::DetailCell, reinstall, "fw_reinstall");
    BRLS_BIND(brls::DetailCell, protect_all, "fw_protect_all");
    BRLS_BIND(brls::DetailCell, lock_check, "fw_lock_check");
    BRLS_BIND(brls::DetailCell, pause_cell, "fw_pause");
};
