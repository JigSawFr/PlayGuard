// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_lists.hpp"

#include <algorithm>
#include <yoga/Yoga.h>

#include "app.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace
{
namespace kit = cadran::kit;

// A card of the tops (not a list: no rows), `grow` shares a row.
brls::Box* hero_card(bool grow)
{
    brls::Box* card = kit::card(false, grow);
    card->setCornerRadius(22);
    card->setPadding(22, 26, 22, 26);
    return card;
}

// A kicker: an icon and a title in capitals.
brls::Box* kicker(const char* icon, const std::string& title)
{
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    head->addView(kit::icon(icon, 18, "cadran/text3"));
    brls::Label* text = kit::label(cadran::upper(title), 15, "cadran/text3");
    text->setSingleLine(true);
    text->setMarginLeft(8);
    head->addView(text);
    return head;
}

// "2026-10-08 16:00:00" -> { "2026-10-08", "16:00:00" }; else { "", text }.
std::pair<std::string, std::string> split_time(const std::string& text)
{
    const size_t space = text.find(' ');
    if (space == std::string::npos || space != 10) return { "", text };
    return { text.substr(0, space), text.substr(space + 1) };
}

std::string detail_of(brls::View* tab, const char* id)
{
    auto* cell = dynamic_cast<brls::DetailCell*>(tab->getView(id));
    return cell ? cell->detail->getFullText() : "";
}
}   // namespace

// ---------------------------------------------------------------------------
// The network clock: both clocks large, the network one amber while it is
// inaccurate. A screen of its own (ClockTab::open_screen).

CadranClockTab::CadranClockTab()
    : list(this, true)
{
    if (brls::Box* column = this->list.content()) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::STRETCH);
        row->setMarginBottom(12);

        brls::Box* console = hero_card(true);
        console->setMarginRight(12);
        console->addView(kicker("gamepad", "playguard/cadran/clock/console"_i18n));
        this->user_time = new CadranDigits();
        this->user_time->setFontSize(46);
        this->user_time->setMarginTop(10);
        console->addView(this->user_time);
        this->user_date = kit::label("", 15.5f, "cadran/text2");
        this->user_date->setMarginTop(4);
        console->addView(this->user_date);
        row->addView(console);

        this->network_card = hero_card(true);
        this->network_card->addView(kicker("globe", "playguard/cadran/clock/network"_i18n));
        this->network_time = new CadranDigits();
        this->network_time->setFontSize(46);
        this->network_time->setMarginTop(10);
        this->network_card->addView(this->network_time);
        auto* state = new brls::Box(brls::Axis::ROW);
        state->setAlignItems(brls::AlignItems::CENTER);
        state->setMarginTop(4);
        this->network_dot = new brls::Rectangle(brls::Application::getTheme()["cadran/teal_fill"]);
        this->network_dot->setWidth(9);
        this->network_dot->setHeight(9);
        this->network_dot->setCornerRadius(4.5f);
        this->network_dot->setMarginRight(8);
        state->addView(this->network_dot);
        this->network_state = kit::label("", 15.5f, "cadran/teal");
        state->addView(this->network_state);
        this->network_card->addView(state);
        row->addView(this->network_card);

        // In place of the three readings it shows.
        brls::View* first = this->getView("ck_user");
        const auto& lines = column->getChildren();
        const auto at = std::find(lines.begin(), lines.end(), first);
        column->addView(row, at == lines.end() ? 0 : (size_t)(at - lines.begin()));
        this->list.park({ "ck_user", "ck_network", "ck_accuracy" });
    }
    this->list.apply({ { "ck_auto", "refresh" }, { "ck_zone", "pin" }, { "ck_region", "globe" }, { "ck_server", "server" },
                       { "ck_custom", "pencil" }, { "ck_measure", "timer" }, { "ck_apply", "check" }, { "ck_copy", "today" },
                       { "ck_check_at_start", "bolt" } });
}

void CadranClockTab::refresh()
{
    ClockTab::refresh();
    this->list.sync();
    if (!this->user_time) return;
    brls::Theme theme = brls::Application::getTheme();
    const auto user = split_time(detail_of(this, "ck_user"));
    const auto network = split_time(detail_of(this, "ck_network"));
    this->user_time->setText(user.second);
    this->user_date->setText(user.first.empty() ? "" : kit::long_date(user.first));
    this->network_time->setText(network.second);

    const std::string accuracy = detail_of(this, "ck_accuracy");
    const bool known = accuracy == "playguard/common/yes"_i18n || accuracy == "playguard/common/no"_i18n;
    const bool bad = accuracy == "playguard/common/no"_i18n;
    std::string state = !known ? accuracy : bad ? "playguard/cadran/clock/inaccurate"_i18n : "playguard/cadran/clock/accurate"_i18n;
    if (known && !network.first.empty() && network.first != user.first) state += " · " + kit::long_date(network.first);
    this->network_state->setText(state);
    this->network_state->setTextColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber"] : theme["cadran/teal"]);
    this->network_dot->setColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber_fill"] : theme["cadran/teal_fill"]);
    this->network_time->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text"]);
    this->network_card->setBackgroundColor(bad ? theme["cadran/amber_surf"] : theme["cadran/surf"]);
    this->network_card->setBorderColor(bad ? theme["cadran/amber_line"] : theme["cadran/line"]);
}

brls::View* CadranClockTab::create() { return new CadranClockTab(); }

// ---------------------------------------------------------------------------
// PIN: the three states as tiles, the deletion in a coral card.

CadranSecurityTab::CadranSecurityTab()
    : list(this)
{
    this->list.tiles({ { "sc_pin", "key" }, { "sc_restrictions", "shield" }, { "sc_temp", "unlock" } }, true);
    this->list.apply({ { "sc_set_pin", "key" }, { "sc_pin_lock", "lock" }, { "sc_pin_ask_again", "timer" },
                       { "sc_show_pin", "eye" }, { "sc_unlock", "unlock" }, { "sc_relock", "lock" },
                       { "sc_auto_relock", "refresh" }, { "sc_console_lock", "gamepad" }, { "pr_active", "phone" },
                       { "pr_updated", "refresh" }, { "pr_decode", "code" }, { "pr_unlink", "stop" }, { "sc_delete", "trash" } });
    if (cadran::Card* danger = this->list.card_of("sc_delete"))
        danger->setAccent(brls::Application::getTheme()["cadran/coral_line"]);
}

void CadranSecurityTab::refresh()
{
    SecurityTab::refresh();
    this->list.sync();
    const std::string pin = detail_of(this, "sc_pin");
    if (detail_of(this, "sc_temp") == "playguard/common/yes"_i18n) cadran::set_pill("playguard/cadran/pill/unlocked"_i18n, "unlock", true);
    else if (pin == "playguard/dashboard/pin_set"_i18n) cadran::set_pill("playguard/cadran/pill/pin_set"_i18n, "key");
    else if (pin == "playguard/common/not_set"_i18n) cadran::set_pill("playguard/cadran/pill/pin_none"_i18n, "key", true);
    else cadran::set_pill("");
}

brls::View* CadranSecurityTab::create() { return new CadranSecurityTab(); }

// ---------------------------------------------------------------------------
// Console: what the console runs as tiles at the top (the firmware,
// Atmosphère, hekate when it is there), then the lines; compatibility the
// first of the console's.

CadranConsoleTab::CadranConsoleTab()
    : list(this)
{
    if (brls::Box* tiles = this->list.tiles({ { "cn_fw", "chip" }, { "cn_ams", "bolt" }, { "cn_hekate", "flag" } }, true)) {
        // Spaced by the row, not by their margins: hekate's tile comes and goes.
        for (brls::View* tile : tiles->getChildren()) tile->setMarginRight(0);
        YGNodeStyleSetGap(tiles->getYGNode(), YGGutterColumn, 12);
        tiles->setMarginBottom(20);
        if (brls::Box* column = tiles->getParent()) {
            column->removeView(tiles, false);
            column->addView(tiles, 0);
        }
    }
    this->list.apply({ { "cn_clock", "globe" }, { "cn_games", "gamepad" }, { "cn_compat", "check" }, { "cn_storage", "folder" },
                       { "cn_blank", "code" }, { "cn_serial", "tag" }, { "cn_patches", "shield" } });
}

void CadranConsoleTab::refresh()
{
    ConsoleTab::refresh();
    this->list.sync();
    cadran::set_pill("");
}

brls::View* CadranConsoleTab::create() { return new CadranConsoleTab(); }

// ---------------------------------------------------------------------------
// History: the backup's two actions as tiles.

CadranHistoryTab::CadranHistoryTab()
    : list(this)
{
    this->list.tiles({ { "hs_backup_save", "save" }, { "hs_backup_restore", "restore" } }, false);
    this->list.apply({ { "hs_history", "history" }, { "hs_backup_keep", "folder" } });
}

void CadranHistoryTab::refresh()
{
    HistoryTab::refresh();
    this->list.sync();
    cadran::set_pill("");
}

brls::View* CadranHistoryTab::create() { return new CadranHistoryTab(); }

// ---------------------------------------------------------------------------
// Preferences: the language, the theme, the appearance and the start screen
// as tiles, the text size and the switches below.

CadranPreferencesTab::CadranPreferencesTab()
    : list(this)
{
    this->getView("pf_text_size")->setVisibility(brls::Visibility::VISIBLE);
    this->getView("pf_effects")->setVisibility(brls::Visibility::VISIBLE);
    this->list.tiles({ { "pf_language", "language" }, { "pf_theme", "contrast" }, { "pf_appearance", "sparkle" },
                       { "pf_start_tab", "home" } }, false);
    this->list.apply({ { "pf_text_size", "eye" }, { "pf_effects", "sparkle" }, { "pf_intro", "bolt" }, { "pf_notices_system", "chip" }, { "pf_notices_summary", "calendar" },
                       { "pf_notices_reminders", "bell" }, { "pf_notices_ack_remind", "alert" },
                       { "pf_support_reminder", "heart" } });
}

void CadranPreferencesTab::refresh()
{
    PreferencesTab::refresh();
    this->list.sync();
    cadran::set_pill("");
}

brls::View* CadranPreferencesTab::create() { return new CadranPreferencesTab(); }

// ---------------------------------------------------------------------------
// About: the mark, the name and the version first.

CadranAboutTab::CadranAboutTab()
    : list(this)
{
    if (brls::Box* column = this->list.content()) {
        brls::Box* hero = hero_card(false);
        hero->setAxis(brls::Axis::ROW);
        hero->setAlignItems(brls::AlignItems::CENTER);
        hero->setMarginTop(10);
        hero->setMarginBottom(6);
        hero->setPadding(24, 30, 24, 30);
        auto* badge = new brls::Box(brls::Axis::ROW);
        badge->setWidth(84);
        badge->setHeight(84);
        badge->setCornerRadius(24);
        badge->setShrink(0);
        badge->setBackgroundColor(brls::Application::getTheme()["cadran/teal_soft"]);
        badge->setAlignItems(brls::AlignItems::CENTER);
        badge->setJustifyContent(brls::JustifyContent::CENTER);
        badge->setMarginRight(22);
        badge->addView(kit::icon("mark", 52, "cadran/teal"));
        hero->addView(badge);
        auto* text = new brls::Box(brls::Axis::COLUMN);
        text->setGrow(1.0f);
        text->setWidth(0);
        auto* name = new CadranDigits();
        name->setText("PlayGuard");
        name->setFontSize(36);
        text->addView(name);
        this->version_line = kit::label("", 16, "cadran/text2");
        this->version_line->setMarginTop(4);
        text->addView(this->version_line);
        brls::Label* tagline = kit::label("playguard/cadran/about/tagline"_i18n, 15, "cadran/text3");
        tagline->setMarginTop(6);
        text->addView(tagline);
        hero->addView(text);
        column->addView(hero, 0);
    }
    this->list.apply({ { "ab_version", "info" }, { "ab_mode", "gamepad" }, { "ab_data", "folder" },
                       { "ab_update", "download" }, { "ab_update_daily", "calendar" }, { "ab_update_via", "server" },
                       { "ab_help", "tools" },
                       { "ab_author", "user" }, { "ab_upstream", "flag" }, { "ab_fixes", "check" }, { "ab_ui", "contrast" },
                       { "ab_license", "file" }, { "ab_source", "code" } });
}

void CadranAboutTab::refresh()
{
    AboutTab::refresh();
    this->list.sync();
    if (this->version_line) {
        const std::string version = app::commit().empty() ? app::version() : app::version() + " · " + app::commit();
        this->version_line->setText(brls::getStr("playguard/cadran/about/version", version));
    }
    cadran::set_pill("");
}

brls::View* CadranAboutTab::create() { return new CadranAboutTab(); }
