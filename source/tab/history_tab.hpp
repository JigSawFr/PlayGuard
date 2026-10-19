// HistoryTab — what happened and how to go back: the changes PlayGuard made
// and saw made outside it (their own screen, HistoryActivity), and the
// settings backups (save one now, restore one, how many are kept).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "tab/cadran_list.hpp"
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
    BRLS_BIND(brls::DetailCell, history,        "hs_history");
    BRLS_BIND(brls::DetailCell, backup_save,    "hs_backup_save");
    BRLS_BIND(brls::DetailCell, backup_restore, "hs_backup_restore");
    BRLS_BIND(brls::DetailCell, backup_keep,    "hs_backup_keep");
    BRLS_BIND(brls::Label,      backup_note,    "hs_backup_note");
};
