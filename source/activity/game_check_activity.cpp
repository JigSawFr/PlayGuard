// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/game_check_activity.hpp"

#include <atomic>
#include <fmt/format.h>
#include <functional>
#include <string>
#include <vector>

#include "action/history_flow.hpp"
#include "app.hpp"
#include "util/paths.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
// From this many broken games, the cause is more likely the SD card or the
// storage the console started on than each game.
constexpr size_t MANY = 3;
constexpr float  ICON_SIZE = 72;

const char* issue_key(GameIssue issue)
{
    switch (issue) {
        case GC_ARCHIVED:            return "archived";
        case GC_STORAGE_UNAVAILABLE: return "storage";
        case GC_NO_BASE:             return "no_base";
        case GC_FILES_MISSING:       return "files";
        case GC_NEEDS_VERIFY:        return "needs_verify";
        case GC_KEY_TOO_NEW:         return "key_too_new";
        case GC_FIRMWARE_TOO_OLD:    return "firmware";
        case GC_TICKET_MISSING:      return "ticket";
        case GC_LAUNCH_VERSION:      return "launch_version";
        case GC_LAUNCH_REFUSED:      return "launch";
        case GC_NO_CONTROL:          return "no_control";
        default:                     return "";
    }
}

bool warn_only(GameIssue issue)
{
    return issue == GC_STORAGE_UNAVAILABLE || issue == GC_FIRMWARE_TOO_OLD || issue == GC_KEY_TOO_NEW ||
           issue == GC_LAUNCH_VERSION;
}

std::string hos_text(u32 v)
{
    char buf[16];
    sysinfo_version_string(v, buf, sizeof(buf));
    return buf;
}

u32 required_hos(const GameFacts& g)
{
    return g.patch_required_hos > g.base_required_hos ? g.patch_required_hos : g.base_required_hos;
}

std::string id_text(u64 id)
{
    return fmt::format("{:016X}", (unsigned long long)id);
}

std::string game_name(const GameFacts& g)
{
    return g.name[0] ? std::string(g.name) : brls::getStr("playguard/games/unnamed", id_text(g.app_id));
}

// What the change history keeps: the name and the ID.
std::string history_label(const GameFacts& g)
{
    return g.name[0] ? fmt::format("{} ({})", g.name, id_text(g.app_id)) : id_text(g.app_id);
}

// A version as people know it: "version 1.4.0" when it is the installed
// one (the game says how it shows it), else "update #4" (an update's version
// is its number times 0x10000) or "the game without its updates".
std::string version_text(const GameFacts& g, u32 version)
{
    if (version == gamecheck_installed_version(&g) && g.display_version[0])
        return brls::getStr("playguard/games/version", std::string(g.display_version));
    if (version >> 16) return brls::getStr("playguard/games/update_n", (int)(version >> 16));
    return "playguard/games/base_only"_i18n;
}

// The cell's value: what the HOME menu shows, in a few words.
std::string short_text(const GameFacts& g, GameIssue issue)
{
    const std::string key = std::string("playguard/games/issues/") + issue_key(issue) + "/short";
    if (issue == GC_FIRMWARE_TOO_OLD) return brls::getStr(key, hos_text(required_hos(g)));
    return brls::getStr(key);
}

std::string cause_text(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    const std::string key = std::string("playguard/games/issues/") + issue_key(issue) + "/cause";
    switch (issue) {
        case GC_FIRMWARE_TOO_OLD: return brls::getStr(key, hos_text(required_hos(g)), hos_text(current_hos));
        case GC_KEY_TOO_NEW:      return brls::getStr(key, hos_text(current_hos), (int)g.key_generation);
        case GC_LAUNCH_VERSION:
            return brls::getStr(key, version_text(g, g.launch_required), version_text(g, gamecheck_installed_version(&g)));
        case GC_LAUNCH_REFUSED:   return brls::getStr(key, ui::rc_text(g.launch_rc));
        default:                  return brls::getStr(key);
    }
}

std::string fix_text(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    // Only the update too new: playing without it is the other way out.
    if (issue == GC_FIRMWARE_TOO_OLD && g.patch_required_hos > current_hos && g.base_required_hos <= current_hos)
        return "playguard/games/issues/firmware/fix_update"_i18n;
    if (issue == GC_KEY_TOO_NEW) return "playguard/games/issues/firmware/fix"_i18n;
    return brls::getStr(std::string("playguard/games/issues/") + issue_key(issue) + "/fix");
}

// What the console reports, for a hardware test (developer mode): not worded.
std::string developer_text(const GameFacts& g)
{
    return fmt::format("event {} · view {} · key generation {} · launch {} / installed {} · ticket {} · contents {}",
                       (int)g.record_event, g.view_ok ? fmt::format("0x{:X}", g.view_flags) : "?", (int)g.key_generation,
                       g.launch_checked ? std::to_string(g.launch_required) : "?", gamecheck_installed_version(&g),
                       g.needs_ticket ? (g.ticket_missing ? "missing" : "present") : "none", (int)g.meta_count);
}

// The fix's button: a few words (a dialog button is half the dialog wide).
std::string fix_button(u32 action)
{
    switch (action) {
        case GA_RESET_LAUNCH:  return "playguard/games/fix_button/reset_launch"_i18n;
        case GA_VERIFY:        return "playguard/games/fix_button/verify"_i18n;
        case GA_REMOVE_UPDATE: return "playguard/games/fix_button/remove_update"_i18n;
        default:               return "playguard/games/fix_button/remove"_i18n;
    }
}

std::string action_label(const GameFacts& g, u32 action, const GameContent* content)
{
    switch (action) {
        case GA_RESET_LAUNCH:  return "playguard/games/action/reset_launch"_i18n;
        case GA_VERIFY:        return "playguard/games/action/verify"_i18n;
        case GA_REMOVE_UPDATE: return brls::getStr("playguard/games/action/remove_update", version_text(g, content ? content->version : g.patch_version));
        case GA_REMOVE_ADDON:  return brls::getStr("playguard/games/action/remove_addon", id_text(content ? content->id : 0));
        default:               return "playguard/games/action/remove"_i18n;
    }
}

brls::Label* text_label(const std::string& text, float size, NVGcolor color)
{
    auto* label = new brls::Label();
    label->setText(text);
    label->setFontSize(size);
    label->setTextColor(color);
    label->setSingleLine(false);
    label->setHorizontalAlign(brls::HorizontalAlign::LEFT);
    return label;
}

// The details: the icon, the name and the problem, then why and what to do
// in two blocks read from the left, the ID and version underneath.
brls::Box* details_view(const GameFacts& g, GameIssue issue, u32 current_hos, const std::vector<unsigned char>& icon)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(26, 36, 18, 36);

    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    if (!icon.empty()) {
        auto* image = new brls::Image();
        image->setImageFromMem(icon.data(), (int)icon.size());
        image->setScalingType(brls::ImageScalingType::FIT);
        image->setWidth(ICON_SIZE);
        image->setHeight(ICON_SIZE);
        image->setMarginRight(18);
        head->addView(image);
    }
    auto* titles = new brls::Box(brls::Axis::COLUMN);
    titles->setGrow(1.0f);
    titles->setShrink(1.0f);
    titles->addView(text_label(game_name(g), 24, ui::color_text()));
    auto* problem = text_label(short_text(g, issue), 19, warn_only(issue) ? ui::color_warn() : ui::color_bad());
    problem->setMarginTop(4);
    titles->addView(problem);
    head->addView(titles);
    box->addView(head);

    auto block = [box](const std::string& title, const std::string& text) {
        auto* t = text_label(title, 18, ui::color_note());
        t->setMarginTop(18);
        box->addView(t);
        auto* body = text_label(text, 20, ui::color_text());
        body->setMarginTop(4);
        box->addView(body);
    };
    block("playguard/games/cause"_i18n, cause_text(g, issue, current_hos));
    block("playguard/games/fix"_i18n, fix_text(g, issue, current_hos));

    std::string footer = brls::getStr("playguard/games/id", id_text(g.app_id));
    if (g.display_version[0]) footer += " · " + brls::getStr("playguard/games/version", std::string(g.display_version));
    if (app::dev_mode()) footer += "\n" + developer_text(g);
    auto* foot = text_label(footer, 16, ui::color_note());
    foot->setMarginTop(18);
    box->addView(foot);
    return box;
}

std::vector<unsigned char> icon_of(u64 app_id)
{
    PlayIcon icon = { app_id, nullptr, 0 };
    playstats_icons(&icon, 1);
    std::vector<unsigned char> bytes;
    if (icon.jpeg) bytes.assign(icon.jpeg, icon.jpeg + icon.size);
    free(icon.jpeg);
    return bytes;
}

// The integrity check's dialog and what its thread shares with it.
struct VerifyState
{
    std::atomic<bool> cancel{ false };
    bool open = true;   // the dialog is on screen (main thread only)
    brls::Dialog* dialog = nullptr;
    brls::Label* label = nullptr;
    std::string name;
};

bool on_verify_progress(void* ctx, u64 current, u64 total)
{
    auto* state = (std::shared_ptr<VerifyState>*)ctx;
    const std::shared_ptr<VerifyState> s = *state;
    const int percent = total ? (int)(current * 100 / total) : 0;
    brls::sync([s, percent]() {
        if (s->open) s->label->setText(brls::getStr("playguard/games/verifying", s->name, percent > 100 ? 100 : percent));
    });
    return !s->cancel.load();
}
}   // namespace

void GameCheckActivity::onContentAvailable()
{
    note->setSingleLine(false);
    status->setSingleLine(false);
    hint->setSingleLine(false);
    // Ⓧ: every game checked again, whatever the cache says.
    this->getContentView()->registerAction("playguard/games/check_again"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->refresh(true, 0);
        return true;
    });

    // The last check at once, then what changed since.
    std::string bytes;
    if (paths::read_file(cache_file(), bytes) && game_cache::decode(bytes, &this->cache)) this->show();
    this->refresh(false, 0);
}

std::string GameCheckActivity::cache_file()
{
    return paths::nand_dir() + "/cache/games.bin";
}

void GameCheckActivity::save()
{
    std::string error;
    if (!paths::ensure_dir(paths::nand_dir() + "/cache") || !paths::atomic_write(cache_file(), game_cache::encode(this->cache), &error))
        brls::Logger::warning("games cache: not saved ({})", error);
}

void GameCheckActivity::refresh(bool all, u64 also)
{
    if (this->checking) {
        if (also) this->waiting.push_back(also);   // after the check under way
        return;
    }
    this->checking = true;
    this->status_text("playguard/games/searching"_i18n);
    std::weak_ptr<bool> weak = this->alive;
    brls::async([this, weak, all, also]() {
        auto records = std::make_shared<std::vector<GameRecord>>(GAMECHECK_MAX);
        u32 count = 0;
        bool truncated = false;
        const Result rc = gamecheck_records(records->data(), GAMECHECK_MAX, &count, &truncated);
        records->resize(count);
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
    if (run->alive.expired()) return false;   // closed: stop
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
    run->on_game = [this](const GameFacts& game, u32 done) {
        game_cache::put(&this->cache, game);
        this->check_done = done;
        this->show();
    };
    std::weak_ptr<bool> weak = this->alive;
    brls::async([this, weak, run, full]() {
        const u32 done = gamecheck_check(run->records.data(), (u32)run->records.size(), run->facts.data(), on_checked, run.get());
        const bool complete = done == run->records.size();
        brls::sync([this, weak, complete, full]() {
            if (weak.expired()) return;
            this->checking = false;
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

void GameCheckActivity::status_text(const std::string& text)
{
    this->status_text(text, ui::color_text());
}

// "Checked just now.", "Checked 12 min ago.", "Checked 3 h ago."
std::string GameCheckActivity::age_text() const
{
    if (this->cache.checked_at <= 0) return "";
    u64 now = 0;
    time_local_now(&now, nullptr);
    const int64_t s = (int64_t)now - this->cache.checked_at;
    if (s < 60) return "playguard/games/checked_now"_i18n;
    if (s < 3600) return brls::getStr("playguard/games/checked_min", (int)(s / 60));
    return brls::getStr("playguard/games/checked_h", (int)(s / 3600));
}

void GameCheckActivity::show()
{
    SysInfo si;
    sysinfo_get(&si);
    const u32 hos = si.hos_version;

    struct Broken { GameFacts game; GameIssue issue; };
    std::vector<Broken> broken;
    for (const GameFacts& g : this->cache.games) {
        const GameIssue issue = gamecheck_classify(&g, hos);
        if (issue != GC_OK) broken.push_back({ g, issue });
    }
    const u32 count = (u32)this->cache.games.size();

    // After a repair, once its game was checked again: whether it left the list.
    if (this->pending.id && !this->checking) {
        bool still = false;
        for (const Broken& b : broken) still |= b.game.app_id == this->pending.id;
        ui::notify(still ? this->pending.done : brls::getStr("playguard/games/fixed", this->pending.name));
        this->pending = Pending{};
    }

    // The cells are reused in place, as in the History screen: one of them
    // may have the focus while the list changes under it.
    auto& cells = list->getChildren();
    for (size_t i = 0; i < broken.size(); i++) {
        brls::DetailCell* cell;
        if (i < cells.size()) {
            cell = (brls::DetailCell*)cells[i];
        } else {
            cell = new brls::DetailCell();
            list->addView(cell);
        }
        const Broken b = broken[i];
        cell->setText(game_name(b.game));
        cell->setDetailText(short_text(b.game, b.issue));
        cell->setDetailTextColor(warn_only(b.issue) ? ui::color_warn() : ui::color_bad());
        cell->registerClickAction([this, b, hos](brls::View*) {
            this->open(b.game, b.issue, hos);
            return true;
        });
    }
    for (size_t i = 0; i < broken.size() && i < cells.size(); i++) ui::set_visible(cells[i], true);
    for (size_t i = broken.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
    // Nothing had the focus before the first game with a problem.
    if (!this->focused && !broken.empty()) {
        this->focused = true;
        brls::Application::giveFocus(cells[0]);
    }

    // The line above the list: the check under way, an error, or the result
    // and when it was made.
    if (this->checking && this->check_total) {
        this->status_text(brls::getStr("playguard/games/checking", (int)this->check_done, (int)this->check_total));
    } else if (this->checking) {
        this->status_text("playguard/games/searching"_i18n);
    } else if (R_FAILED(this->list_rc)) {
        this->status_text("playguard/games/err_list"_i18n + " — " + ui::rc_text(this->list_rc), ui::color_bad());
    } else {
        std::string text = broken.empty() ? brls::getStr("playguard/games/none", (int)count) : "";
        const std::string age = this->age_text();
        if (!age.empty()) text += (text.empty() ? "" : " ") + age;
        if (this->cache.truncated) text += (text.empty() ? "" : "\n") + brls::getStr("playguard/games/truncated", (int)count);
        this->status_text(text, broken.empty() ? ui::color_ok() : ui::color_note());
    }
    header->setTitle(brls::getStr("playguard/games/section_count", (int)broken.size(), (int)count));
    ui::set_visible(header.getView(), !broken.empty());
    hint->setText(brls::getStr("playguard/games/many", ui::storage_short(si)));
    ui::set_visible(hint.getView(), broken.size() >= MANY);
}

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
        auto* note_ro = text_label("playguard/common/read_only_note"_i18n, 16, ui::color_note());
        note_ro->setMarginTop(8);
        box->addView(note_ro);
    }

    // Two buttons at most, side by side (a third goes on a row of its own):
    // the fix first, with the focus, then the other actions; B closes. With
    // no fix: Back, then the actions.
    auto* dialog = new brls::Dialog(box);
    const GameFacts g = game;
    const u32 others = actions & ~recommended;
    if (recommended) {
        GameContent update = {};
        for (u32 i = 0; i < g.content_count; i++)
            if (g.contents[i].type == GC_META_PATCH) update = g.contents[i];
        dialog->addButton(fix_button(recommended), [this, g, recommended, update]() { this->run(g, recommended, update); });
        if (others) dialog->addButton("playguard/games/other_actions"_i18n, [this, g, others]() { this->choose(g, others); });
    } else {
        dialog->addButton("hints/back"_i18n, []() {});
        if (others) dialog->addButton("playguard/games/actions"_i18n, [this, g, others]() { this->choose(g, others); });
    }
    dialog->open();
}

void GameCheckActivity::choose(const GameFacts& g, u32 actions)
{
    // What changes nothing or can be undone first, removals last, the whole
    // game at the very end.
    std::vector<std::string> labels;
    std::vector<std::function<void()>> runs;
    auto add = [&](u32 action, const GameContent& c) {
        labels.push_back(action_label(g, action, &c));
        runs.push_back([this, g, action, c]() { this->run(g, action, c); });
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
    ui::pick(game_name(g), labels, 0, [runs](int i) {
        if (i >= 0 && i < (int)runs.size()) runs[i]();
    });
}

void GameCheckActivity::run(const GameFacts& g, u32 action, const GameContent& content)
{
    switch (action) {
        case GA_RESET_LAUNCH:  this->reset_launch(g); break;
        case GA_VERIFY:        this->verify(g); break;
        case GA_REMOVE_UPDATE:
        case GA_REMOVE_ADDON:  this->remove_content(g, content); break;
        default:               this->remove(g); break;
    }
}

void GameCheckActivity::remove(const GameFacts& g)
{
    const std::string label = history_label(g), name = game_name(g);
    std::weak_ptr<bool> weak = this->alive;
    ui::confirm_danger(name + "\n\n" + "playguard/games/remove_body"_i18n, "playguard/games/remove"_i18n,
                       [this, weak, id = g.app_id, label, name]() {
                           const Result rc = gamecheck_remove(id);
                           if (R_SUCCEEDED(rc)) history_flow::record_event("remove_game", "", label);
                           ui::notify_result(rc, brls::getStr("playguard/games/removed", name), "playguard/games/remove_err"_i18n);
                           if (R_SUCCEEDED(rc) && !weak.expired()) this->refresh(false, id);
                       });
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

void GameCheckActivity::verify(const GameFacts& g)
{
    if (this->verifying) {
        ui::notify("playguard/games/verify_busy"_i18n);
        return;
    }
    this->verifying = true;

    // A dialog of its own: the progress, and Cancel (B too).
    auto state = std::make_shared<VerifyState>();
    state->name = game_name(g);
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(30, 40, 22, 40);
    state->label = text_label(brls::getStr("playguard/games/verifying", state->name, 0), 24, ui::color_text());
    state->label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    box->addView(state->label);
    auto* hint_label = text_label("playguard/games/verify_note"_i18n, 18, ui::color_note());
    hint_label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    hint_label->setMarginTop(14);
    box->addView(hint_label);
    state->dialog = new brls::Dialog(box);
    auto cancel = [state]() {
        state->cancel = true;
        state->open = false;   // the dialog closes itself
    };
    state->dialog->addButton("hints/cancel"_i18n, cancel);
    ui::on_cancel(state->dialog, cancel);
    state->dialog->open();

    // Minutes for a big game: a thread of its own, so the other reads on
    // borealis' queue do not wait behind it.
    std::weak_ptr<bool> weak = this->alive;
    const u64 checked_id = g.app_id;
    ui::in_background("verify", [this, weak, state, id = g.app_id, checked_id]() {
        bool corrupt = false;
        auto ctx = state;
        const Result rc = gamecheck_verify(id, on_verify_progress, &ctx, &corrupt);
        brls::sync([this, weak, state, rc, corrupt, checked_id]() {
            auto result = [state, rc, corrupt]() {
                if (rc == NXM_RC_CANCELLED) ui::notify("playguard/games/verify_cancelled"_i18n);
                else if (R_FAILED(rc)) ui::error("playguard/games/verify_err"_i18n + " — " + ui::rc_text(rc));
                else ui::info(brls::getStr(corrupt ? "playguard/games/verify_bad" : "playguard/games/verify_ok", state->name));
            };
            if (state->open) {
                state->open = false;
                state->dialog->close(result);
            } else {
                result();
            }
            if (weak.expired()) return;
            this->verifying = false;
            if (rc != NXM_RC_CANCELLED) this->refresh(false, checked_id);   // the console's view of it may have changed
        });
    });
}
