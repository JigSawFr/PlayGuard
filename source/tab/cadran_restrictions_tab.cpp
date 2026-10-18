// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_restrictions_tab.hpp"

#include "app.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
// The check on the level in force.
brls::Box* badge()
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setWidth(26);
    box->setHeight(26);
    box->setShrink(0);
    box->setCornerRadius(13);
    box->setBackgroundColor(theme["cadran/teal_fill"]);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setJustifyContent(brls::JustifyContent::CENTER);
    box->setMarginLeft(6);
    box->addView(cadran::kit::icon("check", 15, "cadran/navy"));
    return box;
}
}   // namespace

CadranRestrictionsTab::Tile CadranRestrictionsTab::make_tile(uint32_t level, bool last)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    Tile t;
    t.level = level;
    if (level == PctlSafetyLevel_None) {
        // No restriction: a slim card under the levels, its words, its check.
        t.box = kit::card(true, false);
        t.box->setAxis(brls::Axis::ROW);
        t.box->setAlignItems(brls::AlignItems::CENTER);
        t.box->setCornerRadius(18);
        t.box->setHighlightCornerRadius(18);
        t.box->setPadding(14, 22, 14, 24);
        t.box->setBorderThickness(2);
        t.box->setMarginTop(12);
        brls::Label* title = kit::label(ui::level_name(level), 17);
        title->setSingleLine(true);
        title->setMarginRight(14);
        t.box->addView(title);
        t.note = kit::label("playguard/cadran/restrictions/none"_i18n, 14, "cadran/text3");
        t.note->setGrow(1.0f);
        t.note->setWidth(0);
        t.box->addView(t.note);
        t.badge = badge();
        t.box->addView(t.badge);
    } else {
        t.box = kit::card(true, true);
        t.box->setMinHeight(226);
        t.box->setCornerRadius(22);
        t.box->setHighlightCornerRadius(22);
        t.box->setPadding(18, 18, 16, 18);
        t.box->setBorderThickness(2);
        if (!last) t.box->setMarginRight(12);

        auto* head = new brls::Box(brls::Axis::ROW);
        head->setAlignItems(brls::AlignItems::FLEX_START);
        brls::Label* title = kit::label(ui::level_name(level), 19);
        title->setGrow(1.0f);
        title->setWidth(0);
        head->addView(title);
        t.badge = badge();
        head->addView(t.badge);
        t.box->addView(head);

        t.age = new CadranDigits();
        t.age->setFontSize(26);
        t.age->setMarginTop(10);
        t.age->setFitWidth(200);
        t.box->addView(t.age);
        t.note = kit::label("", 14, "cadran/text3");
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
            r->setMarginBottom(6);
            dot = new brls::Rectangle(theme["cadran/text3"]);
            dot->setWidth(8);
            dot->setHeight(8);
            dot->setCornerRadius(4);
            dot->setMarginRight(6);
            dot->setShrink(0);
            r->addView(dot);
            text = kit::label("", 13.5f, "cadran/text2");
            text->setSingleLine(true);
            r->addView(text);
            t.box->addView(r);
        };
        row(t.sns_dot, t.sns);
        row(t.comm_dot, t.comm);
    }

    t.box->registerAction("playguard/cadran/restrictions/choose"_i18n, brls::BUTTON_A, [this, level](brls::View*) {
        if (ui::refuse_read_only()) return true;
        if (this->st.safety_level_ok && this->st.safety_level == level) return true;
        this->choose_level((int)level);
        return true;
    });
    return t;
}

CadranRestrictionsTab::CadranRestrictionsTab()
    : list(this)
{
    brls::Box* column = this->list.content();
    brls::View* hero = brls::View::createFromXMLResource("view/cadran_restrictions.xml");
    // Under the read-only note.
    const auto& lines = column->getChildren();
    int at = 0;
    for (size_t i = 0; i < lines.size(); i++)
        if (lines[i] == this->getView("tab_ro_note")) at = (int)i + 1;
    column->addView(hero, at);
    auto* row = dynamic_cast<brls::Box*>(hero->getView("crs_tiles"));
    for (uint32_t level : { PctlSafetyLevel_YoungChild, PctlSafetyLevel_Child, PctlSafetyLevel_Teen, PctlSafetyLevel_Custom,
                            PctlSafetyLevel_None }) {
        this->tiles.push_back(this->make_tile(level, level == PctlSafetyLevel_Custom));
        if (level == PctlSafetyLevel_None) static_cast<brls::Box*>(hero)->addView(this->tiles.back().box);
        else row->addView(this->tiles.back().box);
    }
    // The level is the tiles; what a preset imposes is in its tile.
    this->list.hide_header_before("rs_level");
    this->list.park({ "rs_level", "rs_preset_age", "rs_preset_sns", "rs_preset_comm" });
    this->list.apply({ { "rs_age", "calendar" }, { "rs_sns", "user" }, { "rs_comm", "phone" }, { "rs_vr", "eye" },
                       { "rs_org", "flag" }, { "rs_free_comm", "gamepad" } });
    dynamic_cast<brls::Label*>(hero->getView("crs_kicker"))->setText(cadran::upper("playguard/restrictions/section_level"_i18n));
}

void CadranRestrictionsTab::refresh()
{
    RestrictionsTab::refresh();
    this->list.sync();
    brls::Theme theme = brls::Application::getTheme();
    const PctlStatus& st = this->st;
    const std::string org = st.rating_org_ok ? pctl_rating_org_name(st.rating_org) : "";
    // The settings' header: Custom's own lines only (a preset's are in its tile).
    const bool custom = st.safety_level_ok && st.safety_level == PctlSafetyLevel_Custom && st.settings_ok;
    ui::set_visible(this->getView("rs_custom_header"), custom);
    if (st.safety_level_ok) cadran::set_pill(brls::getStr("playguard/cadran/pill/level", ui::level_name(st.safety_level)), "shield");
    else cadran::set_pill("");

    for (Tile& t : this->tiles) {
        const bool selected = st.safety_level_ok && st.safety_level == t.level;
        PctlCustomSettings s = {};
        bool known = true;   // no restriction: nothing restricted
        if (t.level == PctlSafetyLevel_Custom) {
            known = st.settings_ok;
            s = st.settings;
        } else if (t.level != PctlSafetyLevel_None) {
            known = R_SUCCEEDED(pctl_get_level_settings(t.level, &s));
        }
        t.box->setBackgroundColor(selected ? theme["cadran/teal_soft"] : theme["cadran/surf"]);
        t.box->setBorderColor(selected ? theme["cadran/teal_fill"] : theme["cadran/line"]);
        t.badge->setVisibility(selected ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
        // Read-only: the levels stay, dimmed; A says why nothing changes.
        t.box->setAlpha(app::read_only() && !selected ? 0.6f : 1.0f);
        if (!t.age) continue;   // no restriction: its words say it all
        t.age->setTextColor(selected ? theme["cadran/teal"] : theme["cadran/text"]);
        t.age->setText(!known ? "—" : ui::age_text(s.rating_age));
        if (t.level == PctlSafetyLevel_Custom) t.note->setText("playguard/cadran/restrictions/custom"_i18n);
        else if (known && s.rating_age) t.note->setText(brls::getStr("playguard/cadran/restrictions/up_to", org.empty() ? "—" : org, (int)s.rating_age));
        else t.note->setText("playguard/cadran/restrictions/any_age"_i18n);
        t.sns->setText(!known ? "playguard/cadran/restrictions/sns_unknown"_i18n
                       : s.sns_post_restriction ? "playguard/cadran/restrictions/sns_blocked"_i18n
                                                : "playguard/cadran/restrictions/sns_allowed"_i18n);
        t.comm->setText(!known ? "playguard/cadran/restrictions/comm_unknown"_i18n
                        : s.free_communication_restriction ? "playguard/cadran/restrictions/comm_blocked"_i18n
                                                           : "playguard/cadran/restrictions/comm_allowed"_i18n);
        t.sns_dot->setColor(!known ? theme["cadran/text3"] : s.sns_post_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
        t.comm_dot->setColor(!known ? theme["cadran/text3"] : s.free_communication_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
    }
}

brls::View* CadranRestrictionsTab::create() { return new CadranRestrictionsTab(); }
