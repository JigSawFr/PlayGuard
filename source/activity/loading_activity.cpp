// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/loading_activity.hpp"

using namespace brls::literals;

void LoadingActivity::onContentAvailable()
{
    if (this->mode == Mode::Closing) {
        this->step_text->setText("playguard/loading/step_closing"_i18n);
        this->bar->setProgress(1.0f);
        return;
    }
    this->show(Step::Console);
}

void LoadingActivity::show(Step step)
{
    // How far the bar is while the step runs: the first screen showing is the
    // end. Its build (the Overview's first reading) is the longest part.
    float share = 0.0f;
    switch (step) {
        case Step::Console:
            this->step_text->setText("playguard/loading/step_console"_i18n);
            share = 0.15f;
            break;
        case Step::Settings:
            this->step_text->setText("playguard/loading/step_settings"_i18n);
            share = 0.45f;
            break;
        case Step::Screen:
            this->step_text->setText("playguard/loading/step_screen"_i18n);
            share = 0.8f;
            break;
    }
    if (!this->introStart) this->bar->setProgress(share);
}

void LoadingActivity::startIntro()
{
    this->introStart = brls::getCPUTimeUsec();
    this->bar->fillOver(1.0f, INTRO_US);
    // The anchor holds the focus while this screen is the top one.
    for (brls::ControllerButton button : { brls::BUTTON_A, brls::BUTTON_B })
        this->anchor->registerAction("", button, [this](brls::View*) {
            this->skipped = true;
            return true;
        }, true);
}

bool LoadingActivity::inIntro() const
{
    return this->introStart && !this->skipped && brls::getCPUTimeUsec() - this->introStart < INTRO_US;
}
