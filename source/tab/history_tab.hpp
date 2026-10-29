// HistoryTab — what PlayGuard changed and the settings backups: how many
// changes are recorded and the last backup as tiles, then the change
// history and the backups' actions (in Preferences before).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore/cadran/ui/cadran_list.hpp>

#include "tab/tab_base.hpp"

class HistoryTab : public TabBase
{
  public:
    HistoryTab();
    static aurore::View* create();

  protected:
    void refresh() override;

  private:
    cadran::List list;
    AURORE_BIND(aurore::DetailCell, changes,        "hs_changes");
    AURORE_BIND(aurore::DetailCell, last_backup,    "hs_last_backup");
    AURORE_BIND(aurore::DetailCell, history,        "hs_history");
    AURORE_BIND(aurore::DetailCell, backup_save,    "hs_backup_save");
    AURORE_BIND(aurore::DetailCell, backup_restore, "hs_backup_restore");
    AURORE_BIND(aurore::DetailCell, backup_keep,    "hs_backup_keep");
    AURORE_BIND(aurore::Label,      backup_note,    "hs_backup_note");
};
