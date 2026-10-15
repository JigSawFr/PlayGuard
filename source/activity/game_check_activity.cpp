// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/game_check_activity.hpp"

#include <fmt/format.h>
#include <functional>
#include <string>
#include <vector>

#include "action/history_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
// From this many broken games, the cause is more likely the SD card or the
// storage the console started on than each game.
constexpr size_t MANY = 3;

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
        case GC_LAUNCH_VERSION:   return brls::getStr(key, (int)g.launch_required, (int)gamecheck_installed_version(&g));
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

struct VerifyProgress
{
    std::weak_ptr<bool> alive;
    brls::Label* status;
    std::string name;
};

void on_verify_progress(void* ctx, u64 current, u64 total)
{
    auto* p = (VerifyProgress*)ctx;
    const int percent = total ? (int)(current * 100 / total) : 0;
    const VerifyProgress copy = *p;
    brls::sync([copy, percent]() {
        if (copy.alive.expired()) return;
        copy.status->setText(brls::getStr("playguard/games/verifying", copy.name, percent > 100 ? 100 : percent));
    });
}
}   // namespace

void GameCheckActivity::onContentAvailable()
{
    note->setSingleLine(false);
    status->setSingleLine(false);
    hint->setSingleLine(false);
    this->scan();
}

void GameCheckActivity::scan()
{
    if (this->scanning) return;
    this->scanning = true;
    status->setText("playguard/games/scanning"_i18n);
    status->setTextColor(ui::color_text());
    ui::set_visible(status.getView(), true);
    std::weak_ptr<bool> weak = this->alive;
    brls::async([this, weak]() {
        auto check = std::make_shared<GameCheck>();
        gamecheck_scan(check.get());
        brls::sync([this, weak, check]() {
            if (weak.expired()) return;   // the screen was closed meanwhile
            this->scanning = false;
            this->show(*check);
        });
    });
}

void GameCheckActivity::show(const GameCheck& check)
{
    SysInfo si;
    sysinfo_get(&si);
    const u32 hos = si.hos_version;

    struct Broken { GameFacts game; GameIssue issue; };
    std::vector<Broken> broken;
    if (R_SUCCEEDED(check.rc)) {
        for (u32 i = 0; i < check.count; i++) {
            const GameIssue issue = gamecheck_classify(&check.games[i], hos);
            if (issue != GC_OK) broken.push_back({ check.games[i], issue });
        }
    }

    // The cells are reused in place, as in the History screen: one of them
    // may have the focus when this runs again after a change.
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
        const bool warn = b.issue == GC_STORAGE_UNAVAILABLE || b.issue == GC_FIRMWARE_TOO_OLD ||
                          b.issue == GC_KEY_TOO_NEW || b.issue == GC_LAUNCH_VERSION;
        cell->setDetailTextColor(warn ? ui::color_warn() : ui::color_bad());
        cell->registerClickAction([this, b, hos](brls::View*) {
            this->open(b.game, b.issue, hos);
            return true;
        });
    }
    for (size_t i = 0; i < broken.size() && i < cells.size(); i++) ui::set_visible(cells[i], true);
    for (size_t i = broken.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
    // Nothing could have the focus while the games were read.
    if (!this->focused && !broken.empty()) {
        this->focused = true;
        brls::Application::giveFocus(cells[0]);
    }

    std::string text;
    if (R_FAILED(check.rc)) {
        text = "playguard/games/err_list"_i18n + " — " + ui::rc_text(check.rc);
        status->setTextColor(ui::color_bad());
    } else if (broken.empty()) {
        text = brls::getStr("playguard/games/none", (int)check.count);
        status->setTextColor(ui::color_ok());
    } else {
        status->setTextColor(ui::color_text());
    }
    if (check.truncated) text += (text.empty() ? "" : "\n") + brls::getStr("playguard/games/truncated", (int)check.count);
    status->setText(text);
    ui::set_visible(status.getView(), !text.empty());
    ui::set_visible(header.getView(), !broken.empty());
    hint->setText(brls::getStr("playguard/games/many", ui::storage_short(si)));
    ui::set_visible(hint.getView(), broken.size() >= MANY);
}

void GameCheckActivity::open(const GameFacts& game, GameIssue issue, u32 current_hos)
{
    // An unnamed game is already named by its ID.
    std::string body = game_name(game);
    if (game.name[0]) body += "\n" + brls::getStr("playguard/games/id", id_text(game.app_id));
    body += "\n\n" + brls::getStr("playguard/common/line", "playguard/games/cause"_i18n, cause_text(game, issue, current_hos));
    body += "\n\n" + brls::getStr("playguard/common/line", "playguard/games/fix"_i18n, fix_text(game, issue, current_hos));
    if (app::dev_mode()) body += "\n\n" + developer_text(game);

    // Every action but the check changes the console.
    u32 actions = gamecheck_actions(&game, issue);
    const bool hidden = app::read_only() && (actions & ~GA_VERIFY);
    if (app::read_only()) actions &= GA_VERIFY;
    if (hidden) body += "\n\n" + "playguard/common/read_only_note"_i18n;
    if (!actions) {
        ui::info(body);
        return;
    }
    const GameFacts g = game;
    ui::confirm(body, "playguard/games/actions"_i18n, [this, g, actions]() { this->choose(g, actions); });
}

void GameCheckActivity::choose(const GameFacts& g, u32 actions)
{
    std::vector<std::string> labels;
    std::vector<std::function<void()>> runs;
    if (actions & GA_RESET_LAUNCH) {
        labels.push_back(brls::getStr("playguard/games/action/reset_launch", (int)gamecheck_installed_version(&g)));
        runs.push_back([this, g]() { this->reset_launch(g); });
    }
    for (u32 i = 0; i < g.content_count; i++) {
        const GameContent c = g.contents[i];
        if (c.storage != GC_STORAGE_NAND && c.storage != GC_STORAGE_SD) continue;
        if (c.type == GC_META_PATCH && (actions & GA_REMOVE_UPDATE)) {
            labels.push_back(brls::getStr("playguard/games/action/remove_update", (int)c.version));
            runs.push_back([this, g, c]() { this->remove_content(g, c); });
        }
        if (c.type == GC_META_ADDON && (actions & GA_REMOVE_ADDON)) {
            labels.push_back(brls::getStr("playguard/games/action/remove_addon", id_text(c.id)));
            runs.push_back([this, g, c]() { this->remove_content(g, c); });
        }
    }
    if (actions & GA_VERIFY) {
        labels.push_back("playguard/games/action/verify"_i18n);
        runs.push_back([this, g]() { this->verify(g); });
    }
    if (actions & GA_REMOVE) {
        labels.push_back("playguard/games/action/remove"_i18n);
        runs.push_back([this, g]() { this->remove(g); });
    }
    ui::pick(game_name(g), labels, 0, [runs](int i) {
        if (i >= 0 && i < (int)runs.size()) runs[i]();
    });
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
                           if (R_SUCCEEDED(rc) && !weak.expired()) this->scan();
                       });
}

void GameCheckActivity::remove_content(const GameFacts& g, const GameContent& c)
{
    const bool update = c.type == GC_META_PATCH;
    const std::string label = history_label(g), name = game_name(g);
    const std::string what = update ? brls::getStr("playguard/games/action/remove_update", (int)c.version)
                                    : brls::getStr("playguard/games/action/remove_addon", id_text(c.id));
    const std::string body = name + "\n" + what + "\n\n" +
                             (update ? "playguard/games/remove_update_body"_i18n : "playguard/games/remove_addon_body"_i18n);
    std::weak_ptr<bool> weak = this->alive;
    ui::confirm_danger(body, "playguard/games/remove"_i18n, [this, weak, id = g.app_id, c, update, label, name]() {
        const Result rc = gamecheck_remove_content(id, &c);
        if (R_SUCCEEDED(rc))
            history_flow::record_event(update ? "remove_update" : "remove_addon", "",
                                       update ? label : fmt::format("{} · {}", label, id_text(c.id)));
        ui::notify_result(rc, brls::getStr(update ? "playguard/games/update_removed" : "playguard/games/addon_removed", name),
                          "playguard/games/content_err"_i18n);
        if (R_SUCCEEDED(rc) && !weak.expired()) this->scan();
    });
}

void GameCheckActivity::reset_launch(const GameFacts& g)
{
    const u32 installed = gamecheck_installed_version(&g);
    const std::string label = history_label(g), name = game_name(g);
    const std::string body = name + "\n\n" +
                             brls::getStr("playguard/games/reset_launch_body", (int)g.launch_required, (int)installed);
    std::weak_ptr<bool> weak = this->alive;
    ui::confirm(body, "playguard/games/reset_launch_confirm"_i18n, [this, weak, id = g.app_id, installed, label, name]() {
        const Result rc = gamecheck_reset_launch_version(id, installed);
        if (R_SUCCEEDED(rc)) history_flow::record_event("launch_version", "", label);
        ui::notify_result(rc, brls::getStr("playguard/games/launch_reset", name), "playguard/games/launch_err"_i18n);
        if (R_SUCCEEDED(rc) && !weak.expired()) this->scan();
    });
}

void GameCheckActivity::verify(const GameFacts& g)
{
    if (this->verifying) {
        ui::notify("playguard/games/verify_busy"_i18n);
        return;
    }
    this->verifying = true;
    const std::string name = game_name(g);
    status->setText(brls::getStr("playguard/games/verifying", name, 0));
    status->setTextColor(ui::color_text());
    ui::set_visible(status.getView(), true);
    // Minutes for a big game: a thread of its own, so the other reads on
    // borealis' queue do not wait behind it.
    auto progress = std::make_shared<VerifyProgress>(VerifyProgress{ this->alive, status.getView(), name });
    std::weak_ptr<bool> weak = this->alive;
    ui::in_background("verify", [this, weak, progress, id = g.app_id, name]() {
        bool corrupt = false;
        const Result rc = gamecheck_verify(id, on_verify_progress, progress.get(), &corrupt);
        brls::sync([this, weak, rc, corrupt, name]() {
            if (R_FAILED(rc)) ui::error("playguard/games/verify_err"_i18n + " — " + ui::rc_text(rc));
            else ui::info(brls::getStr(corrupt ? "playguard/games/verify_bad" : "playguard/games/verify_ok", name));
            if (weak.expired()) return;
            this->verifying = false;
            this->scan();   // the console's view of it may have changed
        });
    });
}
