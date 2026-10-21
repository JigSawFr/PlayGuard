// The pieces Console › Check the installed games is made of
// (activity/game_check_activity.hpp): the words of a game and its problem,
// its details, and the list's cells and covers; shared by the list and the
// actions on the games (game_check_actions.cpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>
#include <vector>

#include "action/game_check_text.hpp"
#include "action/game_verify_flow.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "view/game_tile.hpp"

namespace game_check_parts
{
// The rules (sections, problems, keys, IDs): action/game_check_text.hpp.
using game_check_text::Broken;
using game_check_text::category_key;
using game_check_text::console_code;
using game_check_text::GROUPS;
using game_check_text::history_label;
using game_check_text::id_text;
using game_check_text::issue_text_key;
using game_check_text::MANY;
using game_check_text::required_hos;
using game_check_text::warn_only;
using game_verify_flow::Verified;

// Covers a row, as in All games: as many as the screen's width takes.
constexpr int COLUMNS = (int)((1280 - 2 * 60) / GameTile::WIDTH);

std::string game_name(const GameFacts& g);
// The cell's value: what the HOME menu shows, in a few words.
std::string short_text(const GameFacts& g, GameIssue issue);
// The screen's words, for what game_check_text puts together.
game_check_text::Words words();
// The games to install again as text, with their problem on this console.
std::string list_text(const std::vector<GameFacts>& games);
// The list of the games to install again could not be saved: why.
void list_error(const std::string& error);
// The fix's button: a few words (a dialog button is half the dialog wide).
std::string fix_button(u32 action);
std::string action_label(const GameFacts& g, u32 action, const GameContent* content);
std::string view_label(bool grid);
// The details: the icon, the name and the problem, then why and what to do
// in two blocks read from the left, the ID and version underneath.
brls::Box* details_view(const GameFacts& g, GameIssue issue, u32 current_hos, const std::vector<unsigned char>& icon);
std::vector<unsigned char> icon_of(u64 app_id);
// The Cadran look: a section's games in a card while they are a list (the
// covers stand on the page), its buttons rounded as the boards' are.
void card_look(brls::Box* list, bool grid);
void round_button(brls::Button* button);

// A game of the list: its name and problem, with borealis' check mark in
// front and the name in the value colour while it is picked (Ⓨ), as a
// RadioCell draws its choice.
class IssueCell : public brls::DetailCell
{
  public:
    IssueCell()
    {
        this->check = new brls::CheckBox();
        this->check->setMarginRight(16);
        this->addView(this->check, 0);
        ui::set_visible(this->check, false);
    }

    void set_selected(bool on)
    {
        ui::set_visible(this->check, on);
        this->setTextColor(on ? brls::Application::getTheme()["brls/list/listItem_value_color"] : cadran::kit::color("cadran/text"));
    }

  private:
    brls::CheckBox* check;
};

// The same game as a cover: its icon, name and problem, borealis' check
// mark over the icon while it is picked.
class IssueTile : public GameTile
{
  public:
    IssueTile()
    {
        this->check = new brls::CheckBox();
        this->check->setPositionType(brls::PositionType::ABSOLUTE);
        this->check->setPositionTop(14);
        this->check->setPositionRight(14);
        this->addView(this->check);
        ui::set_visible(this->check, false);
    }

    void set_selected(bool on)
    {
        ui::set_visible(this->check, on);
        this->set_name_color(on ? brls::Application::getTheme()["brls/list/listItem_value_color"] : cadran::kit::color("cadran/text"));
    }

    u64 app_id = 0;

  private:
    brls::CheckBox* check;
};
}   // namespace game_check_parts
