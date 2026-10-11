// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/loading_activity.hpp"

using namespace brls::literals;

void LoadingActivity::onContentAvailable()
{
    this->show(Step::Console);
}

void LoadingActivity::show(Step step)
{
    // How far the bar is while the step runs: the first screen showing is the
    // end. Its build (the Overview's first reading) is the longest part.
    switch (step) {
        case Step::Console:
            this->step_text->setText("playguard/loading/step_console"_i18n);
            this->bar->setProgress(0.15f);
            break;
        case Step::Settings:
            this->step_text->setText("playguard/loading/step_settings"_i18n);
            this->bar->setProgress(0.45f);
            break;
        case Step::Screen:
            this->step_text->setText("playguard/loading/step_screen"_i18n);
            this->bar->setProgress(0.8f);
            break;
    }
}
