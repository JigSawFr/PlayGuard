// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_restrictions_tab.hpp"

#include <fmt/format.h>

#include "action/history_flow.hpp"
#include "app.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
brls::Label* label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setText(text);
    l->setFontSize(size);
    l->setTextColor(color);
    return l;
}
}   // namespace

CadranRestrictionsTab::Tile CadranRestrictionsTab::make_tile(uint32_t level, const char* name, bool last)
{
    brls::Theme theme = brls::Application::getTheme();
    Tile t;
    t.level = level;
    t.box = new brls::Box(brls::Axis::COLUMN);
    t.box->setGrow(1.0f);
    t.box->setWidth(0);
    t.box->setMinHeight(236);
    t.box->setCornerRadius(22);
    t.box->setPadding(16, 14, 14, 14);
    t.box->setHideHighlightBackground(true);
    t.box->setBorderThickness(2);
    t.box->setFocusable(true);
    if (!last) t.box->setMarginRight(12);

    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    brls::Label* title = label(name, 21, theme["cadran/text"]);
    title->setGrow(1.0f);
    head->addView(title);
    t.badge = new brls::Box(brls::Axis::ROW);
    t.badge->setWidth(28);
    t.badge->setHeight(28);
    t.badge->setCornerRadius(14);
    t.badge->setBackgroundColor(theme["cadran/teal_fill"]);
    t.badge->setAlignItems(brls::AlignItems::CENTER);
    t.badge->setJustifyContent(brls::JustifyContent::CENTER);
    auto* check = new CadranIcon();
    check->setIcon("check");
    check->setWidth(16);
    check->setHeight(16);
    check->setColor(theme["cadran/navy"]);
    t.badge->addView(check);
    head->addView(t.badge);
    t.box->addView(head);

    t.age = new CadranDigits();
    t.age->setFontSize(30);
    t.age->setMarginTop(10);
    t.box->addView(t.age);
    t.note = label("", 14, theme["cadran/text3"]);
    t.note->setMarginTop(4);
    t.box->addView(t.note);

    auto* spacer = new brls::Box();
    spacer->setGrow(1.0f);
    t.box->addView(spacer);
    auto* rule = new brls::Box();
    rule->setHeight(1);
    rule->setBackgroundColor(theme["cadran/line"]);
    rule->setMarginTop(10);
    rule->setMarginBottom(10);
    t.box->addView(rule);

    const auto row = [&](brls::Rectangle*& dot, brls::Label*& text) {
        auto* r = new brls::Box(brls::Axis::ROW);
        r->setAlignItems(brls::AlignItems::CENTER);
        r->setMarginBottom(7);
        dot = new brls::Rectangle(theme["cadran/text3"]);
        dot->setWidth(8);
        dot->setHeight(8);
        dot->setCornerRadius(4);
        dot->setMarginRight(6);
        r->addView(dot);
        text = label("", 13.5f, theme["cadran/text2"]);
        text->setSingleLine(true);
        r->addView(text);
        t.box->addView(r);
    };
    row(t.sns_dot, t.sns);
    row(t.comm_dot, t.comm);

    t.box->registerAction("Choisir ce niveau", brls::BUTTON_A, [this, level](brls::View*) {
        this->choose_level(level);
        return true;
    });
    return t;
}

CadranRestrictionsTab::CadranRestrictionsTab()
    : TabBase("xml/tab/cadran_restrictions.xml")
{
    const std::pair<uint32_t, const char*> levels[] = {
        { PctlSafetyLevel_YoungChild, "Jeune enfant" }, { PctlSafetyLevel_Child, "Enfant" },
        { PctlSafetyLevel_Teen, "Adolescent" }, { PctlSafetyLevel_Custom, "Personnalisé" }
    };
    for (size_t i = 0; i < 4; i++) {
        this->tiles.push_back(this->make_tile(levels[i].first, levels[i].second, i == 3));
        tiles_row->addView(this->tiles.back().box);
    }
    vr_card->registerClickAction([this](brls::View*) {
        if (!this->st.stereo_vision_ok || ui::refuse_read_only()) return true;
        const bool on = !this->st.stereo_vision_restricted;
        const Result rc = pctl_set_stereo_vision_restricted(on);
        if (R_SUCCEEDED(rc)) history_flow::record_values("vr", { (int)!on }, { (int)on });
        ui::notify_result(rc, on ? "Mode VR restreint." : "Mode VR permis.", "Impossible de modifier le mode VR");
        this->refresh();
        return true;
    });
    org_card->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        std::vector<std::string> names;
        for (uint32_t i = 0; i < 13; i++) names.push_back(pctl_rating_org_name(i));
        ui::pick("playguard/restrictions/org"_i18n, names, this->st.rating_org_ok && this->st.rating_org < 13 ? (int)this->st.rating_org : 0,
                 [this](int index) {
                     if (this->st.rating_org_ok && (int)this->st.rating_org == index) return;
                     const Result rc = pctl_set_rating_org((uint32_t)index);
                     if (R_SUCCEEDED(rc) && this->st.rating_org_ok) history_flow::record_values("org", { (int)this->st.rating_org }, { index });
                     ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
                     this->refresh();
                 });
        return true;
    });
    comm_card->registerClickAction([](brls::View*) {
        ui::info("La liste des logiciels autorisés à communiquer n'est pas encore décodée (feuille de route).");
        return true;
    });
}

void CadranRestrictionsTab::choose_level(uint32_t level)
{
    if (ui::refuse_read_only()) return;
    if (this->st.safety_level_ok && this->st.safety_level == level) return;
    const int before = this->st.safety_level_ok ? (int)this->st.safety_level : -1;
    ui::confirm(brls::getStr("playguard/restrictions/confirm_level", ui::level_name(level)), "playguard/play_timer/confirm_set"_i18n,
                [this, level, before]() {
                    const Result rc = pctl_set_safety_level(level);
                    if (R_SUCCEEDED(rc) && before >= 0) history_flow::record_values("level", { before }, { (int)level });
                    ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
                    this->refresh();
                });
}

void CadranRestrictionsTab::refresh()
{
    brls::Theme theme = brls::Application::getTheme();
    const std::string na = "playguard/common/unavailable"_i18n;
    pctl_status_fetch(&this->st);
    const PctlStatus& st = this->st;
    if (!st.pin_length_ok) cadran::set_pill("");
    else if (st.pin_length) cadran::set_pill("PIN défini", "key");
    else cadran::set_pill("Pas de code PIN", "key", true);
    const std::string org = st.rating_org_ok ? pctl_rating_org_name(st.rating_org) : "";

    level_value->setText(st.safety_level_ok ? ui::level_name(st.safety_level) : na);

    for (Tile& t : this->tiles) {
        const bool selected = st.safety_level_ok && st.safety_level == t.level;
        PctlCustomSettings s = {};
        bool known = false;
        if (t.level == PctlSafetyLevel_Custom) {
            known = st.settings_ok;
            s = st.settings;
        } else {
            known = R_SUCCEEDED(pctl_get_level_settings(t.level, &s));
        }
        t.box->setBackgroundColor(selected ? theme["cadran/teal_soft"] : theme["cadran/surf"]);
        t.box->setBorderColor(selected ? theme["cadran/teal_fill"] : theme["cadran/line"]);
        ui::set_visible(t.badge, selected);
        t.age->setTextColor(selected ? theme["cadran/teal"] : theme["cadran/text"]);
        if (t.level == PctlSafetyLevel_Custom) {
            t.age->setText(known && s.rating_age ? fmt::format("{} ans", s.rating_age) : "au choix");
            t.note->setText("chaque réglage se choisit");
        } else {
            t.age->setText(known ? (s.rating_age ? fmt::format("{} ans", s.rating_age) : "tout âge") : na);
            t.note->setText(known && s.rating_age ? fmt::format("logiciels jusqu'à {} {}", org.empty() ? "l'âge" : org, s.rating_age)
                                                  : "logiciels sans restriction");
        }
        const auto state = [&](bool restricted, const char* yes, const char* no) {
            return !known ? std::string("inconnu") : restricted ? std::string(yes) : std::string(no);
        };
        t.sns->setText("Réseaux sociaux · " + state(s.sns_post_restriction, "bloqués", "autorisés"));
        t.comm->setText("Communication · " + state(s.free_communication_restriction, "bloquée", "autorisée"));
        t.sns_dot->setColor(!known ? theme["cadran/text3"] : s.sns_post_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
        t.comm_dot->setColor(!known ? theme["cadran/text3"] : s.free_communication_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
    }

    // The three cards below.
    const bool vr_on = st.stereo_vision_ok && st.stereo_vision_restricted;
    vr_sub->setText(!st.stereo_vision_ok ? na : vr_on ? "restreint" : "permis");
    vr_toggle->setBackgroundColor(vr_on ? theme["cadran/teal_fill"] : theme["cadran/line"]);
    vr_toggle->setJustifyContent(vr_on ? brls::JustifyContent::FLEX_END : brls::JustifyContent::FLEX_START);
    vr_knob->setBackgroundColor(vr_on ? theme["cadran/navy"] : theme["cadran/text2"]);
    org_sub->setText(st.rating_org_ok ? org + " · classification des logiciels" : na);
    comm_sub->setText(st.free_comm_count_ok ? fmt::format("à communiquer · {} logiciel{}", st.free_comm_count, st.free_comm_count > 1 ? "s" : "") : na);
}

brls::View* CadranRestrictionsTab::create() { return new CadranRestrictionsTab(); }
