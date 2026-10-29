// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/loading_mark.hpp"

#include <cmath>
#include <string>

#include "util/paths.hpp"
#ifndef __SWITCH__
#include <cstdlib>
#include "core/sim_env.h"
#endif

namespace
{
// Aurore's aurora (branding/*.svg): its teal and its violet.
const NVGcolor AURORA_TEAL   = nvgRGB(0x00, 0xFF, 0xCC);
const NVGcolor AURORA_VIOLET = nvgRGB(0x8A, 0x5C, 0xFF);

constexpr float PI = 3.14159265f;
// One breath of the glow, teal to violet and back, in seconds.
constexpr float BREATH_S = 3.2f;

NVGcolor with_alpha(NVGcolor c, float a)
{
    c.a = a;
    return c;
}

// The image slot of the loading screen's picture (ui/palette.hpp).
constexpr const char* ART_SLOT = "playguard/loading/art";

// The app's icon (icon.jpg, img/app_icon.jpg in the resources) as a texture,
// made once; 0 when it cannot be read.
int icon_texture(NVGcontext* vg)
{
    static int texture = -1;
    if (texture < 0) {
        std::string bytes;
        texture = paths::read_file(AURORE_ASSET("img/app_icon.jpg"), bytes) && !bytes.empty()
                      ? nvgCreateImageMem(vg, NVG_IMAGE_GENERATE_MIPMAPS, (unsigned char*)bytes.data(), (int)bytes.size())
                      : 0;
    }
    return texture;
}
}   // namespace

LoadingMark::LoadingMark()
{
    this->setFocusable(false);
#ifndef __SWITCH__
    // The desktop smoke test compares screenshots: no movement there.
    this->still = sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS");
#endif
    this->takeThemePictures();
}

void LoadingMark::takeThemePictures()
{
    // The slot's picture only when the theme gives one (Theme::image hands
    // back PlayGuard's own otherwise, which this view draws its own way).
    const aurore::ThemeVariant variant = aurore::Application::getPlatform()->getThemeVariant();
    const aurore::ThemePictures pictures = aurore::Theme::getPictures(variant);
    const auto given = pictures.images.find(ART_SLOT);
    this->art = nullptr;
    if (given != pictures.images.end()) {
        std::shared_ptr<aurore::ThemeImage> image = aurore::Theme::image(ART_SLOT);
        if (image && image->path == given->second) this->art = image;
    }
    this->logo = aurore::Theme::logo();
}

void LoadingMark::onThemeChanged()
{
    aurore::View::onThemeChanged();
    this->takeThemePictures();
}

void LoadingMark::draw(NVGcontext* vg, float x, float y, float width, float height,
                       aurore::Style style, aurore::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    const float size = std::fmin(width, height);
    const float cx = x + width / 2.0f, cy = y + height / 2.0f;

    // The aurora's glow behind the tile, past the view's bounds, breathing
    // from its teal to its violet while the app starts.
    float violet = 0.35f;
    if (!this->still) {
        const double t = (double)aurore::getCPUTimeUsec() / 1e6;
        violet = 0.5f - 0.5f * std::cos((float)std::fmod(t / BREATH_S, 1.0) * 2.0f * PI);
    }
    const struct { NVGcolor colour; float share; } glows[] = { { AURORA_TEAL, 1.0f - violet }, { AURORA_VIOLET, violet } };
    for (const auto& g : glows) {
        NVGpaint glow = nvgRadialGradient(vg, cx, cy, size * 0.35f, size * 1.25f, with_alpha(g.colour, 0.22f * g.share),
                                          with_alpha(g.colour, 0.0f));
        nvgBeginPath(vg);
        nvgCircle(vg, cx, cy, size * 1.25f);
        nvgFillPaint(vg, glow);
        nvgFill(vg);
    }

    // The theme's picture, whole in the square: its loading picture (the
    // icon while it loads), else its logo; the icon when neither is ready.
    const aurore::ThemeImage* picture = nullptr;
    if (this->art && this->art->state != aurore::ThemeImage::State::FAILED) {
        if (this->art->ready()) picture = this->art.get();
    } else if (this->logo && this->logo->ready()) {
        picture = this->logo.get();
    }
    if (picture) {
        picture->draw(vg, cx - size / 2.0f, cy - size / 2.0f, size, size, aurore::BackgroundFit::CONTAIN);
        return;
    }

    // The icon, rounded as the HOME menu rounds it.
    const int texture = icon_texture(vg);
    if (!texture) return;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, cx - size / 2.0f, cy - size / 2.0f, size, size, size * 0.22f);
    nvgFillPaint(vg, nvgImagePattern(vg, cx - size / 2.0f, cy - size / 2.0f, size, size, 0.0f, texture, 1.0f));
    nvgFill(vg);
}

aurore::View* LoadingMark::create()
{
    return new LoadingMark();
}
