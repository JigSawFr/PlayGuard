// HistoryActivity — what PlayGuard changed on the console (util/history.hpp),
// newest first, one group per day. At the top, what is shown (everything,
// one kind of change, or the changes the console refused) and an export of
// what is shown (CSV, JSON, XLSX or PDF, in exports/). A on a change: its
// details and, when its previous value can be put back, the offer to
// (history_flow::open). The list is rebuilt after every change.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "action/history_logic.hpp"
#include "util/history.hpp"

class HistoryActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/history.xml");

    void onContentAvailable() override;

  private:
    void rebuild();
    void choose_filter();
    void export_shown();

    history_logic::Filter filter = history_logic::Filter::All;
    // Every entry, read once and again only after a change was recorded
    // (history_flow::changes()): a new filter does not read the file.
    std::vector<history::Entry> all;
    bool                        read = false;
    uint64_t                    read_at = 0;   // history_flow::changes() when read
    std::vector<history::Entry> shown;   // what rebuild() listed, for the export
    cadran::kit::Empty          empty_card;   // Cadran: in hi_empty's place
    // The lines of hi_list, kept here: the list's children are not only them
    // (a screen rising into place puts a mover first among them, ui/motion.cpp).
    std::vector<aurore::Box*>     slots;

    AURORE_BIND(aurore::Label,      note,   "hi_note");
    AURORE_BIND(aurore::DetailCell, filter_cell, "hi_filter");
    AURORE_BIND(aurore::DetailCell, export_cell, "hi_export");
    AURORE_BIND(aurore::DetailCell, week_cell, "hi_week");
    AURORE_BIND(aurore::Box,        list,   "hi_list");
    AURORE_BIND(aurore::Label,      empty,  "hi_empty");
};
