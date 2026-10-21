// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/forwarders_activity.hpp"

#include <algorithm>

#include "action/forwarder_flow.hpp"
#include "action/game_icons.hpp"
#include "action/hbmenu_lock.hpp"
#include "app.hpp"
#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_text.hpp"
#include "ui/format_text.hpp"
#include "ui/ui.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"
#include "util/paths.hpp"
#include "view/game_cell.hpp"

using namespace brls::literals;
using forwarders_logic::Row;
using forwarders_logic::State;

namespace
{

std::string state_only(const Row& r)
{
    switch (r.state) {
        case State::Ready:        return "playguard/forwarders/state_ready"_i18n;
        case State::NroMissing:   return "playguard/forwarders/state_nro_missing"_i18n;
        case State::GateMissing:  return "playguard/forwarders/state_gate_missing"_i18n;
        case State::NotInstalled: return "playguard/forwarders/state_not_installed"_i18n;
        case State::Elsewhere:    return "playguard/forwarders/state_elsewhere"_i18n;
    }
    return "";
}

std::string state_text_pin(const Row& r, bool full);

// A box of the grouped list: the focus coming from below goes to its last
// line, from above to its first (borealis' own: the line focused last),
// so the arrows walk the lines in order across the groups.
template <class Base>
class Nearest : public Base
{
  public:
    using Base::Base;
    brls::View* getDefaultFocus() override
    {
        if (this->getVisibility() != brls::Visibility::VISIBLE) return nullptr;
        brls::View* now = brls::Application::getCurrentFocus();
        const std::vector<brls::View*>& kids = this->getChildren();
        const bool below = now && !kids.empty() && now->getY() >= this->getY() + this->getHeight();
        if (!below) {
            for (brls::View* kid : kids)
                if (brls::View* v = kid->getDefaultFocus()) return v;
            return nullptr;
        }
        for (size_t i = kids.size(); i-- > 0;)
            if (brls::View* v = kids[i]->getDefaultFocus()) return v;
        return nullptr;
    }
};

// Its state, and whether it asks for the PIN (and when it does not); a
// protected menu seen working through its gate. In the list (`full` false)
// a forwarder with more than one rule says so in a word: Details has them.
std::string state_text(const Row& r, bool full = false)
{
    if (!r.pin) return state_only(r);
    if (r.verified && forwarders_logic::is_menu(r.nro))
        return state_text_pin(r, full) + " · " + "playguard/forwarders/state_verified"_i18n;
    return state_text_pin(r, full);
}

// Protected: the PIN, and when it is not asked.
std::string state_text_pin(const Row& r, bool full)
{
    std::string out = state_only(r) + " · ";
    // Paused after refused PINs: that first, it is what the child sees now.
    if (r.paused_until) return out + brls::getStr("playguard/forwarders/state_paused", forwarder_flow::until_label(r.paused_until));
    // Made before it carried its rules: its free hours wait for it to be made again.
    const std::string remake = r.carried ? "" : " · " + "playguard/forwarders/state_remake"_i18n;
    const int rules = (r.free_days ? 1 : 0) + (r.closed_days ? 1 : 0) + (r.limit ? 1 : 0);
    if (!full && rules > 1) return out + "playguard/forwarders/state_pin_rules"_i18n + remake;
    const forwarder_flow::Hours free = { r.free_days, r.free_from, r.free_to, r.free2_days, r.free2_from, r.free2_to };
    out += !r.free_days ? "playguard/forwarders/state_pin"_i18n
                        : brls::getStr("playguard/forwarders/state_pin_hours", forwarder_flow::hours_label(free));
    if (r.closed_days)
        out += " · " + brls::getStr("playguard/forwarders/state_closed",
                                    forwarder_flow::hours_label(r.closed_days, r.closed_from, r.closed_to));
    if (r.limit) out += " · " + "playguard/forwarders/state_limit"_i18n;
    return out + remake;
}

}   // namespace

void ForwardersActivity::onContentAvailable()
{
    note->setSingleLine(false);
    warning->setSingleLine(false);
    cadran::kit::empty_around(empty, "home");
    game_note->setSingleLine(false);
    self_cell->registerClickAction([this](brls::View*) {
        if (const Row* own = forwarders_logic::self_row(this->list_rows)) {
            if (own->state != State::NotInstalled) {
                this->actions(*own);
                return true;
            }
        }
        if (ui::refuse_read_only()) return true;
        forwarder_flow::create_self(this->then_rebuild());
        return true;
    });
    other->setDetailText(ui::opens());
    other->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        this->pick_homebrew();
        return true;
    });
    ui::show_writable(other, !app::read_only());
    lock_note->setSingleLine(false);
    lock_cell->registerClickAction([this](brls::View*) {
        std::weak_ptr<bool> weak = this->alive;
        // The setup may make forwarders: the list again, the focus kept here.
        const auto after = [this, weak](bool) {
            brls::sync([this, weak]() {
                if (!weak.expired()) this->rebuild(0, lock_cell.getView());
            });
        };
        if (hbmenu_lock::locked()) hbmenu_lock::unlock(after);
        else hbmenu_lock::lock(after);
        return true;
    });
    reinstall->setDetailText(ui::opens());
    reinstall->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        forwarder_flow::reinstall_all(this->then_rebuild());
        return true;
    });
    protect_all->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        forwarder_flow::protect_all(this->then_rebuild());
        return true;
    });
    pause_cell->registerClickAction([this](brls::View*) {
        if (ui::refuse_read_only()) return true;
        this->pick_pause();
        return true;
    });
    ui::show_writable(pause_cell, !app::read_only());
    week_cell->setDetailText(ui::opens());
    week_cell->registerClickAction([](brls::View*) {
        forwarder_flow::show_week();
        return true;
    });
    lock_check->setDetailText(ui::opens());
    lock_check->registerClickAction([](brls::View*) {
        hbmenu_lock::show_check();
        return true;
    });
    ui::show_writable(reinstall, !app::read_only());
    ui::show_writable(protect_all, !app::read_only());
    cadran::List(this->getContentView(), true)
        .apply({ { "fw_self", "home" }, { "fw_other", "plus" }, { "fw_reinstall", "refresh" }, { "fw_lock", "lock" },
                 { "fw_protect_all", "shield" }, { "fw_pause", "timer" }, { "fw_week", "calendar" }, { "fw_lock_check", "eye" } });
    // Without the forwarders' program (a build from elsewhere), nothing can
    // be added: said instead of the two lines.
    if (!forwarder_flow::loader_present()) {
        warning->setText("playguard/forwarders/no_loader"_i18n);
        ui::set_visible(warning.getView(), true);
        ui::set_visible(self_cell.getView(), false);
        ui::set_visible(other.getView(), false);
        ui::set_visible(lock_header.getView(), false);
        ui::set_visible(lock_cell.getView(), false);
        ui::set_visible(lock_note.getView(), false);
        ui::set_visible(reinstall.getView(), false);
        ui::set_visible(protect_all.getView(), false);
        ui::set_visible(lock_check.getView(), false);
        ui::set_visible(pause_cell.getView(), false);
        ui::set_visible(week_cell.getView(), false);
    } else {
        this->show_patches();
    }
    // Y: what this screen does, in the guide (a QR code).
    this->getContentView()->registerAction("playguard/forwarders/help"_i18n, brls::BUTTON_Y, [](brls::View*) {
        ui::qr_dialog("playguard/forwarders/help_body"_i18n, app::doc_url("features", "forwarders"));
        return true;
    });
    // The first list at once: the screen is not on top yet while it opens.
    this->build();
}

void ForwardersActivity::fixes()
{
    // What the card says is missing or open, each a press away; then the check.
    const hbmenu_lock::Summary s = hbmenu_lock::summary();
    const forwarders_logic::LockCheck& c = s.check;
    std::vector<ui::Choice> choices;
    const bool writable = !app::read_only();
    if (writable && !s.locked)
        for (const hbmenu_lock::SetupStep& step : hbmenu_lock::setup_steps(c))
            if (!step.done) {
                const hbmenu_lock::SetupStep::Kind kind = step.kind;
                choices.push_back({ step.label, [this, kind]() { hbmenu_lock::run_step(kind, this->then_rebuild()); } });
            }
    for (const Row& r : this->list_rows) {
        if (!writable || r.state == State::NotInstalled || r.self) continue;
        const Row copy = r;
        if (r.state == State::Elsewhere)
            choices.push_back({ brls::getStr("playguard/forwarders/fix_delete", r.name),
                                [this, copy]() { this->remove_with_undo(copy); } });
        else if (!r.pin && (forwarders_logic::is_menu(r.nro) || !s.locked))
            choices.push_back({ brls::getStr("playguard/forwarders/fix_protect", r.name),
                                [this, copy]() { forwarder_flow::set_pin(copy, true, this->then_rebuild(copy.id)); } });
    }
    // Made before they carried their rules: made again, in one go.
    if (writable && !forwarder_flow::to_remake().empty())
        choices.push_back({ "playguard/forwarders/fix_remake"_i18n, [this]() { forwarder_flow::remake(this->then_rebuild()); } });
    if (writable && !s.locked && c.ready())
        choices.push_back({ "playguard/hbmenu/cell"_i18n, [this]() { hbmenu_lock::lock(this->then_rebuild()); } });
    choices.push_back({ "playguard/hbmenu/check"_i18n, []() { hbmenu_lock::show_check(); } });
    choices.push_back({ "playguard/hbmenu/rescue"_i18n, []() { hbmenu_lock::show_rescue(); } });
    ui::pick_actions("playguard/hbmenu/section"_i18n, choices);
}

void ForwardersActivity::offer_undo(std::function<void()> undo)
{
    // ZR, for a few seconds, as the change history's own undo.
    constexpr long OFFER_MS = 8000;
    this->withdraw_undo();
    brls::View* view = this->getContentView();
    std::weak_ptr<bool> weak = this->alive;
    this->undo_action = view->registerAction("playguard/history/undo_hint"_i18n, brls::BUTTON_RT, [this, weak, undo](brls::View*) {
        if (weak.expired()) return true;
        this->withdraw_undo();
        undo();
        return true;
    });
    brls::Application::getGlobalHintsUpdateEvent()->fire();
    this->undo_timer = brls::delay(OFFER_MS, [this, weak]() {
        if (!weak.expired()) this->withdraw_undo();
    });
}

void ForwardersActivity::withdraw_undo()
{
    if (!this->undo_action) return;
    brls::cancelDelay(this->undo_timer);
    this->getContentView()->unregisterAction(this->undo_action);
    this->undo_action = 0;
    brls::Application::getGlobalHintsUpdateEvent()->fire();
}

void ForwardersActivity::remove_with_undo(const Row& row)
{
    forwarders::Made m;
    const bool known = forwarders::find(row.id, &m);
    std::weak_ptr<bool> weak = this->alive;
    forwarder_flow::remove(row, [this, weak, known, m](bool ok) {
        brls::sync([this, weak, known, m, ok]() {
            if (weak.expired()) return;
            this->rebuild();
            // Made here: installed again from its .nro (the same program ID).
            if (ok && known)
                this->offer_undo([this, m]() { forwarder_flow::put_back(m, true, this->then_rebuild(m.id)); });
        });
    });
}

void ForwardersActivity::show_lock()
{
    using ui::Check;
    const hbmenu_lock::Summary s = hbmenu_lock::summary();
    const bool on = s.locked;
    lock_cell->setDetailText(on ? "playguard/hbmenu/state_locked"_i18n : "playguard/hbmenu/state_open"_i18n);
    ui::show_writable(lock_cell, !app::read_only(), cadran::kit::color("cadran/text"), on ? cadran::kit::color("cadran/teal") : cadran::kit::color("cadran/text2"));
    const GatePauseRule rule = forwarder_flow::pause_rule();
    pause_cell->setDetailText(forwarder_flow::pause_label(rule.tries, rule.minutes));
    if (!forwarder_flow::loader_present()) {
        ui::set_visible(summary.getView(), false);
        return;
    }
    // The ways back: PlayGuard's own, the menus seen working, those not started yet.
    const forwarders_logic::LockCheck& c = s.check;
    std::string routes;
    const auto add = [&routes](const std::string& n) { routes += (routes.empty() ? "" : ", ") + n; };
    if (c.self) add("PlayGuard");
    for (const std::string& m : c.menus) {
        const bool seen = std::find(c.verified.begin(), c.verified.end(), m) != c.verified.end();
        add(seen ? m : brls::getStr("playguard/hbmenu/summary_untried", m));
    }
    const bool both = c.self && !c.verified.empty();
    std::vector<ui::CheckLine> lines = {
        { on ? Check::Ok : Check::Info, on ? "playguard/hbmenu/summary_locked"_i18n : "playguard/hbmenu/summary_open"_i18n },
        { both ? Check::Ok : on ? Check::Bad : Check::Warn,
          brls::getStr("playguard/hbmenu/summary_routes", routes.empty() ? "playguard/hbmenu/check_none"_i18n : routes) },
    };
    if (c.open_doors.empty()) lines.push_back({ Check::Ok, "playguard/hbmenu/line_no_doors"_i18n });
    else {
        std::string doors;
        for (const std::string& d : c.open_doors) doors += (doors.empty() ? "" : ", ") + d;
        lines.push_back({ Check::Warn, brls::getStr("playguard/hbmenu/summary_doors", doors) });
    }
    lines.push_back({ Check::Info, s.module ? "playguard/hbmenu/summary_module"_i18n : "playguard/hbmenu/summary_no_module"_i18n });
    ui::batch_layout(summary.getView(), [this, &lines, on]() {
        summary->clearViews();
        // A on it: what to do about what it says (fixes).
        brls::Box* card = cadran::kit::card(true, false);
        card->setCornerRadius(22);
        card->setHighlightCornerRadius(22);
        card->setPadding(22, 26, 22, 26);
        card->registerClickAction([this](brls::View*) {
            this->fixes();
            return true;
        });
        card->addGestureRecognizer(new brls::TapGestureRecognizer(card));
        auto* head = new brls::Box(brls::Axis::ROW);
        head->setAlignItems(brls::AlignItems::CENTER);
        brls::Box* kick = cadran::kit::kicker(on ? "lock" : "unlock", "playguard/hbmenu/section"_i18n);
        kick->setGrow(1.0f);
        head->addView(kick);
        brls::Box* a = cadran::kit::key("A", 20);
        a->setMarginRight(6);
        head->addView(a);
        head->addView(cadran::kit::label("playguard/hbmenu/card_fix"_i18n, 15, "cadran/text2"));
        card->addView(head);
        for (const ui::CheckLine& l : lines) {
            brls::Box* row = ui::check_row(l);
            row->setMarginTop(10);
            card->addView(row);
        }
        summary->addView(card);
    });
    ui::set_visible(summary.getView(), true);
}

void ForwardersActivity::show_patches()
{
    const auto state = forwarders_logic::patches_state(ui::patch_report());
    if (state == forwarders_logic::Patches::Missing) warning->setText("playguard/forwarders/patches_missing"_i18n);
    if (state == forwarders_logic::Patches::Unknown) warning->setText("playguard/forwarders/patches_unknown"_i18n);
    ui::set_visible(warning.getView(), state != forwarders_logic::Patches::Ready);
}

void ForwardersActivity::show_self()
{
    const Row* own = forwarders_logic::self_row(this->list_rows);
    const bool on = own && own->state != State::NotInstalled;
    self_cell->setDetailText(on ? state_text(*own) : "playguard/forwarders/add"_i18n);
    // Read-only: greyed when it would add it (its details stay a press away).
    ui::show_writable(self_cell, on || !app::read_only(), cadran::kit::color("cadran/text"),
                      on && own->state == State::Ready ? cadran::kit::color("cadran/teal") : cadran::kit::color("cadran/text2"));
}

std::function<void(bool)> ForwardersActivity::then_rebuild(uint64_t id)
{
    std::weak_ptr<bool> weak = this->alive;
    return [this, weak, id](bool) {
        // Next frame: the dialog that ran this gives the focus back first,
        // to a cell the rebuild then moves it away from before deleting it.
        brls::sync([this, weak, id]() {
            if (!weak.expired()) this->rebuild(id);
        });
    };
}

void ForwardersActivity::rebuild(uint64_t focus_id, brls::View* keep)
{
    // Under a dialog (one a change opened after it, the rescue sheet): once
    // it is closed, so the focus is not taken from it.
    if (!ui::on_top(this)) {
        std::weak_ptr<bool> weak = this->alive;
        brls::delay(150, [this, weak, focus_id, keep]() {
            if (!weak.expired()) this->rebuild(focus_id, keep);
        });
        return;
    }
    this->build(focus_id, keep);
}

void ForwardersActivity::build(uint64_t focus_id, brls::View* keep)
{
    // The focus to a cell that stays before the old rows go.
    brls::Application::giveFocus(keep ? keep : (brls::View*)self_cell.getView());
    this->cells.clear();
    this->list_rows = forwarder_flow::rows();
    // By group: PlayGuard's own, protected, not protected, another app's,
    // no longer on the HOME menu; by name in each (forwarders_logic::merge).
    const auto group_of = [](const Row& r) {
        if (r.state == State::NotInstalled) return 4;
        if (r.state == State::Elsewhere) return 3;
        if (r.self) return 0;
        return r.pin ? 1 : 2;
    };
    static const char* const LABELS[] = { nullptr, "playguard/forwarders/group_protected", "playguard/forwarders/group_open",
                                          "playguard/forwarders/group_elsewhere", "playguard/forwarders/group_gone" };
    int groups = 0;
    for (int g = 0; g < 5; g++)
        for (const Row& r : this->list_rows)
            if (group_of(r) == g) {
                groups++;
                break;
            }
    brls::View* focus = nullptr;
    brls::View* first = nullptr;
    ui::batch_layout(list.getView(), [&]() {
        list->clearViews();
        auto* groups_box = new Nearest<brls::Box>(brls::Axis::COLUMN);
        groups_box->setAlignItems(brls::AlignItems::STRETCH);
        list->addView(groups_box);
        for (int g = 0; g < 5; g++) {
            cadran::Card* card = nullptr;
            for (const Row& r : this->list_rows) {
                if (group_of(r) != g) continue;
                if (!card) {
                    // Named when there is more than one group.
                    if (groups > 1 && LABELS[g]) {
                        brls::Label* label = cadran::kit::label(brls::getStr(LABELS[g]), 15, "cadran/text3");
                        label->setMarginTop(groups_box->getChildren().empty() ? 0 : 12);
                        label->setMarginBottom(6);
                        label->setMarginLeft(8);
                        groups_box->addView(label);
                    }
                    card = new Nearest<cadran::Card>();
                    if (!groups_box->getChildren().empty()) card->setMarginTop(groups > 1 && LABELS[g] ? 0 : 10);
                    groups_box->addView(card);
                }
                auto* cell = new GameCell(game_icons::wanted());
                cell->setText(r.name);
                cell->setDetailText(state_text(r));
                cell->setDetailTextColor(r.paused_until                ? cadran::kit::color("cadran/amber")
                                         : r.state == State::Ready     ? cadran::kit::color("cadran/teal")
                                         : r.state == State::Elsewhere ? cadran::kit::color("cadran/text2")
                                                                       : cadran::kit::color("cadran/amber"));
                // On its icon: protected (a lock; a check for a menu seen
                // working), or made here and starting without the PIN.
                if (r.pin) cell->set_badge(r.verified && forwarders_logic::is_menu(r.nro) ? "check" : "lock");
                else if (!r.self && r.state != State::Elsewhere && r.state != State::NotInstalled)
                    cell->set_badge("unlock", "cadran/amber");
                const Row copy = r;
                cell->registerClickAction([this, copy](brls::View*) {
                    this->actions(copy);
                    return true;
                });
                card->addView(cell);
                cadran::List::row(cell);
                if (r.state != State::NotInstalled) this->cells.push_back({ r.id, cell });
                if (r.id == focus_id) focus = cell;
                if (!first) first = cell;
            }
        }
    });
    ui::set_visible(list.getView(), !this->list_rows.empty());
    ui::set_visible(empty.getView(), this->list_rows.empty());
    this->show_self();
    this->show_lock();
    this->load_icons();
    // The view kept, else the forwarder asked for, else the first one, else PlayGuard's line.
    if (keep) focus = keep;
    if (!focus && first && !focus_id) focus = first;
    brls::Application::giveFocus(focus ? focus : (brls::View*)self_cell.getView());
}

void ForwardersActivity::load_icons()
{
    if (!game_icons::wanted()) return;
    std::vector<u64> ids;
    for (const auto& c : this->cells) {
        bool owned = false;
        if (const int tex = game_icons::texture(c.first, owned)) c.second->set_icon_texture(tex, owned);
        else if (!game_icons::known(c.first)) ids.push_back(c.first);
    }
    if (ids.empty()) return;
    std::weak_ptr<bool> weak = this->alive;
    game_icons::load(ids, [this, weak]() {
        if (weak.expired()) return;
        for (const auto& c : this->cells) {
            bool owned = false;
            if (const int tex = game_icons::texture(c.first, owned)) c.second->set_icon_texture(tex, owned);
        }
    });
}

void ForwardersActivity::actions(const Row& row)
{
    const Row r = row;
    // Read-only: its details only (every other line changes the console).
    if (app::read_only()) {
        this->details(r);
        return;
    }
    std::vector<ui::Choice> choices;
    choices.push_back({ "playguard/forwarders/action_details"_i18n, [this, r]() { this->details(r); } });
    if (r.state == State::Ready || r.state == State::NroMissing || r.state == State::GateMissing) {
        choices.push_back({ "playguard/forwarders/action_rename"_i18n, [this, r]() { this->rename(r); } });
        choices.push_back({ "playguard/forwarders/action_author"_i18n, [this, r]() { this->change_author(r); } });
        if (!r.self)
            choices.push_back({ r.pin ? "playguard/forwarders/action_pin_off"_i18n : "playguard/forwarders/action_pin_on"_i18n,
                                [this, r]() { forwarder_flow::set_pin(r, !r.pin, this->then_rebuild(r.id)); } });
        if (!r.self && r.pin) {
            choices.push_back({ "playguard/forwarders/action_hours"_i18n, [this, r]() { this->pick_hours(r, false); } });
            choices.push_back({ "playguard/forwarders/action_closed"_i18n, [this, r]() { this->pick_hours(r, true); } });
            choices.push_back({ r.limit ? "playguard/forwarders/action_limit_off"_i18n : "playguard/forwarders/action_limit_on"_i18n,
                                [this, r]() { this->limit(r, !r.limit); } });
        }
        if (r.paused_until)
            choices.push_back({ "playguard/forwarders/action_lift"_i18n,
                                [this, r]() { forwarder_flow::lift_pause(r, this->then_rebuild(r.id)); } });
        choices.push_back({ "playguard/forwarders/action_icon"_i18n, [this, r]() { this->pick_icon(r); } });
        choices.push_back({ "playguard/forwarders/action_rebuild"_i18n,
                            [this, r]() { forwarder_flow::rebuild(r, "", "", this->then_rebuild(r.id)); } });
    }
    if (r.state == State::NotInstalled) {
        choices.push_back({ "playguard/forwarders/action_rebuild"_i18n,
                            [this, r]() { forwarder_flow::rebuild(r, "", "", this->then_rebuild(r.id)); } });
        choices.push_back({ "playguard/forwarders/action_forget"_i18n,
                            [this, r]() {
                                forwarders::Made m;
                                const bool known = forwarders::find(r.id, &m);
                                std::weak_ptr<bool> weak = this->alive;
                                forwarder_flow::forget(r, [this, weak, known, m](bool ok) {
                                    brls::sync([this, weak, known, m, ok]() {
                                        if (weak.expired()) return;
                                        this->rebuild();
                                        if (ok && known)
                                            this->offer_undo([this, m]() { forwarder_flow::put_back(m, false, this->then_rebuild(m.id)); });
                                    });
                                });
                            } });
    } else {
        choices.push_back({ "playguard/common/delete"_i18n, [this, r]() { this->remove_with_undo(r); } });
    }
    ui::pick_actions(r.name, choices);
}

void ForwardersActivity::details(const Row& r)
{
    const std::string unknown = "playguard/forwarders/unknown"_i18n;
    const std::string nro = r.nro.empty() ? unknown : nro::sd_path(r.nro);
    const std::string made = r.state == State::Elsewhere ? "playguard/forwarders/state_elsewhere"_i18n
                                                         : "playguard/forwarders/made_here"_i18n;
    ui::info(r.name + "\n\n" +
             cadran_text::table({ { "playguard/forwarders/author"_i18n, r.author.empty() ? unknown : r.author },
                                  { "playguard/tools/version"_i18n, r.version.empty() ? unknown : r.version },
                                  { ".nro", nro },
                                  { "playguard/forwarders/program_id"_i18n, forwarders::hex_id(r.id) } }) +
             "\n\n" + (r.state == State::Elsewhere ? made : made + " · " + state_text(r, true)));
}

void ForwardersActivity::rename(const Row& row)
{
    const Row r = row;
    ui::prompt_text("playguard/forwarders/name"_i18n, r.name, (int)forwarders::MAX_NAME, [this, r](std::string typed) {
        const std::string name = forwarders::clean_text(typed, forwarders::MAX_NAME);
        if (name.empty() || name == r.name) return;
        forwarder_flow::rebuild(r, name, "", this->then_rebuild(r.id));
    });
}

void ForwardersActivity::change_author(const Row& row)
{
    const Row r = row;
    ui::prompt_text("playguard/forwarders/author"_i18n, r.author, (int)forwarders::MAX_AUTHOR, [this, r](std::string typed) {
        const std::string author = forwarders::clean_text(typed, forwarders::MAX_AUTHOR);
        if (author.empty() || author == r.author) return;
        forwarder_flow::rebuild(r, "", author, this->then_rebuild(r.id));
    });
}

void ForwardersActivity::hours(const Row& r, bool closed, const forwarder_flow::Hours& h)
{
    std::weak_ptr<bool> weak = this->alive;
    const auto after = [this, weak, r, closed](bool ok) {
        brls::sync([this, weak, r, ok, closed]() {
            if (weak.expired()) return;
            this->rebuild(r.id);
            // The hours as they were, without the PIN again.
            if (ok)
                this->offer_undo([this, r, closed]() {
                    if (closed)
                        forwarder_flow::set_closed(r, r.closed_days, r.closed_from, r.closed_to, this->then_rebuild(r.id), false);
                    else
                        forwarder_flow::set_hours(r, { r.free_days, r.free_from, r.free_to, r.free2_days, r.free2_from, r.free2_to },
                                                  this->then_rebuild(r.id), false);
                });
        });
    };
    if (closed) forwarder_flow::set_closed(r, h.days, h.from, h.to, after);
    else forwarder_flow::set_hours(r, h, after);
}

void ForwardersActivity::limit(const Row& r, bool on)
{
    std::weak_ptr<bool> weak = this->alive;
    forwarder_flow::set_limit(r, on, [this, weak, r, on](bool ok) {
        brls::sync([this, weak, r, ok, on]() {
            if (weak.expired()) return;
            this->rebuild(r.id);
            if (ok) this->offer_undo([this, r, on]() { forwarder_flow::set_limit(r, !on, this->then_rebuild(r.id), false); });
        });
    });
}

void ForwardersActivity::pick_pause()
{
    // Never, then tries in a row and how long: the default (5, 15 min) among them.
    static const unsigned TRIES[] = { 0, 3, 5, 5, 10 }, MINUTES[] = { 0, 15, 15, 60, 60 };
    const GatePauseRule now = forwarder_flow::pause_rule();
    std::vector<std::string> names;
    int current = -1;
    for (size_t i = 0; i < 5; i++) {
        names.push_back(forwarder_flow::pause_label(TRIES[i], MINUTES[i]));
        if (now.tries == TRIES[i] && (!now.tries || now.minutes == MINUTES[i])) current = (int)i;
    }
    std::weak_ptr<bool> weak = this->alive;
    ui::pick("playguard/forwarders/pause_cell"_i18n, names, current, [this, weak](int index) {
        if (weak.expired() || index < 0 || index > 4) return;
        forwarder_flow::set_pause_rule(TRIES[index], MINUTES[index], [this, weak](bool) {
            brls::sync([this, weak]() {
                if (!weak.expired()) this->show_lock();
            });
        });
    }, true, { "", "", "playguard/forwarders/pause_default"_i18n, "", "" });
}

void ForwardersActivity::pick_icon(const Row& row)
{
    const Row r = row;
    const std::vector<std::string> shots = forwarder_flow::album_shots();
    // Its .nro's own first, then the album's screenshots, the newest first.
    std::vector<std::string> names = { "playguard/forwarders/icon_own"_i18n };
    for (const std::string& p : shots) {
        const std::string label = forwarders_logic::shot_label(p);
        names.push_back(label.empty() ? forwarders_logic::file_title(p) : label);
    }
    if (shots.empty()) {
        ui::notice("eye", ui::Check::Info, "playguard/forwarders/action_icon"_i18n, "playguard/forwarders/icon_none"_i18n);
        return;
    }
    std::weak_ptr<bool> weak = this->alive;
    ui::pick("playguard/forwarders/icon_title"_i18n, names, r.custom_icon ? -1 : 0, [this, weak, r, shots](int index) {
        if (weak.expired() || index < 0 || (size_t)index > shots.size()) return;
        if (index == 0 && !r.custom_icon) return;
        forwarder_flow::set_icon(r, index == 0 ? "" : shots[(size_t)index - 1], this->then_rebuild(r.id));
    });
}

void ForwardersActivity::pick_span(const std::string& title, int from_hour, int to_hour, std::function<void(int, int)> done)
{
    std::weak_ptr<bool> weak = this->alive;
    std::vector<std::string> starts, ends;
    for (int h = 0; h < 24; h++) starts.push_back(fmt::format("{:02}:00", h));
    for (int h = 1; h <= 24; h++) ends.push_back(fmt::format("{:02}:00", h));
    const std::string from_title = title.empty() ? "playguard/forwarders/hours_from"_i18n
                                                 : brls::getStr("playguard/forwarders/hours_from_of", title);
    const std::string to_title = title.empty() ? "playguard/forwarders/hours_to"_i18n
                                               : brls::getStr("playguard/forwarders/hours_to_of", title);
    ui::pick(from_title, starts, from_hour, [weak, ends, to_title, to_hour, done](int from) {
        if (weak.expired() || from < 0 || from > 23) return;
        brls::sync([weak, ends, to_title, to_hour, from, done]() {
            ui::pick(to_title, ends, to_hour, [weak, from, done](int to) {
                if (weak.expired() || to < 0 || to > 23) return;
                const int end = (to + 1) * 60, start = from * 60;
                if (end == start) return;
                done(start, end);
            });
        });
    });
}

void ForwardersActivity::pick_hours(const Row& row, bool closed)
{
    const Row r = row;
    const int cur_days = closed ? r.closed_days : r.free_days, cur_from = closed ? r.closed_from : r.free_from,
              cur_to = closed ? r.closed_to : r.free_to;
    // Never, every day, the school days, the weekend; free hours, the school
    // days and the weekend apart too.
    static const int DAYS[] = { 0, 0x7F, 0x3E, 0x41 };
    std::vector<std::string> names = { "playguard/forwarders/hours_never"_i18n, "playguard/forwarders/hours_every_day"_i18n,
                                       "playguard/forwarders/hours_weekdays"_i18n, "playguard/forwarders/hours_weekend"_i18n };
    if (!closed) names.push_back("playguard/forwarders/hours_apart"_i18n);
    int current = 0;
    for (int i = 1; i < 4; i++)
        if (cur_days == DAYS[i]) current = i;
    if (!closed && r.free2_days) current = 4;
    std::weak_ptr<bool> weak = this->alive;
    ui::pick(closed ? "playguard/forwarders/closed_title"_i18n : "playguard/forwarders/hours_title"_i18n, names, current,
             [this, weak, r, closed, cur_days, cur_from, cur_to](int index) {
        if (weak.expired() || index < 0 || index > (closed ? 3 : 4)) return;
        if (index == 0) {
            this->hours(r, closed, {});
            return;
        }
        // Free: 09:00 to 20:00 at first; closed: 21:00 to 07:00.
        const int from_now = cur_days ? cur_from / 60 : closed ? 21 : 9;
        const int to_now = cur_days ? std::max(0, cur_to / 60 - 1) : closed ? 6 : 19;
        if (index == 4) {
            // The school days (16:00 to 19:00 at first), then the weekend (9:00 to 20:00).
            const bool had = r.free2_days != 0;
            const int wd_from = had ? r.free_from / 60 : 16, wd_to = had ? std::max(0, r.free_to / 60 - 1) : 18;
            const int we_from = had ? r.free2_from / 60 : 9, we_to = had ? std::max(0, r.free2_to / 60 - 1) : 19;
            brls::sync([this, weak, r, wd_from, wd_to, we_from, we_to]() {
                this->pick_span("playguard/forwarders/hours_weekdays"_i18n, wd_from, wd_to,
                                [this, weak, r, we_from, we_to](int from, int to) {
                    brls::sync([this, weak, r, from, to, we_from, we_to]() {
                        if (weak.expired()) return;
                        this->pick_span("playguard/forwarders/hours_weekend"_i18n, we_from, we_to,
                                        [this, weak, r, from, to](int from2, int to2) {
                            if (weak.expired()) return;
                            this->hours(r, false, { 0x3E, from, to, 0x41, from2, to2 });
                        });
                    });
                });
            });
            return;
        }
        const int days = DAYS[index];
        brls::sync([this, weak, r, days, from_now, to_now, closed]() {
            this->pick_span("", from_now, to_now, [this, weak, r, days, closed](int from, int to) {
                if (weak.expired()) return;
                this->hours(r, closed, { days, from, to });
            });
        });
    });
}

void ForwardersActivity::pick_homebrew()
{
    const std::vector<std::string> found = forwarder_flow::homebrew();
    if (found.empty()) {
        ui::info("playguard/forwarders/pick_none"_i18n);
        return;
    }
    std::vector<std::string> names, paths_shown;
    for (const std::string& p : found) {
        nro::Info info;
        nro::read((paths::sd_root() == "/" ? std::string() : paths::sd_root()) + p, info);
        names.push_back(forwarders_logic::default_name(info.name, p));
        paths_shown.push_back(p);
    }
    std::weak_ptr<bool> weak = this->alive;
    ui::pick("playguard/forwarders/pick_title"_i18n, names, 0, [this, weak, found](int index) {
        if (weak.expired() || index < 0 || (size_t)index >= found.size()) return;
        forwarder_flow::create(found[(size_t)index], this->then_rebuild());
    }, false, paths_shown);
}
