// ConsoleTab — the console PlayGuard runs on: the network clock (its own
// screen, ClockTab), the installed games check, the forwarders (homebrew on
// the HOME menu, ForwardersActivity), the firmware, Atmosphère, hekate and
// sys-patch (A on each opens its release notes), compatibility, storage, the
// serial number and the game patches.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

#include "tab/tab_base.hpp"

class RowIcon;

class ConsoleTab : public TabBase
{
  public:
    ConsoleTab();
    static aurore::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    std::string storage_note;              // the other system's, shown by A on the storage row
    RowIcon*    storage_info = nullptr;
    RowIcon*    blank_warn   = nullptr;    // the serial number not blanked on emuMMC, by A
    RowIcon*    serial_info  = nullptr;    // where the real serial number comes from, by A
    std::string patches_note;              // what the game patches found, shown by A on their row
    RowIcon*    patches_icon = nullptr;
    RowIcon*    fw_warn      = nullptr;    // a newer firmware not supported yet (its notes say why)

    AURORE_BIND(aurore::DetailCell,  clock,        "cn_clock");
    AURORE_BIND(aurore::DetailCell,  games,        "cn_games");
    AURORE_BIND(aurore::DetailCell,  forwarders,   "cn_forwarders");
    AURORE_BIND(aurore::DetailCell,  remote,       "cn_remote");
    AURORE_BIND(aurore::DetailCell,  modules_cell, "cn_modules");
    AURORE_BIND(aurore::DetailCell,  fw,           "cn_fw");
    AURORE_BIND(aurore::DetailCell,  ams,          "cn_ams");
    AURORE_BIND(aurore::DetailCell,  hekate,       "cn_hekate");
    AURORE_BIND(aurore::DetailCell,  syspatch,     "cn_syspatch");
    AURORE_BIND(aurore::DetailCell,  compat,       "cn_compat");
    AURORE_BIND(aurore::DetailCell,  storage,      "cn_storage");
    AURORE_BIND(aurore::DetailCell,  blank,        "cn_blank");
    AURORE_BIND(aurore::DetailCell,  serial,       "cn_serial");
    AURORE_BIND(aurore::DetailCell,  game_patches, "cn_patches");
};
