// LoadingActivity — the loading screen: the first activity, drawn before the
// rest of the start-up (main.cpp), which takes a moment on the console (the
// service probe, the SD-card reads, the main screen's build) and left the
// screen black until then. main.cpp runs the start-up one step per frame and
// moves the progress bar along; the first real screen is then pushed over this
// one, which stays at the bottom of the stack (borealis' Application::clear(),
// to remove it, is private). Nothing to press here.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "view/loading_bar.hpp"

class LoadingActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/loading.xml");

    // The start-up steps, in order.
    enum class Step
    {
        Console,    // probing the parental-control service
        Settings,   // the firmware choice, the PIN lock, a recovery report, an interrupted change
        Screen,     // building the first screen
    };

    void onContentAvailable() override;
    // `step` is running: its text under the bar, and the bar at its share.
    void show(Step step);

  private:
    BRLS_BIND(LoadingBar, bar, "loading_bar");
    BRLS_BIND(brls::Label, step_text, "loading_step");
};
