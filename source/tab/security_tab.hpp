// SecurityTab — where the console stands as tiles (the PIN, restrictions,
// the temporary unlock, the homebrew menu), then the PIN (set / show),
// protecting PlayGuard and the homebrew menu (a line to Console ›
// Forwarders), temporary unlock, the companion phone app link, delete
// everything; last, Locked out? (the recovery sysmodule, in Console ›
// Optional modules). "Block all games" is in Play timer.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

#include "tab/tab_base.hpp"

class SecurityTab : public TabBase
{
  public:
    SecurityTab();
    static aurore::View* create();
    void willAppear(bool resetState = false) override;

  protected:
    void refresh() override;

  private:
    cadran::List list;
    // The homebrew menu's lock, read from the SD card when the tab appears
    // (back from Console › Forwarders too), not at each 5 s refresh; the
    // recovery module's line (Locked out?) too, after Optional modules.
    bool hbmenu_locked = false;
    // Set or change the PIN (the system PIN screen), recorded in the history.
    void change_pin();

    AURORE_BIND(aurore::DetailCell, pin,            "sc_pin");
    AURORE_BIND(aurore::DetailCell, restrictions,   "sc_restrictions");
    AURORE_BIND(aurore::DetailCell, temp,           "sc_temp");
    AURORE_BIND(aurore::DetailCell, hbmenu,         "sc_hbmenu");
    AURORE_BIND(aurore::DetailCell, hbmenu_lock,    "sc_hbmenu_lock");
    AURORE_BIND(aurore::Header,     actions_header, "sc_actions_header");
    AURORE_BIND(aurore::DetailCell, set_pin,        "sc_set_pin");
    AURORE_BIND(aurore::DetailCell, pin_lock_cell,  "sc_pin_lock");
    AURORE_BIND(aurore::Label,      pin_lock_note,  "sc_pin_lock_note");
    AURORE_BIND(aurore::DetailCell, show_pin,       "sc_show_pin");
    AURORE_BIND(aurore::DetailCell, unlock,         "sc_unlock");
    AURORE_BIND(aurore::BooleanCell, auto_relock,   "sc_auto_relock");
    AURORE_BIND(aurore::BooleanCell, pin_ask_again, "sc_pin_ask_again");
    AURORE_BIND(aurore::BooleanCell, pin_pauses,    "sc_pin_pauses");
    AURORE_BIND(aurore::DetailCell, pr_active,      "pr_active");
    AURORE_BIND(aurore::DetailCell, pr_updated,     "pr_updated");
    AURORE_BIND(aurore::DetailCell, pr_decode,      "pr_decode");
    AURORE_BIND(aurore::DetailCell, pr_unlink,      "pr_unlink");
    AURORE_BIND(aurore::Label,      pr_note,        "pr_note");
    AURORE_BIND(aurore::Header,     danger_header,  "sc_danger_header");
    AURORE_BIND(aurore::DetailCell, del,            "sc_delete");
    AURORE_BIND(aurore::DetailCell, rescue,         "sc_rescue");
    AURORE_BIND(aurore::Label,      rescue_about,   "sc_rescue_about");
};
