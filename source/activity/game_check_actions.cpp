// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
// The actions on the games of Console › Check the installed games: a game's
// details and its fix, removing games or their content, the selection (Ⓨ)
// and the integrity check (activity/game_check_activity.hpp).
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
#include "ui/cadran_list.hpp"
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

void GameCheckActivity::open(const GameFacts& game, GameIssue issue, u32 current_hos)
{
    // Every action but the check changes the console.
    u32 actions = gamecheck_actions(&game, issue);
    u32 recommended = gamecheck_recommended(&game, issue, current_hos);
    if (app::read_only()) {
        actions &= GA_VERIFY;
        recommended &= GA_VERIFY;
    }
    auto* box = details_view(game, issue, current_hos, icon_of(game.app_id));
    if (app::read_only() && gamecheck_actions(&game, issue) & ~GA_VERIFY) {
        brls::Label* note_ro = cadran::kit::label("playguard/common/read_only_note"_i18n, 15.5f, "cadran/text3");
        note_ro->setMarginTop(8);
        box->addView(note_ro);
    }

    // Two buttons at most, side by side (a third goes on a row of its own):
    // the fix first ("its first button", the texts say), in teal, with the
    // focus, then the other actions; B closes. With no fix: OK, then the
    // actions.
    auto* dialog = ui::dialog("", box);   // scrolls when its text is long
    const GameFacts g = game;
    const u32 others = actions & ~recommended;
    if (recommended) {
        GameContent update = {};
        for (u32 i = 0; i < g.content_count; i++)
            if (g.contents[i].type == GC_META_PATCH) update = g.contents[i];
        dialog->addButton(fix_button(recommended), [this, g, recommended, update]() { this->run(g, recommended, update); });
        if (others) dialog->addButton("playguard/games/other_actions"_i18n, [this, g, others]() { this->choose(g, others); });
        cadran::style_dialog(dialog);
        if (auto* fix = dynamic_cast<brls::Button*>(dialog->getView("brls/dialog/button1")))
            cadran::style_button(fix, cadran::Emphasis::PRIMARY);
    } else {
        dialog->addButton("hints/ok"_i18n, []() {});
        if (others) dialog->addButton("playguard/games/actions"_i18n, [this, g, others]() { this->choose(g, others); });
    }
    dialog->open();
}

void GameCheckActivity::choose(const GameFacts& g, u32 actions)
{
    // What changes nothing or can be undone first, removals last, the whole
    // game at the very end.
    std::vector<ui::Choice> choices;
    auto add = [&](u32 action, const GameContent& c) {
        // Removals in coral, the others as they are.
        const bool removal = action == GA_REMOVE || action == GA_REMOVE_UPDATE || action == GA_REMOVE_ADDON;
        const char* icon = action == GA_RESET_LAUNCH ? "restore" : action == GA_VERIFY ? "check" : "trash";
        choices.push_back({ action_label(g, action, &c), [this, g, action, c]() { this->run(g, action, c); }, icon, "", "",
                            removal ? ui::Choice::Tone::DANGER : ui::Choice::Tone::PLAIN });
    };
    const GameContent none = {};
    if (actions & GA_RESET_LAUNCH) add(GA_RESET_LAUNCH, none);
    if (actions & GA_VERIFY) add(GA_VERIFY, none);
    for (u32 i = 0; i < g.content_count; i++) {
        const GameContent& c = g.contents[i];
        if (c.storage != GC_STORAGE_NAND && c.storage != GC_STORAGE_SD) continue;
        if (c.type == GC_META_PATCH && (actions & GA_REMOVE_UPDATE)) add(GA_REMOVE_UPDATE, c);
    }
    for (u32 i = 0; i < g.content_count; i++) {
        const GameContent& c = g.contents[i];
        if (c.storage != GC_STORAGE_NAND && c.storage != GC_STORAGE_SD) continue;
        if (c.type == GC_META_ADDON && (actions & GA_REMOVE_ADDON)) add(GA_REMOVE_ADDON, c);
    }
    if (actions & GA_REMOVE) add(GA_REMOVE, none);
    ui::pick_actions(game_name(g), choices);
}

void GameCheckActivity::run(const GameFacts& g, u32 action, const GameContent& content)
{
    switch (action) {
        case GA_RESET_LAUNCH:  this->reset_launch(g); break;
        case GA_VERIFY:        this->verify({ g }); break;
        case GA_REMOVE_UPDATE:
        case GA_REMOVE_ADDON:  this->remove_content(g, content); break;
        default:               this->remove(g); break;
    }
}

void GameCheckActivity::remove(const GameFacts& g, bool asked)
{
    const std::string label = history_label(g), name = game_name(g);
    const auto read_again = ui::guarded(this->alive, [this, id = g.app_id]() { this->refresh(false, id); });
    auto now = [read_again, id = g.app_id, label, name]() {
        const Result rc = gamecheck_remove(id);
        if (R_SUCCEEDED(rc)) history_flow::record_event("remove_game", "", label);
        ui::notify_result(rc, brls::getStr("playguard/games/removed", name), "playguard/games/remove_err"_i18n);
        if (R_SUCCEEDED(rc)) read_again();
    };
    if (asked) now();
    else ui::confirm_danger(name + "\n\n" + "playguard/games/remove_body"_i18n, "playguard/games/remove"_i18n, now);
}

// The games to install again in a JSON table (exports/, as the Activity
// tab's exports) and as text next to it (game_check_text::list_text).
void GameCheckActivity::save_list(const std::vector<GameFacts>& games,
                                  std::function<void(const std::string& path, const std::string& error)> done) const
{
    const u32 hos = sysinfo_hos_version();
    table_export::Table t;
    t.title = "playguard/games/list_title"_i18n;
    t.sheet = t.title;
    t.columns = { { "name", "playguard/games/col_name"_i18n, false },
                  { "title_id", "playguard/games/col_id"_i18n, false },
                  { "version", "playguard/games/col_version"_i18n, false },
                  { "problem", "playguard/games/col_problem"_i18n, false } };
    for (const GameFacts& g : games) {
        const GameIssue issue = gamecheck_classify(&g, hos);
        t.rows.push_back({ game_name(g), id_text(g.app_id), g.display_version, short_text(g, issue) });
    }
    const std::string text = list_text(games);
    // Written off the UI thread, as the other exports; `done` back on it.
    ui::write_later("games list", [t, text, done]() {
        std::string error;
        std::string path;
        if (!paths::ensure_dir(paths::exports_dir())) {
            error = paths::exports_dir();
        } else {
            path = table_export::save(t, table_export::Format::Json, paths::exports_dir(), "games_to_reinstall", &error);
            if (!path.empty() && !paths::atomic_write(path.substr(0, path.size() - 4) + "txt", text, &error)) path.clear();
        }
        brls::sync([done, path, error]() { done(path, error); });
    });
}

static constexpr int OPEN_MS = 300;   // remove_games()' dialog opening, before the first game

// What remove_games() shares with its dialog (main thread only).
struct GameCheckActivity::Removal
{
    std::vector<GameFacts> games;
    std::string path;               // their list, saved first
    std::vector<GameFacts> failed;  // not removed, for the summary
    u32 removed = 0;
    bool cancel = false;
    bool open = true;   // the dialog is on screen
    brls::Dialog* dialog = nullptr;
    // Centred, as the full check's: the covers leafed through, a bar, the
    // game's place in the batch, its name, how many were removed and not,
    // and what stays.
    CoverStrip* strip = nullptr;
    LoadingBar* bar = nullptr;
    brls::Label* title = nullptr;
    brls::Label* name = nullptr;
    brls::Label* counts = nullptr;
};

// A line of a centred dialog: `size` px in `color_key`, as wide as the
// dialog (centred text wraps there, or is cut on one line).
static brls::Label* centred(const std::string& text, float size, const char* color_key, float top)
{
    brls::Label* label = cadran::kit::label(text, size, color_key);
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setAlignSelf(brls::AlignSelf::STRETCH);
    label->setMarginTop(top);
    return label;
}

void GameCheckActivity::remove_games(std::vector<GameFacts> games, bool asked)
{
    if (games.empty() || this->removing) return;
    const std::string body = brls::getStr("playguard/games/remove_all_body", (int)games.size()) + "\n" +
                             game_check_text::name_lines(games, words());
    std::weak_ptr<bool> weak = this->alive;
    auto now = [this, weak, games]() {
        if (weak.expired()) return;
        // The list first: nothing is removed without it.
        this->save_list(games, [this, weak, games](const std::string& path, const std::string& error) {
            if (path.empty()) {
                list_error(error);
                return;
            }
            if (weak.expired() || this->removing) return;
            // Read-only mode and the PIN: asked once, for them all.
            const Result rc = core_change_allowed();
            if (R_FAILED(rc)) {
                ui::notify_result(rc, "", "playguard/games/remove_err"_i18n);
                return;
            }
            this->removing = true;
            auto removal = std::make_shared<Removal>();
            removal->games = games;
            removal->path = path;
            const std::string file = path.substr(path.find_last_of('/') + 1);
            auto* box = new brls::Box(brls::Axis::COLUMN);
            box->setAlignItems(brls::AlignItems::CENTER);
            box->setPadding(30, 36, 22, 36);
            std::vector<u64> ids;
            for (const GameFacts& g : games) ids.push_back(g.app_id);
            removal->strip = new CoverStrip();
            removal->strip->setFocusable(false);   // Cancel keeps it
            removal->strip->setHeight(112);
            removal->strip->setAlignSelf(brls::AlignSelf::STRETCH);
            removal->strip->set_games(ids);
            box->addView(removal->strip);
            removal->bar = new LoadingBar();
            removal->bar->setHeight(6);
            removal->bar->setWidth(360);
            removal->bar->setMarginTop(20);
            box->addView(removal->bar);
            removal->title = centred("", 20, "cadran/text", 12);
            box->addView(removal->title);
            removal->name = centred("", 16, "cadran/text2", 2);
            removal->name->setSingleLine(true);   // a long name is cut, not wrapped over the dialog
            box->addView(removal->name);
            removal->counts = centred("", 15, "cadran/text3", 6);
            box->addView(removal->counts);
            box->addView(centred(brls::getStr("playguard/games/remove_note", file), 14, "cadran/text3", 14));
            removal->dialog = new brls::Dialog(box);
            ui::add_cancel(removal->dialog, [removal]() {
                removal->cancel = true;
                removal->open = false;   // the dialog closes itself
            });
            removal->dialog->open();
            this->remove_next(removal, 0);
        });
    };
    if (asked) now();
    else ui::confirm_danger(body, "playguard/games/remove"_i18n, now);
}

// What a removal stopped or short of some games did: how many went, the
// games left by name (in coral), what stays and where their list is.
void GameCheckActivity::removal_summary(const Removal& r)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(30, 36, 22, 36);
    // A warning in a soft coral circle: not everything went.
    auto* circle = new brls::Box(brls::Axis::ROW);
    circle->setWidth(64);
    circle->setHeight(64);
    circle->setCornerRadius(32);
    circle->setBackgroundColor(brls::Application::getTheme()["cadran/coral_soft"]);
    circle->setAlignItems(brls::AlignItems::CENTER);
    circle->setJustifyContent(brls::JustifyContent::CENTER);
    circle->addView(cadran::kit::icon(r.cancel ? "pause" : "alert", 28, "cadran/coral"));
    box->addView(circle);
    box->addView(centred(r.cancel ? "playguard/games/remove_stopped"_i18n : "playguard/games/remove_finished"_i18n, 16,
                         "cadran/text3", 16));
    const std::string summary = r.removed == 1
                                    ? brls::getStr("playguard/games/remove_summary_one", (int)r.games.size())
                                    : brls::getStr("playguard/games/remove_summary", (int)r.removed, (int)r.games.size());
    box->addView(centred(summary, 22, "cadran/text", 2));
    if (!r.failed.empty()) {
        box->addView(centred(brls::getStr("playguard/games/remove_count_err", (int)r.failed.size()), 17, "cadran/coral", 16));
        std::string names = game_check_text::name_lines(r.failed, words());
        if (!names.empty() && names[0] == '\n') names.erase(0, 1);
        box->addView(centred(names, 16, "cadran/text2", 4));
    }
    box->addView(centred(brls::getStr("playguard/games/remove_note", r.path), 14, "cadran/text3", 16));
    brls::Dialog* dialog = ui::box_dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

// One game a frame, in a dialog of its own: the game under way (icon, name),
// its place in the batch, the bar, how many were removed and not, and Cancel
// (B too), which stops after the game being removed. Then a toast when all
// went, else a summary: how many, the games left by name, their list.
void GameCheckActivity::remove_next(std::shared_ptr<Removal> removal, size_t at)
{
    const std::vector<GameFacts>& games = removal->games;
    const size_t total = games.size();
    if (at == total || removal->cancel) {
        this->removing = false;
        auto report = [removal, total]() {
            if (removal->removed == total) {
                ui::notify(removal->removed == 1
                               ? brls::getStr("playguard/games/removed_all_one", removal->path)
                               : brls::getStr("playguard/games/removed_all", (int)removal->removed, removal->path));
                return;
            }
            removal_summary(*removal);
        };
        if (removal->open) {
            removal->open = false;
            removal->dialog->close(report);
        } else {
            report();
        }
        this->refresh(false, 0);
        return;
    }
    const GameFacts& g = games[at];
    removal->strip->set_done((u32)at);
    cover_strip_read(removal->strip);
    removal->bar->setProgress((float)at / (float)total);
    removal->title->setText(total == 1 ? "playguard/games/remove_title"_i18n
                                       : brls::getStr("playguard/games/remove_title_n", (int)at + 1, (int)total));
    removal->name->setText(game_name(g));
    const u32 removed = removal->removed;
    const size_t failed = removal->failed.size();
    std::string detail = removed == 1 ? "playguard/games/remove_done_one"_i18n
                         : removed    ? brls::getStr("playguard/games/remove_done", (int)removed)
                                      : "";
    if (failed)
        detail += (detail.empty() ? "" : " · ") + (failed == 1 ? "playguard/games/remove_failed_one"_i18n
                                                               : brls::getStr("playguard/games/remove_failed", (int)failed));
    removal->counts->setText(detail);
    ui::set_visible(removal->counts, !detail.empty());
    if (failed) removal->counts->setTextColor(cadran::kit::color("cadran/coral"));
    // On the UI thread, as the PIN it may ask for: the first game once the
    // dialog is fully on screen, each next one a frame later.
    std::weak_ptr<bool> weak = this->alive;
    auto step = [this, weak, removal, at]() {
        if (weak.expired()) {
            if (removal->open) {
                removal->open = false;
                removal->dialog->close();
            }
            return;
        }
        if (removal->cancel) {
            this->remove_next(removal, at);
            return;
        }
        const GameFacts& game = removal->games[at];
        const Result rc = gamecheck_remove(game.app_id);
        if (R_SUCCEEDED(rc)) {
            history_flow::record_event("remove_game", "", history_label(game));
            removal->removed++;
        } else {
            removal->failed.push_back(game);
            brls::Logger::warning("games: {} not removed ({})", id_text(game.app_id), ui::rc_text(rc));
        }
        this->remove_next(removal, at + 1);
    };
    if (at) brls::sync(step);
    else brls::delay(OPEN_MS, step);
}

void GameCheckActivity::toggle(u64 id)
{
    if (!this->selected.erase(id)) this->selected.insert(id);
    this->show();
}

std::string GameCheckActivity::selection_text() const
{
    if (this->selected.empty()) return "playguard/games/select_many"_i18n;
    if (this->selected.size() == 1) return "playguard/games/selected_one"_i18n;
    return brls::getStr("playguard/games/selected", (int)this->selected.size());
}

std::vector<GameFacts> GameCheckActivity::selected_games() const
{
    std::vector<GameFacts> games;
    for (const Group& group : this->groups)
        for (const GameFacts& g : group.games)
            if (this->selected.count(g.app_id)) games.push_back(g);
    return games;
}

// What can be done to every game picked: check their data, remove them
// (each listed game but one whose SD card cannot be read), then pick a
// whole section, or none. With none picked yet: the sections only.
void GameCheckActivity::selection_menu()
{
    const u32 hos = sysinfo_hos_version();
    const std::vector<GameFacts> games = this->selected_games();
    std::vector<GameFacts> to_verify, to_remove;
    for (const GameFacts& g : games) {
        const u32 actions = gamecheck_actions(&g, gamecheck_classify(&g, hos));
        if (actions & GA_VERIFY) to_verify.push_back(g);
        if (actions & GA_REMOVE) to_remove.push_back(g);
    }
    std::vector<ui::Choice> choices;
    if (!to_verify.empty())
        choices.push_back({ brls::getStr("playguard/games/selection/verify", (int)to_verify.size()),
                            [this, to_verify]() { this->verify(to_verify); }, "check" });
    if (!to_remove.empty() && !app::read_only())
        choices.push_back({ brls::getStr("playguard/games/selection/remove", (int)to_remove.size()),
                            [this, to_remove]() { this->remove_games(to_remove); }, "trash", "", "",
                            ui::Choice::Tone::DANGER });
    for (GameCategory category : GROUPS) {
        const Group& group = this->groups[category];
        bool all = true;
        for (const GameFacts& g : group.games) all &= this->selected.count(g.app_id) != 0;
        if (group.games.empty() || all) continue;
        choices.push_back({ brls::getStr("playguard/games/selection/select_group",
                                         brls::getStr(std::string("playguard/games/groups/") + category_key(category),
                                                      (int)group.games.size())),
                            [this, category]() {
                                for (const GameFacts& g : this->groups[category].games) this->selected.insert(g.app_id);
                                this->show();
                            },
                            "list" });
    }
    if (!this->selected.empty())
        choices.push_back({ "playguard/games/selection/clear"_i18n, [this]() {
                               this->selected.clear();
                               this->show();
                           },
                           "stop" });
    ui::pick_actions(this->selection_text(), choices);
}

void GameCheckActivity::remove_content(const GameFacts& g, const GameContent& c)
{
    const bool update = c.type == GC_META_PATCH;
    const std::string label = history_label(g), name = game_name(g);
    const std::string body = name + "\n" + action_label(g, update ? GA_REMOVE_UPDATE : GA_REMOVE_ADDON, &c) + "\n\n" +
                             (update ? "playguard/games/remove_update_body"_i18n : "playguard/games/remove_addon_body"_i18n);
    std::weak_ptr<bool> weak = this->alive;
    ui::confirm_danger(body, "playguard/games/remove"_i18n, [this, weak, id = g.app_id, c, update, label, name]() {
        const Result rc = gamecheck_remove_content(id, &c);
        const std::string done = brls::getStr(update ? "playguard/games/update_removed" : "playguard/games/addon_removed", name);
        if (R_FAILED(rc)) {
            ui::notify_result(rc, done, "playguard/games/content_err"_i18n);
            return;
        }
        history_flow::record_event(update ? "remove_update" : "remove_addon", "",
                                   update ? label : fmt::format("{} · {}", label, id_text(c.id)));
        if (weak.expired()) {
            ui::notify(done);
            return;
        }
        // Removing the update is a repair: the next read says if it worked.
        if (update) this->pending = Pending{ id, name, done };
        else ui::notify(done);
        this->refresh(false, id);
    });
}

void GameCheckActivity::reset_launch(const GameFacts& g)
{
    // Nothing is deleted and nothing is lost: no question first.
    const u32 installed = gamecheck_installed_version(&g);
    const std::string label = history_label(g), name = game_name(g);
    const Result rc = gamecheck_reset_launch_version(g.app_id, installed);
    const std::string done = brls::getStr("playguard/games/launch_reset", name);
    if (R_FAILED(rc)) {
        ui::notify_result(rc, done, "playguard/games/launch_err"_i18n);
        return;
    }
    history_flow::record_event("launch_version", "", label);
    this->pending = Pending{ g.app_id, name, done };
    this->refresh(false, g.app_id);
}

// What the check of `total` games found, once the dialog is closed: the
// activity may be gone by then (`weak`), only the way out needs it.
void GameCheckActivity::report_verified(GameCheckActivity* self, const std::weak_ptr<bool>& weak,
                                        const std::vector<Verified>& results, size_t total)
{
    if (results.empty()) {
        ui::notify("playguard/games/verify_cancelled"_i18n);
        return;
    }
    if (total == 1) {
        // What the console found, with its code as the HOME menu
        // would show it, and the way out at hand.
        const Verified& v = results.front();
        const std::string name = game_name(v.game);
        if (R_FAILED(v.rc)) {
            ui::error("playguard/games/verify_err"_i18n, v.rc);
        } else if (!v.corrupt) {
            ui::info(brls::getStr("playguard/games/verify_ok", name));
        } else {
            std::string body = brls::getStr("playguard/games/verify_bad", name);
            if (v.found) body += "\n\n" + brls::getStr("playguard/games/verify_code", console_code(v.found));
            if (app::read_only() || weak.expired()) ui::info(body);
            // One question: what removing does is said here, in coral.
            else ui::confirm_danger(body + "\n\n" + "playguard/games/remove_body"_i18n,
                                    "playguard/games/fix_button/remove"_i18n, [self, weak, g = v.game]() {
                if (!weak.expired()) self->remove(g, true);   // closed while the question was open
            });
        }
        return;
    }
    // A batch: how many of each, the corrupt ones by name, and
    // removing them all at hand.
    const game_check_text::Tally t = game_check_text::tally(results);
    std::string body = brls::getStr("playguard/games/verify_done", (int)results.size(), (int)total);
    body += "\n\n" + brls::getStr("playguard/games/verify_count_bad", (int)t.corrupt.size()) +
            game_check_text::name_lines(t.corrupt, words());
    body += "\n" + brls::getStr("playguard/games/verify_count_ok", (int)t.intact);
    if (t.failed) body += "\n" + brls::getStr("playguard/games/verify_count_err", (int)t.failed);
    if (t.corrupt.empty() || app::read_only() || weak.expired()) ui::info(body);
    else
        ui::confirm_danger(body + "\n\n" + "playguard/games/remove_body"_i18n,
                           t.corrupt.size() == 1 ? "playguard/games/remove_corrupt_one"_i18n
                                                 : brls::getStr("playguard/games/remove_corrupt", (int)t.corrupt.size()),
                           [self, weak, corrupt = t.corrupt]() {
                               if (!weak.expired()) self->remove_games(corrupt, true);   // closed while the question was open
                           });
}

void GameCheckActivity::verify(std::vector<GameFacts> games)
{
    if (games.empty()) return;
    if (this->verifying) {
        ui::notify("playguard/games/verify_busy"_i18n);
        return;
    }
    this->verifying = true;
    const size_t total = games.size();

    // A dialog of its own (action/game_verify_flow): the game, its place in a
    // batch, the bar, and Cancel, which stops the batch after what was checked.
    std::vector<u64> ids;
    for (const GameFacts& g : games) ids.push_back(g.app_id);
    std::shared_ptr<game_verify_flow::State> state = game_verify_flow::make_dialog(ids);

    // The game under way, in the dialog (main thread).
    auto start = [state, total](const GameFacts& g, size_t at) {
        if (!state->open) return;
        game_verify_flow::show_game(state.get(), icon_of(g.app_id), game_name(g), at, total);
    };
    start(games[0], 0);
    state->dialog->open();

    // Minutes for a big game: a thread of its own, so the other reads on
    // borealis' queue do not wait behind it. What each check found is kept
    // with its game as soon as it is known: a batch cancelled halfway keeps
    // what was checked.
    std::weak_ptr<bool> weak = this->alive;
    ui::in_background("verify", [this, weak, state, games, start]() {
        auto results = std::make_shared<std::vector<Verified>>();
        for (size_t at = 0; at < games.size() && !state->cancel.load(); at++) {
            const GameFacts& g = games[at];
            if (at) brls::sync([start, g, at]() { start(g, at); });
            Verified v;
            v.game = g;
            v.rc = game_verify_flow::verify(state, g.app_id, &v.corrupt, &v.found);
            if (v.rc == NXM_RC_CANCELLED) break;
            results->push_back(v);
            if (R_FAILED(v.rc)) continue;
            u64 now = 0;
            time_local_now(&now, nullptr);
            brls::sync(ui::guarded(weak, [this, v, now]() {
                if (game_cache::set_verified(&this->cache, v.game.app_id, v.corrupt ? GC_VERIFIED_CORRUPT : GC_VERIFIED_INTACT,
                                             (int64_t)now, v.found))
                    this->save();
            }));
        }
        brls::sync([this, weak, state, results, total = games.size()]() {
            game_verify_flow::close(state, [this, weak, results, total]() { report_verified(this, weak, *results, total); });
            if (weak.expired()) return;
            this->verifying = false;
            // The console's view of them may have changed: checked again.
            for (const Verified& v : *results) this->waiting.push_back(v.game.app_id);
            this->refresh(false, 0);
        });
    });
}
