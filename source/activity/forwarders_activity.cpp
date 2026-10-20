// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/forwarders_activity.hpp"

#include <algorithm>

#include "action/forwarder_flow.hpp"
#include "action/game_icons.hpp"
#include "action/hbmenu_lock.hpp"
#include "app.hpp"
#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
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

// Its state, and whether it asks for the PIN (and when it does not).
std::string state_text(const Row& r)
{
    if (!r.pin) return state_only(r);
    if (!r.free_days) return state_only(r) + " · " + "playguard/forwarders/state_pin"_i18n;
    return state_only(r) + " · " +
           brls::getStr("playguard/forwarders/state_pin_hours", forwarder_flow::hours_label(r.free_days, r.free_from, r.free_to));
}

}   // namespace

void ForwardersActivity::onContentAvailable()
{
    note->setSingleLine(false);
    warning->setSingleLine(false);
    empty->setSingleLine(false);
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
        const auto after = [this, weak](bool) {
            brls::sync([this, weak]() {
                if (!weak.expired()) this->show_lock();
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
    lock_check->setDetailText(ui::opens());
    lock_check->registerClickAction([](brls::View*) {
        hbmenu_lock::show_check();
        return true;
    });
    ui::show_writable(reinstall, !app::read_only());
    ui::show_writable(protect_all, !app::read_only());
    cadran::List(this->getContentView(), true)
        .apply({ { "fw_self", "home" }, { "fw_other", "plus" }, { "fw_reinstall", "refresh" }, { "fw_lock", "lock" },
                 { "fw_protect_all", "shield" }, { "fw_lock_check", "eye" } });
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
    } else {
        this->show_patches();
    }
    this->rebuild();
}

void ForwardersActivity::show_lock()
{
    using ui::Check;
    const hbmenu_lock::Summary s = hbmenu_lock::summary();
    const bool on = s.locked;
    lock_cell->setDetailText(on ? "playguard/hbmenu/state_locked"_i18n : "playguard/hbmenu/state_open"_i18n);
    ui::show_writable(lock_cell, !app::read_only(), ui::color_text(), on ? ui::color_ok() : ui::color_neutral());
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
        lines.push_back({ Check::Warn, brls::getStr("playguard/hbmenu/open_doors", doors) });
    }
    lines.push_back({ Check::Info, s.module ? "playguard/hbmenu/summary_module"_i18n : "playguard/hbmenu/summary_no_module"_i18n });
    ui::batch_layout(summary.getView(), [this, &lines, on]() {
        summary->clearViews();
        brls::Box* card = cadran::kit::top_card(false);
        card->addView(cadran::kit::kicker(on ? "lock" : "unlock", "playguard/hbmenu/section"_i18n));
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
    ui::show_writable(self_cell, on || !app::read_only(), ui::color_text(),
                      on && own->state == State::Ready ? ui::color_ok() : ui::color_neutral());
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

void ForwardersActivity::rebuild(uint64_t focus_id)
{
    // The focus to a cell that stays before the old rows go.
    brls::Application::giveFocus(self_cell);
    brls::Box* rows = cadran::List::rows_in(list);
    rows->clearViews();
    this->cells.clear();
    this->list_rows = forwarder_flow::rows();
    brls::View* focus = nullptr;
    for (const Row& r : this->list_rows) {
        auto* cell = new GameCell(game_icons::wanted());
        cell->setText(r.name);
        cell->setDetailText(state_text(r));
        cell->setDetailTextColor(r.state == State::Ready ? ui::color_ok()
                                 : r.state == State::Elsewhere ? ui::color_neutral()
                                                               : ui::color_warn());
        const Row copy = r;
        cell->registerClickAction([this, copy](brls::View*) {
            this->actions(copy);
            return true;
        });
        rows->addView(cell);
        cadran::List::row(cell);
        if (r.state != State::NotInstalled) this->cells.push_back({ r.id, cell });
        if (r.id == focus_id) focus = cell;
    }
    ui::set_visible(list.getView(), !this->list_rows.empty());
    ui::set_visible(empty.getView(), this->list_rows.empty());
    this->show_self();
    this->show_lock();
    this->load_icons();
    // The forwarder asked for, else the first one, else PlayGuard's line.
    if (!focus && !this->list_rows.empty() && !focus_id) focus = rows->getChildren().front();
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
        if (!r.self && r.pin)
            choices.push_back({ "playguard/forwarders/action_hours"_i18n, [this, r]() { this->pick_hours(r); } });
        choices.push_back({ "playguard/forwarders/action_rebuild"_i18n,
                            [this, r]() { forwarder_flow::rebuild(r, "", "", this->then_rebuild(r.id)); } });
    }
    if (r.state == State::NotInstalled) {
        choices.push_back({ "playguard/forwarders/action_rebuild"_i18n,
                            [this, r]() { forwarder_flow::rebuild(r, "", "", this->then_rebuild(r.id)); } });
        choices.push_back({ "playguard/forwarders/action_forget"_i18n,
                            [this, r]() { forwarder_flow::forget(r, this->then_rebuild()); } });
    } else {
        choices.push_back({ "playguard/common/delete"_i18n, [this, r]() { forwarder_flow::remove(r, this->then_rebuild()); } });
    }
    ui::pick_actions(r.name, choices);
}

void ForwardersActivity::details(const Row& r)
{
    const std::string unknown = "playguard/forwarders/unknown"_i18n;
    const std::string nro = r.nro.empty() ? unknown : nro::sd_path(r.nro);
    const std::string made = r.state == State::Elsewhere ? "playguard/forwarders/state_elsewhere"_i18n
                                                         : "playguard/forwarders/made_here"_i18n;
    ui::info(brls::getStr("playguard/forwarders/details", r.name, r.author.empty() ? unknown : r.author,
                          r.version.empty() ? unknown : r.version, nro, forwarders::hex_id(r.id),
                          r.state == State::Elsewhere ? made : made + " · " + state_text(r)));
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

void ForwardersActivity::pick_hours(const Row& row)
{
    const Row r = row;
    // Never, every day, the school days, the weekend.
    static const int DAYS[] = { 0, 0x7F, 0x3E, 0x41 };
    const std::vector<std::string> names = { "playguard/forwarders/hours_never"_i18n, "playguard/forwarders/hours_every_day"_i18n,
                                             "playguard/forwarders/hours_weekdays"_i18n, "playguard/forwarders/hours_weekend"_i18n };
    int current = 0;
    for (int i = 1; i < 4; i++)
        if (r.free_days == DAYS[i]) current = i;
    std::weak_ptr<bool> weak = this->alive;
    ui::pick("playguard/forwarders/hours_title"_i18n, names, current, [this, weak, r](int index) {
        if (weak.expired() || index < 0 || index > 3) return;
        if (index == 0) {
            forwarder_flow::set_hours(r, 0, 0, 0, this->then_rebuild(r.id));
            return;
        }
        const int days = DAYS[index];
        std::vector<std::string> hours;
        for (int h = 0; h < 24; h++) hours.push_back(fmt::format("{:02}:00", h));
        const int from_now = r.free_days ? r.free_from / 60 : 9;
        brls::sync([this, weak, r, days, hours, from_now]() {
            ui::pick("playguard/forwarders/hours_from"_i18n, hours, from_now, [this, weak, r, days](int from) {
                if (weak.expired() || from < 0 || from > 23) return;
                std::vector<std::string> ends;
                for (int h = 1; h <= 24; h++) ends.push_back(fmt::format("{:02}:00", h));
                const int to_now = r.free_days ? std::max(0, r.free_to / 60 - 1) : 19;
                brls::sync([this, weak, r, days, from, ends, to_now]() {
                    ui::pick("playguard/forwarders/hours_to"_i18n, ends, to_now, [this, weak, r, days, from](int to) {
                        if (weak.expired() || to < 0 || to > 23) return;
                        const int end = (to + 1) * 60, start = from * 60;
                        if (end == start) return;
                        // An end before the start runs past midnight.
                        forwarder_flow::set_hours(r, days, start, end, this->then_rebuild(r.id));
                    });
                });
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
