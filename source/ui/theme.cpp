// ui — the palette's colours by role, and the console's system font (see ui.hpp).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "util/text.hpp"

namespace ui
{

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

NVGcolor color_ok()      { return brls::Application::getTheme()["cadran/teal"]; }
NVGcolor color_warn()    { return brls::Application::getTheme()["cadran/amber"]; }
NVGcolor color_bad()     { return brls::Application::getTheme()["cadran/coral"]; }
NVGcolor color_track()   { return brls::Application::getTheme()["cadran/well"]; }
NVGcolor color_note()    { return brls::Application::getTheme()["cadran/text3"]; }
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
