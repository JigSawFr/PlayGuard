// LockActivity — shown instead of the main screen when PIN › Protect PlayGuard
// with the PIN is "To open PlayGuard": the system's PIN screen comes up at once; the
// right PIN opens the main screen, B quits. Also pushed again over the open
// screens when PlayGuard comes back after a while away (lock_again()): the
// right PIN then uncovers them.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include "action/session_pause.hpp"

class LockActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/lock.xml");

    // `over`: pushed over the screens already open, which the PIN uncovers.
    explicit LockActivity(bool pushed_over = false) : over(pushed_over) {}

    void onContentAvailable() override;

    // Over whatever is open, unless a lock or recovery screen is already up.
    static void lock_again();

  private:
    bool over;
    session_pause::Hold hold;   // play counts as usual until the PIN uncovers PlayGuard

    void try_unlock();

    AURORE_BIND(aurore::Button, enter, "lock_enter");
};
