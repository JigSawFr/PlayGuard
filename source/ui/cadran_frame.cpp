// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_frame.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "app.hpp"
#include "core/sim_env.h"
#include "ui/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/hooks.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/cadran_palette.hpp"
#include "ui/effects.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "view/cadran_icon.hpp"
#include "view/title_state.hpp"

using namespace brls::literals;
namespace kit = cadran::kit;

namespace
{
// How much the frame's texts grow with the larger text (the content's:
// text_size::LARGE): the rail's labels as much, as their room allows (the
// rail keeps its width, the screens theirs), the header's and the footer's
// a little less, their rows being full already.
constexpr float RAIL_SCALE   = 1.25f;
constexpr float TITLE_SCALE  = 28.0f / 24.0f;
constexpr float PILL_SCALE   = 1.2f;
constexpr float DATE_SCALE   = 1.15f;
constexpr float HINTS_SCALE  = 1.15f;
}   // namespace

void cadran::install_theme()
{
    namespace pal = cadran::palette;
    const NVGcolor navy = nvgRGB(pal::NAVY.r, pal::NAVY.g, pal::NAVY.b);
    // The palettes and why they are what they are: ui/cadran_palette.hpp.
    const pal::Palette* dark = &pal::DARK;
    const pal::Palette* light = &pal::LIGHT;
    const std::pair<brls::Theme*, const pal::Palette*> variants[] = { { &brls::Theme::getDarkTheme(), dark }, { &brls::Theme::getLightTheme(), light } };
    const auto c = [](pal::Rgba x) { return nvgRGBA(x.r, x.g, x.b, x.a); };
    for (const auto& [t, p] : variants) {
        t->addColor("brls/background", c(p->bg));
        t->addColor("brls/clear", c(p->bg));
        t->addColor("brls/text", c(p->text));
        t->addColor("brls/sidebar/background", c(p->bg));
        t->addColor("brls/sidebar/separator", c(p->line));
        t->addColor("brls/sidebar/active_item", c(p->teal));
        t->addColor("brls/applet_frame/separator", c(p->line));
        t->addColor("brls/header/border", c(p->line));
        t->addColor("brls/highlight/background", c(p->surf));
        t->addColor("brls/highlight/color1", c(p->hl1));
        t->addColor("brls/highlight/color2", c(p->hl2));
#ifndef __SWITCH__
        // The smoke test's still focus (PLAYGUARD_SIM_STILL_FOCUS): the glow in
        // the stroke's own colour, so a screenshot does not depend on when.
        if (sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS")) t->addColor("brls/highlight/color2", c(p->hl1));
#endif
        NVGcolor pulse = c(p->teal_fill);
        pulse.a = 38 / 255.0f;
        t->addColor("brls/click_pulse", pulse);
        t->addColor("brls/button/primary_enabled_background", c(p->teal_fill));
        t->addColor("brls/button/primary_enabled_text", navy);
        t->addColor("brls/button/default_enabled_background", c(p->surf2));
        t->addColor("brls/button/default_enabled_text", c(p->text));
        t->addColor("brls/button/enabled_border_color", c(p->border));
        t->addColor("brls/button/highlight_enabled_text", c(p->teal));
        // A list line's plain value in the second ink, so the list screens
        // (cadran::List) take it as it is.
        t->addColor("brls/list/listItem_value_color", c(p->text2));
        t->addColor("cadran/navy", navy);
        t->addColor("cadran/text", c(p->text));
        t->addColor("cadran/text2", c(p->text2));
        t->addColor("cadran/text3", c(p->text3));
        t->addColor("cadran/teal", c(p->teal));
        t->addColor("cadran/teal_fill", c(p->teal_fill));
        t->addColor("cadran/teal_soft", c(p->teal_soft));
        t->addColor("cadran/amber", c(p->amber));
        t->addColor("cadran/amber_fill", c(p->amber_fill));
        t->addColor("cadran/coral", c(p->coral));
        t->addColor("cadran/coral_fill", c(p->coral_fill));
        t->addColor("cadran/coral_line", c(p->coral_line));
        t->addColor("cadran/coral_soft", p == dark ? nvgRGBA(0xFF, 0x7A, 0x7A, 36) : nvgRGBA(0xE5, 0x48, 0x4D, 28));
        t->addColor("cadran/track", c(p->track));
        t->addColor("cadran/surf", c(p->surf));
        t->addColor("cadran/surf2", c(p->surf2));
        t->addColor("cadran/line", c(p->line));
        t->addColor("cadran/glow", c(p->glow));
        t->addColor("cadran/glow_strong", c(p->glow_strong));
        t->addColor("cadran/amber_surf", c(p->amber_surf));
        t->addColor("cadran/amber_line", c(p->amber_line));
        t->addColor("cadran/pill_ok", c(p->pill_ok));
        t->addColor("cadran/pill_ok_ink", c(p->pill_ok_ink));
        t->addColor("cadran/rail_active", c(p->rail_active));
        t->addColor("cadran/key_bg", c(p->key_bg));
        t->addColor("cadran/key_fg", c(p->key_fg));
        t->addColor("cadran/none", nvgRGBA(0, 0, 0, 0));
        // The empty part of a progress bar, behind a game's missing cover.
        t->addColor("cadran/well", p == dark ? nvgRGBA(255, 255, 255, 46) : nvgRGBA(0, 0, 0, 34));
        // The inks the palettes have no use for elsewhere: a release note's
        // alerts and a build's tags (blue, violet), each as dark on the
        // light theme as the other inks (4.5:1 on white).
        t->addColor("cadran/blue", c(p->blue));
        t->addColor("cadran/violet", c(p->violet));
        // The action tiles' fourth ink (cadran::List::tiles()), after teal,
        // blue and violet: the third avatar's pink, darker on white.
        t->addColor("cadran/pink", p == dark ? nvgRGB(0xE9, 0x6A, 0xA8) : nvgRGB(0xC2, 0x45, 0x7F));
        // The accounts' avatars, in turn: fills under a navy initial, alike
        // on both themes.
        t->addColor("cadran/avatar0", nvgRGB(0x7C, 0x6C, 0xF2));
        t->addColor("cadran/avatar1", nvgRGB(0xFF, 0xB5, 0x47));
        t->addColor("cadran/avatar2", nvgRGB(0x4F, 0xA3, 0xFF));
        t->addColor("cadran/avatar3", nvgRGB(0xE9, 0x6A, 0xA8));
        // A switch's knob, white on either track, and its edge while off.
        t->addColor("cadran/knob", nvgRGB(0xFF, 0xFF, 0xFF));
        t->addColor("cadran/knob_line", nvgRGBA(0x0B, 0x14, 0x24, 40));
        t->addColor("cadran/bar", p == dark ? nvgRGB(0x33, 0x48, 0x6B) : nvgRGB(0xB9, 0xC6, 0xDA));
        t->addColor("cadran/bar_today", p == dark ? nvgRGB(0x5A, 0x71, 0x96) : nvgRGB(0x7E, 0x90, 0xAE));
        // Behind a dialog or a picker: the navy, dimmed enough that the screen
        // under it does not read through around a dialog's text and QR codes.
        // A toast: a card of the second surface on the dark theme, navy on
        // the light one.
        t->addColor("brls/backdrop", p == dark ? nvgRGBA(3, 8, 18, 225) : nvgRGBA(0x0B, 0x14, 0x24, 160));
        t->addColor("brls/notification/background", p == dark ? nvgRGBA(0x1A, 0x2A, 0x45, 250) : nvgRGBA(0x0B, 0x14, 0x24, 240));
        t->addColor("brls/notification/text", p == dark ? c(p->text) : nvgRGB(0xEE, 0xF2, 0xF8));
    }
#ifndef __SWITCH__
    // And held where it is (patches/borealis/0009). The footer's hints too:
    // drawn as once a screen or a dialog has opened, not greyed while its
    // fade blocks the input (0014). A slow frame (the web simulator's
    // software WebGL) could otherwise catch them greyed.
    if (sim_env_flag("PLAYGUARD_SIM_STILL_FOCUS")) {
        brls::setHighlightAnimationStill(true);
        brls::setHintsSteady(true);
    }
#endif
    // The footer's hints and clock grow less than the rest with the larger
    // text: a footer full of hints still has room for them (patches/borealis/0007).
    brls::setHintsFontScaleMax(HINTS_SCALE);
    brls::Style style = brls::getStyle();
    // The corners of the kit, for the XML (@style/cadran/radius/card…).
    style.addMetric("cadran/radius/row", kit::RADIUS_ROW);
    style.addMetric("cadran/radius/icon", kit::RADIUS_ICON);
    style.addMetric("cadran/radius/card", kit::RADIUS_CARD);
    style.addMetric("cadran/radius/top", kit::RADIUS_TOP);
    style.addMetric("cadran/radius/modal", kit::RADIUS_MODAL);
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
// The frame (header, rail) with Preferences › Text size's larger text
// (ui/text_size): at most `scale` times its size, as the room allows, and
// smaller again, down to its size, where a language's words need it
// (patches/borealis/0007). The normal text size leaves it as it is.
void chrome(brls::Label* label, float size, float scale)
{
    label->setFontSize(size);
    label->setFontScaleMax(scale);
    label->setShrinkToFit(true);
}

class Glow : public brls::View
{
  public:
    Glow(float centre_x, float centre_y, float r, const char* color_key)
        : cx(centre_x), cy(centre_y), radius(r), glow(brls::Application::getTheme()[color_key]), clear(nvgRGBA(0x2E, 0xC4, 0xA6, 0))
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
cadran::Tone s_pill_tone = cadran::Tone::OK;
}   // namespace

void cadran::set_pill(const std::string& text, const char* glyph, Tone tone)
{
    s_pill_text  = text;
    s_pill_glyph = glyph ? glyph : "check";
    s_pill_tone  = tone;
    refresh_pill();
}

void cadran::refresh_pill()
{
    if (!s_main_pill || !s_main_pill_label || !s_main_pill_icon) return;
    // Unlocked, then read-only: that first, on every screen (what a tab
    // says comes back once locked again, or writable).
    const bool unlocked = ui::known_unlocked();
    const bool read_only = !unlocked && app::read_only();
    const std::string text = unlocked    ? "playguard/cadran/pill/unlocked"_i18n
                             : read_only ? "playguard/cadran/pill/read_only"_i18n
                                         : s_pill_text;
    const char* glyph = unlocked ? "unlock" : read_only ? "pencil-off" : s_pill_glyph.c_str();
    const Tone tone = unlocked || read_only ? Tone::WARN : s_pill_tone;
    brls::Theme theme = brls::Application::getTheme();
    const NVGcolor ink = tone == Tone::DANGER ? theme["cadran/coral"]
                         : tone == Tone::WARN ? theme["cadran/amber"]
                                              : theme["cadran/pill_ok_ink"];
    s_main_pill_label->setText(text);
    s_main_pill_label->setTextColor(ink);
    s_main_pill_icon->setIcon(glyph);
    s_main_pill_icon->setColor(ink);
    s_main_pill->setBackgroundColor(tone == Tone::DANGER ? theme["cadran/coral_soft"]
                                    : tone == Tone::WARN ? theme["cadran/amber_surf"]
                                                         : theme["cadran/pill_ok"]);
    s_main_pill->setBorderColor(tone == Tone::DANGER ? theme["cadran/coral_line"] : theme["cadran/amber_line"]);
    s_main_pill->setBorderThickness(tone == Tone::OK ? 0 : 1);
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
        chrome(title, 24, TITLE_SCALE);
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
    chrome(pill_label, 15, PILL_SCALE);
    pill_label->setTextColor(ink);
    pill->addView(pill_label);
    right->addView(pill);
    if (with_date) {   // the main screen: its pill is the one set_pill() changes
        // Touched while unlocked or read-only, it goes where that is undone.
        pill->addGestureRecognizer(new brls::TapGestureRecognizer(pill, []() {
            if (ui::known_unlocked()) TitleState::open(TitleState::Kind::UNLOCKED);
            else if (app::read_only()) TitleState::open(TitleState::Kind::READ_ONLY);
        }));
        s_main_pill = pill;
        s_main_pill_label = pill_label;
        s_main_pill_icon = glyph;
        auto* date = new brls::Label();
        date->setText(cadran::kit::long_date());
        chrome(date, 16, DATE_SCALE);
        date->setTextColor(theme["cadran/text3"]);
        date->setMarginLeft(10);
        right->addView(date);
    }
    right->setMarginRight(16);   // clear of the header's tools (language, theme, bell) after it
    if (brls::Box* header = frame->getHeader()) header->addView(right, 1);
}

namespace
{
// The icons of the lines of the screens dressed here, by id (the screens
// with a list of their own pass theirs to cadran::List).
const std::vector<std::pair<const char*, const char*>> SCREEN_ICONS = {
    // First steps
    { "ob_pin", "key" }, { "ob_limit", "timer" }, { "ob_clock", "globe" }, { "ob_unlink", "phone" },
    { "ob_alarm", "bell" }, { "ob_protect", "lock" }, { "ob_at_start", "sparkle" }, { "ob_close", "check" },
    { "ob_support", "heart" },
    // An update
    { "up_update", "download" }, { "up_method", "settings" }, { "up_later", "calendar" },
    // A firmware not checked yet
    { "fg_update", "download" }, { "fg_read_only", "eye" }, { "fg_probe", "tools" }, { "fg_risk", "alert" },
    { "fg_remember", "save" }, { "fg_quit", "stop" },
    // Recovery
    { "rc_reset_pin", "key" }, { "rc_delete", "trash" }, { "rc_continue", "chevron-right" },
    // The play timer's Advanced
    { "pa_show", "eye" }, { "pa_alarm", "bell" }, { "pa_pause", "pause" }, { "pa_resume", "today" },
    { "pa_diag", "code" },
    // A game
    { "ga_today", "today" }, { "ga_week", "calendar" }, { "ga_total", "history" }, { "ga_launches", "gamepad" },
    { "ga_first", "flag" }, { "ga_last", "timer" }, { "ga_rating", "shield" },
    // What's new
    { "wn_full", "list" }, { "wn_close", "check" },
};

// Where `frame` was opened from: the title of the screen under it, or the
// main screen's tab; "" when it is not known.
std::string crumb_of(brls::AppletFrame* frame)
{
    // A new screen takes the focus before it is on the stack: the one under
    // it is then the top one.
    const std::vector<brls::Activity*> stack = brls::Application::getActivitiesStack();
    brls::View* below = nullptr;
    for (size_t i = 0; i < stack.size(); i++)
        if (stack[i]->getContentView() == frame) below = i > 0 ? stack[i - 1]->getContentView() : nullptr;
    if (!below && !stack.empty() && stack.back()->getContentView() != frame) below = stack.back()->getContentView();
    if (!below) return "";
    if (below->getView("main_tabs")) {
        const auto& main_tab = ui::hooks::get().main_screen;
        const std::string tab = main_tab ? main_tab() : "";   // "tab/console"
        return tab.rfind("tab/", 0) == 0 ? brls::getStr("playguard/tabs/" + tab.substr(4)) : "";
    }
    auto* title = dynamic_cast<brls::Label*>(below->getView("brls/applet_frame/title_label"));
    return title ? title->getFullText() : "";
}

// Before a screen's title, small: where it was opened from ("Console ›").
void add_crumb(brls::AppletFrame* frame)
{
    auto* title = dynamic_cast<brls::Label*>(frame->getView("brls/applet_frame/title_label"));
    auto* row = title ? dynamic_cast<brls::Box*>(title->getParent()) : nullptr;
    // Not on the child screen: it is the child's, not a step of the parent's.
    const std::string crumb = row && !frame->getView("child_root") ? crumb_of(frame) : "";
    // Nothing said twice (a game's screen is titled as its tab: Activity).
    if (crumb.empty() || title->getFullText().empty() || crumb == title->getFullText()) return;
    brls::Label* label = cadran::kit::label(crumb + "  ›", cadran::kit::TEXT_LEAD, "cadran/text3");
    label->setSingleLine(true);
    label->setMarginRight(10);
    const auto& views = row->getChildren();
    row->addView(label, std::find(views.begin(), views.end(), title) - views.begin());
}

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
    add_crumb(frame);
    if (!frame->getView("cadran_list")) cadran::List(frame, true).apply(SCREEN_ICONS);   // a Cadran screen dresses itself
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
    // main.xml's order: Today | timer, activity, restrictions, security, history | console | preferences, about.
    static const char* icons[] = { "today", "", "timer", "activity", "shield", "lock", "history", "", "chip", "", "settings", "info" };
    std::vector<brls::SidebarItem*> items;
    int position = 0;
    for (brls::View* v : list->getChildren()) {
        const char* name = position < (int)(sizeof(icons) / sizeof(icons[0])) ? icons[position] : "";
        position++;
        auto* item = dynamic_cast<brls::SidebarItem*>(v);
        if (!item) {   // a separator: its line is drawn at mid-height
            // 13 px: the items and the three separators fit between the
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
        // The library shows its accent again on each change of tab: without
        // a size or a colour, that moves nothing (the pill says it instead).
        if (auto* accent = dynamic_cast<brls::Rectangle*>(item->getView("brls/sidebar/item_accent"))) {
            accent->setWidth(0);
            accent->setMargins(0, 0, 0, 0);
            accent->setColor(theme["cadran/none"]);
            accent->setVisibility(brls::Visibility::GONE);
        }
        if (auto* label = dynamic_cast<brls::Label*>(item->getView("brls/sidebar/item_label"))) {
            chrome(label, 17, RAIL_SCALE);
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
            // In this frame, not the next (the frame would be drawn in the
            // library's colours first, the text flashing at each move): the
            // tab left is made inactive here, before the library recolours it
            // after this event, which then leaves it as it is.
            for (brls::SidebarItem* it : items)
                if (it != item) it->setActive(false);
            tint_rail(items, item);
        });
    }
    if (!items.empty()) tint_rail(items, items[0]);   // the first tab is already active
}

void cadran::style_frame(brls::AppletFrame* frame, brls::View* tab_frame)
{
    if (frame) frame->addView(make_glow(300.0f, -80.0f, 640.0f, "cadran/glow"), 0);
    style_header(frame, "check", "playguard/cadran/pill/pc_on"_i18n, true, true);
    style_rail(tab_frame);
}
