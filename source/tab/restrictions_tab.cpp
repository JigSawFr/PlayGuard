// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/restrictions_tab.hpp"

#include "action/history_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/rating_flow.hpp"
#include "action/session_pause.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_text.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
const uint8_t AGES[] = { 0, 3, 4, 6, 7, 8, 10, 12, 13, 14, 15, 16, 17, 18 };

// The level in force: a check and the word, on teal. Said in words, not as
// a ring: the teal ring is the focus's (a level focused is not the one in
// force).
brls::Box* badge()
{
    brls::Theme theme = brls::Application::getTheme();
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setHeight(26);
    box->setShrink(0);
    box->setCornerRadius(13);
    box->setPaddingLeft(7);
    box->setPaddingRight(10);
    box->setBackgroundColor(theme["cadran/teal_fill"]);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setMarginLeft(6);
    CadranIcon* check = cadran::kit::icon("check", 15, "cadran/navy");
    check->setMarginRight(4);
    box->addView(check);
    brls::Label* words = cadran::kit::label("playguard/cadran/restrictions/current"_i18n, cadran::kit::TEXT_META, "cadran/navy");
    words->setSingleLine(true);
    box->addView(words);
    return box;
}
}   // namespace

RestrictionsTab::RestrictionsTab()
    : TabBase("xml/tab/restrictions.xml"), list(this)
{
    // The levels: four tiles in a row, no restriction as a slim card under them.
    auto* levels = static_cast<brls::Box*>(this->getView("rs_levels"));
    auto* row = static_cast<brls::Box*>(this->getView("rs_tiles"));
    for (uint32_t level : { PctlSafetyLevel_YoungChild, PctlSafetyLevel_Child, PctlSafetyLevel_Teen, PctlSafetyLevel_Custom,
                            PctlSafetyLevel_None }) {
        this->tiles.push_back(this->make_tile(level, level == PctlSafetyLevel_Custom));
        if (level == PctlSafetyLevel_None) levels->addView(this->tiles.back().box);
        else row->addView(this->tiles.back().box);
    }
    cadran::kit::set_kicker(static_cast<brls::Label*>(this->getView("rs_kicker")), "playguard/restrictions/section_level"_i18n);
    this->list.apply({ { "rs_age", "calendar" }, { "rs_sns", "user" }, { "rs_comm", "phone" }, { "rs_vr", "eye" },
                       { "rs_org", "flag" }, { "rs_blocked", "list" } });

    custom_note->setSingleLine(false);
    note->setSingleLine(false);
    blocked_note->setSingleLine(false);

    blocked->registerClickAction([this](brls::View*) {
        rating_flow::show_blocked(this->st, this->age_now);
        return true;
    });

    age->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        std::vector<std::string> names;
        int selected = 0;
        for (size_t i = 0; i < sizeof(AGES); i++) {
            names.push_back(ui::age_text(AGES[i]));
            if (AGES[i] == this->shown.rating_age) selected = (int)i;
        }
        ui::pick("playguard/restrictions/age"_i18n, names, selected, [this](int index) {
            PctlCustomSettings s = this->shown;
            const uint8_t before = s.rating_age;
            s.rating_age = AGES[index];
            // 0 is no limit: any other change from it is stricter.
            const bool looser = before != 0 && (s.rating_age == 0 || s.rating_age > before);
            this->write_custom(s, looser ? brls::getStr("playguard/restrictions/confirm_age", ui::age_text(s.rating_age)) : "");
        });
        return true;
    });

    sns->init("playguard/restrictions/sns"_i18n, false, [this](bool on) {
        PctlCustomSettings s = this->shown;
        s.sns_post_restriction = on;
        this->write_custom(s, on ? "" : "playguard/restrictions/confirm_sns_off"_i18n);
    });
    comm->init("playguard/restrictions/comm"_i18n, false, [this](bool on) {
        PctlCustomSettings s = this->shown;
        s.free_communication_restriction = on;
        this->write_custom(s, on ? "" : "playguard/restrictions/confirm_comm_off"_i18n);
    });
    org->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        // Each with the countries it rates for (nn::ns::RatingOrganization's
        // order), as a parent may not know which is theirs.
        static const char* ORG_WHERE[] = { "cero", "grac", "gsrmr", "esrb", "classind", "usk", "pegi",
                                           "pegi_portugal", "pegi_bbfc", "russian", "acb", "oflc", "iarc" };
        std::vector<std::string> names, countries;
        for (uint32_t i = 0; i < 13; i++) {
            names.push_back(pctl_rating_org_name(i));
            countries.push_back(brls::getStr(std::string("playguard/restrictions/org_where/") + ORG_WHERE[i]));
        }
        ui::pick("playguard/restrictions/org"_i18n, names, this->st.rating_org_ok && this->st.rating_org < 13 ? (int)this->st.rating_org : 0,
                 [this](int index) {
                     if (this->st.rating_org_ok && (int)this->st.rating_org == index) return;
                     ui::confirm(pt_flow::with_link_note(brls::getStr("playguard/restrictions/confirm_org",
                                                                      pctl_rating_org_name((uint32_t)index)), &this->st),
                                 "playguard/play_timer/confirm_set"_i18n, [this, index]() {
                                     Result rc = pctl_set_rating_org((uint32_t)index);
                                     if (R_SUCCEEDED(rc) && this->st.rating_org_ok)
                                         history_flow::record_values("org", { (int)this->st.rating_org }, { index });
                                     if (R_FAILED(rc))
                                         history_flow::record_failure("org", rc, this->st.rating_org_ok ? std::vector<int>{ (int)this->st.rating_org }
                                                                                                        : std::vector<int>{},
                                                                      { index });
                                     ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
                                     this->refresh();
                                 });
                 }, true, countries);
        return true;
    });

    vr->init("playguard/restrictions/vr"_i18n, false, [this](bool on) {
        Result rc = pctl_set_stereo_vision_restricted(on);
        if (R_SUCCEEDED(rc)) history_flow::record_values("vr", { on ? 0 : 1 }, { on ? 1 : 0 });
        else history_flow::record_failure("vr", rc, { on ? 0 : 1 }, { on ? 1 : 0 });
        ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
        this->refresh();
    });
    ui::guard_switch(sns);
    ui::guard_switch(comm);
    ui::guard_switch(vr);
}

RestrictionsTab::Tile RestrictionsTab::make_tile(uint32_t level, bool last)
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
        t.box->setCornerRadius(cadran::kit::RADIUS_CARD);
        t.box->setHighlightCornerRadius(cadran::kit::RADIUS_CARD);
        t.box->setPadding(14, 22, 14, 24);
        t.box->setBorderThickness(2);
        t.box->setMarginTop(12);
        brls::Label* title = kit::label(ui::level_name(level), 17);
        title->setSingleLine(true);
        title->setMarginRight(14);
        t.box->addView(title);
        t.note = kit::label("playguard/cadran/restrictions/none"_i18n, 15, "cadran/text3");
        t.note->setGrow(1.0f);
        t.note->setWidth(0);
        t.box->addView(t.note);
        t.badge = badge();
        t.box->addView(t.badge);
    } else {
        t.box = kit::card(true, true);
        t.box->setMinHeight(226);
        t.box->setCornerRadius(cadran::kit::RADIUS_TOP);
        t.box->setHighlightCornerRadius(cadran::kit::RADIUS_TOP);
        t.box->setPadding(18, 18, 16, 18);
        t.box->setBorderThickness(2);
        if (!last) t.box->setMarginRight(12);

        auto* head = new brls::Box(brls::Axis::ROW);
        head->setAlignItems(brls::AlignItems::FLEX_START);
        brls::Label* title = kit::label(ui::level_name(level), 19);
        // Large already; a quarter of the row has no room for larger text
        // ("Nutzerdefiniert" would break inside the word).
        title->setFontScaled(false);
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
        t.note = kit::label("", 15, "cadran/text3");
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
        const auto row = [&](brls::Label*& text, brls::Label*& state, CadranIcon*& icon) {
            auto* r = new brls::Box(brls::Axis::ROW);
            r->setAlignItems(brls::AlignItems::CENTER);
            r->setMarginBottom(6);
            // What it is about, on two lines when a language needs them (the
            // tiles of the row grow together); its state at the right.
            text = kit::label("", 15, "cadran/text2");
            text->setGrow(1.0f);
            text->setWidth(0);
            text->setMarginRight(6);
            r->addView(text);
            state = kit::label("", 15, "cadran/text");
            state->setSingleLine(true);
            state->setShrink(0);
            state->setMarginRight(6);
            r->addView(state);
            icon = kit::icon("check", 18, "cadran/teal_fill");
            icon->setShrink(0);
            r->addView(icon);
            t.box->addView(r);
        };
        row(t.sns, t.sns_state, t.sns_icon);
        row(t.comm, t.comm_state, t.comm_icon);
        row(t.games, t.games_state, t.games_icon);
        t.games_row = static_cast<brls::Box*>(t.games->getParent());
    }

    t.box->registerAction("playguard/cadran/restrictions/choose"_i18n, brls::BUTTON_A, [this, level](brls::View*) {
        if (this->st.safety_level_ok && this->st.safety_level == level) {
            // Custom in force: its settings, the lines under the tiles.
            brls::View* first = level == PctlSafetyLevel_Custom ? this->getView("rs_age") : nullptr;
            if (first && first->getVisibility() == brls::Visibility::VISIBLE) brls::Application::giveFocus(first);
            return true;
        }
        if (ui::refuse_read_only()) return true;
        this->choose_level((int)level);
        return true;
    });
    return t;
}

void RestrictionsTab::choose_level(int index)
{
    // A preset: say what it restricts (1034) before choosing it.
    std::string body = brls::getStr("playguard/restrictions/confirm_level", ui::level_name((uint32_t)index));
    PctlCustomSettings preset;
    if (index >= PctlSafetyLevel_YoungChild && R_SUCCEEDED(pctl_get_level_settings((uint32_t)index, &preset)))
        body += "\n\n" + this->settings_text(preset);
    else if (index == PctlSafetyLevel_Custom)
        body += "\n\n" + "playguard/restrictions/custom_hint"_i18n;
    else if (index == PctlSafetyLevel_None)   // say what goes
        body += "\n\n" + "playguard/restrictions/none_body"_i18n;
    body = pt_flow::with_link_note(body, &this->st);
    const int before = this->st.safety_level_ok ? (int)this->st.safety_level : -1;
    const auto apply = [this, index, before]() {
        Result rc = pctl_set_safety_level((uint32_t)index);
        if (R_SUCCEEDED(rc) && before >= 0) history_flow::record_values("level", { before }, { index });
        if (R_FAILED(rc))
            history_flow::record_failure("level", rc, before >= 0 ? std::vector<int>{ before } : std::vector<int>{}, { index });
        ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
        this->refresh();
    };
    // No restriction at all lifts every one: confirmed in coral.
    if (index == PctlSafetyLevel_None) ui::confirm_danger(body, "playguard/play_timer/confirm_set"_i18n, apply);
    else ui::confirm(body, "playguard/play_timer/confirm_set"_i18n, apply);
}

std::string RestrictionsTab::settings_text(const PctlCustomSettings& s) const
{
    const std::string org_suffix = this->st.rating_org_ok ? std::string(" (") + pctl_rating_org_name(this->st.rating_org) + ")" : "";
    return cadran_text::table({ { "playguard/restrictions/age"_i18n, ui::age_text(s.rating_age) + org_suffix },
                                { "playguard/restrictions/sns"_i18n, ui::yes_no(s.sns_post_restriction) },
                                { "playguard/restrictions/comm"_i18n, ui::yes_no(s.free_communication_restriction) } });
}

void RestrictionsTab::write_custom(const PctlCustomSettings& s, const std::string& loosens)
{
    // Under a preset: the level becomes Custom, from its settings; asked
    // first, with what the change allows when it does.
    const bool to_custom = !this->custom_now;
    if (to_custom || !loosens.empty()) {
        std::string body = loosens;
        if (to_custom) {
            body = brls::getStr("playguard/restrictions/to_custom",
                                this->st.safety_level_ok ? ui::level_name(this->st.safety_level) : "—") +
                   (loosens.empty() ? "" : "\n\n" + loosens) + "\n\n" + this->settings_text(s);
        }
        ui::confirm(pt_flow::with_link_note(body, &this->st), "playguard/play_timer/confirm_set"_i18n,
                    [this, s, to_custom]() { this->apply_custom(s, to_custom); }, this->refresher());
        return;
    }
    this->apply_custom(s, false);
}

void RestrictionsTab::apply_custom(const PctlCustomSettings& s, bool to_custom)
{
    const int before = this->st.safety_level_ok ? (int)this->st.safety_level : -1;
    Result rc = 0;
    if (to_custom) {
        rc = pctl_set_safety_level(PctlSafetyLevel_Custom);
        if (R_SUCCEEDED(rc) && before >= 0) history_flow::record_values("level", { before }, { (int)PctlSafetyLevel_Custom });
        if (R_FAILED(rc))
            history_flow::record_failure("level", rc, before >= 0 ? std::vector<int>{ before } : std::vector<int>{},
                                         { (int)PctlSafetyLevel_Custom });
    }
    // Custom's own settings until now, which the level switched to holds.
    const bool had = this->st.settings_ok && !to_custom;
    if (R_SUCCEEDED(rc)) {
        rc = pctl_set_custom_settings(&s);
        if (R_SUCCEEDED(rc) && had)
            history_flow::record_values("custom", history_flow::custom_values(this->st.settings), history_flow::custom_values(s));
        if (R_FAILED(rc)) {
            history_flow::record_failure("custom", rc, had ? history_flow::custom_values(this->st.settings) : std::vector<int>{},
                                         history_flow::custom_values(s));
            // Not left on Custom's older settings, which may allow more than
            // the preset did: the level before, again.
            if (to_custom && before >= 0 && R_SUCCEEDED(pctl_set_safety_level((uint32_t)before)))
                history_flow::record_values("level", { (int)PctlSafetyLevel_Custom }, { before });
        }
    }
    ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
    this->refresh();   // re-reads, so a refused change flips the switch back
}

void RestrictionsTab::refresh()
{
    pctl_status_fetch(&this->st);
    ui::note_unlocked(this->st.temp_unlocked_ok, this->st.temp_unlocked);
    session_pause::shown_as_locked(&this->st, nullptr);   // the restrictions as they are for the child
    const std::string na = "playguard/common/unavailable"_i18n;
    if (this->st.safety_level_ok) cadran::set_pill(brls::getStr("playguard/cadran/pill/level", ui::level_name(this->st.safety_level)), "shield");
    else cadran::set_pill("");

    // Custom in force: its settings, editable, under the tiles (a preset's
    // are in its tile).
    const bool custom = this->st.safety_level_ok && this->st.safety_level == PctlSafetyLevel_Custom &&
                        this->st.settings_ok;
    PctlCustomSettings preset = {};
    const bool preset_ok = !custom && this->st.safety_level_ok && this->st.safety_level != PctlSafetyLevel_Custom &&
                           R_SUCCEEDED(pctl_get_level_settings(this->st.safety_level, &preset));
    // Under a preset, its settings: changing one makes the level Custom
    // (write_custom), the note under them says so.
    this->custom_now = custom;
    const bool settings = custom || preset_ok;
    if (settings) this->shown = custom ? this->st.settings : preset;
    custom_header->setTitle(custom ? "playguard/restrictions/section_custom"_i18n
                                   : "playguard/restrictions/section_level_settings"_i18n);
    ui::set_visible(custom_header.getView(), settings);
    ui::set_visible(age.getView(), settings);
    ui::set_visible(sns.getView(), settings);
    ui::set_visible(comm.getView(), settings);
    ui::set_visible(custom_note.getView(), !custom);
    if (settings) {
        age->setDetailText(ui::age_text(this->shown.rating_age));
        sns->setOn(this->shown.sns_post_restriction, false);
        comm->setOn(this->shown.free_communication_restriction, false);
    }

    ui::set_visible(vr.getView(), this->st.stereo_vision_ok);
    if (this->st.stereo_vision_ok) vr->setOn(this->st.stereo_vision_restricted, false);

    org->setDetailText(this->st.rating_org_ok ? pctl_rating_org_name(this->st.rating_org) : na);
    // The installed games the level blocks: from the last game check.
    this->age_now = custom ? this->st.settings.rating_age : preset_ok ? preset.rating_age : -1;
    size_t n = 0, of = 0;
    const bool counted = rating_flow::count_blocked(this->st, this->age_now, &n, &of);
    if (counted) blocked->setDetailText(ui::opens(brls::getStr("playguard/restrictions/blocked_value", (int)n, (int)of)));
    ui::set_visible(blocked.getView(), counted);
    ui::set_visible(blocked_note.getView(), counted);
    // Read-only: everything stays in sight, greyed; A says why nothing changes.
    const bool writable = !app::read_only();
    for (brls::DetailCell* c : { (brls::DetailCell*)age.getView(), (brls::DetailCell*)sns.getView(),
                                 (brls::DetailCell*)comm.getView(), (brls::DetailCell*)vr.getView(),
                                 (brls::DetailCell*)org.getView() })
        ui::show_writable(c, writable);
    this->list.sync();
    this->show_tiles(custom);
}

namespace
{
// What a line is about in `name`; its state: a padlock in coral when
// `restricts`, else "Allowed" in words (a check could read as "protected");
// `words` beside the padlock (a count), "—" when not `known`.
void line(brls::Label* name, brls::Label* state, CadranIcon* icon, const std::string& what, bool known,
          bool restricts, const std::string& words = "")
{
    name->setText(what);
    const std::string said = !words.empty() ? words : restricts ? "" : "playguard/cadran/restrictions/allowed"_i18n;
    state->setText(known ? said : "\u2014");
    state->setTextColor(brls::Application::getTheme()[known && !restricts && words.empty() ? "cadran/text2" : "cadran/text"]);
    ui::set_visible(state, !state->getFullText().empty());
    icon->setIcon("lock");
    icon->setColor(brls::Application::getTheme()["cadran/coral_fill"]);
    icon->setVisibility(known && restricts ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}
}   // namespace

void RestrictionsTab::show_tiles(bool custom)
{
    brls::Theme theme = brls::Application::getTheme();
    const PctlStatus& status = this->st;
    const std::string org_name = status.rating_org_ok ? pctl_rating_org_name(status.rating_org) : "";
    for (Tile& t : this->tiles) {
        const bool selected = status.safety_level_ok && status.safety_level == t.level;
        PctlCustomSettings s = {};
        bool known = true;   // no restriction: nothing restricted
        if (t.level == PctlSafetyLevel_Custom) {
            known = status.settings_ok;
            s = status.settings;
        } else if (t.level != PctlSafetyLevel_None) {
            known = R_SUCCEEDED(pctl_get_level_settings(t.level, &s));
        }
        // The level in force: a soft fill and its check; the teal ring is
        // the focus's alone.
        t.box->setBackgroundColor(selected ? theme["cadran/teal_soft"] : theme["cadran/surf"]);
        t.box->setBorderColor(theme["cadran/line"]);
        // Gone, not only hidden: its words would take the title's room.
        t.badge->setVisibility(selected ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        if (t.level == PctlSafetyLevel_Custom && t.to_settings != custom) {
            t.to_settings = custom;
            t.box->updateActionHint(brls::BUTTON_A, custom ? "playguard/cadran/restrictions/to_settings"_i18n
                                                           : "playguard/cadran/restrictions/choose"_i18n);
            brls::Application::getGlobalHintsUpdateEvent()->fire();
        }
        // Read-only: the levels stay, dimmed; A says why nothing changes.
        t.box->setAlpha(app::read_only() && !selected ? 0.6f : 1.0f);
        if (!t.age) continue;   // no restriction: its words say it all
        t.age->setTextColor(selected ? theme["cadran/teal"] : theme["cadran/text"]);
        t.age->setText(!known ? "—" : ui::age_text(s.rating_age));
        // "PEGI 12" kept on one line; Custom says its own settings too.
        const std::string rated = known && s.rating_age
            ? brls::getStr("playguard/cadran/restrictions/up_to", (org_name.empty() ? "—" : org_name) + "\u00A0" + std::to_string((int)s.rating_age))
            : "playguard/cadran/restrictions/any_age"_i18n;
        t.note->setText(t.level == PctlSafetyLevel_Custom ? rated + "\n" + "playguard/cadran/restrictions/custom"_i18n : rated);
        line(t.sns, t.sns_state, t.sns_icon, "playguard/cadran/restrictions/sns"_i18n, known, s.sns_post_restriction);
        line(t.comm, t.comm_state, t.comm_icon, "playguard/cadran/restrictions/comm"_i18n, known,
             s.free_communication_restriction);
        // The installed games it blocks, once a game check was made.
        size_t n = 0, of = 0;
        const bool counted = known && rating_flow::count_blocked(status, s.rating_age, &n, &of);
        ui::set_visible(t.games_row, counted);
        if (counted) {
            line(t.games, t.games_state, t.games_icon, "playguard/cadran/restrictions/games"_i18n, true, n > 0,
                 n ? std::to_string(n) : "playguard/cadran/restrictions/games_none"_i18n);
        }
    }
}

brls::View* RestrictionsTab::create()
{
    return new RestrictionsTab();
}
