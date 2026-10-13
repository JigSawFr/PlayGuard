// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_lists.hpp"

#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>

#include "app.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

namespace
{
brls::Label* label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setText(text);
    l->setFontSize(size);
    l->setTextColor(color);
    return l;
}

CadranIcon* icon(const char* name, float size, NVGcolor color)
{
    auto* view = new CadranIcon();
    view->setIcon(name);
    view->setWidth(size);
    view->setHeight(size);
    view->setColor(color);
    return view;
}

// A card of the heroes (not a list: no rows), `grow` shares a row.
brls::Box* hero_card(bool grow)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* card = new brls::Box(brls::Axis::COLUMN);
    card->setBackgroundColor(theme["cadran/surf"]);
    card->setBorderColor(theme["cadran/line"]);
    card->setBorderThickness(1);
    card->setCornerRadius(22);
    card->setPadding(18, 22, 18, 22);
    if (grow) {
        card->setGrow(1.0f);
        card->setWidth(0);
    }
    return card;
}

// "2026-10-08" -> "8 octobre 2026" (the text itself when it is not a date).
std::string long_date(const std::string& date)
{
    static const char* months[] = { "janvier", "février", "mars", "avril", "mai", "juin", "juillet",
                                    "août", "septembre", "octobre", "novembre", "décembre" };
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return date;
    const int month = std::atoi(date.substr(5, 2).c_str());
    const int day = std::atoi(date.substr(8, 2).c_str());
    if (month < 1 || month > 12 || day < 1) return date;
    return fmt::format("{} {} {}", day, months[month - 1], date.substr(0, 4));
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
// Clock: both clocks large, the network one amber while it is inaccurate.

CadranClockTab::CadranClockTab()
    : list(this)
{
    brls::Theme theme = brls::Application::getTheme();
    if (brls::Box* column = this->list.content()) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::STRETCH);
        row->setMarginBottom(12);

        brls::Box* console = hero_card(true);
        console->setMarginRight(12);
        auto* head = new brls::Box(brls::Axis::ROW);
        head->setAlignItems(brls::AlignItems::CENTER);
        head->addView(icon("gamepad", 18, theme["cadran/text3"]));
        brls::Label* kicker = label("CONSOLE", 14, theme["cadran/text3"]);
        kicker->setMarginLeft(8);
        head->addView(kicker);
        console->addView(head);
        this->user_time = new CadranDigits();
        this->user_time->setFontSize(46);
        this->user_time->setMarginTop(10);
        console->addView(this->user_time);
        this->user_date = label("", 15.5f, theme["cadran/text2"]);
        this->user_date->setMarginTop(4);
        console->addView(this->user_date);
        row->addView(console);

        this->network_card = hero_card(true);
        auto* nhead = new brls::Box(brls::Axis::ROW);
        nhead->setAlignItems(brls::AlignItems::CENTER);
        nhead->addView(icon("globe", 18, theme["cadran/text3"]));
        brls::Label* nkicker = label("RÉSEAU · UTILISÉE PAR LE MINUTEUR", 14, theme["cadran/text3"]);
        nkicker->setMarginLeft(8);
        nhead->addView(nkicker);
        this->network_card->addView(nhead);
        this->network_time = new CadranDigits();
        this->network_time->setFontSize(46);
        this->network_time->setMarginTop(10);
        this->network_card->addView(this->network_time);
        auto* state = new brls::Box(brls::Axis::ROW);
        state->setAlignItems(brls::AlignItems::CENTER);
        state->setMarginTop(4);
        this->network_dot = new brls::Rectangle(theme["cadran/teal_fill"]);
        this->network_dot->setWidth(9);
        this->network_dot->setHeight(9);
        this->network_dot->setCornerRadius(4.5f);
        this->network_dot->setMarginRight(8);
        state->addView(this->network_dot);
        this->network_state = label("", 15.5f, theme["cadran/teal"]);
        state->addView(this->network_state);
        this->network_card->addView(state);
        row->addView(this->network_card);

        // In place of the three readings it shows.
        brls::View* first = this->getView("ck_user");
        const auto& lines = column->getChildren();
        const auto at = std::find(lines.begin(), lines.end(), first);
        column->addView(row, at == lines.end() ? 0 : (size_t)(at - lines.begin()));
        for (const char* id : { "ck_user", "ck_network", "ck_accuracy" })
            this->getView(id)->setVisibility(brls::Visibility::GONE);
    }
    this->list.apply({ { "ck_auto", "refresh" }, { "ck_zone", "pin" }, { "ck_region", "globe" }, { "ck_server", "server" },
                       { "ck_custom", "pencil" }, { "ck_measure", "timer" }, { "ck_apply", "check" }, { "ck_copy", "today" } });
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
    this->user_date->setText(long_date(user.first));
    this->network_time->setText(network.second);

    const std::string accuracy = detail_of(this, "ck_accuracy");
    const bool known = accuracy == "playguard/common/yes"_i18n || accuracy == "playguard/common/no"_i18n;
    const bool bad = accuracy == "playguard/common/no"_i18n;
    std::string state = !known ? accuracy : bad ? "imprécise · à régler ci-dessous" : "précise";
    if (known && !network.first.empty() && network.first != user.first) state += " · " + long_date(network.first);
    this->network_state->setText(state);
    this->network_state->setTextColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber"] : theme["cadran/teal"]);
    this->network_dot->setColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber_fill"] : theme["cadran/teal_fill"]);
    this->network_time->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text"]);
    this->network_card->setBackgroundColor(bad ? theme["cadran/amber_surf"] : theme["cadran/surf"]);
    this->network_card->setBorderColor(bad ? theme["cadran/amber_line"] : theme["cadran/line"]);
    if (!known) cadran::set_pill("");
    else if (bad) cadran::set_pill("Horloge réseau imprécise", "alert", true);
    else cadran::set_pill("Horloge réseau précise", "globe");
}

brls::View* CadranClockTab::create() { return new CadranClockTab(); }

// ---------------------------------------------------------------------------
// Security: the three states as tiles, the deletion in a coral card.

CadranSecurityTab::CadranSecurityTab()
    : list(this)
{
    this->list.tiles({ { "sc_pin", "key" }, { "sc_restrictions", "shield" }, { "sc_temp", "unlock" } }, true);
    this->list.apply({ { "sc_set_pin", "key" }, { "sc_pin_lock", "lock" }, { "sc_show_pin", "eye" },
                       { "sc_unlock", "unlock" }, { "sc_relock", "lock" }, { "sc_console_lock", "gamepad" },
                       { "pr_active", "phone" }, { "pr_updated", "refresh" }, { "pr_decode", "code" },
                       { "pr_unlink", "stop" }, { "sc_delete", "trash" } });
    brls::Theme theme = brls::Application::getTheme();
    if (cadran::Card* danger = this->list.card_of("sc_delete")) danger->setAccent(theme["cadran/coral_line"]);
}

void CadranSecurityTab::refresh()
{
    SecurityTab::refresh();
    this->list.sync();
    const std::string pin = detail_of(this, "sc_pin");
    const bool unlocked = detail_of(this, "sc_temp") == "playguard/common/yes"_i18n;
    if (unlocked) cadran::set_pill("Déverrouillé temporairement", "unlock", true);
    else if (pin == "playguard/dashboard/pin_set"_i18n) cadran::set_pill("PIN défini", "key");
    else if (pin == "playguard/common/not_set"_i18n) cadran::set_pill("Pas de code PIN", "key", true);
    else cadran::set_pill("");
}

brls::View* CadranSecurityTab::create() { return new CadranSecurityTab(); }

// ---------------------------------------------------------------------------
// Preferences: language, theme and start screen as tiles, switches below.

CadranPreferencesTab::CadranPreferencesTab()
    : list(this)
{
    this->list.tiles({ { "pf_language", "language" }, { "pf_theme", "contrast" }, { "pf_start_tab", "home" } }, false);
    this->list.apply({ { "pf_auto_relock", "lock" }, { "pf_extra_amounts", "plus" }, { "pf_extra_auto", "refresh" },
                       { "pf_advanced", "settings" }, { "pf_intro", "sparkle" }, { "pf_clock_check", "timer" },
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
// Tools: the everyday actions as tiles, the console's readings as tiles.

CadranToolsTab::CadranToolsTab()
    : list(this)
{
    this->list.hide_header_before("tl_history");
    this->list.tiles({ { "tl_first_steps", "sparkle" }, { "tl_history", "history" }, { "tl_backup_save", "save" },
                       { "tl_backup_restore", "restore" } }, false);
    this->list.tiles({ { "tl_fw", "chip" }, { "tl_ams", "bolt" }, { "tl_storage", "folder" } }, true);
    this->list.apply({ { "tl_backup_keep", "save" }, { "tl_export", "download" }, { "tl_upload", "upload" },
                       { "tl_logs_keep", "file" }, { "tl_logs_keep_days", "calendar" }, { "tl_logs_keep_mb", "folder" },
                       { "tl_compat", "check" }, { "tl_blank", "file" }, { "tl_serial", "tag" }, { "tl_patches", "gamepad" },
                       { "tl_dev_mode", "code" }, { "tl_dev_read_only", "eye" }, { "tl_dev_pt_log", "file" },
                       { "tl_dev_frame_stats", "activity" }, { "tl_dev_cadran", "sparkle" },
                       { "tl_dev_clear_cache", "trash" }, { "tl_dev_clear_logs", "trash" }, { "tl_dev_report", "alert" },
                       { "tl_dev_pt_block", "code" }, { "tl_dev_gate", "lock" }, { "tl_dev_forget", "user" },
                       { "tl_dev_github", "user" }, { "tl_dev_build", "download" } });
}

void CadranToolsTab::refresh()
{
    ToolsTab::refresh();
    this->list.sync();
    cadran::set_pill(app::dev_mode() ? "Mode développeur" : "", "code");
}

brls::View* CadranToolsTab::create() { return new CadranToolsTab(); }

// ---------------------------------------------------------------------------
// About: the mark, the name and the version first.

CadranAboutTab::CadranAboutTab()
    : list(this)
{
    brls::Theme theme = brls::Application::getTheme();
    if (brls::Box* column = this->list.content()) {
        brls::Box* hero = hero_card(false);
        hero->setAxis(brls::Axis::ROW);
        hero->setAlignItems(brls::AlignItems::CENTER);
        hero->setMarginTop(10);
        hero->setMarginBottom(6);
        hero->setPadding(22, 26, 22, 26);
        auto* badge = new brls::Box(brls::Axis::ROW);
        badge->setWidth(84);
        badge->setHeight(84);
        badge->setCornerRadius(24);
        badge->setShrink(0);
        badge->setBackgroundColor(theme["cadran/teal_soft"]);
        badge->setAlignItems(brls::AlignItems::CENTER);
        badge->setJustifyContent(brls::JustifyContent::CENTER);
        badge->setMarginRight(22);
        badge->addView(icon("mark", 52, theme["cadran/teal"]));
        hero->addView(badge);
        auto* text = new brls::Box(brls::Axis::COLUMN);
        text->setGrow(1.0f);
        auto* name = new CadranDigits();
        name->setText("PlayGuard");
        name->setFontSize(36);
        text->addView(name);
        this->version_line = label("", 16, theme["cadran/text2"]);
        this->version_line->setMarginTop(4);
        text->addView(this->version_line);
        brls::Label* tagline = label("Contrôle parental pour Nintendo Switch · logiciel libre (GPLv3)", 15, theme["cadran/text3"]);
        tagline->setMarginTop(6);
        text->addView(tagline);
        hero->addView(text);
        column->addView(hero, 0);
    }
    if (cadran::Card* notes = this->list.wrap("ab_changelog")) notes->setMarginBottom(10);
    this->list.apply({ { "ab_version", "info" }, { "ab_mode", "gamepad" }, { "ab_data", "folder" },
                       { "ab_update", "download" }, { "ab_update_daily", "calendar" }, { "ab_update_via", "server" },
                       { "ab_changelog_full", "list" }, { "ab_author", "user" }, { "ab_upstream", "flag" },
                       { "ab_fixes", "check" }, { "ab_ui", "contrast" }, { "ab_license", "file" }, { "ab_source", "code" } });
}

void CadranAboutTab::refresh()
{
    AboutTab::refresh();
    this->list.sync();
    if (this->version_line) {
        const std::string commit = app::commit().empty() ? "" : " · " + app::commit();
        this->version_line->setText("Version " + app::version() + commit);
    }
    cadran::set_pill("");
}

brls::View* CadranAboutTab::create() { return new CadranAboutTab(); }
