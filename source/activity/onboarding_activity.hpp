// OnboardingActivity — "First steps": set a PIN, choose a daily limit, check
// the network clock. Each line shows whether the step is done and does it on
// A (the system PIN screen, the same limit picker as the Play timer tab, the
// guided clock flow), plus unlinking the phone app while it is linked, the
// alarm while it is off, and, once a PIN is set, Protect PlayGuard with the PIN. Shown
// at start-up while no PIN is set (writable builds on a checked firmware)
// unless turned off here; the bell's "no PIN" entry and About › Help &
// diagnostics bring it back.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

class OnboardingActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/onboarding.xml");

    void onContentAvailable() override;
    void willAppear(bool resetState = false) override;   // back from a step: the states again

    // Whether to show it by itself at start-up: no PIN yet, a build that can
    // set one (not read-only, checked firmware), and its switch left on.
    static bool wanted_at_start();

  private:
    bool clock_inaccurate = false;

    void refresh();

    AURORE_BIND(aurore::Label,      headline, "ob_headline");
    AURORE_BIND(aurore::DetailCell, pin,      "ob_pin");
    AURORE_BIND(aurore::DetailCell, limit,    "ob_limit");
    AURORE_BIND(aurore::DetailCell, clock,    "ob_clock");
    AURORE_BIND(aurore::DetailCell, unlink,   "ob_unlink");
    AURORE_BIND(aurore::DetailCell, alarm,    "ob_alarm");
    AURORE_BIND(aurore::DetailCell, protect,  "ob_protect");
    AURORE_BIND(aurore::Label,      note,     "ob_note");
    AURORE_BIND(aurore::Label,      storage_note, "ob_storage_note");
    AURORE_BIND(aurore::BooleanCell, at_start, "ob_at_start");
    AURORE_BIND(aurore::DetailCell, close,    "ob_close");
    AURORE_BIND(aurore::DetailCell, support,  "ob_support");
};
