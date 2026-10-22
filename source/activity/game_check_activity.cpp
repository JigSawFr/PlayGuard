// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/game_check_activity.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fmt/format.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "action/game_check_text.hpp"
#include "action/game_icons.hpp"
#include "action/game_notice.hpp"
#include "action/game_scan.hpp"
#include "action/game_verify_flow.hpp"
#include "action/history_flow.hpp"
#include "action/games_logic.hpp"
#include "action/upload_flow.hpp"
#include "app.hpp"
#include "tab/cadran_list.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/perf.hpp"
#include "util/table_export.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "view/game_tile.hpp"
#include "activity/game_check_parts.hpp"

using namespace brls::literals;
using namespace game_check_parts;

void GameCheckActivity::onContentAvailable()
{
    note->setSingleLine(false);
    status->setSingleLine(false);
    hint->setSingleLine(false);
    // A list or covers, as All games: Ⓡ, out of the way of the games.
    this->grid = config::get().check_grid;
    this->view_action = this->getContentView()->registerAction(view_label(!this->grid), brls::BUTTON_RB, [this](brls::View*) {
        this->switch_view();
        return true;
    });
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    // The desktop build: the keyboard has no Ⓡ nor Ⓨ, so the R and Y keys
    // (smoke test), Y for the game with the focus.
    this->getContentView()->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_R), [this](brls::View*) {
        this->switch_view();
        return true;
    });
    this->getContentView()->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Y), [this](brls::View*) {
        if (const u64 id = this->focused_game()) this->toggle(id);
        return true;
    });
#endif
    // The games picked with Ⓨ: how many, and A for what to do with them.
    this->selection = new brls::DetailCell();
    this->selection->setMarginBottom(6);
    this->selection->setDetailText("playguard/games/actions"_i18n);
    this->selection->registerClickAction([this](brls::View*) {
        this->selection_menu();
        return true;
    });
    list->addView(this->selection);
    ui::set_visible(this->selection, false);
    cadran::List::row(this->selection, "check", true);
    // No game with a problem: a card instead of the line above the list.
    this->all_clear = cadran::kit::empty("check");
    this->all_clear.box->setMarginTop(6);
    this->check_again = cadran::kit::chip("playguard/games/check_again"_i18n, 44);
    this->check_again->setMarginTop(18);
    this->check_again->registerClickAction([this](brls::View*) {
        this->refresh(true, 0);
        return true;
    });
    this->all_clear.box->addView(this->check_again);
    list->addView(this->all_clear.box);
    ui::set_visible(this->all_clear.box, false);
    // A section per way out, under its title and what to do; each hidden
    // while it has no game.
    for (GameCategory category : GROUPS) this->build_group(category);
    // Ⓧ: every game checked again, whatever the cache says.
    this->getContentView()->registerAction("playguard/games/check_again"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->refresh(true, 0);
        return true;
    });

    // The last check at once, then what changed since.
    if (game_scan::load(&this->cache)) this->show();
    this->refresh(false, 0);
}

void GameCheckActivity::build_group(GameCategory category)
{
    Group& group = this->groups[category];
    group.box = new brls::Box(brls::Axis::COLUMN);
    group.box->setAlignItems(brls::AlignItems::STRETCH);
    group.title = new brls::Header();
    group.title->setMarginBottom(4);
    group.title->setMarginTop(category == GROUPS[0] ? 10 : 20);
    group.box->addView(group.title);
    group.note = cadran::kit::label(brls::getStr(std::string("playguard/games/group_note/") + category_key(category)), 18, "cadran/text3");
    group.note->setMarginBottom(8);
    group.box->addView(group.note);
    cadran::List::section(group.title);
    group.note->setFontSize(15.5f);
    group.note->setTextColor(brls::Application::getTheme()["cadran/text3"]);
    group.note->setMarginLeft(8);
    // The games to install again: their list kept on the SD card, then
    // all of them removed at once (an installer counts them as installed
    // until then). Side by side, above the games, in reach however long
    // the list.
    if (category == GC_CAT_REINSTALL) {
        auto* actions = new brls::Box(brls::Axis::ROW);
        actions->setMarginBottom(12);
        group.save = new brls::Button();
        group.save->setText("playguard/games/save_list"_i18n);
        group.save->setGrow(1.0f);
        group.save->setMarginRight(12);
        group.save->registerClickAction([this, category](brls::View*) {
            this->save_list(this->groups[category].games, [](const std::string& path, const std::string& error) {
                if (path.empty()) list_error(error);
                else ui::notify(brls::getStr("playguard/games/list_saved", path));
            });
            return true;
        });
        actions->addView(group.save);
        // The same list to a secret gist: its link (and QR code) opens it
        // on a phone or a computer, next to an installer.
        group.send = new brls::Button();
        group.send->setText("playguard/games/send_list"_i18n);
        group.send->setGrow(1.0f);
        group.send->setMarginRight(12);
        group.send->registerClickAction([this, category](brls::View*) {
            upload_flow::send_list("games_to_reinstall.txt", list_text(this->groups[category].games), "games to reinstall");
            return true;
        });
        actions->addView(group.send);
        group.remove_all = new brls::Button();
        group.remove_all->setGrow(1.0f);
        group.remove_all->setTextColor(cadran::kit::color("cadran/coral"));
        group.remove_all->registerClickAction([this, category](brls::View*) {
            this->remove_games(this->groups[category].games);
            return true;
        });
        actions->addView(group.remove_all);
        for (brls::Button* button : { group.save, group.send, group.remove_all }) round_button(button);
        group.box->addView(actions);
    }
    // The data to check: every game of the section, one after the
    // other, above the games as the Reinstall actions are.
    if (category == GC_CAT_VERIFY) {
        group.verify_all = new brls::Button();
        group.verify_all->setAlignSelf(brls::AlignSelf::FLEX_START);   // as wide as its text: the covers first
        group.verify_all->setMarginBottom(12);
        group.verify_all->registerClickAction([this, category](brls::View*) {
            this->verify(this->groups[category].games);
            return true;
        });
        round_button(group.verify_all);
        group.box->addView(group.verify_all);
    }
    group.list = new cadran::Card();
    group.list->setAlignItems(this->grid ? brls::AlignItems::CENTER : brls::AlignItems::STRETCH);
    card_look(group.list, this->grid);
    group.box->addView(group.list);
    list->addView(group.box);
    ui::set_visible(group.box, false);
}

void GameCheckActivity::save()
{
    game_scan::save(this->cache);
}

void GameCheckActivity::refresh(bool all, u64 also)
{
    if (this->removing) return;   // read again once every game is removed
    if (this->checking) {
        if (also) this->waiting.push_back(also);   // after the check under way
        return;
    }
    this->checking = true;
    this->checking_text("playguard/games/searching"_i18n);
    std::weak_ptr<bool> weak = this->alive;
    const auto list = [this, weak, all, also]() {
        auto records = std::make_shared<std::vector<GameRecord>>(GAMECHECK_MAX);
        u32 count = 0;
        bool truncated = false;
        const Result rc = gamecheck_records(records->data(), GAMECHECK_MAX, &count, &truncated);
        records->resize(count);
        if (ui::quitting()) return;
        brls::sync([this, weak, all, also, records, rc, truncated]() {
            if (weak.expired()) return;   // the screen was closed meanwhile
            if (R_FAILED(rc)) {
                this->checking = false;
                this->list_rc = rc;
                this->show();
                return;
            }
            this->list_rc = 0;
            this->cache.truncated = truncated;
            SysInfo si;
            sysinfo_get(&si);
            u64 now = 0;
            time_local_now(&now, nullptr);
            const bool full = all || game_cache::expired(this->cache, (int64_t)now, si.hos_version);
            std::vector<GameRecord> check = game_cache::to_check(&this->cache, *records, (int64_t)now, si.hos_version);
            if (all) check = *records;
            // Asked for besides (a game just repaired: its entry may not say so).
            std::vector<u64> extra = this->waiting;
            this->waiting.clear();
            if (also) extra.push_back(also);
            for (u64 id : extra) {
                bool listed = false;
                for (const GameRecord& c : check) listed |= c.app_id == id;
                if (listed) continue;
                for (const GameRecord& r : *records)
                    if (r.app_id == id) check.push_back(r);
            }
            this->show();
            if (check.empty()) {
                this->checking = false;
                this->save();   // the games no longer installed are gone
                this->show();
                return;
            }
            this->check(std::move(check), full);
        });
    };
    // After the start-up check when it is still under way (game_scan): what
    // it checked first.
    game_scan::when_free([this, weak, list](bool waited) {
        if (weak.expired()) return;
        if (waited && game_scan::load(&this->cache)) this->show();
        ui::in_background("games list", list);
    });
}

namespace
{
// What the checking thread shares with the screen.
struct CheckRun
{
    std::weak_ptr<bool> alive;
    std::vector<GameRecord> records;
    std::vector<GameFacts> facts;
    std::function<void(const GameFacts&, u32)> on_game;   // run on the main thread
};

bool on_checked(void* ctx, const GameFacts* game, u32 done)
{
    auto* run = (CheckRun*)ctx;
    if (run->alive.expired() || ui::quitting()) return false;   // closed: stop
    const GameFacts copy = *game;
    auto weak = run->alive;
    auto on_game = run->on_game;
    brls::sync([weak, on_game, copy, done]() {
        if (!weak.expired()) on_game(copy, done);
    });
    return true;
}
}   // namespace

void GameCheckActivity::check(std::vector<GameRecord> records, bool full)
{
    this->check_total = (u32)records.size();
    this->check_done = 0;
    this->show();
    auto run = std::make_shared<CheckRun>();
    run->alive = this->alive;
    run->records = std::move(records);
    run->facts.resize(run->records.size());
    // The list is built again only when a game's line changes (a few in a
    // whole check); otherwise only the count moves.
    run->on_game = [this](const GameFacts& checked, u32 done) {
        GameFacts game = checked;
        game_cache::carry_verified(this->cache, &game);   // what the scan never says
        const bool redraw = this->changes_list(game);
        game_cache::put(&this->cache, game);
        this->check_done = done;
        if (redraw) this->show();
        else this->checking_text(brls::getStr("playguard/games/checking", (int)this->check_done, (int)this->check_total));
    };
    std::weak_ptr<bool> weak = this->alive;
    ui::in_background("games check", [this, weak, run, full]() {
        const u32 done = gamecheck_check(run->records.data(), (u32)run->records.size(), run->facts.data(), on_checked, run.get());
        const bool complete = done == run->records.size();
        GameCheckTiming t;
        gamecheck_last_timing(&t);
        const auto ms = [](u64 ns) { return (unsigned long long)(ns / 1000000ULL); };
        const std::string timing = fmt::format("{} games in {} ms: setup {}, control {}, contents {}, state {}", t.games,
                                               ms(t.total_ns), ms(t.setup_ns), ms(t.control_ns), ms(t.contents_ns), ms(t.state_ns));
        brls::Logger::info("games check: {}", timing);
        perf::record("Installed games check", (double)t.total_ns / 1e6);
        if (ui::quitting()) return;
        brls::sync([this, weak, complete, full, timing]() {
            if (weak.expired()) return;
            this->checking = false;
            this->timing = timing;
            this->check_total = 0;
            if (complete && full) {
                SysInfo si;
                sysinfo_get(&si);
                u64 now = 0;
                time_local_now(&now, nullptr);
                this->cache.checked_at = (int64_t)now;
                this->cache.hos = si.hos_version;
            }
            this->save();
            // A game asked for meanwhile (a repair during the check): checked
            // before anything is said about it.
            if (!this->waiting.empty()) {
                this->refresh(false, 0);
                return;
            }
            this->show();
        });
    });
}

void GameCheckActivity::status_text(const std::string& text, NVGcolor color)
{
    status->setText(text);
    status->setTextColor(color);
    ui::set_visible(status.getView(), !text.empty());
}

void GameCheckActivity::checking_text(const std::string& text)
{
    if (this->all_clear.box->getVisibility() != brls::Visibility::VISIBLE) {
        this->status_text(text);
        return;
    }
    this->all_clear.title->setText(text);
    this->all_clear.text->setText("");
    this->status_text("");
}

void GameCheckActivity::status_text(const std::string& text)
{
    this->status_text(text, cadran::kit::color("cadran/text"));
}

void GameCheckActivity::show()
{
    SysInfo si;
    sysinfo_get(&si);
    const u32 hos = si.hos_version;
    const game_check_text::Words w = words();

    // By section, the games with the same problem together, by name.
    const game_check_text::Sorted sorted = game_check_text::sort_broken(this->cache.games, hos, w);
    const std::vector<Broken>& broken = sorted.all;
    const u32 count = (u32)this->cache.games.size();
    // The bell follows every check, repairs included (not halfway through one).
    if (!this->checking && !this->list_rc) game_notice::note(this->cache.games, hos);

    // Two games with the same name (two regions, a demo): told apart by
    // their version, else by their ID.
    std::map<u64, std::string> line_names = game_check_text::line_names(broken, w);

    // The games picked that left the list are no longer picked.
    this->selected = game_check_text::still_selected(this->selected, broken);

    // After a repair, once its game was checked again: whether it left the list.
    if (this->pending.id && !this->checking) {
        bool still = false;
        for (const Broken& b : broken) still |= b.game.app_id == this->pending.id;
        ui::notify(still ? this->pending.done : brls::getStr("playguard/games/fixed", this->pending.name));
        this->pending = Pending{};
    }

    // The cells are reused in place, as in the History screen: one of them
    // may have the focus while the list changes under it.
    for (GameCategory category : GROUPS) {
        const std::vector<Broken>& shown = sorted.by_group[category];
        Group& group = this->groups[category];
        auto& cells = group.list->getChildren();
        group.games.clear();
        if (this->grid) this->show_tiles(group, shown.size());
        for (size_t i = 0; i < shown.size(); i++) {
            const Broken b = shown[i];
            const bool picked = this->selected.count(b.game.app_id) != 0;
            const NVGcolor color = warn_only(b.issue) ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/coral");
            group.games.push_back(b.game);
            brls::View* view;
            if (this->grid) {
                auto* tile = (IssueTile*)group.tiles[i];
                if (tile->app_id != b.game.app_id) {
                    // Another game in this place: not its icon.
                    tile->app_id = b.game.app_id;
                    tile->clear_icon();
                }
                tile->set_name(line_names[b.game.app_id]);
                tile->set_selected(picked);
                tile->set_detail(short_text(b.game, b.issue));
                tile->set_detail_color(color);
                view = tile;
            } else {
                IssueCell* cell;
                if (i < cells.size()) {
                    cell = (IssueCell*)cells[i];
                } else {
                    cell = new IssueCell();
                    cadran::List::row(cell);
                    group.list->addView(cell);
                }
                cell->setText(line_names[b.game.app_id]);
                cell->set_selected(picked);
                cell->setDetailText(short_text(b.game, b.issue));
                cell->setDetailTextColor(color);
                view = cell;
            }
            view->registerClickAction([this, b, hos](brls::View*) {
                this->open(b.game, b.issue, hos);
                return true;
            });
            view->registerAction(picked ? "playguard/games/unselect"_i18n : "playguard/games/select"_i18n, brls::BUTTON_Y,
                                 [this, id = b.game.app_id](brls::View*) {
                                     this->toggle(id);
                                     return true;
                                 });
            ui::set_visible(view, true);
        }
        if (!this->grid)
            for (size_t i = shown.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
        const std::string title = brls::getStr(std::string("playguard/games/groups/") + category_key(category), (int)shown.size());
        group.title->setTitle(cadran::upper(title));
        // A section's own cells go with it: borealis would still give the
        // focus to a visible cell inside a box that is gone.
        if (category == GC_CAT_REINSTALL) {
            group.remove_all->setText(brls::getStr("playguard/games/remove_all", (int)shown.size()));
            ui::set_visible(group.save, !shown.empty());
            ui::set_visible(group.send, !shown.empty());
            ui::set_visible(group.remove_all, !shown.empty() && !app::read_only());
        }
        if (category == GC_CAT_VERIFY) {
            group.verify_all->setText(shown.size() == 1 ? "playguard/games/action/verify"_i18n
                                                        : brls::getStr("playguard/games/verify_all", (int)shown.size()));
            ui::set_visible(group.verify_all, !shown.empty());
        }
        ui::set_visible(group.box, !shown.empty());
    }
    // Above the sections once a game is picked (Ⓨ, in the footer on each
    // game): how many, A for what to do with them.
    this->selection->setText(this->selection_text());
    this->selection->setTextColor(brls::Application::getTheme()["brls/list/listItem_value_color"]);
    ui::set_visible(this->selection, !this->selected.empty());

    // The card while no game has a problem, a check under way included (its
    // count then in the card's title).
    const bool clear = broken.empty() && R_SUCCEEDED(this->list_rc);
    if (!clear && brls::Application::getCurrentFocus() == this->check_again) this->park_focus();
    ui::set_visible(this->all_clear.box, clear);

    // Nothing had the focus before the first game with a problem; and never
    // the focus left in a section that went (its last game repaired). While
    // the games are checked, until the user moves it, the focus stays on the
    // first game: the games found later go into the sections above and would
    // push the cell it was given below the screen (the scroll does not follow).
    brls::View* focus = brls::Application::getCurrentFocus();
    bool lost = focus == status.getView();   // parked there while the games were rebuilt
    for (brls::View* v = focus; v && !lost; v = v->getParent()) lost = v->getVisibility() != brls::Visibility::VISIBLE;
    brls::View* first = this->first_game();
    const bool untouched = this->auto_focus && focus == this->auto_focus;
    if (first && (!this->focused || lost || untouched)) {
        this->focused = true;
        this->auto_focus = first;
        brls::Application::giveFocus(first);
    }
    // Nothing else to focus then (not under a dialog, which keeps its own).
    if (clear && brls::Application::getCurrentFocus() != this->check_again && ui::has_focus(this))
        brls::Application::giveFocus(this->check_again);
    // The check over (this was its last show): the focus is the user's.
    if (!this->checking) this->auto_focus = nullptr;
    // Never a stop of its own: only while the games are rebuilt.
    if (brls::Application::getCurrentFocus() != status.getView()) status->setFocusable(false);

    // The line above the list: the check under way, an error, or the result
    // and when it was made.
    if (this->checking) {
        const std::string text = this->check_total ? brls::getStr("playguard/games/checking", (int)this->check_done,
                                                                  (int)this->check_total)
                                                   : "playguard/games/searching"_i18n;
        this->checking_text(text);
    } else if (R_FAILED(this->list_rc)) {
        this->status_text("playguard/games/err_list"_i18n + " — " + ui::rc_text(this->list_rc), cadran::kit::color("cadran/coral"));
    } else {
        const std::string found = broken.empty() ? (count == 1 ? "playguard/games/none_one"_i18n
                                                               : brls::getStr("playguard/games/none", (int)count))
                                                 : brls::getStr("playguard/games/section_count", (int)broken.size(), (int)count);
        u64 now = 0;
        if (this->cache.checked_at > 0) time_local_now(&now, nullptr);
        const std::string age = game_check_text::age_text(this->cache.checked_at, (int64_t)now, w);
        const std::string truncated = this->cache.truncated ? brls::getStr("playguard/games/truncated", (int)count) : "";
        const std::vector<std::string> notes = { truncated, app::dev_mode() ? this->timing : "" };
        if (clear) {
            this->all_clear.title->setText(found);
            this->all_clear.text->setText(game_check_text::result_line(age, true, "", notes));
            this->status_text("");
        } else {
            this->status_text(game_check_text::result_line(found, false, age, notes), cadran::kit::color("cadran/text3"));
        }
    }
    // What this screen is for, while there is no game above which it would
    // push the games down; no "A on a game" then, nor Ⓡ for covers.
    note->setText(clear ? "playguard/games/note_clear"_i18n : "playguard/games/note"_i18n);
    ui::set_visible(note.getView(), broken.empty());
    this->show_view_action(!clear);
    hint->setText(brls::getStr("playguard/games/many", ui::storage_short(si)));
    ui::set_visible(hint.getView(), broken.size() >= MANY);
    this->load_icons();
}

void GameCheckActivity::show_tiles(Group& group, size_t count)
{
    if (group.tiles.size() == count) return;
    // The focus on one of these covers, about to be deleted: kept on the
    // same place, or the first game.
    bool focus_in = false;
    for (brls::View* v = brls::Application::getCurrentFocus(); v && !focus_in; v = v->getParent())
        focus_in = v == group.list;
    size_t at = 0;
    if (focus_in) {
        const auto it = std::find(group.tiles.begin(), group.tiles.end(), brls::Application::getCurrentFocus());
        if (it != group.tiles.end()) at = (size_t)(it - group.tiles.begin());
        this->park_focus();
    }
    const bool was_auto = focus_in && this->auto_focus;
    group.list->clearViews();
    group.tiles.clear();
    brls::Box* row = nullptr;
    for (size_t i = 0; i < count; i++) {
        if (!row || (int)row->getChildren().size() >= COLUMNS) {
            row = new brls::Box(brls::Axis::ROW);
            row->setWidth(COLUMNS * GameTile::WIDTH);
            group.list->addView(row);
        }
        auto* tile = new IssueTile();
        row->addView(tile);
        group.tiles.push_back(tile);
    }
    // Up and down to the same column (games_logic).
    const int n = (int)count;
    for (int i = 0; i < n; i++) {
        const int up = games_logic::grid_neighbour(i, n, COLUMNS, -1);
        const int down = games_logic::grid_neighbour(i, n, COLUMNS, 1);
        if (up >= 0) group.tiles[i]->setCustomNavigationRoute(brls::FocusDirection::UP, group.tiles[up]);
        if (down >= 0) group.tiles[i]->setCustomNavigationRoute(brls::FocusDirection::DOWN, group.tiles[down]);
    }
    if (focus_in && count) {
        brls::View* to = group.tiles[std::min(at, count - 1)];
        brls::Application::giveFocus(to);
        if (was_auto) this->auto_focus = to;
    }
}

void GameCheckActivity::show_view_action(bool shown)
{
    if (shown == (this->view_action != 0)) return;
    // Not while borealis goes through the actions (one of them may have
    // led here): next frame.
    std::weak_ptr<bool> weak = this->alive;
    brls::sync([this, weak, shown]() {
        if (weak.expired() || shown == (this->view_action != 0)) return;
        if (shown) {
            this->view_action = this->getContentView()->registerAction(view_label(!this->grid), brls::BUTTON_RB,
                                                                       [this](brls::View*) {
                                                                           this->switch_view();
                                                                           return true;
                                                                       });
        } else {
            this->getContentView()->unregisterAction(this->view_action);
            this->view_action = 0;
        }
        brls::Application::getGlobalHintsUpdateEvent()->fire();
    });
}

void GameCheckActivity::set_view_hint()
{
    // Its text only: borealis goes through these actions while one runs.
    this->getContentView()->updateActionHint(brls::BUTTON_RB, view_label(!this->grid));
    brls::Application::getGlobalHintsUpdateEvent()->fire();
}

void GameCheckActivity::switch_view()
{
    const u64 id = this->focused_game();   // found in the look it has now
    this->grid = !this->grid;
    config::get().check_grid = this->grid;   // the next visit opens on it
    ui::save_config();
    this->set_view_hint();
    // Next frame, out of the action under way: every game rebuilt.
    std::weak_ptr<bool> weak = this->alive;
    brls::sync([this, weak, id]() {
        if (weak.expired()) return;
        if (id) this->park_focus();
        this->auto_focus = nullptr;
        for (Group& group : this->groups) {
            if (!group.list) continue;
            group.list->clearViews();
            group.tiles.clear();
            group.list->setAlignItems(this->grid ? brls::AlignItems::CENTER : brls::AlignItems::STRETCH);
            card_look(group.list, this->grid);
        }
        this->show();
        // The same game, in the other look.
        if (brls::View* to = id ? this->game_view(id) : nullptr) {
            brls::Application::giveFocus(to);
            this->auto_focus = nullptr;
        }
    });
}

void GameCheckActivity::park_focus()
{
    status->setFocusable(true);
    brls::Application::giveFocus(status.getView());
}

u64 GameCheckActivity::focused_game() const
{
    const brls::View* focus = brls::Application::getCurrentFocus();
    for (const Group& group : this->groups) {
        if (!group.list) continue;
        if (this->grid) {
            for (brls::View* tile : group.tiles)
                if (tile == focus) return ((IssueTile*)tile)->app_id;
            continue;
        }
        const auto& cells = group.list->getChildren();
        for (size_t i = 0; i < cells.size() && i < group.games.size(); i++)
            if (cells[i] == focus) return group.games[i].app_id;
    }
    return 0;
}

brls::View* GameCheckActivity::game_view(u64 id) const
{
    for (const Group& group : this->groups) {
        for (size_t i = 0; i < group.games.size(); i++) {
            if (group.games[i].app_id != id) continue;
            if (this->grid) return i < group.tiles.size() ? group.tiles[i] : nullptr;
            const auto& cells = group.list->getChildren();
            return i < cells.size() ? cells[i] : nullptr;
        }
    }
    return nullptr;
}

void GameCheckActivity::load_icons()
{
    if (!this->grid || !game_icons::wanted()) return;
    std::vector<u64> ids;
    for (const Group& group : this->groups)
        for (brls::View* v : group.tiles) {
            auto* tile = (IssueTile*)v;
            if (v->getVisibility() != brls::Visibility::VISIBLE || tile->has_icon()) continue;
            bool owned = false;
            if (const int tex = game_icons::texture(tile->app_id, owned)) tile->set_icon_texture(tex, owned);
            else if (!game_icons::known(tile->app_id)) ids.push_back(tile->app_id);
        }
    if (ids.empty()) return;
    std::weak_ptr<bool> weak = this->alive;
    game_icons::load(ids, [this, weak]() {
        if (!weak.expired()) this->load_icons();
    });
}

// The first game of the list, in the first section shown.
brls::View* GameCheckActivity::first_game() const
{
    for (GameCategory category : GROUPS) {
        const Group& group = this->groups[category];
        if (group.box->getVisibility() != brls::Visibility::VISIBLE) continue;
        if (this->grid) {
            if (!group.tiles.empty()) return group.tiles[0];
            continue;
        }
        for (brls::View* cell : group.list->getChildren())
            if (cell->getVisibility() == brls::Visibility::VISIBLE) return cell;
    }
    return nullptr;
}

// Back from a dialog: "Check these games" may have gone meanwhile (its
// games left To check while the result asked whether to remove them); the
// first game then has the focus (ui::keep_focus).
void GameCheckActivity::onResume()
{
    brls::View* fallback = this->first_game();
    if (!fallback && this->all_clear.box->getVisibility() == brls::Visibility::VISIBLE) fallback = this->check_again;
    ui::keep_focus(this, fallback);
}

bool GameCheckActivity::changes_list(const GameFacts& game) const
{
    SysInfo si;
    sysinfo_get(&si);
    const GameIssue now = gamecheck_classify(&game, si.hos_version);
    for (const GameFacts& g : this->cache.games) {
        if (g.app_id != game.app_id) continue;
        const GameIssue was = gamecheck_classify(&g, si.hos_version);
        if (was != now) return true;
        // Listed: its line and its dialog are made from what was found.
        return now != GC_OK && std::memcmp(&g, &game, sizeof g) != 0;
    }
    return now != GC_OK;
}
