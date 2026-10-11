// LoadingActivity — the loading screen: the first activity, drawn once before
// the rest of the start-up (main.cpp), which takes a moment on the console
// (the service probe, the SD-card reads, the main screen's build) and left the
// screen black until then. main.cpp replaces it with the first real screen at
// the end of the first frame. Nothing to focus, and B does nothing.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class LoadingActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/loading.xml");

    void onContentAvailable() override;
};
