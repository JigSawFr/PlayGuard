// HistoryTab — what PlayGuard changed and the settings backups: how many
// changes are recorded and the last backup as tiles, then the change
// history and the backups' actions (in Preferences before).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "ui/cadran_list.hpp"
#include "tab/tab_base.hpp"

class HistoryTab : public TabBase
{
  public:
    HistoryTab();
    static brls::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    BRLS_BIND(brls::DetailCell, changes,        "hs_changes");
    BRLS_BIND(brls::DetailCell, last_backup,    "hs_last_backup");
    BRLS_BIND(brls::DetailCell, history,        "hs_history");
    BRLS_BIND(brls::DetailCell, backup_save,    "hs_backup_save");
    BRLS_BIND(brls::DetailCell, backup_restore, "hs_backup_restore");
    BRLS_BIND(brls::DetailCell, backup_keep,    "hs_backup_keep");
    BRLS_BIND(brls::Label,      backup_note,    "hs_backup_note");
};
