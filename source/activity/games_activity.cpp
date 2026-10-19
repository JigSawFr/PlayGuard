// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/games_activity.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <string>

#include "action/game_icons.hpp"
#include "action/games_logic.hpp"
#include "action/play_data.hpp"
#include "activity/game_activity.hpp"
#include "tab/cadran_list.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"
#include "util/perf.hpp"

using namespace brls::literals;

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

std::string game_name(const GameStat& g)
{
    if (g.name[0]) return g.name;
    return brls::getStr("playguard/activity/deleted_game", fmt::format("{:016X}", (unsigned long long)g.app_id));
}

std::string period_label(int period)
{
    return brls::getStr(fmt::format("playguard/activity/periods/{}", period));
}
}   // namespace

GamesActivity::GamesActivity(int period, int account)
    : period(period), account_index(account)
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
    if (auto* frame = dynamic_cast<brls::AppletFrame*>(this->getContentView()); frame && who)
        frame->setTitle("playguard/library/title"_i18n + " · " + (who->nickname[0] ? std::string(who->nickname) : "?"));
    view_cell->registerClickAction([this](brls::View*) {
        this->switch_view();
        return true;
    });
    period_cell->registerClickAction([this](brls::View*) {
        this->pick_period();
        return true;
    });
    if (cadran::enabled()) {
        // The Cadran look: the game's figures in a card, the view and the
        // period as two rows of their own.
        brls::Theme theme = brls::Application::getTheme();
        auto* head = static_cast<brls::Box*>(this->getView("gs_head"));
        head->setLineBottom(0);
        head->setMargins(16, 60, 0, 60);
        head->setPadding(18, 24, 18, 24);
        head->setCornerRadius(20);
        head->setBackgroundColor(theme["cadran/surf"]);
        head->setBorderColor(theme["cadran/line"]);
        head->setBorderThickness(1);
        cadran::List::row(view_cell, "list", true);
        cadran::List::row(period_cell, "calendar", true);
    }
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
// (once borealis has put the focus back on the cell it came from).
void GamesActivity::onResume()
{
    if (!this->stale) return;
    this->stale = false;
    std::weak_ptr<bool> weak = this->alive;
    brls::sync([this, weak]() {
        if (!weak.expired()) this->rebuild();
    });
}

const PlayAccount* GamesActivity::account() const
{
    const auto& all = play_data::accounts();
    return this->account_index >= 0 && this->account_index < (int)all.size() ? &all[this->account_index] : nullptr;
}

void GamesActivity::set_hints()
{
    // Ⓨ says what it switches to.
    brls::View* frame = this->getContentView();
    frame->registerAction(brls::getStr(fmt::format("playguard/library/views/{}", this->grid ? 0 : 1)), brls::BUTTON_Y,
                          [this](brls::View*) {
                              this->switch_view();
                              return true;
                          });
    frame->registerAction("playguard/activity/period"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->pick_period();
        return true;
    });
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    // The desktop build: the keyboard has no Ⓨ, so the Y key (smoke test). Not in
    // a browser, where the page presses Ⓨ itself for the Y key (web/README.md).
    frame->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Y), [this](brls::View*) {
        this->switch_view();
        return true;
    });
#endif
    view_cell->setDetailText(brls::getStr(fmt::format("playguard/library/views/{}", this->grid ? 1 : 0)));
    period_cell->setDetailText(period_label(this->period));
    period_caption->setText(period_label(this->period));
    brls::Application::getGlobalHintsUpdateEvent()->fire();
}

void GamesActivity::switch_view()
{
    this->grid = !this->grid;
    config::get().games_grid = this->grid;   // the next visit opens on it
    ui::save_config();
    this->set_hints();
    // Next frame: rebuilding deletes the cell whose action runs now.
    const u64 focus_id = this->selected < this->entries.size() ? this->entries[this->selected].app_id : 0;
    std::weak_ptr<bool> weak = this->alive;
    brls::sync([this, weak, focus_id]() {
        if (!weak.expired()) this->build(focus_id);
    });
}

void GamesActivity::pick_period()
{
    std::vector<std::string> labels;
    for (int p = 0; p < 3; p++) labels.push_back(period_label(p));
    ui::pick("playguard/activity/period"_i18n, labels, this->period, [this](int index) {
        if (index == this->period) return;
        this->period = index;
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
        if (auto* cell = dynamic_cast<brls::DetailCell*>(this->more)) cell->setDetailText(std::to_string(this->rows.size()));
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
    for (brls::View* v = brls::Application::getCurrentFocus(); v && !focus_in_list; v = v->getParent())
        focus_in_list = v == list.getView();
    if (focus_in_list) brls::Application::giveFocus(view_cell);
    list->clearViews();
    this->entries.clear();
    this->more = nullptr;
    this->grid_row = nullptr;
    list->setAxis(brls::Axis::COLUMN);
    list->setAlignItems(this->grid ? brls::AlignItems::CENTER : brls::AlignItems::STRETCH);

    // Built out of the screen's layout, laid out once (ui::batch_layout).
    const size_t shown_n = std::min(this->rows.size(), this->cap());
    ui::batch_layout(list, [&]() {
        for (size_t i = 0; i < shown_n; i++) this->add_entry(i);
        if (!this->show_all && shown_n < this->rows.size()) this->add_more();
    });

    // The grid: up and down to the same column (games_logic).
    if (this->grid) {
        std::vector<brls::View*> cells;
        for (const Entry& e : this->entries) cells.push_back(e.view);
        if (this->more) cells.push_back(this->more);
        const int n = (int)cells.size();
        for (int i = 0; i < n; i++) {
            const int up = games_logic::grid_neighbour(i, n, COLUMNS, -1);
            const int down = games_logic::grid_neighbour(i, n, COLUMNS, 1);
            if (up >= 0) cells[i]->setCustomNavigationRoute(brls::FocusDirection::UP, cells[up]);
            else cells[i]->setCustomNavigationRoute(brls::FocusDirection::UP, view_cell.getView());
            if (down >= 0) cells[i]->setCustomNavigationRoute(brls::FocusDirection::DOWN, cells[down]);
        }
    }

    // Applet mode: the games left out, and those the read itself could not
    // hold (PLAYSTATS_MAX).
    const size_t left_out = this->show_all ? this->rows.size() - shown_n : 0;
    std::string cap_text = left_out ? brls::getStr("playguard/activity/applet_cap", (int)left_out) : std::string();
    if (this->data && this->data->truncated)
        cap_text += (cap_text.empty() ? "" : "\n") + brls::getStr("playguard/activity/truncated", PLAYSTATS_MAX);
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
            std::weak_ptr<bool> weak = this->alive;
            const u64 id = this->entries[focus].app_id;
            brls::sync([this, weak, id]() {
                if (weak.expired()) return;
                for (const Entry& e : this->entries)
                    if (e.app_id == id) brls::Application::giveFocus(e.view);
            });
        }
    } else {
        this->show_selected(nullptr);
    }
    brls::Logger::info("games list: {} of {} games ({})", shown_n, this->rows.size(), this->grid ? "grid" : "list");
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
    e.view->getFocusEvent()->subscribe([this, index](brls::View*) { this->select(index); });
    this->entries.push_back(e);
}

size_t GamesActivity::cap() const
{
    if (!this->show_all) return ROWS_SHOWN;
    return game_icons::wanted() ? std::numeric_limits<size_t>::max() : APPLET_ROWS_MAX;
}

// The grid's row the next cover goes in: a new one when the last is full.
brls::Box* GamesActivity::next_row()
{
    if (!this->grid_row || (int)this->grid_row->getChildren().size() >= COLUMNS) {
        this->grid_row = new brls::Box(brls::Axis::ROW);
        this->grid_row->setWidth(COLUMNS * GameTile::WIDTH);
        list->addView(this->grid_row);
    }
    return this->grid_row;
}

// "Show every game": a cell after the list, a cover after the grid's.
void GamesActivity::add_more()
{
    auto on_click = [this](brls::View*) {
        // Next frame: rebuilding deletes this cell, whose action runs now.
        std::weak_ptr<bool> weak = this->alive;
        const size_t first_new = this->entries.size();
        brls::sync([this, weak, first_new]() {
            if (weak.expired()) return;
            this->show_all = true;
            this->build(first_new < this->rows.size() ? this->rows[first_new]->app_id : 0);
        });
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
        auto* cell = new brls::DetailCell();
        cell->setText("playguard/activity/show_all"_i18n);
        cell->setDetailText(std::to_string(this->rows.size()));
        cell->registerClickAction(on_click);
        list->addView(cell);
        this->more = cell;
    }
}

void GamesActivity::fill(Entry& e, const GameStat& g) const
{
    const std::string time = ui::fmt_play_time(activity_summary::value(g, this->period));
    if (e.tile) {
        e.tile->set_name(game_name(g));
        e.tile->set_detail(time);
    } else {
        e.cell->setText(game_name(g));
        e.cell->setDetailText(time);
    }
    const GameStat copy = g;
    const std::shared_ptr<const PlayStats> stats = this->data;
    const bool every = this->account_index < 0;
    e.view->registerClickAction([copy, stats, every](brls::View*) {
        // Its own screen: the seven days as bars, the figures, each account.
        std::vector<unsigned char> bytes;
        if (const auto* b = game_icons::bytes(copy.app_id)) bytes = *b;
        brls::Application::pushActivity(new GameActivity(copy, stats, std::move(bytes), every));
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
    const std::string na = "playguard/common/unavailable"_i18n;
    icon->clear();
    if (!g) {
        name->setText(" ");
        period_value->setText("—");
        total->setText("—");
        launches->setText("—");
        last->setText("—");
        return;
    }
    name->setText(game_name(*g));
    const bool windows = this->data && this->data->windows_ok;
    const bool known = this->period == 2 ? g->totals_ok : windows;
    period_value->setText(known ? ui::fmt_play_time(activity_summary::value(*g, this->period)) : na);
    total->setText(g->totals_ok ? ui::fmt_play_time(g->total_s) : na);
    launches->setText(g->totals_ok ? std::to_string(g->launches) : na);
    last->setText(g->totals_ok && g->last_played ? ui::time_text(g->last_played) : "—");
    bool owned = false;
    if (const int tex = game_icons::texture(g->app_id, owned)) {
        // As GameCell: the flag set once the old one is replaced.
        icon->setScalingType(brls::ImageScalingType::FIT);
        icon->innerSetImage(tex);
        icon->setFreeTexture(owned);
    } else if (const int waiting = game_icons::placeholder()) {
        // Until it arrives, or for a game without one, as the covers.
        icon->setScalingType(brls::ImageScalingType::CENTER);
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
    } else if (this->period != 2 && !this->data->windows_ok) {
        text = "playguard/activity/err_log"_i18n + " — " + ui::rc_text(this->data->events_rc);
    } else if (this->period == 2 && R_FAILED(this->data->stats_rc)) {
        text = "playguard/activity/err_stats"_i18n + " — " + ui::rc_text(this->data->stats_rc);
    } else if (this->rows.empty()) {
        text = brls::getStr(fmt::format("playguard/activity/none/{}", this->period));
        error = false;
    }
    status->setText(text);
    status->setTextColor(error ? ui::color_warn() : ui::color_note());
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
    std::weak_ptr<bool> weak = this->alive;
    game_icons::load(ids, [this, weak]() {
        if (weak.expired()) return;
        this->apply_icons();
        if (this->selected < this->rows.size()) this->show_selected(this->rows[this->selected]);
        this->load_icons(this->selected);
    });
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
