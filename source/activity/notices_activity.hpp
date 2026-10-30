// NoticesActivity — the notification bell's list (action/notices): "To do",
// then "For information", about the parental controls; then "Console", the
// console's entries on one line that A opens (open at once when they are all
// there is). A on an entry closes the list and runs its action
// (the fix, or the screen where it is made); an entry for information with
// nothing to do is put away. X puts one for information away, or ignores one
// to do the parent may ignore (notice_logic::ackable): those come last,
// under "Ignored", where A counts them again. Before them, a section per
// other Boréale homebrew with notices (action/shared_bell_logic), under its
// name: A starts it at the screen its notice names once PlayGuard exits
// (only when PlayGuard was started through hbloader; else it says where to
// go). Nothing of another homebrew runs here. Y puts every entry for
// information away. Rebuilt when the entries change; closes by itself once
// none is left.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <memory>
#include <vector>

#include "action/notices.hpp"
#include "action/shared_bell_logic.hpp"

class NoticesActivity : public aurore::Activity
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
    aurore::Box* content = nullptr;

    void rebuild();
    // rebuild()'s cells: the console's entries folded into one line, and an entry.
    aurore::DetailCell* console_line(const std::vector<notices::Entry>& all);
    aurore::DetailCell* entry_cell(const notices::Entry& e, bool todo);
    aurore::DetailCell* ignored_cell(const notices::Entry& e);
    aurore::DetailCell* other_cell(const shared_bell_logic::Other& o, const shared_bell_logic::Notice& n);

    AURORE_BIND(aurore::Box, root, "nt_root");
};
