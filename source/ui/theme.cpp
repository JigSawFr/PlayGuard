// ui — the theme colours and the console's system font (see ui.hpp).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#ifdef __SWITCH__
#include <switch.h>
#else
#include <cstdlib>
#endif

#include "core/sim_env.h"
#include "util/text.hpp"

namespace ui
{

void register_theme_colors()
{
    // What cadran::install_theme() does not set: the empty part of a
    // progress bar and of a game's missing cover. The rest of the palette is
    // the boards' (ui/cadran_frame.cpp).
    brls::Theme::getLightTheme().addColor("brand/gauge_track", nvgRGBA(0, 0, 0, 34));
    brls::Theme::getDarkTheme().addColor("brand/gauge_track", nvgRGBA(255, 255, 255, 46));
#ifndef __SWITCH__
    // Desktop simulation, PLAYGUARD_SIM_STILL_FOCUS=1 (the smoke test): the
    // glow borealis moves around the focus with the clock stays where it is
    // (patches/borealis/0009); cadran::install_theme() draws it in the
    // stroke's own colour, so a screenshot does not depend on when it was taken.
    if (sim_env("PLAYGUARD_SIM_STILL_FOCUS")) brls::setHighlightAnimationStill(true);
#endif
}

void use_latin_font()
{
#ifdef __SWITCH__
    // borealis (wiliwili's branch) draws every label with the Simplified
    // Chinese system font, the Latin one only filling its gaps. That font has
    // the pinyin letters é, è, ê… as full-width glyphs ("prot é g é e"), and a
    // full-width "·" and "…". Chinese keeps it: the han in its own style.
    const std::string locale = brls::Application::getLocale();
    if (text::starts_with(locale, "zh")) return;

    PlFontData standard;
    if (R_FAILED(plGetSharedFontByType(&standard, PlSharedFontType_Standard))) return;
    const int chinese = brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED);
    // Registered again under the name the default font is read from (once,
    // by the first label): the font data is the system's, not copied.
    if (!brls::Application::loadFontFromMemory(brls::FONT_CHINESE_SIMPLIFIED, standard.address, standard.size, false))
        return;
    const int latin = brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED);
    // The fallbacks the Chinese font had (fontstash looks one level deep).
    NVGcontext* vg = brls::Application::getNVGContext();
    for (int font : { chinese, brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED_EXT),
                      brls::Application::getFont(brls::FONT_CHINESE_TRADITIONAL),
                      brls::Application::getFont(brls::FONT_KOREAN_REGULAR),
                      brls::Application::getFont(brls::FONT_SWITCH_ICONS),
                      brls::Application::getFont(brls::FONT_MATERIAL_ICONS),
                      brls::Application::getFont(brls::FONT_EMOJI) })
        if (font != brls::FONT_INVALID) nvgAddFallbackFontId(vg, latin, font);
#endif
}

NVGcolor color_ok()      { return brls::Application::getTheme()["brand/ok"]; }
NVGcolor color_warn()    { return brls::Application::getTheme()["brand/warn"]; }
NVGcolor color_bad()     { return brls::Application::getTheme()["brand/bad"]; }
NVGcolor color_track()   { return brls::Application::getTheme()["brand/gauge_track"]; }
NVGcolor color_note()    { return brls::Application::getTheme()["brand/note"]; }
NVGcolor color_neutral() { return brls::Application::getTheme()["brls/list/listItem_value_color"]; }
NVGcolor color_text()    { return brls::Application::getTheme()["brls/text"]; }
NVGcolor color_accent()  { return brls::Application::getTheme()["brls/sidebar/active_item"]; }

brls::Label* text_label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setSingleLine(false);
    l->setFontSize(size);
    l->setTextColor(color);
    l->setText(text);
    return l;
}

brls::Label* centered_label(const std::string& text, float size, NVGcolor color)
{
    brls::Label* l = text_label(text, size, color);
    l->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    l->setAlignSelf(brls::AlignSelf::STRETCH);   // wraps at the box's width (ui::dialog)
    return l;
}

}   // namespace ui
