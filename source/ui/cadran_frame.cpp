// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_frame.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "activity/main_activity.hpp"
#include "core/sim_env.h"
#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/effects.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "view/cadran_icon.hpp"
#include "view/title_state.hpp"

using namespace brls::literals;

void cadran::install_theme()
{
    const NVGcolor navy = nvgRGB(0x0B, 0x14, 0x24);
    // Dark: the boards' navy. Light: the style board's cool white, the inks
    // darkened to 4.5:1 on every surface they are drawn on, the rail's active
    // pill included (teal #0A705F, amber #8A5200, coral #B71C1C); the quiet
    // text too, on both (text3 4.5:1 on the second surface). The focus ring
    // in the dark teal, its glow no lighter than the fill teal: 3:1 or more
    // on the background at any point of its pulse (WCAG 1.4.11). The dark
    // theme's hairline at 14 % white: the cards stay apart on a TV that
    // crushes its blacks.
    struct P { NVGcolor bg, text, text2, text3, surf, surf2, line, teal, teal_fill, teal_soft, amber, amber_fill,
               coral, coral_fill, coral_line, track, glow, glow_strong, amber_surf, amber_line, pill_ok, pill_ok_ink,
               rail_active, key_bg, key_fg, hl1, hl2, border; };
    const P dark = { navy, nvgRGB(0xEE, 0xF2, 0xF8), nvgRGB(0xA9, 0xB5, 0xC9), nvgRGB(0x83, 0x92, 0xAA),
                     nvgRGB(0x12, 0x1E, 0x33), nvgRGB(0x1A, 0x2A, 0x45), nvgRGBA(255, 255, 255, 36),
                     nvgRGB(0x2E, 0xC4, 0xA6), nvgRGB(0x2E, 0xC4, 0xA6), nvgRGBA(0x2E, 0xC4, 0xA6, 26),
                     nvgRGB(0xFF, 0xB5, 0x47), nvgRGB(0xFF, 0xB5, 0x47),
                     nvgRGB(0xFF, 0x7A, 0x7A), nvgRGB(0xFF, 0x7A, 0x7A), nvgRGBA(0xFF, 0x7A, 0x7A, 140),
                     nvgRGBA(255, 255, 255, 23), nvgRGBA(0x2E, 0xC4, 0xA6, 31), nvgRGBA(0x2E, 0xC4, 0xA6, 51),
                     nvgRGBA(0xFF, 0xB5, 0x47, 26), nvgRGBA(0xFF, 0xB5, 0x47, 90),
                     nvgRGBA(0x2E, 0xC4, 0xA6, 41), nvgRGB(0x2E, 0xC4, 0xA6), nvgRGBA(0x2E, 0xC4, 0xA6, 36),
                     nvgRGB(0xEE, 0xF2, 0xF8), navy, nvgRGB(0x2E, 0xC4, 0xA6), nvgRGB(0x9A, 0xF0, 0xDC), nvgRGBA(255, 255, 255, 60) };
    const P light = { nvgRGB(0xED, 0xF0, 0xF4), navy, nvgRGB(0x3F, 0x4C, 0x63), nvgRGB(0x5F, 0x6E, 0x88),
                      nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0xF3, 0xF5, 0xF8), nvgRGBA(0x0B, 0x14, 0x24, 26),
                      nvgRGB(0x0A, 0x70, 0x5F), nvgRGB(0x14, 0xA3, 0x8A), nvgRGBA(0x14, 0xA3, 0x8A, 26),
                      nvgRGB(0x8A, 0x52, 0x00), nvgRGB(0xE0, 0x93, 0x1B),
                      nvgRGB(0xB7, 0x1C, 0x1C), nvgRGB(0xE5, 0x48, 0x4D), nvgRGB(0xE3, 0x9A, 0x9A),
                      nvgRGBA(0x0B, 0x14, 0x24, 26), nvgRGBA(0x2E, 0xC4, 0xA6, 41), nvgRGBA(0x2E, 0xC4, 0xA6, 56),
                      nvgRGB(0xFF, 0xF3, 0xDF), nvgRGB(0xF3, 0xD5, 0x9C),
                      nvgRGB(0xD8, 0xF3, 0xEC), nvgRGB(0x0A, 0x6E, 0x5C), nvgRGBA(0x14, 0xA3, 0x8A, 36),
                      navy, nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0x0A, 0x70, 0x5F), nvgRGB(0x0F, 0x8A, 0x74), nvgRGBA(0x0B, 0x14, 0x24, 60) };
    const std::pair<brls::Theme*, const P*> variants[] = { { &brls::Theme::getDarkTheme(), &dark }, { &brls::Theme::getLightTheme(), &light } };
    for (const auto& [t, p] : variants) {
        t->addColor("brls/background", p->bg);
        t->addColor("brls/clear", p->bg);
        t->addColor("brls/text", p->text);
        t->addColor("brls/sidebar/background", p->bg);
        t->addColor("brls/sidebar/separator", p->line);
        t->addColor("brls/sidebar/active_item", p->teal);
        t->addColor("brls/applet_frame/separator", p->line);
        t->addColor("brls/header/border", p->line);
        t->addColor("brls/highlight/background", p->surf);
        t->addColor("brls/highlight/color1", p->hl1);
        t->addColor("brls/highlight/color2", p->hl2);
#ifndef __SWITCH__
        // The smoke test's still focus (PLAYGUARD_SIM_STILL_FOCUS): the glow in
        // the stroke's own colour, so a screenshot does not depend on when.
        if (sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS")) t->addColor("brls/highlight/color2", p->hl1);
#endif
        t->addColor("brls/click_pulse", nvgRGBA(0x2E, 0xC4, 0xA6, 38));
        t->addColor("brls/button/primary_enabled_background", p->teal_fill);
        t->addColor("brls/button/primary_enabled_text", navy);
        t->addColor("brls/button/default_enabled_background", p->surf2);
        t->addColor("brls/button/default_enabled_text", p->text);
        t->addColor("brls/button/enabled_border_color", p->border);
        t->addColor("brls/button/highlight_enabled_text", p->teal);
        // A list line's plain value in the second ink, so the list screens
        // (cadran::List) take it as it is.
        t->addColor("brls/list/listItem_value_color", p->text2);
        t->addColor("cadran/navy", navy);
        t->addColor("cadran/text", p->text);
        t->addColor("cadran/text2", p->text2);
        t->addColor("cadran/text3", p->text3);
        t->addColor("cadran/teal", p->teal);
        t->addColor("cadran/teal_fill", p->teal_fill);
        t->addColor("cadran/teal_soft", p->teal_soft);
        t->addColor("cadran/amber", p->amber);
        t->addColor("cadran/amber_fill", p->amber_fill);
        t->addColor("cadran/coral", p->coral);
        t->addColor("cadran/coral_fill", p->coral_fill);
        t->addColor("cadran/coral_line", p->coral_line);
        t->addColor("cadran/coral_soft", p == &dark ? nvgRGBA(0xFF, 0x7A, 0x7A, 36) : nvgRGBA(0xE5, 0x48, 0x4D, 28));
        t->addColor("cadran/track", p->track);
        t->addColor("cadran/surf", p->surf);
        t->addColor("cadran/surf2", p->surf2);
        t->addColor("cadran/line", p->line);
        t->addColor("cadran/glow", p->glow);
        t->addColor("cadran/glow_strong", p->glow_strong);
        t->addColor("cadran/amber_surf", p->amber_surf);
        t->addColor("cadran/amber_line", p->amber_line);
        t->addColor("cadran/pill_ok", p->pill_ok);
        t->addColor("cadran/pill_ok_ink", p->pill_ok_ink);
        t->addColor("cadran/rail_active", p->rail_active);
        t->addColor("cadran/key_bg", p->key_bg);
        t->addColor("cadran/key_fg", p->key_fg);
        t->addColor("cadran/none", nvgRGBA(0, 0, 0, 0));
        // The empty part of a progress bar, behind a game's missing cover.
        t->addColor("cadran/well", p == &dark ? nvgRGBA(255, 255, 255, 46) : nvgRGBA(0, 0, 0, 34));
        t->addColor("cadran/bar", p == &dark ? nvgRGB(0x33, 0x48, 0x6B) : nvgRGB(0xB9, 0xC6, 0xDA));
        t->addColor("cadran/bar_today", p == &dark ? nvgRGB(0x5A, 0x71, 0x96) : nvgRGB(0x7E, 0x90, 0xAE));
        // Behind a dialog or a picker: the navy, dimmed. A toast: a card of
        // the second surface on the dark theme, navy on the light one.
        t->addColor("brls/backdrop", p == &dark ? nvgRGBA(3, 8, 18, 190) : nvgRGBA(0x0B, 0x14, 0x24, 115));
        t->addColor("brls/notification/background", p == &dark ? nvgRGBA(0x1A, 0x2A, 0x45, 250) : nvgRGBA(0x0B, 0x14, 0x24, 240));
        t->addColor("brls/notification/text", p == &dark ? p->text : nvgRGB(0xEE, 0xF2, 0xF8));
    }
#ifndef __SWITCH__
    // And held where it is (patches/borealis/0009).
    if (sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS")) brls::setHighlightAnimationStill(true);
#endif
    brls::Style style = brls::getStyle();
    style.addMetric("brls/applet_frame/header_height", 84.0f);
    // 232 px, not the boards' 196: the console's font is wider than the
    // boards' (Nunito Sans), and the longest names must stay whole
    // ("Tiempo de juego", "Einschränkungen", "Code PIN" and its badge).
    style.addMetric("brls/tab_frame/sidebar_width", 232.0f);
    style.addMetric("brls/sidebar/padding_top", 6.0f);
    style.addMetric("brls/sidebar/padding_right", 8.0f);
    style.addMetric("brls/sidebar/padding_bottom", 10.0f);
    style.addMetric("brls/sidebar/padding_left", 16.0f);
    style.addMetric("brls/sidebar/item_height", 52.0f);
    // The pickers' rows inside their card (cadran_modal), the toasts as
    // cards under the header, and the pills' hairline.
    style.addMetric("brls/dropdown/listPadding", 14.0f);
    style.addMetric("brls/dropdown/listPaddingSides", 20.0f);
    style.addMetric("brls/notification/width", 400.0f);
    style.addMetric("brls/notification/padding", 18.0f);
    style.addMetric("brls/notification/corner_radius", 16.0f);
    style.addMetric("brls/notification/margin_top", 96.0f);
    style.addMetric("brls/notification/margin_right", 32.0f);
    style.addMetric("brls/notification/spacing", 10.0f);
    style.addMetric("cadran/hairline", 1.0f);
    // The boards' display face for the figures (Sora Bold, OFL).
    if (!brls::Application::loadFontFromFile("cadran/digits", BRLS_ASSET("font/sora_bold_latin.ttf"))) {
        brls::Logger::warning("cadran: display font not loaded, the default one is used");
        return;
    }
    // Latin only: the units in other scripts ("40 мин", "+1 ч", "40分") from
    // the app's own fonts (fontstash looks one level deep: each of them).
    NVGcontext* vg = brls::Application::getNVGContext();
    const int digits = brls::Application::getFont("cadran/digits");
    for (int font : { brls::Application::getDefaultFont(), brls::Application::getFont(brls::FONT_REGULAR),
                      brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED),
                      brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED_EXT),
                      brls::Application::getFont(brls::FONT_CHINESE_TRADITIONAL),
                      brls::Application::getFont(brls::FONT_KOREAN_REGULAR) })
        if (font != brls::FONT_INVALID && font != digits) nvgAddFallbackFontId(vg, digits, font);
}

namespace
{
// The frame (header, rail) keeps its size with Preferences › Text size's
// larger text: it has no room to grow (ui/text_size).
void chrome(brls::Label* label, float size)
{
    label->setFontSize(size);
    label->setFontScaled(false);
}

class Glow : public brls::View
{
  public:
    Glow(float cx, float cy, float radius, const char* color_key)
        : cx(cx), cy(cy), radius(radius), glow(brls::Application::getTheme()[color_key]), clear(nvgRGBA(0x2E, 0xC4, 0xA6, 0))
    {
        this->setFocusable(false);
        this->setPositionType(brls::PositionType::ABSOLUTE);
        this->setPositionTop(0);
        this->setPositionLeft(0);
        this->setWidth(brls::Application::contentWidth);
        this->setHeight(brls::Application::contentHeight);
    }
    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style, brls::FrameContext*) override
    {
        static const bool off = sim_env_flag("PLAYGUARD_SIM_CADRAN_NOGLOW");
        if (off || !config::get().cadran_glow || effects::light()) return;
        const NVGpaint paint = nvgRadialGradient(vg, x + this->cx, y + this->cy, 0.0f, this->radius, this->glow, this->clear);
        // Only the square around the light: past its radius it is clear, so
        // the screen's other pixels are left alone (the same picture, about
        // half the fill).
        const float left = std::max(x, x + this->cx - this->radius), top = std::max(y, y + this->cy - this->radius);
        const float right = std::min(x + width, x + this->cx + this->radius), bottom = std::min(y + height, y + this->cy + this->radius);
        if (right <= left || bottom <= top) return;
        nvgBeginPath(vg);
        nvgRect(vg, left, top, right - left, bottom - top);
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }

  private:
    float cx, cy, radius;
    NVGcolor glow, clear;
};

CadranIcon* icon(const char* name, float size, NVGcolor color)
{
    auto* view = new CadranIcon();
    view->setIcon(name);
    view->setWidth(size);
    view->setHeight(size);
    view->setColor(color);
    return view;
}

// The sidebar items in their active / inactive look (teal on a pill, or
// text2), after the library's own colouring of the frame.
void tint_rail(const std::vector<brls::SidebarItem*>& items, brls::SidebarItem* active)
{
    brls::Theme theme = brls::Application::getTheme();
    for (brls::SidebarItem* it : items) {
        const bool on = it == active;
        it->setBackgroundColor(on ? theme["cadran/rail_active"] : theme["cadran/none"]);
        if (brls::View* accent = it->getView("brls/sidebar/item_accent")) accent->setVisibility(brls::Visibility::GONE);
        const NVGcolor color = on ? theme["cadran/teal"] : theme["cadran/text2"];
        for (brls::View* c : it->getChildren()) {
            if (auto* label = dynamic_cast<brls::Label*>(c)) label->setTextColor(color);
            else if (auto* ic = dynamic_cast<CadranIcon*>(c)) ic->setColor(color);
        }
    }
}
}   // namespace

namespace
{
// The rail's items (style_rail): their inner padding and their icons' size;
// and the header's mark.
constexpr float RAIL_ITEM_PAD = 12.0f;
constexpr float RAIL_ICON     = 22.0f;
constexpr float MARK_SIZE     = 38.0f;

// The header's pill: not focusable, but a tap reaches it (brls::View::hitTest
// skips a view that cannot take the focus).
class HeaderPill : public brls::Box
{
  public:
    HeaderPill() : brls::Box(brls::Axis::ROW) {}

    brls::View* hitTest(brls::Point point) override
    {
        if (this->getAlpha() == 0.0f || this->getVisibility() != brls::Visibility::VISIBLE) return nullptr;
        return this->getFrame().pointInside(point) ? this : nullptr;
    }
};

// The main screen's pill (style_frame): its label and icon, and the pill for
// hiding and colouring it.
brls::Box*   s_main_pill = nullptr;
brls::Label* s_main_pill_label = nullptr;
CadranIcon*  s_main_pill_icon = nullptr;
// What the screen shown last asked the pill to say.
std::string s_pill_text;
std::string s_pill_glyph = "check";
bool        s_pill_warn  = false;
}   // namespace

void cadran::set_pill(const std::string& text, const char* glyph, bool warn)
{
    s_pill_text  = text;
    s_pill_glyph = glyph ? glyph : "check";
    s_pill_warn  = warn;
    refresh_pill();
}

void cadran::refresh_pill()
{
    if (!s_main_pill || !s_main_pill_label || !s_main_pill_icon) return;
    // Unlocked: that first, on every screen (what a tab says comes back
    // once locked again).
    const bool unlocked = ui::known_unlocked();
    const std::string text = unlocked ? "playguard/cadran/pill/unlocked"_i18n : s_pill_text;
    const char* glyph = unlocked ? "unlock" : s_pill_glyph.c_str();
    const bool warn = unlocked || s_pill_warn;
    brls::Theme theme = brls::Application::getTheme();
    const NVGcolor ink = warn ? theme["cadran/amber"] : theme["cadran/pill_ok_ink"];
    s_main_pill_label->setText(text);
    s_main_pill_label->setTextColor(ink);
    s_main_pill_icon->setIcon(glyph);
    s_main_pill_icon->setColor(ink);
    s_main_pill->setBackgroundColor(warn ? theme["cadran/amber_surf"] : theme["cadran/pill_ok"]);
    s_main_pill->setBorderColor(theme["cadran/amber_line"]);
    s_main_pill->setBorderThickness(warn ? 1 : 0);
    s_main_pill->setVisibility(text.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
}

brls::View* cadran::make_glow(float cx, float cy, float radius, const char* color_key)
{
    return new Glow(cx, cy, radius, color_key);
}

void cadran::style_header(brls::AppletFrame* frame, const char* pill_icon, const std::string& pill_text, bool filled, bool with_date)
{
    if (!frame || frame->getView("cadran_mark")) return;
    brls::Theme theme = brls::Application::getTheme();
    brls::View* app_icon = frame->getView("brls/applet_frame/title_icon");
    brls::Box* left = app_icon ? app_icon->getParent() : nullptr;
    if (app_icon) app_icon->setVisibility(brls::Visibility::GONE);
    if (left) {
        left->setAlignItems(brls::AlignItems::CENTER);
        left->setGrow(1.0f);
        CadranIcon* mark = icon("mark", MARK_SIZE, theme["cadran/text"]);
        mark->setId("cadran_mark");
        mark->setMarginRight(14);
        if (with_date) {
            // The main screen: the mark centred over the rail's icons.
            brls::Style style = brls::getStyle();
            const float icons = style["brls/sidebar/padding_left"] + RAIL_ITEM_PAD + RAIL_ICON / 2;
            const float start = style["brls/applet_frame/padding_sides"] + style["brls/applet_frame/header_padding_sides"];
            mark->setMarginLeft(icons - MARK_SIZE / 2 - start);
            mark->setCulled(false);   // left of its box, which borealis would take for off-screen
        }
        left->addView(mark, 0);
    }
    if (auto* title = dynamic_cast<brls::Label*>(frame->getView("brls/applet_frame/title_label"))) {
        chrome(title, 24);
        title->setMarginTop(0);
    }
    if (!pill_icon) return;   // a screen of its own: the mark and the title
    auto* right = new brls::Box(brls::Axis::ROW);
    right->setAlignItems(brls::AlignItems::CENTER);
    const NVGcolor ink = filled ? theme["cadran/pill_ok_ink"] : theme["cadran/text2"];
    auto* pill = new HeaderPill();
    pill->setAlignItems(brls::AlignItems::CENTER);
    pill->setHeight(32);
    pill->setPaddingLeft(14);
    pill->setPaddingRight(14);
    pill->setCornerRadius(16);
    if (filled) {
        pill->setBackgroundColor(theme["cadran/pill_ok"]);
    } else {
        pill->setBorderColor(theme["cadran/line"]);
        pill->setBorderThickness(1);
    }
    CadranIcon* glyph = icon(pill_icon, 14, ink);
    glyph->setMarginRight(8);
    pill->addView(glyph);
    auto* pill_label = new brls::Label();
    pill_label->setText(pill_text);
    chrome(pill_label, 15);
    pill_label->setTextColor(ink);
    pill->addView(pill_label);
    right->addView(pill);
    if (with_date) {   // the main screen: its pill is the one set_pill() changes
        // Touched while unlocked, it goes where the padlock it replaces went.
        pill->addGestureRecognizer(new brls::TapGestureRecognizer(pill, []() {
            if (ui::known_unlocked()) TitleState::open(TitleState::Kind::UNLOCKED);
        }));
        s_main_pill = pill;
        s_main_pill_label = pill_label;
        s_main_pill_icon = glyph;
        auto* date = new brls::Label();
        date->setText(cadran::kit::long_date());
        chrome(date, 16);
        date->setTextColor(theme["cadran/text3"]);
        date->setMarginLeft(10);
        right->addView(date);
    }
    right->setMarginRight(16);   // clear of the header's tools (language, theme, bell) after it
    if (brls::Box* header = frame->getHeader()) header->addView(right, 1);
}

namespace
{
// Every screen but the main one (MainActivity styles that one, rail and all):
// the header after the boards, and its list in cards; a dialog as a card.
// Once per screen: the mark it adds says it is done.
void skin_screen_of(brls::View* focus)
{
    if (!focus) return;
    while (focus->hasParent()) focus = focus->getParent();
    if (auto* dialog = dynamic_cast<brls::Dialog*>(focus)) {
        cadran::style_dialog(dialog);
        return;
    }
    auto* frame = dynamic_cast<brls::AppletFrame*>(focus);
    if (!frame || frame->getView("main_tabs") || frame->getView("cadran_mark")) return;
    if (!frame->getView("brls/applet_frame/header")) return;
    cadran::style_header(frame, nullptr, "", false, false);
    if (!frame->getView("cadran_list")) cadran::List(frame, true).apply({});   // a Cadran screen dresses itself
}
}   // namespace

void cadran::install_screens()
{
    brls::Application::getGlobalFocusChangeEvent()->subscribe(skin_screen_of);
}

void cadran::style_rail(brls::View* tab_frame)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* sidebar = tab_frame ? dynamic_cast<brls::Sidebar*>(tab_frame->getView("brls/tab_frame/sidebar")) : nullptr;
    brls::SidebarItem* first = sidebar ? sidebar->getItem(0) : nullptr;
    brls::Box* list = first ? first->getParent() : nullptr;
    if (!list) return;
    list->setPadding(6, 8, 10, 16);
    // main.xml's order: Today | timer, activity, restrictions, PIN | console | history, preferences, about.
    static const char* icons[] = { "today", "", "timer", "activity", "shield", "lock", "", "chip", "", "history", "settings", "info" };
    std::vector<brls::SidebarItem*> items;
    int position = 0;
    for (brls::View* v : list->getChildren()) {
        const char* name = position < (int)(sizeof(icons) / sizeof(icons[0])) ? icons[position] : "";
        position++;
        auto* item = dynamic_cast<brls::SidebarItem*>(v);
        if (!item) {   // a separator: its line is drawn at mid-height
            // 13 px: the nine items and three separators fit between the
            // header and the footer (559 of 563 px), so the rail never
            // scrolls, nor moves from one tab to the next.
            v->setHeight(13);
            v->setMarginLeft(14);
            v->setMarginRight(14);
            continue;
        }
        item->setHeight(52);
        item->setCornerRadius(14);
        item->setPaddingLeft(RAIL_ITEM_PAD);
        item->setPaddingRight(RAIL_ITEM_PAD);
        item->setMarginBottom(4);
        item->setAlignItems(brls::AlignItems::CENTER);
        if (brls::View* accent = item->getView("brls/sidebar/item_accent")) accent->setVisibility(brls::Visibility::GONE);
        if (auto* label = dynamic_cast<brls::Label*>(item->getView("brls/sidebar/item_label"))) {
            chrome(label, 17);
            label->setMarginTop(0);
            label->setMarginBottom(0);
            label->setMarginRight(0);
            label->setMarginLeft(10);
            label->setSingleLine(true);
        }
        item->addView(icon(name, RAIL_ICON, theme["cadran/text2"]), 0);
        items.push_back(item);
    }
    for (brls::SidebarItem* item : items) {
        item->getActiveEvent()->subscribe([items, item](brls::View*) {
            // Next frame: the library colours every item of the group first.
            brls::sync([items, item]() { tint_rail(items, item); });
        });
    }
    if (!items.empty()) tint_rail(items, items[0]);   // the first tab is already active
}

void cadran::style_frame(brls::AppletFrame* frame, brls::View* tab_frame)
{
    if (frame) frame->addView(make_glow(300.0f, -80.0f, 640.0f, "cadran/glow"), 0);
    style_header(frame, "check", "Contrôle parental actif", true, true);
    style_rail(tab_frame);
}
