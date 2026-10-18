// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_tab.hpp"

#include <cctype>
#include <algorithm>
#include <yoga/Yoga.h>
#include <cstdlib>
#include <fmt/format.h>
#include <utility>
#include <vector>

#include "action/clock_flow.hpp"
#include "activity/cadran_child_activity.hpp"
#include "action/pt_flow.hpp"
#include "app.hpp"
#include "action/pt_logic.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

bool cadran::enabled()
{
    // Read once: a change in Preferences takes effect at the next launch.
    static const bool on = []() {
#ifndef __SWITCH__
        if (const char* forced = std::getenv("PLAYGUARD_SIM_APPEARANCE")) return std::string(forced) != "classic";
#endif
        return config::get().appearance != "classic";
    }();
    return on;
}

void cadran::install_theme()
{
    const NVGcolor navy = nvgRGB(0x0B, 0x14, 0x24);
    // Dark: the boards' navy. Light: the style board's cool white, the inks
    // darkened to 4.5:1 on every surface they are drawn on, the rail's active
    // pill included (teal #0A705F, amber #8A5200, coral #B71C1C); the quiet
    // text too, on both (text3 4.5:1 on the second surface).
    struct P { NVGcolor bg, text, text2, text3, surf, surf2, line, teal, teal_fill, teal_soft, amber, amber_fill,
               coral, coral_fill, coral_line, track, glow, glow_strong, amber_surf, amber_line, pill_ok, pill_ok_ink,
               rail_active, key_bg, key_fg, hl1, hl2, border; };
    const P dark = { navy, nvgRGB(0xEE, 0xF2, 0xF8), nvgRGB(0xA9, 0xB5, 0xC9), nvgRGB(0x83, 0x92, 0xAA),
                     nvgRGB(0x12, 0x1E, 0x33), nvgRGB(0x1A, 0x2A, 0x45), nvgRGBA(255, 255, 255, 20),
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
                      navy, nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0x14, 0xA3, 0x8A), nvgRGB(0x4A, 0xD6, 0xBA), nvgRGBA(0x0B, 0x14, 0x24, 60) };
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
        // The smoke test's still focus (ui::register_theme_colors): the glow
        // in the stroke's own colour, so a screenshot does not depend on when.
        if (std::getenv("PLAYGUARD_SIM_STILL_FOCUS")) t->addColor("brls/highlight/color2", p->hl1);
#endif
        t->addColor("brls/click_pulse", nvgRGBA(0x2E, 0xC4, 0xA6, 38));
        t->addColor("brls/button/primary_enabled_background", p->teal_fill);
        t->addColor("brls/button/primary_enabled_text", navy);
        t->addColor("brls/button/default_enabled_background", p->surf2);
        t->addColor("brls/button/default_enabled_text", p->text);
        t->addColor("brls/button/enabled_border_color", p->border);
        t->addColor("brls/button/highlight_enabled_text", p->teal);
        // The app's status colours and plain values in the boards' inks, so the
        // list screens (cadran::List) take them as they are.
        t->addColor("brand/ok", p->teal);
        t->addColor("brand/warn", p->amber);
        t->addColor("brand/bad", p->coral);
        t->addColor("brand/note", p->text3);
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
        t->addColor("cadran/bar", p == &dark ? nvgRGB(0x33, 0x48, 0x6B) : nvgRGB(0xB9, 0xC6, 0xDA));
        t->addColor("cadran/bar_today", p == &dark ? nvgRGB(0x5A, 0x71, 0x96) : nvgRGB(0x7E, 0x90, 0xAE));
        // Behind a dialog or a picker: the navy, dimmed. A toast: a card of
        // the second surface on the dark theme, navy on the light one.
        t->addColor("brls/backdrop", p == &dark ? nvgRGBA(3, 8, 18, 190) : nvgRGBA(0x0B, 0x14, 0x24, 115));
        t->addColor("brls/notification/background", p == &dark ? nvgRGBA(0x1A, 0x2A, 0x45, 250) : nvgRGBA(0x0B, 0x14, 0x24, 240));
        t->addColor("brls/notification/text", p == &dark ? p->text : nvgRGB(0xEE, 0xF2, 0xF8));
    }
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
        static const bool off = std::getenv("PLAYGUARD_SIM_CADRAN_NOGLOW") != nullptr;
        if (off) return;
        const NVGpaint paint = nvgRadialGradient(vg, x + this->cx, y + this->cy, 0.0f, this->radius, this->glow, this->clear);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, height);
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
// The main screen's pill (style_frame): its label and icon, and the pill for
// hiding and colouring it.
brls::Box*   s_main_pill = nullptr;
brls::Label* s_main_pill_label = nullptr;
CadranIcon*  s_main_pill_icon = nullptr;
}   // namespace

void cadran::set_pill(const std::string& text, const char* glyph, bool warn)
{
    if (!s_main_pill || !s_main_pill_label || !s_main_pill_icon) return;
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
        CadranIcon* mark = icon("mark", 38, theme["cadran/text"]);
        mark->setId("cadran_mark");
        mark->setMarginRight(14);
        left->addView(mark, 0);
    }
    if (auto* title = dynamic_cast<brls::Label*>(frame->getView("brls/applet_frame/title_label"))) {
        title->setFontSize(24);
        title->setMarginTop(0);
    }
    if (!pill_icon) return;   // a screen of its own: the mark and the title
    auto* right = new brls::Box(brls::Axis::ROW);
    right->setAlignItems(brls::AlignItems::CENTER);
    const NVGcolor ink = filled ? theme["cadran/pill_ok_ink"] : theme["cadran/text2"];
    auto* pill = new brls::Box(brls::Axis::ROW);
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
    pill_label->setFontSize(15);
    pill_label->setTextColor(ink);
    pill->addView(pill_label);
    right->addView(pill);
    if (with_date) {   // the main screen: its pill is the one set_pill() changes
        s_main_pill = pill;
        s_main_pill_label = pill_label;
        s_main_pill_icon = glyph;
        auto* date = new brls::Label();
        date->setText(cadran::kit::long_date());
        date->setFontSize(16);
        date->setTextColor(theme["cadran/text3"]);
        date->setMarginLeft(10);
        right->addView(date);
    }
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
    // main_cadran.xml's order: Overview | timer, activity, restrictions, PIN | console | history, preferences, about.
    static const char* icons[] = { "today", "", "timer", "activity", "shield", "lock", "", "chip", "", "history", "settings", "info" };
    std::vector<brls::SidebarItem*> items;
    int position = 0;
    for (brls::View* v : list->getChildren()) {
        const char* name = position < (int)(sizeof(icons) / sizeof(icons[0])) ? icons[position] : "";
        position++;
        auto* item = dynamic_cast<brls::SidebarItem*>(v);
        if (!item) {   // a separator: its line is drawn at mid-height
            v->setHeight(17);
            v->setMarginLeft(14);
            v->setMarginRight(14);
            continue;
        }
        item->setHeight(52);
        item->setCornerRadius(14);
        item->setPaddingLeft(12);
        item->setPaddingRight(12);
        item->setMarginBottom(4);
        item->setAlignItems(brls::AlignItems::CENTER);
        if (brls::View* accent = item->getView("brls/sidebar/item_accent")) accent->setVisibility(brls::Visibility::GONE);
        if (auto* label = dynamic_cast<brls::Label*>(item->getView("brls/sidebar/item_label"))) {
            label->setFontSize(17);
            label->setMarginTop(0);
            label->setMarginBottom(0);
            label->setMarginRight(0);
            label->setMarginLeft(10);
            label->setSingleLine(true);
        }
        item->addView(icon(name, 22, theme["cadran/text2"]), 0);
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

CadranTab::CadranTab()
    : list(this)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    brls::Box* column = this->list.content();
    brls::View* hero = brls::View::createFromXMLResource("view/cadran_today.xml");
    // Above the hero: what needs the parent first (the notices and banners
    // of the Overview); under it, the lines it has no place for.
    int at = 0;
    for (const char* id : { "dash_applet", "dash_first_steps", "dash_unlocked_banner", "dash_outside_text", "dash_outside" }) {
        const auto& lines = column->getChildren();
        for (size_t i = 0; i < lines.size(); i++)
            if (lines[i] == this->getView(id)) at = std::max(at, (int)i + 1);
    }
    column->addView(hero, at);
    YGNodeStyleSetFlexWrap(pills->getYGNode(), YGWrapWrap);

    // The gauge's own words under the ring's title; the timer's warnings with them.
    auto* gauge_text = dynamic_cast<brls::Label*>(this->getView("dash_gauge_text"));
    brls::Box* gauge_box = gauge_text ? gauge_text->getParent() : nullptr;
    int slot = 1;
    for (const char* id : { "dash_gauge_text", "dash_not_counting", "dash_home_hint" }) {
        auto* text = dynamic_cast<brls::Label*>(this->getView(id));
        if (!text) continue;
        if (text->getParent()) text->getParent()->removeView(text, false);
        today_text->addView(text, slot++);
        text->setMarginTop(id == std::string("dash_gauge_text") ? 0 : 6);
        text->setFontSize(id == std::string("dash_gauge_text") ? 30 : 16.5f);
        if (id == std::string("dash_gauge_text")) text->setTextColor(theme["cadran/text"]);
    }
    // What stops play first, under the time played.
    this->next = new brls::Box(brls::Axis::ROW);
    this->next->setAlignItems(brls::AlignItems::CENTER);
    this->next->setMarginTop(8);
    this->next_icon = kit::icon("timer", 18, "cadran/teal");
    this->next_icon->setMarginRight(8);
    this->next->addView(this->next_icon);
    this->next_text = kit::label("", 16.5f, "cadran/text2");
    this->next_text->setSingleLine(true);
    this->next->addView(this->next_text);
    this->next->setVisibility(brls::Visibility::GONE);
    today_text->addView(this->next, 2);
    this->list.park({ "dash_gauge", "dash_extra", "dash_stop", "dash_remaining", "dash_bedtime", "dash_pc", "dash_pin",
                      "dash_level", "dash_clock", "dash_pairing", "dash_serial", "dash_patches", "dash_fw", "dash_compat" });
    if (gauge_box) gauge_box->setVisibility(brls::Visibility::GONE);   // emptied
    // The Overview's section titles: their lines are up there now.
    for (brls::View* line : column->getChildren())
        if (dynamic_cast<brls::Header*>(line)) line->setVisibility(brls::Visibility::GONE);
    this->list.apply({ { "dash_first_steps", "sparkle" }, { "dash_outside", "alert" }, { "dash_today_limit", "timer" },
                       { "dash_extra_pending", "refresh" }, { "dash_alarm", "bell" }, { "dash_unlocked_banner", "unlock" } });
    this->list.banners({ "dash_first_steps", "dash_unlocked_banner", "dash_outside" });

    // The buttons' icons, before their label.
    CadranIcon* plus = kit::icon("plus", 20, "cadran/navy");
    plus->setMarginRight(10);
    extra->addView(plus, 0);
    extra->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* square = kit::icon("stop", 18, "cadran/coral");
    square->setMarginRight(10);
    stop->addView(square, 0);
    stop->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* eye = kit::icon("eye", 18, "cadran/text2");
    eye->setMarginRight(10);
    child->addView(eye, 0);
    child->setAlignItems(brls::AlignItems::CENTER);

    // The Overview's own actions, from where they are shown now.
    extra->registerClickAction([this](brls::View*) {
        kit::press(this->getView("dash_extra"));
        return true;
    });
    for (int minutes : config::get().extra_amounts) {
        brls::Box* amount = kit::chip("+" + ui::fmt_minutes((uint16_t)minutes));
        amount->setMarginRight(10);
        amount->registerClickAction([this, minutes](brls::View*) {
            if (ui::refuse_read_only()) return true;
            pt_flow::add_extra_minutes(this->pt, (uint16_t)minutes, [this]() { this->refresh(); });
            return true;
        });
        chips->addView(amount);
    }
    stop->registerClickAction([this](brls::View*) {
        kit::press(this->getView("dash_stop"));
        return true;
    });
    child->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new CadranChildActivity());
        return true;
    });
    bed_card->registerClickAction([this](brls::View*) {
        kit::press(this->getView("dash_bedtime"));
        return true;
    });
    pc_card->registerClickAction([this](brls::View*) {
        kit::press(this->getView("dash_pc"));
        return true;
    });
    clock_card->registerClickAction([this](brls::View*) {
        kit::press(this->getView("dash_clock"));
        return true;
    });
}

void CadranTab::show_ring(bool unlocked)
{
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    kicker->setText(cadran::upper(brls::getStr("playguard/cadran/today/kicker", ui::day_name_in_text(today))));
    float frac = -1.0f;
    std::string left = "—", caption;
    if (pt.fw_supported && pt.valid && pt.enabled_valid) {
        const uint16_t limit = pt.day_min[today];
        const int over = pt_logic::over_limit_min(pt, today);
        if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            left = "playguard/cadran/today/free"_i18n;
            caption = "playguard/cadran/today/no_limit"_i18n;
        } else if (unlocked) {
            left = ui::fmt_minutes(limit);
            caption = "playguard/cadran/today/unlocked"_i18n;
        } else if (over >= 0 || (pt.restricted_valid && pt.restricted)) {
            frac = 1.0f;
            left = ui::fmt_played(0);
            caption = "playguard/cadran/today/time_up"_i18n;
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left_min = (uint64_t)pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left_min >= limit ? 0 : (uint16_t)(limit - left_min);
            frac = (float)used / (float)limit;
            left = ui::fmt_duration_ns((uint64_t)pt.remaining_ns);
            caption = "playguard/cadran/today/left"_i18n;
        } else {
            left = ui::fmt_minutes(limit);
            caption = "playguard/cadran/today/today"_i18n;
        }
    }
    ring->setFraction(frac);
    ring->setValue(left);
    ring->setCaption(caption);
}

void CadranTab::show_next(bool unlocked)
{
    const PtState& pt = this->pt;
    const int today = ui::today_weekday();
    pt_logic::NextStop n;
    // Unlocked, or the timer off: nothing stops play.
    if (!unlocked && pt.fw_supported && pt.valid && pt.enabled_valid && pt.enabled) {
        const uint16_t limit = pt.day_min[today];
        int left = -1;
        if (limit != PT_DAY_NOLIMIT && pt.remaining_valid && pt.remaining_ns > 0 && pt_logic::over_limit_min(pt, today) < 0)
            left = (int)((uint64_t)pt.remaining_ns / 60000000000ULL);
        const LocalTime now = ui::local_now();
        n = pt_logic::next_stop(now.hour * 60 + now.minute, left, pt.bed[today]);
    }
    const auto clock = [](int minutes) { return fmt::format("{:02d}:{:02d}", minutes / 60, minutes % 60); };
    std::string text;
    if (n.kind == pt_logic::NextStop::TIME_UP) text = brls::getStr("playguard/cadran/today/next_time_up", clock(n.at));
    else if (n.kind == pt_logic::NextStop::BEDTIME)
        text = brls::getStr("playguard/cadran/today/next_bedtime", clock(n.at), ui::fmt_minutes((uint16_t)n.in));
    else if (n.kind == pt_logic::NextStop::BEDTIME_NOW) text = brls::getStr("playguard/cadran/today/next_bedtime_now", clock(n.at));
    if (!text.empty()) {
        ui::set_text(this->next_text, text);
        this->next_icon->setIcon(n.kind == pt_logic::NextStop::TIME_UP ? "timer" : "moon");
    }
    ui::set_visible(this->next, !text.empty());
}

void CadranTab::show_cards(bool unlocked, bool has_pin)
{
    brls::Theme theme = brls::Application::getTheme();
    const std::string na = "playguard/common/unavailable"_i18n;
    const PtState& pt = this->pt;
    const PctlStatus& s = this->status;

    // Bedtime.
    if (pt.bedtime_valid) {
        bed_value->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
        bed_state->setText(pt.bedtime_enabled ? "playguard/cadran/today/on"_i18n : "playguard/cadran/today/off"_i18n);
        bed_state->setTextColor(pt.bedtime_enabled ? theme["cadran/teal"] : theme["cadran/text2"]);
        bed_sub->setText(pt.bedtime_reset_valid ? brls::getStr("playguard/cadran/today/bed_sub",
                                                               fmt::format("{:02d}:{:02d}", pt.bedtime_reset_hour, pt.bedtime_reset_minute))
                                                : "");
    } else {
        bed_value->setText("—");
        bed_state->setText("");
        bed_sub->setText(pt.fw_supported ? na : "");
    }

    // Parental controls: the level, the PIN, the overall state.
    pc_value->setText(s.safety_level_ok ? ui::level_name(s.safety_level) : na);
    if (unlocked) {
        pc_state->setText("playguard/cadran/today/pc_unlocked"_i18n);
        pc_state->setTextColor(theme["cadran/amber"]);
    } else {
        pc_state->setText(has_pin ? "playguard/cadran/today/pin_set"_i18n : "playguard/cadran/today/no_pin"_i18n);
        pc_state->setTextColor(has_pin ? theme["cadran/teal"] : theme["cadran/amber"]);
    }
    pc_sub->setText(!s.restriction_enabled_ok ? na
                    : s.restriction_enabled   ? "playguard/dashboard/pc_active"_i18n
                    : has_pin                 ? "playguard/dashboard/pc_pin_only"_i18n
                                              : "playguard/dashboard/pc_off"_i18n);

    // Network clock: amber, and a call to action, only when it needs one.
    const bool bad = this->clock_inaccurate;
    clock_value->setText(R_FAILED(this->accuracy_rc) ? na
                         : this->accurate              ? "playguard/cadran/today/clock_ok"_i18n
                                                       : "playguard/cadran/today/clock_bad"_i18n);
    clock_value->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text"]);
    clock_title->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text3"]);
    clock_state->setText(bad ? "playguard/cadran/today/to_set"_i18n : "playguard/cadran/today/ok"_i18n);
    clock_state->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/teal"]);
    clock_sub->setText(bad ? "playguard/cadran/today/clock_fix"_i18n : "playguard/cadran/today/clock_fine"_i18n);
    clock_sub->setTextColor(bad ? theme["cadran/text"] : theme["cadran/text2"]);
    ui::set_visible(clock_key.getView(), bad && !app::read_only());
    clock_card->setBackgroundColor(bad ? theme["cadran/amber_surf"] : theme["cadran/surf"]);
    clock_card->setBorderColor(bad ? theme["cadran/amber_line"] : theme["cadran/line"]);
}

// The console's state as pills: what the Overview's System lines say, in
// their colours; A opens where each is set, as the line would.
void CadranTab::show_pills()
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    if (this->pill_slots.empty()) {
        for (const char* line : { "dash_compat", "dash_pairing", "dash_serial", "dash_patches" }) {
            Pill p;
            p.line = line;
            p.box = kit::pill("", "cadran/teal_fill");
            p.box->setFocusable(true);
            p.box->setHideHighlightBackground(true);
            p.dot = dynamic_cast<brls::Rectangle*>(p.box->getChildren()[0]);
            p.text = dynamic_cast<brls::Label*>(p.box->getChildren()[1]);
            brls::View* cell = this->getView(line);
            p.box->registerClickAction([cell](brls::View*) {
                kit::press(cell);
                return true;
            });
            pills->addView(p.box);
            this->pill_slots.push_back(p);
        }
    }
    const SysInfo& si = this->sys;
    NVGcolor c = ui::color_neutral();
    const std::string compat = ui::compat_text(si, &c);
    for (Pill& p : this->pill_slots) {
        auto* cell = dynamic_cast<brls::DetailCell*>(this->getView(p.line));
        if (!cell) continue;
        // A line shown says there is something to look at; else the all-fine words.
        const bool shown = cell->getVisibility() == brls::Visibility::VISIBLE;
        std::string ok;
        if (p.line == std::string("dash_compat")) ok = ui::fw_text(si) + " · " + compat;
        else if (p.line == std::string("dash_serial")) ok = "playguard/cadran/today/serial_hidden"_i18n;
        const std::string text = shown ? cell->title->getFullText() + " · " + kit::without_chevron(cell->detail->getFullText()) : ok;
        ui::set_text(p.text, text);
        p.dot->setColor(theme[shown ? kit::dot_of(cell->detail->getTextColor()) : "cadran/teal_fill"]);
        ui::set_visible(p.box, !text.empty());
    }
}

void CadranTab::refresh()
{
    DashboardTab::refresh();
    this->list.sync();
    const PctlStatus& s = this->status;
    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    if (!s.restriction_enabled_ok) cadran::set_pill("");
    else if (unlocked) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", true);
    else if (s.restriction_enabled) cadran::set_pill("playguard/cadran/pill/pc_on"_i18n);
    else cadran::set_pill("playguard/cadran/pill/pc_off"_i18n, "alert", true);

    this->show_ring(unlocked);
    this->show_next(unlocked);
    this->show_cards(unlocked, has_pin);
    this->show_pills();
    // The actions while their lines would be shown.
    cadran::kit::follow(extra.getView(), this->getView("dash_extra"));
    cadran::kit::follow(chips.getView(), this->getView("dash_extra"));
    cadran::kit::follow(stop.getView(), this->getView("dash_stop"));
}

brls::View* CadranTab::create() { return new CadranTab(); }
