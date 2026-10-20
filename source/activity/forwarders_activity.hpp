// ForwardersActivity — Console › Forwarders: the homebrew on the HOME menu as
// games (NRO forwarders, core/forwarder.h), each with its icon. A on one:
// its details, rename it, change its publisher, install it again, delete
// it; one made by another app can be seen and deleted. Below: PlayGuard
// itself on the HOME menu in one press, or another homebrew of /switch.
// Above the list, whether the console can start them (its signature
// patches). The list is rebuilt after every change. Last, the homebrew menu
// locked (action/hbmenu_lock.hpp).
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

    void rebuild(uint64_t focus_id = 0);
    void show_patches();
    void show_self();
    void show_lock();
    void load_icons();
    void actions(const forwarders_logic::Row& row);
    void details(const forwarders_logic::Row& row);
    void rename(const forwarders_logic::Row& row);
    void change_author(const forwarders_logic::Row& row);
    void pick_homebrew();
    // Hours when a protected one starts without the PIN: the days, then the span.
    void pick_hours(const forwarders_logic::Row& row);
    // After a change: the list again, the focus on `id` when it is there.
    std::function<void(bool)> then_rebuild(uint64_t id = 0);

    BRLS_BIND(brls::Label,      note,      "fw_note");
    BRLS_BIND(brls::Label,      warning,   "fw_patches");
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
};
