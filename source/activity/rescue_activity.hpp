// RescueActivity — the recovery screen (action/rescue.hpp). Shown at start-up,
// before anything else, when the playguard-rescue sysmodule reports it acted
// on a RESCUE file: it says what happened and lets the parent set a new PIN
// or delete every parental control, then continue to PlayGuard. It never
// shows the PIN: a report is only a file, and a new PIN fixes a forgotten one.
//
// While it is on screen the PIN-before-a-change check is off, but only for a
// report the console confirms (rescue::confirmed): the sysmodule really
// unlocked (its proof, made with the stored PIN, says it wrote the report) or
// deleted, which needs it installed and the console restarted.
// The parent came here because they forgot the PIN. A report the console does
// not confirm (written by hand, or a recovery that failed) is only shown: no
// action is offered and "Open PlayGuard" goes through the usual lock screen.
// An unlock left by a module older than this PlayGuard (`old_module`: no
// proof, the console agreeing) says to update the module instead, as doing
// RESCUE again with it gives the same.
// Proceeding installs the normal gate and opens the app.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include "action/rescue.hpp"

class RescueActivity : public aurore::Activity
{
  public:
    RescueActivity(RescueReport report, bool confirmed, bool old_module = false);

    CONTENT_FROM_XML_RES("activity/rescue.xml");

    void onContentAvailable() override;
    void willAppear(bool resetState = false) override;

  private:
    RescueReport report;
    bool confirmed;
    bool old_module;

    void refresh();
    void proceed();

    AURORE_BIND(aurore::Label,      outcome,   "rc_outcome");
    AURORE_BIND(aurore::Label,      note,      "rc_note");
    AURORE_BIND(aurore::Header,     actions_header, "rc_actions_header");
    AURORE_BIND(aurore::DetailCell, reset_pin, "rc_reset_pin");
    AURORE_BIND(aurore::DetailCell, del,       "rc_delete");
    AURORE_BIND(aurore::DetailCell, cont,      "rc_continue");
};
