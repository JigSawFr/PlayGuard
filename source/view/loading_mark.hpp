// LoadingMark — the PlayGuard icon (icon.jpg, from branding/icon.svg) drawn at
// the view's size for the loading screen, its corners rounded, over Aurore's
// aurora glow, which breathes from teal to violet while the app starts. Brand
// colours, not the theme's: the loading screen looks the same in both themes,
// like the store banner. A theme may replace the icon: with its picture of
// the image slot playguard/loading/art (ui/palette.hpp), else with its logo,
// drawn whole in the view (docs/themes.md); the icon until it is loaded, or
// when it cannot be read. It follows the theme as it changes.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <borealis/core/theme_image.hpp>

#include <memory>

class LoadingMark : public brls::View
{
  public:
    LoadingMark();

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    void onThemeChanged() override;

    static brls::View* create();

  private:
    bool still = false;   // desktop smoke test: the glow at rest
    // The theme's pictures for the variant in use: its loading picture,
    // its logo (nullptr without one).
    std::shared_ptr<brls::ThemeImage> art, logo;
    void takeThemePictures();
};
