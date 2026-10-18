// ConsoleTab — the console PlayGuard runs on: the network clock (its own
// screen, ClockTab), the installed games check, the firmware, Atmosphère and
// hekate (A on each opens its release notes), compatibility, storage, the
// serial number and the game patches.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/tab_base.hpp"

class InfoIcon;

class ConsoleTab : public TabBase
{
  public:
    ConsoleTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    bool        serial_revealed = false;   // until the tab is left (tabs are rebuilt when reopened)
    std::string storage_note;              // the other system's, shown by A on the storage row
    InfoIcon*   storage_info = nullptr;

    BRLS_BIND(brls::DetailCell,  clock,        "cn_clock");
    BRLS_BIND(brls::DetailCell,  games,        "cn_games");
    BRLS_BIND(brls::DetailCell,  fw,           "cn_fw");
    BRLS_BIND(brls::DetailCell,  ams,          "cn_ams");
    BRLS_BIND(brls::DetailCell,  hekate,       "cn_hekate");
    BRLS_BIND(brls::Label,       sys_advice,   "cn_sys_advice");
    BRLS_BIND(brls::DetailCell,  compat,       "cn_compat");
    BRLS_BIND(brls::DetailCell,  storage,      "cn_storage");
    BRLS_BIND(brls::DetailCell,  blank,        "cn_blank");
    BRLS_BIND(brls::DetailCell,  serial,       "cn_serial");
    BRLS_BIND(brls::Label,       serial_note,  "cn_serial_note");
    BRLS_BIND(brls::DetailCell,  game_patches, "cn_patches");
    BRLS_BIND(brls::Label,       patches_note, "cn_patches_note");
};
