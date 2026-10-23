// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/game_check_parts.hpp"

#include <fmt/format.h>

#include "app.hpp"
#include "ui/cadran_list.hpp"

using namespace brls::literals;

namespace game_check_parts
{
namespace
{
constexpr float ICON_SIZE = 72;

// A game without a name: its ID.
std::string unnamed_text(const std::string& id)
{
    return brls::getStr("playguard/games/unnamed", id);
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

std::string cause_text(const GameFacts& g, GameIssue issue, u32 current_hos)
{
    const std::string key = issue_text_key(issue, "cause");
    switch (issue) {
        case GC_FIRMWARE_TOO_OLD: return brls::getStr(key, ui::hos_text(required_hos(g)), ui::hos_text(current_hos));
        case GC_KEY_TOO_NEW:      return brls::getStr(key, ui::hos_text(current_hos), (int)g.key_generation);
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
}   // namespace

std::string game_name(const GameFacts& g)
{
    return game_check_text::game_name(g, unnamed_text);
}

// The cell's value: what the HOME menu shows, in a few words.
std::string short_text(const GameFacts& g, GameIssue issue)
{
    const std::string key = issue_text_key(issue, "short");
    if (issue == GC_FIRMWARE_TOO_OLD) return brls::getStr(key, ui::hos_text(required_hos(g)));
    return brls::getStr(key);
}

// The screen's words, for what game_check_text puts together.
game_check_text::Words words()
{
    game_check_text::Words w;
    w.unnamed = unnamed_text;
    w.version = [](const std::string& v) { return brls::getStr("playguard/games/version", v); };
    w.problem = short_text;
    w.list_title = "playguard/games/list_title"_i18n;
    w.checked_now = "playguard/games/checked_now"_i18n;
    w.checked_min = [](int n) { return brls::getStr("playguard/games/checked_min", n); };
    w.checked_h = [](int n) { return brls::getStr("playguard/games/checked_h", n); };
    return w;
}

// The games to install again as text, with their problem on this console.
std::string list_text(const std::vector<GameFacts>& games)
{
    return game_check_text::list_text(games, sysinfo_hos_version(), words());
}

// The list of the games to install again could not be saved: why.
void list_error(const std::string& error)
{
    ui::error("playguard/games/list_err"_i18n, error);
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

std::string view_label(bool grid)
{
    return brls::getStr(fmt::format("playguard/library/views/{}", grid ? 1 : 0));
}

// The details: the icon, the name and the problem, then why and what to do
// in two blocks read from the left, the ID and version underneath.
brls::Box* details_view(const GameFacts& g, GameIssue issue, u32 current_hos, const std::vector<unsigned char>& icon)
{
    namespace kit = cadran::kit;
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);

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
    titles->addView(kit::display(game_name(g), 23));
    auto* problem = kit::label(short_text(g, issue), 17, warn_only(issue) ? "cadran/amber" : "cadran/coral");
    problem->setMarginTop(4);
    titles->addView(problem);
    head->addView(titles);
    box->addView(head);

    // Each block on the second surface, its title a kicker.
    brls::Theme theme = brls::Application::getTheme();
    auto block = [box, &theme](const std::string& title, const std::string& text) {
        auto* card = new brls::Box(brls::Axis::COLUMN);
        card->setBackgroundColor(theme["cadran/surf2"]);
        card->setCornerRadius(14);
        card->setPadding(14, 18, 16, 18);
        card->setMarginTop(14);
        brls::Label* t = kit::label(cadran::upper(title), 15, "cadran/text3");
        card->addView(t);
        brls::Label* body = kit::label(text, 18, "cadran/text");
        body->setMarginTop(14);   // the kicker apart from what it heads
        card->addView(body);
        box->addView(card);
    };
    block("playguard/games/cause"_i18n, cause_text(g, issue, current_hos));
    block("playguard/games/fix"_i18n, fix_text(g, issue, current_hos));

    std::string footer = brls::getStr("playguard/games/id", id_text(g.app_id));
    if (g.display_version[0]) footer += " · " + brls::getStr("playguard/games/version", std::string(g.display_version));
    if (g.verified == GC_VERIFIED_INTACT) footer += "\n" + brls::getStr("playguard/games/verified_intact", ui::time_text(g.verified_at));
    if (app::dev_mode()) footer += "\n" + developer_text(g);
    brls::Label* foot = kit::label(footer, 15.5f, "cadran/text3");
    foot->setMarginTop(14);
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

// The Cadran look: a section's games in a card while they are a list (the
// covers stand on the page), its buttons rounded as the boards' are.
void card_look(brls::Box* list, bool grid)
{
    auto* card = dynamic_cast<cadran::Card*>(list);
    if (!card) return;
    brls::Theme theme = brls::Application::getTheme();
    const NVGcolor none = nvgRGBA(0, 0, 0, 0);
    card->setFill(grid ? none : theme["cadran/surf"]);
    card->setAccent(grid ? none : theme["cadran/line"]);
}

void round_button(brls::Button* button)
{
    button->setHeight(48);
    button->setCornerRadius(24);
    button->setHighlightCornerRadius(24);
}
}   // namespace game_check_parts
