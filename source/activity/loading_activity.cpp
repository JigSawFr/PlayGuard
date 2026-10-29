// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/loading_activity.hpp"

using namespace aurore::literals;

namespace
{
// The image slot of the wordmark (ui/palette.hpp).
constexpr const char* WORDMARK_SLOT = "playguard/loading/wordmark";
}   // namespace

LoadingActivity::~LoadingActivity()
{
    if (this->following) aurore::Application::getThemeChangedEvent()->unsubscribe(this->themeChanged);
}

void LoadingActivity::takeTheme()
{
    // The theme's background and inks (the XML's), so the screen does not
    // flash navy before the first one on the light theme (the icon keeps
    // its tile): there, the wordmark in its inks and the bar in the theme's
    // teal. Else the brand's bar, as the store banner.
    const aurore::ThemeVariant variant = aurore::Application::getPlatform()->getThemeVariant();
    const bool light = variant == aurore::ThemeVariant::LIGHT;
    this->bar->setBrand(!light);

    // The theme's picture behind the screens shows through this one too.
    const bool picture = aurore::Theme::getBackground(variant).hasImage();
    if (picture != this->transparent) {
        this->root->setThemeColor("backgroundColor", picture ? "cadran/none" : "aurore/background");
        this->transparent = picture;
    }

    // The theme's wordmark when it gives one, else PlayGuard's for the variant.
    Wordmark wanted = light ? Wordmark::Light : Wordmark::Dark;
    if (aurore::Theme::getPictures(variant).images.count(WORDMARK_SLOT)) wanted = Wordmark::Theme;
    if (wanted == this->shownWordmark) return;
    if (wanted == Wordmark::Theme)
        this->wordmark->setThemeImage(WORDMARK_SLOT);
    else
        this->wordmark->setImageFromRes(wanted == Wordmark::Light ? "img/wordmark-light.png" : "img/wordmark.png");
    this->shownWordmark = wanted;
}

void LoadingActivity::onContentAvailable()
{
    this->takeTheme();
    this->themeChanged = aurore::Application::getThemeChangedEvent()->subscribe([this](aurore::ThemeVariant) { this->takeTheme(); });
    this->following = true;
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
    // end. Its build (Today's first reading) is the longest part.
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
    this->introStart = aurore::getCPUTimeUsec();
    this->bar->fillOver(1.0f, INTRO_US);
    // The anchor holds the focus while this screen is the top one.
    for (aurore::ControllerButton button : { aurore::BUTTON_A, aurore::BUTTON_B })
        this->anchor->registerAction("", button, [this](aurore::View*) {
            this->skipped = true;
            return true;
        }, true);
}

bool LoadingActivity::inIntro() const
{
    return this->introStart && !this->skipped && aurore::getCPUTimeUsec() - this->introStart < INTRO_US;
}
