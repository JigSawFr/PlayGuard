// LoadingActivity — the loading screen: the first activity, drawn before the
// rest of the start-up (main.cpp), which takes a moment on the console (the
// service probe, the SD-card reads, the main screen's build) and left the
// screen black until then. main.cpp runs the start-up one step per frame and
// moves the progress bar along; the first real screen is then pushed over this
// one, which stays at the bottom of the stack (Aurore's Application::clear(),
// to remove it, is private).
// The intro (Preferences › Show the intro): the start-up takes
// a fraction of a second, too short to see this screen, so it stays at least
// INTRO_US, its bar filling at a steady pace; Ⓐ or Ⓑ skip the rest. Nothing
// else to press here.
// It follows the theme as it changes: its inks, the theme's background
// picture showing through it, the theme's picture for the wordmark (the
// image slot playguard/loading/wordmark, ui/palette.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

#include <aurore/cadran/view/loading_bar.hpp>

class LoadingActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/loading.xml");

    enum class Mode
    {
        Start,     // the start-up
        Closing,   // "Turn back on and quit": the last screen, the bar full, until PlayGuard is gone
    };

    explicit LoadingActivity(Mode shown = Mode::Start) : mode(shown) {}
    ~LoadingActivity() override;

    // The start-up steps, in order.
    enum class Step
    {
        Console,    // probing the parental-control service
        Settings,   // the firmware choice, the PIN lock, a recovery report, an interrupted change
        Screen,     // building the first screen
    };

    static constexpr aurore::Time INTRO_US = 2000000;

    void onContentAvailable() override;
    // `step` is running: its text under the bar, and the bar at its share
    // (during the intro, the bar keeps its pace).
    void show(Step step);
    // From now on, the intro: INTRO_US, unless skipped.
    void startIntro();
    // Still on the intro: started, not skipped, INTRO_US not over yet.
    bool inIntro() const;

  private:
    AURORE_BIND(LoadingBar, bar, "loading_bar");
    AURORE_BIND(aurore::Label, step_text, "loading_step");
    AURORE_BIND(aurore::Rectangle, anchor, "loading_anchor");
    AURORE_BIND(aurore::Image, wordmark, "loading_wordmark");
    AURORE_BIND(aurore::Box, root, "loading_root");

    // The look of the theme in use: the background (none over a theme's
    // picture), the wordmark, the bar's colours.
    void takeTheme();
    enum class Wordmark
    {
        Dark,    // wordmark.png, the XML's
        Light,   // wordmark-light.png
        Theme,   // the theme's picture of the slot
    };
    Wordmark   shownWordmark = Wordmark::Dark;
    bool       transparent   = false;
    bool       following     = false;
    aurore::Event<aurore::ThemeVariant>::Subscription themeChanged;

    Mode       mode;
    aurore::Time introStart = 0;   // 0: no intro
    bool       skipped    = false;
};
