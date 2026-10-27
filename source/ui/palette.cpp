// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/palette.hpp"

#include "ui/ui.hpp"

namespace ui
{

void install_palette()
{
    namespace pal = playguard::palette;
    const auto c = [](pal::Rgba x) { return nvgRGBA(x.r, x.g, x.b, x.a); };
    brls::Theme& dark = brls::Theme::getDarkTheme();
    brls::Theme& light = brls::Theme::getLightTheme();
    for (const pal::Key& key : pal::KEYS) {
        dark.addColor(key.name, c(key.dark));
        light.addColor(key.name, c(key.light));
    }
    for (const pal::Contrast& pair : pal::CONTRAST) brls::Theme::addContrastCheck(pair.ink, pair.surface, pair.minimum);
}

}   // namespace ui
