// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_tab.hpp"

#include <cstdlib>
#include <fmt/format.h>

#include "action/clock_flow.hpp"
#include "action/pt_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"

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
        t->addColor("brls/sidebar/background", nvgRGB(0x0E, 0x1A, 0x2E));
        t->addColor("brls/sidebar/separator", nvgRGBA(255, 255, 255, 26));
        t->addColor("brls/sidebar/active_item", teal);
        t->addColor("brls/applet_frame/separator", nvgRGBA(255, 255, 255, 30));
        t->addColor("brls/header/border", nvgRGBA(255, 255, 255, 30));
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
    }
    brls::Style style = brls::getStyle();
    style.addMetric("brls/tab_frame/sidebar_width", 320.0f);
    style.addMetric("brls/sidebar/padding_left", 44.0f);
    style.addMetric("brls/sidebar/padding_right", 24.0f);
}

CadranTab::CadranTab()
    : TabBase("xml/tab/cadran.xml")
{
    brls::Theme theme = brls::Application::getTheme();
    this->glow       = theme["cadran/glow"];
    this->clear      = nvgRGBA(0x2E, 0xC4, 0xA6, 0);
    this->teal       = theme["cadran/teal"];
    this->amber      = theme["cadran/amber"];
    this->text2      = theme["cadran/text2"];
    this->surf       = theme["cadran/surf"];
    this->amber_surf = theme["cadran/amber_surf"];
    this->line       = theme["cadran/line"];
    this->amber_line = theme["cadran/amber_line"];

    extra->registerClickAction([this](brls::View*) {
        pt_flow::add_extra_time(this->pt, [this]() { this->refresh(); });
        return true;
    });
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

void CadranTab::draw(NVGcontext* vg, float x, float y, float width, float height,
                     brls::Style style, brls::FrameContext* ctx)
{
    // One quad with a radial gradient: the only extra fill of the design.
    // PLAYGUARD_SIM_CADRAN_NOGLOW=1 leaves it out, to measure its cost alone.
    static const bool no_glow = std::getenv("PLAYGUARD_SIM_CADRAN_NOGLOW") != nullptr;
    if (no_glow) { brls::Box::draw(vg, x, y, width, height, style, ctx); return; }
    const NVGpaint paint = nvgRadialGradient(vg, x + 140.0f, y - 180.0f, 0.0f, 620.0f, this->glow, this->clear);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    brls::Box::draw(vg, x, y, width, height, style, ctx);
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
    kicker->setText("AUJOURD'HUI · LIMITE DU " + ui::day_name_in_text(today));
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
    clock_value->setTextColor(bad ? this->amber : brls::Application::getTheme()["brls/text"]);
    clock_title->setTextColor(bad ? this->amber : brls::Application::getTheme()["cadran/text3"]);
    clock_state->setText(bad ? "à régler" : "ok");
    clock_state->setTextColor(bad ? this->amber : this->teal);
    clock_sub->setText(bad ? "A · Mesurer et régler maintenant" : "le minuteur compte juste");
    clock_card->setBackgroundColor(bad ? this->amber_surf : this->surf);
    clock_card->setBorderColor(bad ? this->amber_line : this->line);

    updated->setText(brls::getStr("playguard/dashboard/updated", ui::now_hms()));
}

brls::View* CadranTab::create() { return new CadranTab(); }
