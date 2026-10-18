// HistoryActivity — what PlayGuard changed on the console (util/history.hpp),
// newest first, one group per day. At the top, what is shown (everything,
// one kind of change, or the changes the console refused) and an export of
// what is shown (CSV, JSON, XLSX or PDF, in exports/). A on a change: its
// details and, when its previous value can be put back, the offer to
// (history_flow::open). The list is rebuilt after every change.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "action/history_logic.hpp"
#include "ui/cadran_kit.hpp"
#include "util/history.hpp"

class HistoryActivity : public brls::Activity
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

    BRLS_BIND(brls::Label,      note,   "hi_note");
    BRLS_BIND(brls::DetailCell, filter_cell, "hi_filter");
    BRLS_BIND(brls::DetailCell, export_cell, "hi_export");
    BRLS_BIND(brls::Box,        list,   "hi_list");
    BRLS_BIND(brls::Label,      empty,  "hi_empty");
};
