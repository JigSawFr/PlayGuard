// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/loading_activity.hpp"

void LoadingActivity::onContentAvailable()
{
    // Not borealis' back action: on the first activity it opens its own
    // "exit?" dialog, over a screen about to go.
    this->getContentView()->registerAction("", brls::BUTTON_B, [](brls::View*) { return true; }, true);
}
