// NoticesActivity — the notification bell's list (action/notices): "To do",
// then "For information". A on an entry closes the list and runs its action
// (the fix, or the screen where it is made); an entry for information with
// nothing to do is put away. Rebuilt when the entries change; closes by
// itself once none is left.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <memory>

class NoticesActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/notices.xml");
    ~NoticesActivity() override;

    void onContentAvailable() override;

    // Y on the main screen: the list, or a toast when it is empty.
    static void open();

  private:
    int listener = 0;
    bool closing = false;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    brls::Box* content = nullptr;

    void rebuild();

    BRLS_BIND(brls::Box, root, "nt_root");
};
