// NoticesActivity — the notification bell's list (action/notices): "To do",
// then "For information", about the parental controls; then "Console", the
// console's entries on one line that A opens (open at once when they are all
// there is). A on an entry closes the list and runs its action
// (the fix, or the screen where it is made); an entry for information with
// nothing to do is put away. Rebuilt when the entries change; closes by
// itself once none is left.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <memory>
#include <vector>

#include "action/notices.hpp"

class NoticesActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/notices.xml");
    ~NoticesActivity() override;

    void onContentAvailable() override;
    void onResume() override;

    // Y on the main screen: the list, or a toast when it is empty.
    static void open();

  private:
    int listener = 0;
    bool closing = false;
    bool stale = false;   // changed under a dialog: rebuilt once back on top
    bool console_open = false;   // the console's entries shown, not only their line
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    brls::Box* content = nullptr;

    void rebuild();
    // rebuild()'s cells: the console's entries folded into one line, and an entry.
    brls::DetailCell* console_line(const std::vector<notices::Entry>& all);
    brls::DetailCell* entry_cell(const notices::Entry& e, bool todo);

    BRLS_BIND(brls::Box, root, "nt_root");
};
