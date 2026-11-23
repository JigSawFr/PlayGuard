// LogActivity — Accès à distance › Journal: the lines of the agent's and the
// link's log (sync_flow::log_lines()) as a screen of their own, newest first:
// the time beside each line, a line in amber when the agent's loop stopped,
// a filter (everything, the app, the link, the loop) and an export of what
// is shown (CSV, JSON, XLSX or PDF, in exports/). The lines are read when the
// screen opens and when the filter changes.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <vector>

#include "action/log_logic.hpp"

class LogActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/log.xml");

    void onContentAvailable() override;

  private:
    void rebuild();
    void choose_filter();
    void export_shown();

    log_logic::Filter filter = log_logic::Filter::All;
    std::vector<log_logic::Line> all;     // newest first
    std::vector<log_logic::Line> shown;   // what rebuild() listed, for the export
    // The lines of lg_list, kept here (the list's children are not only them).
    std::vector<aurore::DetailCell*> slots;

    AURORE_BIND(aurore::Label,      note,   "lg_note");
    AURORE_BIND(aurore::DetailCell, filter_cell, "lg_filter");
    AURORE_BIND(aurore::DetailCell, export_cell, "lg_export");
    AURORE_BIND(aurore::Box,        list,   "lg_list");
    AURORE_BIND(aurore::Label,      empty,  "lg_empty");
};
