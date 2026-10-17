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
#include <strings.h>
#include <vector>

#include "action/game_check_text.hpp"
#include "action/game_notice.hpp"
#include "action/game_scan.hpp"
#include "action/history_flow.hpp"
#include "action/upload_flow.hpp"
#include "app.hpp"
#include "util/paths.hpp"
#include "util/table_export.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
// The rules (sections, problems, keys, IDs): action/game_check_text.hpp.
using game_check_text::category_key;
using game_check_text::console_code;
using game_check_text::GROUPS;
using game_check_text::history_label;
using game_check_text::id_text;
using game_check_text::issue_text_key;
using game_check_text::MANY;
using game_check_text::required_hos;
using game_check_text::warn_only;

constexpr float ICON_SIZE = 72;

std::string hos_text(u32 v)
{
    char buf[16];
    sysinfo_version_string(v, buf, sizeof(buf));
    return buf;
}

std::string game_name(const GameFacts& g)
{
    return g.name[0] ? std::string(g.name) : brls::getStr("playguard/games/unnamed", id_text(g.app_id));
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
    const std::string key = issue_text_key(issue, "short");
    if (issue == GC_FIRMWARE_TOO_OLD) return brls::getStr(key, hos_text(required_hos(g)));
    return brls::getStr(key);
}

std::string cause_text(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    const std::string key = issue_text_key(issue, "cause");
    switch (issue) {
        case GC_FIRMWARE_TOO_OLD: return brls::getStr(key, hos_text(required_hos(g)), hos_text(current_hos));
        case GC_KEY_TOO_NEW:      return brls::getStr(key, hos_text(current_hos), (int)g.key_generation);
        case GC_LAUNCH_VERSION:
            return brls::getStr(key, version_text(g, g.launch_required), version_text(g, gamecheck_installed_version(&g)));
        case GC_LAUNCH_REFUSED:   return brls::getStr(key, ui::rc_text(g.launch_rc));
        case GC_CORRUPT: {
            std::string text = brls::getStr(key, ui::time_text(g.verified_at));
            if (g.verified_code) text += " " + brls::getStr("playguard/games/verify_code", console_code(g.verified_code));
            return text;
        }
        default:                  return brls::getStr(key);
    }
}

std::string fix_text(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    return brls::getStr(game_check_text::fix_key(g, issue, current_hos));
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

// A game of the list: its name and problem, with borealis' check mark in
// front and the name in the value colour while it is picked (Ⓨ), as a
// RadioCell draws its choice.
class GameCell : public brls::DetailCell
{
  public:
    GameCell()
    {
        this->check = new brls::CheckBox();
        this->check->setMarginRight(16);
        this->addView(this->check, 0);
        ui::set_visible(this->check, false);
    }

    void set_selected(bool on)
    {
        ui::set_visible(this->check, on);
        this->setTextColor(on ? brls::Application::getTheme()["brls/list/listItem_value_color"] : ui::color_text());
    }

  private:
    brls::CheckBox* check;
};

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
    titles->addView(ui::text_label(game_name(g), 24, ui::color_text()));
    auto* problem = ui::text_label(short_text(g, issue), 19, warn_only(issue) ? ui::color_warn() : ui::color_bad());
    problem->setMarginTop(4);
    titles->addView(problem);
    head->addView(titles);
    box->addView(head);

    auto block = [box](const std::string& title, const std::string& text) {
        auto* t = ui::text_label(title, 18, ui::color_note());
        t->setMarginTop(18);
        box->addView(t);
        auto* body = ui::text_label(text, 20, ui::color_text());
        body->setMarginTop(4);
        box->addView(body);
    };
    block("playguard/games/cause"_i18n, cause_text(g, issue, current_hos));
    block("playguard/games/fix"_i18n, fix_text(g, issue, current_hos));

    std::string footer = brls::getStr("playguard/games/id", id_text(g.app_id));
    if (g.display_version[0]) footer += " · " + brls::getStr("playguard/games/version", std::string(g.display_version));
    if (g.verified == GC_VERIFIED_INTACT) footer += "\n" + brls::getStr("playguard/games/verified_intact", ui::time_text(g.verified_at));
    if (app::dev_mode()) footer += "\n" + developer_text(g);
    auto* foot = ui::text_label(footer, 18, ui::color_note());
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
// A plain progress bar: the track, then the part done in the icon's teal.
class ProgressBar : public brls::View
{
  public:
    ProgressBar()
        : track(ui::color_track()), done(ui::color_ok())
    {
        this->setHeight(12);
        this->setFocusable(false);
    }

    void set(float fraction)
    {
        this->fraction = fraction < 0 ? 0 : fraction > 1 ? 1 : fraction;
    }

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style, brls::FrameContext*) override
    {
        const float radius = height / 2.0f;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, width, height, radius);
        nvgFillColor(vg, this->track);
        nvgFill(vg);
        if (this->fraction <= 0) return;
        float w = width * this->fraction;
        if (w < height) w = height;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, height, radius);
        nvgFillColor(vg, this->done);
        nvgFill(vg);
    }

  private:
    float fraction = 0;
    NVGcolor track, done;
};

// "850 MB", "3.8 GB" (the language's decimal mark).
std::string size_text(u64 bytes)
{
    const u64 mb = bytes >> 20;
    if (mb < 1024) return brls::getStr("playguard/games/size_mb", (int)mb);
    std::string gb = fmt::format("{:.1f}", (double)bytes / (double)(1ULL << 30));
    const std::string mark = "playguard/games/decimal_mark"_i18n;
    const size_t dot = gb.find('.');
    if (dot != std::string::npos) gb.replace(dot, 1, mark);
    return brls::getStr("playguard/games/size_gb", gb);
}

// The integrity check's dialog and what its thread shares with it.
struct VerifyState
{
    std::atomic<bool> cancel{ false };
    bool open = true;   // the dialog is on screen (main thread only)
    brls::Dialog* dialog = nullptr;
    brls::Image* image = nullptr;
    brls::Label* title = nullptr;   // "Checking the data", with the game's place in a batch
    brls::Label* name = nullptr;
    ProgressBar* bar = nullptr;
    brls::Label* percent = nullptr;
    brls::Label* detail = nullptr;
    std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
};

// What the check of one game of a batch gave.
struct Verified
{
    GameFacts game;
    Result rc = 0;
    bool corrupt = false;
    Result found = 0;
};

// What the dialog says at `current` of `total` bytes: the share done, how
// much, and the time left once it can be told (a few seconds in).
void show_progress(VerifyState* s, u64 current, u64 total)
{
    if (!total) {
        s->bar->set(0);
        s->percent->setText("playguard/games/verify_preparing"_i18n);
        s->detail->setText("");
        return;
    }
    if (current > total) current = total;
    s->bar->set((float)((double)current / (double)total));
    s->percent->setText(fmt::format("{} %", (int)(current * 100 / total)));
    std::string detail = brls::getStr("playguard/games/verify_amount", size_text(current), size_text(total));
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - s->started).count();
    if (current > 0 && elapsed >= 3) {
        const double left = elapsed * (double)(total - current) / (double)current;
        detail += " · " + (left < 60 ? "playguard/games/verify_left_less"_i18n
                                      : brls::getStr("playguard/games/verify_left_min", (int)(left / 60 + 0.5)));
    }
    s->detail->setText(detail);
}

bool on_verify_progress(void* ctx, u64 current, u64 total)
{
    auto* state = (std::shared_ptr<VerifyState>*)ctx;
    const std::shared_ptr<VerifyState> s = *state;
    brls::sync([s, current, total]() {
        if (s->open) show_progress(s.get(), current, total);
    });
    return !s->cancel.load();
}
}   // namespace

void GameCheckActivity::onContentAvailable()
{
    note->setSingleLine(false);
    status->setSingleLine(false);
    hint->setSingleLine(false);
    // The games picked with Ⓨ: how many, and A for what to do with them.
    this->selection = new brls::DetailCell();
    this->selection->setMarginTop(10);
    this->selection->setDetailText("playguard/games/actions"_i18n);
    this->selection->registerClickAction([this](brls::View*) {
        this->selection_menu();
        return true;
    });
    list->addView(this->selection);
    ui::set_visible(this->selection, false);
    // A section per way out, under its title and what to do; each hidden
    // while it has no game.
    for (GameCategory category : GROUPS) {
        Group& group = this->groups[category];
        group.box = new brls::Box(brls::Axis::COLUMN);
        group.box->setAlignItems(brls::AlignItems::STRETCH);
        group.title = new brls::Header();
        group.title->setMarginBottom(4);
        group.title->setMarginTop(category == GROUPS[0] ? 10 : 20);
        group.box->addView(group.title);
        group.note = ui::text_label(brls::getStr(std::string("playguard/games/group_note/") + category_key(category)), 18, ui::color_note());
        group.note->setMarginBottom(8);
        group.box->addView(group.note);
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
                std::string error;
                const std::string path = this->save_list(this->groups[category].games, &error);
                if (path.empty()) ui::error("playguard/games/list_err"_i18n + (error.empty() ? "" : ": " + error));
                else ui::notify(brls::getStr("playguard/games/list_saved", path));
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
                upload_flow::send_list("games_to_reinstall.txt", this->list_text(this->groups[category].games),
                                       "games to reinstall");
                return true;
            });
            actions->addView(group.send);
            group.remove_all = new brls::Button();
            group.remove_all->setGrow(1.0f);
            group.remove_all->setTextColor(ui::color_bad());
            group.remove_all->registerClickAction([this, category](brls::View*) {
                this->remove_games(this->groups[category].games);
                return true;
            });
            actions->addView(group.remove_all);
            group.box->addView(actions);
        }
        // The data to check: every game of the section, one after the
        // other, above the games as the Reinstall actions are.
        if (category == GC_CAT_VERIFY) {
            group.verify_all = new brls::Button();
            group.verify_all->setGrow(1.0f);
            group.verify_all->setMarginBottom(12);
            group.verify_all->registerClickAction([this, category](brls::View*) {
                this->verify(this->groups[category].games);
                return true;
            });
            group.box->addView(group.verify_all);
        }
        group.list = new brls::Box(brls::Axis::COLUMN);
        group.list->setAlignItems(brls::AlignItems::STRETCH);
        group.box->addView(group.list);
        list->addView(group.box);
        ui::set_visible(group.box, false);
    }
    // Ⓧ: every game checked again, whatever the cache says.
    this->getContentView()->registerAction("playguard/games/check_again"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->refresh(true, 0);
        return true;
    });

    // The last check at once, then what changed since.
    if (game_scan::load(&this->cache)) this->show();
    this->refresh(false, 0);
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
    this->status_text("playguard/games/searching"_i18n);
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
        else this->status_text(brls::getStr("playguard/games/checking", (int)this->check_done, (int)this->check_total));
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
    std::vector<Broken> by_group[GC_CAT_COUNT];
    for (const GameFacts& g : this->cache.games) {
        const GameIssue issue = gamecheck_classify(&g, hos);
        if (issue == GC_OK) continue;
        broken.push_back({ g, issue });
        by_group[gamecheck_category(&g, issue, hos)].push_back({ g, issue });
    }
    const u32 count = (u32)this->cache.games.size();
    // The bell follows every check, repairs included (not halfway through one).
    if (!this->checking && !this->list_rc) game_notice::note(this->cache.games, hos);
    // In a section, the games with the same problem together, by name.
    for (auto& games : by_group)
        std::stable_sort(games.begin(), games.end(), [](const Broken& a, const Broken& b) {
            if (a.issue != b.issue) return a.issue < b.issue;
            return strcasecmp(game_name(a.game).c_str(), game_name(b.game).c_str()) < 0;
        });

    // Two games with the same name (two regions, a demo): told apart by
    // their version, else by their ID.
    std::map<std::string, std::vector<const GameFacts*>> same_name;
    for (const Broken& b : broken) same_name[game_name(b.game)].push_back(&b.game);
    auto line_name = [&same_name](const GameFacts& g) {
        const std::string name = game_name(g);
        const auto& twins = same_name[name];
        if (twins.size() < 2 || !g.name[0]) return name;
        bool versions_differ = g.display_version[0] != '\0';
        for (const GameFacts* t : twins)
            if (t->app_id != g.app_id && !std::strcmp(t->display_version, g.display_version)) versions_differ = false;
        return name + " · " + (versions_differ ? std::string(g.display_version) : id_text(g.app_id));
    };

    // The games picked that left the list are no longer picked.
    std::set<u64> still;
    for (const Broken& b : broken)
        if (this->selected.count(b.game.app_id)) still.insert(b.game.app_id);
    this->selected = std::move(still);

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
        const std::vector<Broken>& shown = by_group[category];
        Group& group = this->groups[category];
        auto& cells = group.list->getChildren();
        group.games.clear();
        for (size_t i = 0; i < shown.size(); i++) {
            GameCell* cell;
            if (i < cells.size()) {
                cell = (GameCell*)cells[i];
            } else {
                cell = new GameCell();
                group.list->addView(cell);
            }
            const Broken b = shown[i];
            const bool picked = this->selected.count(b.game.app_id) != 0;
            group.games.push_back(b.game);
            cell->setText(line_name(b.game));
            cell->set_selected(picked);
            cell->setDetailText(short_text(b.game, b.issue));
            cell->setDetailTextColor(warn_only(b.issue) ? ui::color_warn() : ui::color_bad());
            cell->registerClickAction([this, b, hos](brls::View*) {
                this->open(b.game, b.issue, hos);
                return true;
            });
            cell->registerAction(picked ? "playguard/games/unselect"_i18n : "playguard/games/select"_i18n, brls::BUTTON_Y,
                                 [this, id = b.game.app_id](brls::View*) {
                                     this->toggle(id);
                                     return true;
                                 });
            ui::set_visible(cell, true);
        }
        for (size_t i = shown.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
        group.title->setTitle(brls::getStr(std::string("playguard/games/groups/") + category_key(category), (int)shown.size()));
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
    // Above the sections from two games on: what is picked, or how to pick.
    this->selection->setText(this->selection_text());
    this->selection->setTextColor(this->selected.empty() ? ui::color_text()
                                                         : brls::Application::getTheme()["brls/list/listItem_value_color"]);
    ui::set_visible(this->selection, broken.size() >= 2 || !this->selected.empty());

    // Nothing had the focus before the first game with a problem; and never
    // the focus left in a section that went (its last game repaired). While
    // the games are checked, until the user moves it, the focus stays on the
    // first game: the games found later go into the sections above and would
    // push the cell it was given below the screen (the scroll does not follow).
    brls::View* focus = brls::Application::getCurrentFocus();
    bool lost = false;
    for (brls::View* v = focus; v && !lost; v = v->getParent()) lost = v->getVisibility() != brls::Visibility::VISIBLE;
    brls::View* first = this->first_game();
    const bool untouched = this->auto_focus && focus == this->auto_focus;
    if (first && (!this->focused || lost || untouched)) {
        this->focused = true;
        this->auto_focus = first;
        brls::Application::giveFocus(first);
    }
    // The check over (this was its last show): the focus is the user's.
    if (!this->checking) this->auto_focus = nullptr;

    // The line above the list: the check under way, an error, or the result
    // and when it was made.
    if (this->checking && this->check_total) {
        this->status_text(brls::getStr("playguard/games/checking", (int)this->check_done, (int)this->check_total));
    } else if (this->checking) {
        this->status_text("playguard/games/searching"_i18n);
    } else if (R_FAILED(this->list_rc)) {
        this->status_text("playguard/games/err_list"_i18n + " — " + ui::rc_text(this->list_rc), ui::color_bad());
    } else {
        std::string text = broken.empty() ? brls::getStr("playguard/games/none", (int)count)
                                          : brls::getStr("playguard/games/section_count", (int)broken.size(), (int)count);
        const std::string age = this->age_text();
        if (!age.empty()) text += (text.empty() ? "" : broken.empty() ? " " : " · ") + age;
        if (this->cache.truncated) text += (text.empty() ? "" : "\n") + brls::getStr("playguard/games/truncated", (int)count);
        if (app::dev_mode() && !this->timing.empty()) text += (text.empty() ? "" : "\n") + this->timing;
        this->status_text(text, broken.empty() ? ui::color_ok() : ui::color_note());
    }
    hint->setText(brls::getStr("playguard/games/many", ui::storage_short(si)));
    ui::set_visible(hint.getView(), broken.size() >= MANY);
}

// The first game of the list, in the first section shown.
brls::View* GameCheckActivity::first_game() const
{
    for (GameCategory category : GROUPS) {
        const Group& group = this->groups[category];
        if (group.box->getVisibility() != brls::Visibility::VISIBLE) continue;
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
    ui::keep_focus(this, this->first_game());
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
        auto* note_ro = ui::text_label("playguard/common/read_only_note"_i18n, 18, ui::color_note());
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
        case GA_VERIFY:        this->verify({ g }); break;
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

// The games to install again (a section's, or those picked) as plain text:
// the title, then a line each with its ID, its name (as an installer searches
// it), its version and what was wrong.
std::string GameCheckActivity::list_text(const std::vector<GameFacts>& games) const
{
    SysInfo si;
    sysinfo_get(&si);
    std::string text = "playguard/games/list_title"_i18n + "\n\n";
    for (const GameFacts& g : games) {
        const GameIssue issue = gamecheck_classify(&g, si.hos_version);
        text += fmt::format("{}  {}{} · {}\n", id_text(g.app_id), game_name(g),
                            g.display_version[0] ? " · " + brls::getStr("playguard/games/version", std::string(g.display_version)) : "",
                            short_text(g, issue));
    }
    return text;
}

// The same games in a JSON table (exports/, as the Activity tab's exports)
// and that text next to it.
std::string GameCheckActivity::save_list(const std::vector<GameFacts>& games, std::string* error) const
{
    SysInfo si;
    sysinfo_get(&si);
    table_export::Table t;
    t.title = "playguard/games/list_title"_i18n;
    t.sheet = t.title;
    t.columns = { { "name", "playguard/games/col_name"_i18n, false },
                  { "title_id", "playguard/games/col_id"_i18n, false },
                  { "version", "playguard/games/col_version"_i18n, false },
                  { "problem", "playguard/games/col_problem"_i18n, false } };
    for (const GameFacts& g : games) {
        const GameIssue issue = gamecheck_classify(&g, si.hos_version);
        t.rows.push_back({ game_name(g), id_text(g.app_id), g.display_version, short_text(g, issue) });
    }
    const std::string text = this->list_text(games);
    if (!paths::ensure_dir(paths::exports_dir())) {
        if (error) *error = paths::exports_dir();
        return "";
    }
    const std::string path = table_export::save(t, table_export::Format::Json, paths::exports_dir(), "games_to_reinstall", error);
    if (path.empty()) return "";
    const std::string txt = path.substr(0, path.size() - 4) + "txt";
    if (!paths::atomic_write(txt, text, error)) return "";
    return path;
}

void GameCheckActivity::remove_games(std::vector<GameFacts> games)
{
    if (games.empty() || this->removing) return;
    std::string names;
    for (size_t i = 0; i < games.size() && i < 6; i++) names += "\n" + game_name(games[i]);
    if (games.size() > 6) names += "\n…";
    const std::string body = brls::getStr("playguard/games/remove_all_body", (int)games.size()) + "\n" + names;
    std::weak_ptr<bool> weak = this->alive;
    ui::confirm_danger(body, "playguard/games/remove"_i18n, [this, weak, games]() {
        if (weak.expired()) return;
        // The list first: nothing is removed without it.
        std::string error;
        const std::string path = this->save_list(games, &error);
        if (path.empty()) {
            ui::error("playguard/games/list_err"_i18n + (error.empty() ? "" : ": " + error));
            return;
        }
        // Read-only mode and the PIN: asked once, for them all.
        const Result rc = core_change_allowed();
        if (R_FAILED(rc)) {
            ui::notify_result(rc, "", "playguard/games/remove_err"_i18n);
            return;
        }
        this->removing = true;
        this->remove_next(games, 0, 0, path);
    });
}

// One game a frame, the count above the list in between.
void GameCheckActivity::remove_next(std::vector<GameFacts> games, size_t at, u32 removed, std::string path)
{
    if (at == games.size()) {
        this->removing = false;
        const u32 total = (u32)games.size();
        if (removed == total) ui::notify(brls::getStr("playguard/games/removed_all", (int)removed, path));
        else ui::error(brls::getStr("playguard/games/removed_some", (int)removed, (int)total, path));
        this->refresh(false, 0);
        return;
    }
    this->status_text(brls::getStr("playguard/games/removing", (int)at + 1, (int)games.size()));
    std::weak_ptr<bool> weak = this->alive;
    brls::sync([this, weak, games, at, removed, path]() {
        if (weak.expired()) return;
        const GameFacts& g = games[at];
        const Result rc = gamecheck_remove(g.app_id);
        u32 done = removed;
        if (R_SUCCEEDED(rc)) {
            history_flow::record_event("remove_game", "", history_label(g));
            done++;
        } else {
            brls::Logger::warning("games: {} not removed ({})", id_text(g.app_id), ui::rc_text(rc));
        }
        this->remove_next(games, at + 1, done, path);
    });
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
    SysInfo si;
    sysinfo_get(&si);
    const std::vector<GameFacts> games = this->selected_games();
    std::vector<GameFacts> to_verify, to_remove;
    for (const GameFacts& g : games) {
        const u32 actions = gamecheck_actions(&g, gamecheck_classify(&g, si.hos_version));
        if (actions & GA_VERIFY) to_verify.push_back(g);
        if (actions & GA_REMOVE) to_remove.push_back(g);
    }
    std::vector<std::string> labels;
    std::vector<std::function<void()>> runs;
    if (!to_verify.empty()) {
        labels.push_back(brls::getStr("playguard/games/selection/verify", (int)to_verify.size()));
        runs.push_back([this, to_verify]() { this->verify(to_verify); });
    }
    if (!to_remove.empty() && !app::read_only()) {
        labels.push_back(brls::getStr("playguard/games/selection/remove", (int)to_remove.size()));
        runs.push_back([this, to_remove]() { this->remove_games(to_remove); });
    }
    for (GameCategory category : GROUPS) {
        const Group& group = this->groups[category];
        bool all = true;
        for (const GameFacts& g : group.games) all &= this->selected.count(g.app_id) != 0;
        if (group.games.empty() || all) continue;
        labels.push_back(brls::getStr("playguard/games/selection/select_group",
                                      brls::getStr(std::string("playguard/games/groups/") + category_key(category), (int)group.games.size())));
        runs.push_back([this, category]() {
            for (const GameFacts& g : this->groups[category].games) this->selected.insert(g.app_id);
            this->show();
        });
    }
    if (!this->selected.empty()) {
        labels.push_back("playguard/games/selection/clear"_i18n);
        runs.push_back([this]() {
            this->selected.clear();
            this->show();
        });
    }
    ui::pick(this->selection_text(), labels, 0, [runs](int i) {
        if (i >= 0 && i < (int)runs.size()) runs[i]();
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

void GameCheckActivity::verify(std::vector<GameFacts> games)
{
    if (games.empty()) return;
    if (this->verifying) {
        ui::notify("playguard/games/verify_busy"_i18n);
        return;
    }
    this->verifying = true;
    const size_t total = games.size();

    // A dialog of its own: the game (icon, name on one line), its place in a
    // batch, the bar, the share done with how much and the time left, and
    // Cancel (B too), which stops the batch after what was checked.
    auto state = std::make_shared<VerifyState>();
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(26, 36, 20, 36);

    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    state->image = new brls::Image();
    state->image->setScalingType(brls::ImageScalingType::FIT);
    state->image->setWidth(56);
    state->image->setHeight(56);
    state->image->setMarginRight(16);
    head->addView(state->image);
    auto* titles = new brls::Box(brls::Axis::COLUMN);
    titles->setGrow(1.0f);
    titles->setShrink(1.0f);
    state->title = ui::text_label("playguard/games/verify_title"_i18n, 18, ui::color_note());
    titles->addView(state->title);
    state->name = ui::text_label("", 22, ui::color_text());
    state->name->setSingleLine(true);   // a long name is cut, not wrapped over the dialog
    state->name->setMarginTop(2);
    titles->addView(state->name);
    head->addView(titles);
    box->addView(head);

    state->bar = new ProgressBar();
    state->bar->setMarginTop(22);
    box->addView(state->bar);
    auto* numbers = new brls::Box(brls::Axis::ROW);
    numbers->setAlignItems(brls::AlignItems::CENTER);
    numbers->setMarginTop(10);
    state->percent = ui::text_label("", 22, ui::color_text());
    state->percent->setSingleLine(true);
    state->percent->setMarginRight(14);
    numbers->addView(state->percent);
    state->detail = ui::text_label("", 18, ui::color_note());
    state->detail->setSingleLine(true);
    state->detail->setGrow(1.0f);
    state->detail->setShrink(1.0f);
    state->detail->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
    numbers->addView(state->detail);
    box->addView(numbers);

    auto* note_label = ui::text_label("playguard/games/verify_note"_i18n, 18, ui::color_note());
    note_label->setMarginTop(16);
    box->addView(note_label);
    state->dialog = new brls::Dialog(box);
    auto cancel = [state]() {
        state->cancel = true;
        state->open = false;   // the dialog closes itself
    };
    state->dialog->addButton("hints/cancel"_i18n, cancel);
    ui::on_cancel(state->dialog, cancel);

    // The game under way, in the dialog (main thread).
    auto start = [state, total](const GameFacts& g, size_t at) {
        if (!state->open) return;
        const std::vector<unsigned char> icon = icon_of(g.app_id);
        if (!icon.empty()) state->image->setImageFromMem(icon.data(), (int)icon.size());
        ui::set_visible(state->image, !icon.empty());
        state->title->setText(total == 1 ? "playguard/games/verify_title"_i18n
                                         : brls::getStr("playguard/games/verify_title_n", (int)at + 1, (int)total));
        state->name->setText(game_name(g));
        state->started = std::chrono::steady_clock::now();
        show_progress(state.get(), 0, 0);
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
            auto ctx = state;
            v.rc = gamecheck_verify(g.app_id, on_verify_progress, &ctx, &v.corrupt, &v.found);
            if (v.rc == NXM_RC_CANCELLED) break;
            results->push_back(v);
            if (R_FAILED(v.rc)) continue;
            u64 now = 0;
            time_local_now(&now, nullptr);
            brls::sync([this, weak, v, now]() {
                if (weak.expired()) return;
                if (game_cache::set_verified(&this->cache, v.game.app_id, v.corrupt ? GC_VERIFIED_CORRUPT : GC_VERIFIED_INTACT,
                                             (int64_t)now, v.found))
                    this->save();
            });
        }
        brls::sync([this, weak, state, results, total = games.size()]() {
            auto result = [this, weak, state, results, total]() {
                if (results->empty()) {
                    ui::notify("playguard/games/verify_cancelled"_i18n);
                    return;
                }
                if (total == 1) {
                    // What the console found, with its code as the HOME menu
                    // would show it, and the way out at hand.
                    const Verified& v = results->front();
                    const std::string name = game_name(v.game);
                    if (R_FAILED(v.rc)) {
                        ui::error("playguard/games/verify_err"_i18n + " — " + ui::rc_text(v.rc));
                    } else if (!v.corrupt) {
                        ui::info(brls::getStr("playguard/games/verify_ok", name));
                    } else {
                        std::string body = brls::getStr("playguard/games/verify_bad", name);
                        if (v.found) body += "\n\n" + brls::getStr("playguard/games/verify_code", console_code(v.found));
                        if (app::read_only() || weak.expired()) ui::info(body);
                        else ui::confirm(body, "playguard/games/fix_button/remove"_i18n, [this, g = v.game]() { this->remove(g); });
                    }
                    return;
                }
                // A batch: how many of each, the corrupt ones by name, and
                // removing them all at hand.
                std::vector<GameFacts> corrupt;
                std::string names;
                size_t intact = 0, failed = 0;
                for (const Verified& v : *results) {
                    if (R_FAILED(v.rc)) {
                        failed++;
                    } else if (v.corrupt) {
                        corrupt.push_back(v.game);
                        if (corrupt.size() <= 6) names += "\n" + game_name(v.game);
                    } else {
                        intact++;
                    }
                }
                if (corrupt.size() > 6) names += "\n…";
                std::string body = brls::getStr("playguard/games/verify_done", (int)results->size(), (int)total);
                body += "\n\n" + brls::getStr("playguard/games/verify_count_bad", (int)corrupt.size()) + names;
                body += "\n" + brls::getStr("playguard/games/verify_count_ok", (int)intact);
                if (failed) body += "\n" + brls::getStr("playguard/games/verify_count_err", (int)failed);
                if (corrupt.empty() || app::read_only() || weak.expired()) ui::info(body);
                else
                    ui::confirm(body,
                                corrupt.size() == 1 ? "playguard/games/remove_corrupt_one"_i18n
                                                    : brls::getStr("playguard/games/remove_corrupt", (int)corrupt.size()),
                                [this, corrupt]() { this->remove_games(corrupt); });
            };
            if (state->open) {
                state->open = false;
                state->dialog->close(result);
            } else {
                result();
            }
            if (weak.expired()) return;
            this->verifying = false;
            // The console's view of them may have changed: checked again.
            for (const Verified& v : *results) this->waiting.push_back(v.game.app_id);
            this->refresh(false, 0);
        });
    });
}
