// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_tab.hpp"

#include <cctype>
#include <cstdlib>
#include <fmt/format.h>
#include <vector>

#include "action/clock_flow.hpp"
#include "action/pt_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

void cadran::install_theme()
{
    const NVGcolor navy  = nvgRGB(0x0B, 0x14, 0x24);
    const NVGcolor text  = nvgRGB(0xEE, 0xF2, 0xF8);
    const NVGcolor teal  = nvgRGB(0x2E, 0xC4, 0xA6);
    const NVGcolor surf  = nvgRGB(0x12, 0x1E, 0x33);
    const NVGcolor surf2 = nvgRGB(0x1A, 0x2A, 0x45);
    for (brls::Theme* t : { &brls::Theme::getDarkTheme(), &brls::Theme::getLightTheme() }) {
        t->addColor("brls/background", navy);
        t->addColor("brls/clear", navy);
        t->addColor("brls/text", text);
        t->addColor("brls/sidebar/background", navy);
        t->addColor("brls/sidebar/separator", nvgRGBA(255, 255, 255, 20));
        t->addColor("brls/sidebar/active_item", teal);
        t->addColor("brls/applet_frame/separator", nvgRGBA(255, 255, 255, 20));
        t->addColor("brls/header/border", nvgRGBA(255, 255, 255, 20));
        t->addColor("brls/highlight/background", surf);
        t->addColor("brls/highlight/color1", teal);
        t->addColor("brls/highlight/color2", nvgRGB(0x9A, 0xF0, 0xDC));
        t->addColor("brls/click_pulse", nvgRGBA(0x2E, 0xC4, 0xA6, 38));
        t->addColor("brls/button/primary_enabled_background", teal);
        t->addColor("brls/button/primary_enabled_text", navy);
        t->addColor("brls/button/default_enabled_background", surf2);
        t->addColor("brls/button/default_enabled_text", text);
        t->addColor("brls/button/enabled_border_color", nvgRGBA(255, 255, 255, 60));
        t->addColor("brls/button/highlight_enabled_text", teal);
        t->addColor("cadran/navy", navy);
        t->addColor("cadran/text", text);
        t->addColor("cadran/teal", teal);
        t->addColor("cadran/amber", nvgRGB(0xFF, 0xB5, 0x47));
        t->addColor("cadran/coral", nvgRGB(0xFF, 0x7A, 0x7A));
        t->addColor("cadran/coral_line", nvgRGBA(0xFF, 0x7A, 0x7A, 140));
        t->addColor("cadran/track", nvgRGBA(255, 255, 255, 23));
        t->addColor("cadran/surf", surf);
        t->addColor("cadran/surf2", surf2);
        t->addColor("cadran/line", nvgRGBA(255, 255, 255, 20));
        t->addColor("cadran/text2", nvgRGB(0xA9, 0xB5, 0xC9));
        t->addColor("cadran/text3", nvgRGB(0x7B, 0x8A, 0xA3));
        t->addColor("cadran/glow", nvgRGBA(0x2E, 0xC4, 0xA6, 31));
        t->addColor("cadran/amber_surf", nvgRGBA(0xFF, 0xB5, 0x47, 26));
        t->addColor("cadran/amber_line", nvgRGBA(0xFF, 0xB5, 0x47, 90));
        t->addColor("cadran/pill_ok", nvgRGBA(0x2E, 0xC4, 0xA6, 41));
        t->addColor("cadran/rail_active", nvgRGBA(0x2E, 0xC4, 0xA6, 36));
        t->addColor("cadran/key_bg", text);
        t->addColor("cadran/none", nvgRGBA(0, 0, 0, 0));
    }
    brls::Style style = brls::getStyle();
    style.addMetric("brls/applet_frame/header_height", 84.0f);
    // 220 px, not the boards' 196: the console's font is wider than the
    // boards' (Nunito Sans), and "Préférences" must stay on one line.
    style.addMetric("brls/tab_frame/sidebar_width", 220.0f);
    style.addMetric("brls/sidebar/padding_top", 6.0f);
    style.addMetric("brls/sidebar/padding_right", 8.0f);
    style.addMetric("brls/sidebar/padding_bottom", 10.0f);
    style.addMetric("brls/sidebar/padding_left", 16.0f);
    style.addMetric("brls/sidebar/item_height", 52.0f);
    // The boards' display face for the figures (Sora Bold, OFL).
    if (!brls::Application::loadFontFromFile("cadran/digits", BRLS_ASSET("font/sora_bold_latin.ttf")))
        brls::Logger::warning("cadran: display font not loaded, the default one is used");
}

namespace
{
// "jeudi 8 octobre", from the console's clock.
std::string date_text()
{
    static const char* months[] = { "janvier", "février", "mars", "avril", "mai", "juin", "juillet",
                                    "août", "septembre", "octobre", "novembre", "décembre" };
    const std::string date = ui::today_date();   // "2026-10-08"
    int month = 0, day = 0;
    if (date.size() >= 10) {
        month = std::atoi(date.substr(5, 2).c_str());
        day = std::atoi(date.substr(8, 2).c_str());
    }
    if (month < 1 || month > 12 || day < 1) return date;
    return fmt::format("{} {} {}", ui::day_name_in_text(ui::today_weekday()), day, months[month - 1]);
}

// The soft glow of the boards, behind the whole screen (header and rail
// included): one quad with a radial gradient, the only extra fill of the
// design. PLAYGUARD_SIM_CADRAN_NOGLOW=1 leaves it out, to measure its cost.
class Glow : public brls::View
{
  public:
    Glow()
        : glow(brls::Application::getTheme()["cadran/glow"]), clear(nvgRGBA(0x2E, 0xC4, 0xA6, 0))
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
        const NVGpaint paint = nvgRadialGradient(vg, x + 300.0f, y - 80.0f, 0.0f, 640.0f, this->glow, this->clear);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, height);
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }

  private:
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

void cadran::style_frame(brls::AppletFrame* frame, brls::View* tab_frame)
{
    brls::Theme theme = brls::Application::getTheme();

    // The glow first, behind everything.
    if (frame) frame->addView(new Glow(), 0);

    // Header: the mark for the app icon, the title at 24, a status pill and
    // the date at the right.
    if (frame) {
        brls::View* app_icon = frame->getView("brls/applet_frame/title_icon");
        brls::Box* left = app_icon ? app_icon->getParent() : nullptr;
        if (app_icon) app_icon->setVisibility(brls::Visibility::GONE);
        if (left) {
            left->setAlignItems(brls::AlignItems::CENTER);
            left->setGrow(1.0f);
            CadranIcon* mark = icon("mark", 38, theme["cadran/text"]);
            mark->setMarginRight(14);
            left->addView(mark, 0);
        }
        if (auto* title = dynamic_cast<brls::Label*>(frame->getView("brls/applet_frame/title_label"))) {
            title->setFontSize(24);
            title->setMarginTop(0);
        }
        auto* right = new brls::Box(brls::Axis::ROW);
        right->setAlignItems(brls::AlignItems::CENTER);
        auto* pill = new brls::Box(brls::Axis::ROW);
        pill->setAlignItems(brls::AlignItems::CENTER);
        pill->setHeight(32);
        pill->setPaddingLeft(14);
        pill->setPaddingRight(14);
        pill->setCornerRadius(16);
        pill->setBackgroundColor(theme["cadran/pill_ok"]);
        CadranIcon* check = icon("check", 14, theme["cadran/teal"]);
        check->setMarginRight(8);
        pill->addView(check);
        auto* pill_text = new brls::Label();
        pill_text->setText("Contrôle parental actif");
        pill_text->setFontSize(15);
        pill_text->setTextColor(theme["cadran/teal"]);
        pill->addView(pill_text);
        right->addView(pill);
        auto* date = new brls::Label();
        date->setText(date_text());
        date->setFontSize(16);
        date->setTextColor(theme["cadran/text3"]);
        date->setMarginLeft(10);
        right->addView(date);
        if (brls::Box* header = frame->getHeader()) header->addView(right, 1);
    }

    // Rail: an icon before each label, 52 px pills, the accent bar gone.
    auto* sidebar = tab_frame ? dynamic_cast<brls::Sidebar*>(tab_frame->getView("brls/tab_frame/sidebar")) : nullptr;
    brls::SidebarItem* first = sidebar ? sidebar->getItem(0) : nullptr;
    brls::Box* list = first ? first->getParent() : nullptr;
    if (!list) return;
    list->setPadding(6, 8, 10, 16);
    // main_cadran.xml's order: Overview | timer, activity, restrictions, clock | security | preferences, tools, about.
    static const char* icons[] = { "today", "", "timer", "activity", "shield", "globe", "", "lock", "", "settings", "tools", "info" };
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

CadranTab::CadranTab()
    : TabBase("xml/tab/cadran.xml")
{
    brls::Theme theme = brls::Application::getTheme();
    this->teal       = theme["cadran/teal"];
    this->amber      = theme["cadran/amber"];
    this->text       = theme["cadran/text"];
    this->text2      = theme["cadran/text2"];
    this->text3      = theme["cadran/text3"];
    this->surf       = theme["cadran/surf"];
    this->amber_surf = theme["cadran/amber_surf"];
    this->line       = theme["cadran/line"];
    this->amber_line = theme["cadran/amber_line"];

    // The buttons' icons, before their label.
    CadranIcon* plus = icon("plus", 20, theme["cadran/navy"]);
    plus->setMarginRight(10);
    extra->addView(plus, 0);
    extra->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* square = icon("stop", 18, theme["cadran/coral"]);
    square->setMarginRight(10);
    stop->addView(square, 0);
    stop->setAlignItems(brls::AlignItems::CENTER);
    CadranIcon* eye = icon("eye", 18, this->text2);
    eye->setMarginRight(10);
    child->addView(eye, 0);
    child->setAlignItems(brls::AlignItems::CENTER);

    const auto extra_time = [this](brls::View*) {
        pt_flow::add_extra_time(this->pt, [this]() { this->refresh(); });
        return true;
    };
    extra->registerClickAction(extra_time);
    chip15->registerClickAction(extra_time);
    chip30->registerClickAction(extra_time);
    chip60->registerClickAction(extra_time);
    stop->registerClickAction([this](brls::View*) {
        pt_flow::stop_today(this->pt, [this]() { this->refresh(); });
        return true;
    });
    clock_card->registerClickAction([this](brls::View*) {
        if (this->clock_inaccurate && !app::read_only()) clock_flow::guided([this]() { this->refresh(); });
        return true;
    });
    this->enable_auto_refresh(5000);
}

void CadranTab::refresh()
{
    const std::string na = "playguard/common/unavailable"_i18n;

    PctlStatus s;
    pctl_overview_fetch(&s, &this->pt);
    const PtState& pt = this->pt;
    bool accurate = false;
    const Result accuracy_rc = time_network_accuracy(&accurate);

    const bool unlocked = s.temp_unlocked_ok && s.temp_unlocked;
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    ui::note_unlocked(s.temp_unlocked_ok, s.temp_unlocked);

    // The ring: today's limit, what is left of it.
    const int today = ui::today_weekday();
    std::string day = ui::day_name_in_text(today);
    for (char& c : day) c = (char)std::toupper((unsigned char)c);   // ASCII: the French day names have no accent
    kicker->setText("AUJOURD'HUI · LIMITE DU " + day);
    float frac = -1.0f;
    std::string left = "—", caption = "restantes", sentence;
    if (!pt.fw_supported || !pt.valid || !pt.enabled_valid) {
        sentence = "Temps de jeu indisponible";
    } else {
        const uint16_t limit = pt.day_min[today];
        if (limit == PT_DAY_NOLIMIT || !pt.enabled) {
            left = "libre";
            caption = "pas de limite";
            sentence = "Pas de limite de temps de jeu aujourd'hui";
        } else if (unlocked) {
            left = ui::fmt_minutes(limit);
            caption = "déverrouillé";
            sentence = "Peut-être non décompté pendant le déverrouillage";
        } else if (pt.restricted_valid && pt.restricted) {
            frac = 1.0f;
            left = "0 min";
            caption = "temps écoulé";
            sentence = "La limite du jour est atteinte";
        } else if (pt.remaining_valid && pt.remaining_ns > 0 && limit > 0) {
            const uint64_t left_min = pt.remaining_ns / 60000000000ULL;
            const uint16_t used = left_min >= limit ? 0 : (uint16_t)(limit - left_min);
            frac = (float)used / (float)limit;
            left = ui::fmt_duration_ns(pt.remaining_ns);
            sentence = ui::fmt_played(used) + " jouées sur " + ui::fmt_minutes(limit);
        } else {
            left = ui::fmt_minutes(limit);
            caption = "aujourd'hui";
            sentence = "Le temps restant s'affiche pendant qu'un jeu tourne.";
        }
    }
    ring->setFraction(frac);
    ring->setValue(left);
    ring->setCaption(caption);
    played->setText(sentence);

    // Bedtime card.
    if (pt.bedtime_valid) {
        bed_value->setText(fmt::format("{:02d}:{:02d}", pt.bedtime_hour, pt.bedtime_minute));
        bed_state->setText(pt.bedtime_enabled ? "activée" : "désactivée");
        bed_state->setTextColor(pt.bedtime_enabled ? this->teal : this->text2);
        bed_sub->setText(pt.bedtime_reset_valid
                             ? fmt::format("jeu de nouveau permis à {:02d}:{:02d}", pt.bedtime_reset_hour, pt.bedtime_reset_minute)
                             : "");
    } else {
        bed_value->setText("—");
        bed_state->setText("");
        bed_sub->setText(pt.fw_supported ? na : "");
    }

    // Parental-control card: the level, the PIN, the overall state.
    pc_value->setText(s.safety_level_ok ? ui::level_name(s.safety_level) : na);
    if (unlocked) {
        pc_state->setText("déverrouillé");
        pc_state->setTextColor(this->amber);
    } else {
        pc_state->setText(has_pin ? "PIN défini" : "pas de PIN");
        pc_state->setTextColor(has_pin ? this->teal : this->amber);
    }
    pc_sub->setText(!s.restriction_enabled_ok ? na
                    : s.restriction_enabled   ? "contrôle parental actif"
                    : has_pin                 ? "code PIN défini, sans restriction"
                                              : "non configuré");

    // Network clock card: amber, and a call to action, only when it needs one.
    this->clock_inaccurate = R_SUCCEEDED(accuracy_rc) && !accurate;
    const bool bad = this->clock_inaccurate;
    clock_value->setText(R_FAILED(accuracy_rc) ? na : accurate ? "Précise" : "Imprécise");
    clock_value->setTextColor(bad ? this->amber : this->text);
    clock_title->setTextColor(bad ? this->amber : this->text3);
    clock_state->setText(bad ? "à régler" : "ok");
    clock_state->setTextColor(bad ? this->amber : this->teal);
    clock_sub->setText(bad ? "Mesurer et régler maintenant" : "le minuteur compte juste");
    clock_sub->setTextColor(bad ? this->text : this->text2);
    ui::set_visible(clock_key.getView(), bad);
    clock_card->setBackgroundColor(bad ? this->amber_surf : this->surf);
    clock_card->setBorderColor(bad ? this->amber_line : this->line);

    updated->setText(brls::getStr("playguard/dashboard/updated", ui::now_hms()));
}

brls::View* CadranTab::create() { return new CadranTab(); }
