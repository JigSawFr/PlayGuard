// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/games_activity.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <string>

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/game_icons.hpp"
#include "action/games_logic.hpp"
#include "action/play_data.hpp"
#include "activity/game_activity.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace aurore::literals;

namespace
{
// Games built at first: each is a cell or a cover, and a large library holds
// hundreds. "Show every game" builds the rest on demand.
constexpr size_t ROWS_SHOWN = 50;

// Games "Show every game" builds at most in applet mode (opened from the
// album, a small heap): about 50 KB each.
constexpr size_t APPLET_ROWS_MAX = 100;

// Covers a row: as many as the screen's width takes (1280 less 60 a side).
constexpr int COLUMNS = (int)((1280 - 2 * 60) / GameTile::WIDTH);

// Icons read around the selected game: a few rows behind, more ahead.
constexpr size_t ICONS_BEHIND = 12;
constexpr size_t ICONS_AHEAD = 30;

std::string period_label(int period)
{
    return aurore::getStr(fmt::format("playguard/activity/periods/{}", period));
}
}   // namespace

GamesActivity::GamesActivity(int shown_period, int account)
    : period(shown_period), account_index(account)
{
}

GamesActivity::~GamesActivity()
{
    play_data::unlisten(this->listener);
}

void GamesActivity::onContentAvailable()
{
    status->setSingleLine(false);
    capped->setSingleLine(false);
    this->grid = config::get().games_grid;
    const PlayAccount* who = this->account();
    if (auto* frame = dynamic_cast<aurore::AppletFrame*>(this->getContentView()); frame && who)
        frame->setTitle("playguard/library/title"_i18n + " · " + (who->nickname[0] ? std::string(who->nickname) : "?"));
    view_cell->registerClickAction([this](aurore::View*) {
        this->switch_view();
        return true;
    });
    period_cell->registerClickAction([this](aurore::View*) {
        this->pick_period();
        return true;
    });
    // The game's figures in a card, the view and the period as two rows of
    // their own.
    auto* head = static_cast<aurore::Box*>(this->getView("gs_head"));
    head->setLineBottom(0);
    head->setMargins(16, 60, 0, 60);
    head->setPadding(18, 24, 18, 24);
    head->setCornerRadius(cadran::kit::RADIUS_CARD);
    head->setThemeColor("backgroundColor", "cadran/surf");
    head->setThemeColor("borderColor", "cadran/line");
    head->setBorderThickness(1);
    cadran::List::row(view_cell, "list", true);
    cadran::List::row(period_cell, "calendar", true);
    this->set_hints();
    this->listener = play_data::listen([this]() {
        if (!ui::on_top(this)) {
            this->stale = true;
            return;
        }
        this->rebuild();
    });
    ui::set_visible(progress.getView(), false);
    this->rebuild();
    // The screen opens on the first game (the list, after the view and the
    // period), when there is one.
    const auto& children = content->getChildren();
    const auto at = std::find(children.begin(), children.end(), list.getView());
    if (!this->entries.empty() && at != children.end()) content->setDefaultFocusedIndex((int)(at - children.begin()));
}

// Back from a game's screen or a dialog: what was read meanwhile, next frame
// (once Aurore has put the focus back on the cell it came from).
void GamesActivity::onResume()
{
    if (!this->stale) return;
    this->stale = false;
    aurore::sync(ui::guarded(this->alive, [this]() { this->rebuild(); }));
}

const PlayAccount* GamesActivity::account() const
{
    const auto& all = play_data::accounts();
    return this->account_index >= 0 && this->account_index < (int)all.size() ? &all[this->account_index] : nullptr;
}

void GamesActivity::set_hints()
{
    // Ⓨ says what it switches to.
    aurore::View* frame = this->getContentView();
    frame->registerAction(aurore::getStr(fmt::format("playguard/library/views/{}", this->grid ? 0 : 1)), aurore::BUTTON_Y,
                          [this](aurore::View*) {
                              this->switch_view();
                              return true;
                          });
    frame->registerAction("playguard/activity/period"_i18n, aurore::BUTTON_X, [this](aurore::View*) {
        this->pick_period();
        return true;
    });
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    // The desktop build: the keyboard has no Ⓨ, so the Y key (smoke test). Not in
    // a browser, where the page presses Ⓨ itself for the Y key (web/README.md).
    frame->registerAction(aurore::AuroreKeyCombination(aurore::AURORE_KBD_KEY_Y), [this](aurore::View*) {
        this->switch_view();
        return true;
    });
#endif
    view_cell->setDetailText(aurore::getStr(fmt::format("playguard/library/views/{}", this->grid ? 1 : 0)));
    period_cell->setDetailText(period_label(this->period));
    period_caption->setText(period_label(this->period));
    aurore::Application::getGlobalHintsUpdateEvent()->fire();
}

void GamesActivity::switch_view()
{
    this->grid = !this->grid;
    config::get().games_grid = this->grid;   // the next visit opens on it
    ui::save_config();
    // Next frame: rebuilding deletes the cell whose action runs now, and the
    // hints register the frame's actions again, this one among them.
    const u64 focus_id = this->selected < this->entries.size() ? this->entries[this->selected].app_id : 0;
    aurore::sync(ui::guarded(this->alive, [this, focus_id]() {
        this->set_hints();
        this->build(focus_id);
    }));
}

void GamesActivity::pick_period()
{
    std::vector<std::string> labels;
    int current = 0;
    for (int i = 0; i < activity_summary::PERIOD_COUNT; i++) {
        labels.push_back(period_label(activity_summary::PERIODS[i]));
        if (activity_summary::PERIODS[i] == this->period) current = i;
    }
    ui::pick("playguard/activity/period"_i18n, labels, current, [this](int index) {
        if (activity_summary::PERIODS[index] == this->period) return;
        this->period = activity_summary::PERIODS[index];
        this->show_all = false;
        config::get().activity_period = index;   // the Activity tab's too
        ui::save_config();
        this->set_hints();
        this->rebuild();
    });
}

void GamesActivity::rebuild()
{
    const play_data::Key key = play_data::key_of(this->account());
    const bool reading = play_data::busy(key);
    this->data = play_data::latest(key);
    std::vector<const GameStat*> now;
    if (this->data) {
        for (uint32_t i = 0; i < this->data->count; i++)
            if (activity_summary::value(this->data->games[i], this->period) > 0) now.push_back(&this->data->games[i]);
        activity_summary::sort_by_play(now, this->period, false);
    }
    ui::set_visible(progress.getView(), reading && now.empty());

    // The same games in the same order (a read in the background): only
    // their figures change, no cell is rebuilt and the focus stays.
    const size_t shown_n = std::min(now.size(), this->cap());
    const bool more_now = !this->show_all && shown_n < now.size();
    bool same = !this->entries.empty() && this->entries.size() == shown_n && (this->more != nullptr) == more_now;
    for (size_t i = 0; same && i < this->entries.size(); i++) same = this->entries[i].app_id == now[i]->app_id;
    this->rows = std::move(now);
    if (same) {
        for (size_t i = 0; i < this->entries.size(); i++) this->fill(this->entries[i], *this->rows[i]);
        if (auto* cell = dynamic_cast<aurore::DetailCell*>(this->more)) cell->setDetailText(std::to_string(this->rows.size()));
        if (auto* tile = dynamic_cast<GameTile*>(this->more)) tile->set_detail(std::to_string(this->rows.size()));
        this->select(std::min(this->selected, this->entries.size() - 1));
    } else {
        const u64 focus_id = this->selected < this->entries.size() ? this->entries[this->selected].app_id : 0;
        this->build(focus_id);
    }
    this->show_status();
}

void GamesActivity::build(u64 focus_id)
{
    perf::Timed timed("All games list");   // the diagnostic report's timings
    // The cells are about to be deleted: never leave the focus on one.
    bool focus_in_list = false;
    for (aurore::View* v = aurore::Application::getCurrentFocus(); v && !focus_in_list; v = v->getParent())
        focus_in_list = v == list.getView();
    if (focus_in_list) aurore::Application::giveFocus(view_cell);
    list->clearViews();
    this->entries.clear();
    this->more = nullptr;
    this->grid_row = nullptr;
    list->setAxis(aurore::Axis::COLUMN);
    list->setAlignItems(this->grid ? aurore::AlignItems::CENTER : aurore::AlignItems::STRETCH);

    // Built out of the screen's layout, laid out once (ui::batch_layout).
    const size_t shown_n = std::min(this->rows.size(), this->cap());
    ui::batch_layout(list, [&]() {
        for (size_t i = 0; i < shown_n; i++) this->add_entry(i);
        if (!this->show_all && shown_n < this->rows.size()) this->add_more();
    });

    // The grid: up and down to the same column (games_logic).
    if (this->grid) {
        std::vector<aurore::View*> cells;
        for (const Entry& e : this->entries) cells.push_back(e.view);
        if (this->more) cells.push_back(this->more);
        const int n = (int)cells.size();
        for (int i = 0; i < n; i++) {
            const int up = games_logic::grid_neighbour(i, n, COLUMNS, -1);
            const int down = games_logic::grid_neighbour(i, n, COLUMNS, 1);
            if (up >= 0) cells[i]->setCustomNavigationRoute(aurore::FocusDirection::UP, cells[up]);
            else cells[i]->setCustomNavigationRoute(aurore::FocusDirection::UP, view_cell.getView());
            if (down >= 0) cells[i]->setCustomNavigationRoute(aurore::FocusDirection::DOWN, cells[down]);
        }
    }

    // Applet mode: the games left out, and those the read itself could not
    // hold (PLAYSTATS_MAX).
    const size_t left_out = this->show_all ? this->rows.size() - shown_n : 0;
    std::string cap_text = left_out ? aurore::getStr("playguard/activity/applet_cap", (int)left_out) : std::string();
    if (this->data && this->data->truncated)
        cap_text += (cap_text.empty() ? "" : "\n") + aurore::getStr("playguard/activity/truncated", PLAYSTATS_MAX);
    capped->setText(cap_text);
    ui::set_visible(capped, !cap_text.empty());

    size_t focus = 0;
    for (size_t i = 0; i < this->entries.size(); i++)
        if (this->entries[i].app_id == focus_id) {
            focus = i;
            break;
        }
    this->selected = focus;
    if (!this->entries.empty()) {
        this->select(focus);
        // Back on the same game (else the first), next frame, once laid out,
        // so the list scrolls to it.
        if (focus_in_list) {
            const u64 id = this->entries[focus].app_id;
            aurore::sync(ui::guarded(this->alive, [this, id]() {
                for (const Entry& e : this->entries)
                    if (e.app_id == id) aurore::Application::giveFocus(e.view);
            }));
        }
    } else {
        this->show_selected(nullptr);
    }
    aurore::Logger::info("games list: {} of {} games ({})", shown_n, this->rows.size(), this->grid ? "grid" : "list");
}

void GamesActivity::add_entry(size_t i)
{
    const GameStat& g = *this->rows[i];
    Entry e{ g.app_id, nullptr, nullptr, nullptr, false };
    if (this->grid) {
        e.tile = new GameTile();
        e.view = e.tile;
        this->next_row()->addView(e.tile);
    } else {
        e.cell = new GameCell(game_icons::wanted());
        e.view = e.cell;
        list->addView(e.cell);
    }
    this->fill(e, g);
    const size_t index = this->entries.size();
    e.view->getFocusEvent()->subscribe([this, index](aurore::View*) { this->select(index); });
    this->entries.push_back(e);
}

size_t GamesActivity::cap() const
{
    if (!this->show_all) return ROWS_SHOWN;
    return game_icons::wanted() ? std::numeric_limits<size_t>::max() : APPLET_ROWS_MAX;
}

// The grid's row the next cover goes in: a new one when the last is full.
aurore::Box* GamesActivity::next_row()
{
    if (!this->grid_row || (int)this->grid_row->getChildren().size() >= COLUMNS) {
        this->grid_row = new aurore::Box(aurore::Axis::ROW);
        this->grid_row->setWidth(COLUMNS * GameTile::WIDTH);
        list->addView(this->grid_row);
    }
    return this->grid_row;
}

// "Show every game": a cell after the list, a cover after the grid's.
void GamesActivity::add_more()
{
    auto on_click = [this](aurore::View*) {
        // Next frame: rebuilding deletes this cell, whose action runs now.
        const size_t first_new = this->entries.size();
        aurore::sync(ui::guarded(this->alive, [this, first_new]() {
            this->show_all = true;
            this->build(first_new < this->rows.size() ? this->rows[first_new]->app_id : 0);
        }));
        return true;
    };
    if (this->grid) {
        auto* tile = new GameTile();
        tile->set_name("playguard/activity/show_all"_i18n);
        tile->set_detail(std::to_string(this->rows.size()));
        tile->registerClickAction(on_click);
        this->next_row()->addView(tile);
        this->more = tile;
    } else {
        auto* cell = new aurore::DetailCell();
        cell->setText("playguard/activity/show_all"_i18n);
        cell->setDetailText(std::to_string(this->rows.size()));
        cell->registerClickAction(on_click);
        list->addView(cell);
        this->more = cell;
    }
}

void GamesActivity::fill(Entry& e, const GameStat& g) const
{
    const std::string time = ui::fmt_duration(activity_summary::value(g, this->period));
    if (e.tile) {
        e.tile->set_name(play_data::game_name(g));
        e.tile->set_detail(time);
    } else {
        e.cell->setText(play_data::game_name(g));
        e.cell->setDetailText(time);
    }
    const GameStat copy = g;
    const std::shared_ptr<const PlayStats> stats = this->data;
    const bool every = this->account_index < 0;
    e.view->registerClickAction([copy, stats, every](aurore::View*) {
        // Its own screen: the seven days as bars, the figures, each account.
        std::vector<unsigned char> bytes;
        if (const auto* b = game_icons::bytes(copy.app_id)) bytes = *b;
        aurore::Application::pushActivity(new GameActivity(copy, stats, std::move(bytes), every));
        return true;
    });
}

void GamesActivity::select(size_t i)
{
    if (i >= this->entries.size() || i >= this->rows.size()) return;
    this->selected = i;
    this->show_selected(this->rows[i]);
    this->load_icons(i);
}

void GamesActivity::show_selected(const GameStat* g)
{
    const std::string na = "cadran/common/unavailable"_i18n;
    icon->clear();
    if (!g) {
        name->setText(" ");
        period_value->setText("—");
        total->setText("—");
        launches->setText("—");
        last->setText("—");
        return;
    }
    name->setText(play_data::game_name(*g));
    const bool known = this->period == activity_summary::AllTime ? g->totals_ok
                                                                  : this->data && activity_summary::period_read(*this->data, this->period);
    period_value->setText(known ? ui::fmt_duration(activity_summary::value(*g, this->period)) : na);
    total->setText(g->totals_ok ? ui::fmt_duration(g->total_s) : na);
    launches->setText(g->totals_ok ? std::to_string(g->launches) : na);
    last->setText(g->totals_ok && g->last_played ? ui::time_text(g->last_played) : "—");
    bool owned = false;
    if (const int tex = game_icons::texture(g->app_id, owned)) {
        // As GameCell: the flag set once the old one is replaced.
        icon->setScalingType(aurore::ImageScalingType::FIT);
        icon->innerSetImage(tex);
        icon->setFreeTexture(owned);
    } else if (const int waiting = game_icons::placeholder()) {
        // Until it arrives, or for a game without one, as the covers.
        icon->setScalingType(aurore::ImageScalingType::CENTER);
        icon->innerSetImage(waiting);
        icon->setFreeTexture(false);
    }
}

void GamesActivity::show_status()
{
    std::string text;
    bool error = true;
    if (!this->data) {
        text = "";
    } else if (R_FAILED(this->data->rc)) {
        text = "playguard/activity/err_list"_i18n + " — " + ui::rc_text(this->data->rc);
    } else if (this->period != activity_summary::AllTime && !activity_summary::period_read(*this->data, this->period)) {
        text = "playguard/activity/err_log"_i18n + " — " + ui::rc_text(this->data->events_rc);
    } else if (this->period == activity_summary::AllTime && R_FAILED(this->data->stats_rc)) {
        text = "playguard/activity/err_stats"_i18n + " — " + ui::rc_text(this->data->stats_rc);
    } else if (this->rows.empty()) {
        text = aurore::getStr(fmt::format("playguard/activity/none/{}", this->period));
        error = false;
    }
    status->setText(text);
    status->setThemeColor("textColor", error ? "cadran/amber" : "cadran/text3");
    ui::set_visible(status, !text.empty());
}

void GamesActivity::load_icons(size_t i)
{
    if (!game_icons::wanted()) return;
    this->apply_icons();
    const auto window = games_logic::icon_window(i, this->entries.size(), ICONS_BEHIND, ICONS_AHEAD);
    std::vector<u64> ids;
    for (size_t k = window.first; k < window.second; k++)
        if (!game_icons::known(this->entries[k].app_id)) ids.push_back(this->entries[k].app_id);
    if (ids.empty()) return;
    game_icons::load(ids, ui::guarded(this->alive, [this]() {
        this->apply_icons();
        if (this->selected < this->rows.size()) this->show_selected(this->rows[this->selected]);
        this->load_icons(this->selected);
    }));
}

void GamesActivity::apply_icons()
{
    if (this->entries.empty()) return;
    // Those near the selected game only: the others decode theirs when the
    // selection comes near.
    const auto window = games_logic::icon_window(this->selected, this->entries.size(), ICONS_BEHIND, ICONS_AHEAD);
    for (size_t k = window.first; k < window.second; k++) {
        Entry& e = this->entries[k];
        if (e.iconed) continue;
        bool owned = false;
        const int tex = game_icons::texture(e.app_id, owned);
        if (!tex) continue;
        if (e.tile) e.tile->set_icon_texture(tex, owned);
        else e.cell->set_icon_texture(tex, owned);
        e.iconed = true;
    }
}
