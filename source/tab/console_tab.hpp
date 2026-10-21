// ConsoleTab — the console PlayGuard runs on: the network clock (its own
// screen, ClockTab), the installed games check, the forwarders (homebrew on
// the HOME menu, ForwardersActivity), the firmware, Atmosphère, hekate and
// sys-patch (A on each opens its release notes), compatibility, storage, the
// serial number and the game patches.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/cadran_list.hpp"
#include "tab/tab_base.hpp"

class RowIcon;

class ConsoleTab : public TabBase
{
  public:
    ConsoleTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    brls::Label* headline = nullptr;   // the state in one line, at the top (List::headline)
    bool        serial_revealed = false;   // until the tab is left (tabs are rebuilt when reopened)
    std::string storage_note;              // the other system's, shown by A on the storage row
    RowIcon*    storage_info = nullptr;
    RowIcon*    blank_warn   = nullptr;    // the serial number not blanked on emuMMC, by A
    std::string patches_note;              // what the game patches found, shown by A on their row
    RowIcon*    patches_icon = nullptr;
    RowIcon*    fw_warn      = nullptr;    // a newer firmware not supported yet (its notes say why)

    BRLS_BIND(brls::DetailCell,  clock,        "cn_clock");
    BRLS_BIND(brls::DetailCell,  games,        "cn_games");
    BRLS_BIND(brls::DetailCell,  forwarders,   "cn_forwarders");
    BRLS_BIND(brls::DetailCell,  fw,           "cn_fw");
    BRLS_BIND(brls::DetailCell,  ams,          "cn_ams");
    BRLS_BIND(brls::DetailCell,  hekate,       "cn_hekate");
    BRLS_BIND(brls::DetailCell,  syspatch,     "cn_syspatch");
    BRLS_BIND(brls::DetailCell,  compat,       "cn_compat");
    BRLS_BIND(brls::DetailCell,  storage,      "cn_storage");
    BRLS_BIND(brls::DetailCell,  blank,        "cn_blank");
    BRLS_BIND(brls::DetailCell,  serial,       "cn_serial");
    BRLS_BIND(brls::DetailCell,  game_patches, "cn_patches");
};
