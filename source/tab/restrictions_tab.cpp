// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/restrictions_tab.hpp"

#include "action/history_flow.hpp"
#include "action/pt_flow.hpp"
#include "action/rating_flow.hpp"
#include "app.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
const uint8_t AGES[] = { 0, 3, 4, 6, 7, 8, 10, 12, 13, 14, 15, 16, 17, 18 };

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
    static_cast<brls::Label*>(this->getView("rs_kicker"))->setText(cadran::upper("playguard/restrictions/section_level"_i18n));
    this->list.apply({ { "rs_age", "calendar" }, { "rs_sns", "user" }, { "rs_comm", "phone" }, { "rs_vr", "eye" },
                       { "rs_org", "flag" }, { "rs_free_comm", "gamepad" }, { "rs_blocked", "list" } });

    custom_note->setSingleLine(false);
    note->setSingleLine(false);
    blocked_note->setSingleLine(false);

    blocked->registerClickAction([this](brls::View*) {
        rating_flow::show_blocked(this->st, this->age_now);
        return true;
    });

    age->registerClickAction([this](brls::View*) {
        if (!this->st.settings_ok) return true;
        if (ui::refuse_read_only()) return true;
        std::vector<std::string> names;
        int selected = 0;
        for (size_t i = 0; i < sizeof(AGES); i++) {
            names.push_back(ui::age_text(AGES[i]));
            if (AGES[i] == this->st.settings.rating_age) selected = (int)i;
        }
        ui::pick("playguard/restrictions/age"_i18n, names, selected, [this](int index) {
            PctlCustomSettings s = this->st.settings;
            const uint8_t before = s.rating_age;
            s.rating_age = AGES[index];
            // 0 is no limit: any other change from it is stricter.
            const bool looser = before != 0 && (s.rating_age == 0 || s.rating_age > before);
            this->write_custom(s, looser ? brls::getStr("playguard/restrictions/confirm_age", ui::age_text(s.rating_age)) : "");
        });
        return true;
    });

    sns->init("playguard/restrictions/sns"_i18n, false, [this](bool on) {
        PctlCustomSettings s = this->st.settings;
        s.sns_post_restriction = on;
        this->write_custom(s, on ? "" : "playguard/restrictions/confirm_sns_off"_i18n);
    });
    comm->init("playguard/restrictions/comm"_i18n, false, [this](bool on) {
        PctlCustomSettings s = this->st.settings;
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
        t.box->setCornerRadius(18);
        t.box->setHighlightCornerRadius(18);
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
        t.box->setCornerRadius(22);
        t.box->setHighlightCornerRadius(22);
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
        const auto row = [&](brls::Rectangle*& dot, brls::Label*& text, brls::Label*& state) {
            auto* r = new brls::Box(brls::Axis::ROW);
            r->setAlignItems(brls::AlignItems::FLEX_START);   // the dot by the first line
            r->setMarginBottom(6);
            dot = new brls::Rectangle(theme["cadran/text3"]);
            dot->setWidth(8);
            dot->setHeight(8);
            dot->setCornerRadius(4);
            dot->setMarginTop(5);
            dot->setMarginRight(6);
            dot->setShrink(0);
            r->addView(dot);
            // What it is about, then its state at the right; the state goes
            // under it when both do not fit (the tiles of the row grow
            // together), neither cut in the middle of a word.
            auto* words = new brls::Box(brls::Axis::ROW);
            words->setAlignItems(brls::AlignItems::CENTER);
            words->setGrow(1.0f);
            words->setWidth(0);
            kit::wrap(words);
            text = kit::label("", 15, "cadran/text2");
            text->setSingleLine(true);
            text->setMarginRight(6);
            words->addView(text);
            auto* gap = new brls::Box();
            gap->setGrow(1.0f);
            words->addView(gap);
            state = kit::label("", 15, "cadran/text");
            state->setSingleLine(true);
            state->setShrink(0);
            words->addView(state);
            r->addView(words);
            t.box->addView(r);
        };
        row(t.sns_dot, t.sns, t.sns_state);
        row(t.comm_dot, t.comm, t.comm_state);
        row(t.games_dot, t.games, t.games_state);
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
    ui::confirm(body, "playguard/play_timer/confirm_set"_i18n, [this, index, before]() {
                    Result rc = pctl_set_safety_level((uint32_t)index);
                    if (R_SUCCEEDED(rc) && before >= 0) history_flow::record_values("level", { before }, { index });
                    if (R_FAILED(rc))
                        history_flow::record_failure("level", rc, before >= 0 ? std::vector<int>{ before } : std::vector<int>{},
                                                     { index });
                    ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
                    this->refresh();
                });
}

std::string RestrictionsTab::settings_text(const PctlCustomSettings& s) const
{
    const std::string org = this->st.rating_org_ok ? std::string(" (") + pctl_rating_org_name(this->st.rating_org) + ")" : "";
    return brls::getStr("playguard/common/line", "playguard/restrictions/age"_i18n, ui::age_text(s.rating_age) + org) + "\n" +
           brls::getStr("playguard/common/line", "playguard/restrictions/sns"_i18n, ui::yes_no(s.sns_post_restriction)) + "\n" +
           brls::getStr("playguard/common/line", "playguard/restrictions/comm"_i18n, ui::yes_no(s.free_communication_restriction));
}

void RestrictionsTab::write_custom(const PctlCustomSettings& s, const std::string& loosens)
{
    if (!loosens.empty()) {
        ui::confirm(pt_flow::with_link_note(loosens, &this->st), "playguard/play_timer/confirm_set"_i18n,
                    [this, s]() { this->write_custom(s); }, [this]() { this->refresh(); });
        return;
    }
    Result rc = pctl_set_custom_settings(&s);
    if (R_SUCCEEDED(rc) && this->st.settings_ok)
        history_flow::record_values("custom", history_flow::custom_values(this->st.settings), history_flow::custom_values(s));
    if (R_FAILED(rc))
        history_flow::record_failure("custom", rc,
                                     this->st.settings_ok ? history_flow::custom_values(this->st.settings) : std::vector<int>{},
                                     history_flow::custom_values(s));
    ui::notify_result(rc, "playguard/restrictions/saved"_i18n, "playguard/restrictions/save_err"_i18n);
    this->refresh();   // re-reads, so a refused change flips the switch back
}

void RestrictionsTab::refresh()
{
    pctl_status_fetch(&this->st);
    ui::note_unlocked(this->st.temp_unlocked_ok, this->st.temp_unlocked);
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
    ui::set_visible(custom_header.getView(), custom);
    ui::set_visible(age.getView(), custom);
    ui::set_visible(sns.getView(), custom);
    ui::set_visible(comm.getView(), custom);
    ui::set_visible(custom_note.getView(), !custom);
    if (this->st.settings_ok) {
        age->setDetailText(ui::age_text(this->st.settings.rating_age));
        sns->setOn(this->st.settings.sns_post_restriction, false);
        comm->setOn(this->st.settings.free_communication_restriction, false);
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
    free_comm->setDetailText(this->st.free_comm_count_ok
        ? (this->st.free_comm_count == 1 ? "playguard/restrictions/free_comm_value_one"_i18n
                                         : brls::getStr("playguard/restrictions/free_comm_value", (int)this->st.free_comm_count))
        : na);
    this->list.sync();
    this->show_tiles(custom);
}

namespace
{
// "Communication · restricted" (or "交流・制限"): what it is about in `name`,
// its state in `state`; all of it in `name` when it has no separator.
void line(brls::Label* name, brls::Label* state, const std::string& text)
{
    for (const char* sep : { " \u00B7 ", "\u30FB" }) {
        const size_t at = text.rfind(sep);
        if (at == std::string::npos) continue;
        name->setText(text.substr(0, at));
        state->setText(text.substr(at + std::string(sep).size()));
        return;
    }
    name->setText(text);
    state->setText("");
}
}   // namespace

void RestrictionsTab::show_tiles(bool custom)
{
    brls::Theme theme = brls::Application::getTheme();
    const PctlStatus& st = this->st;
    const std::string org = st.rating_org_ok ? pctl_rating_org_name(st.rating_org) : "";
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
        // The level in force: a soft fill and its check; the teal ring is
        // the focus's alone.
        t.box->setBackgroundColor(selected ? theme["cadran/teal_soft"] : theme["cadran/surf"]);
        t.box->setBorderColor(theme["cadran/line"]);
        t.badge->setVisibility(selected ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
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
            ? brls::getStr("playguard/cadran/restrictions/up_to", (org.empty() ? "—" : org) + "\u00A0" + std::to_string((int)s.rating_age))
            : "playguard/cadran/restrictions/any_age"_i18n;
        t.note->setText(t.level == PctlSafetyLevel_Custom ? rated + "\n" + "playguard/cadran/restrictions/custom"_i18n : rated);
        line(t.sns, t.sns_state, !known ? "playguard/cadran/restrictions/sns_unknown"_i18n
                                 : s.sns_post_restriction ? "playguard/cadran/restrictions/sns_blocked"_i18n
                                                          : "playguard/cadran/restrictions/sns_allowed"_i18n);
        line(t.comm, t.comm_state, !known ? "playguard/cadran/restrictions/comm_unknown"_i18n
                                   : s.free_communication_restriction ? "playguard/cadran/restrictions/comm_blocked"_i18n
                                                                      : "playguard/cadran/restrictions/comm_allowed"_i18n);
        t.sns_dot->setColor(!known ? theme["cadran/text3"] : s.sns_post_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
        t.comm_dot->setColor(!known ? theme["cadran/text3"] : s.free_communication_restriction ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
        // The installed games it blocks, once a game check was made.
        size_t n = 0, of = 0;
        const bool counted = known && rating_flow::count_blocked(st, s.rating_age, &n, &of);
        ui::set_visible(t.games_row, counted);
        if (counted) {
            line(t.games, t.games_state, n ? brls::getStr("playguard/cadran/restrictions/games_blocked", (int)n)
                                           : "playguard/cadran/restrictions/games_none"_i18n);
            t.games_dot->setColor(n ? theme["cadran/coral_fill"] : theme["cadran/teal_fill"]);
        }
    }
}

brls::View* RestrictionsTab::create()
{
    return new RestrictionsTab();
}
