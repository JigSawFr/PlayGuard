// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_activity_tab.hpp"

#include <algorithm>
#include <fmt/format.h>

#include "action/game_icons.hpp"
#include "action/play_data.hpp"
#include "activity/game_activity.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace
{
constexpr size_t CARDS = 4;
constexpr float  WEEK_BAR_MAX = 56.0f;
constexpr float  SPARK_MAX = 40.0f;
constexpr long   GREW_MIN = 30;    // a game card says "+1 h vs last week" from half an hour more
// The accounts' avatars, in turn (no picture of theirs is read).
const NVGcolor AVATARS[] = { nvgRGB(0x7C, 0x6C, 0xF2), nvgRGB(0xFF, 0xB5, 0x47), nvgRGB(0x4F, 0xA3, 0xFF), nvgRGB(0xE9, 0x6A, 0xA8) };

std::string game_name(const GameStat& g)
{
    if (g.name[0]) return g.name;
    return brls::getStr("playguard/activity/deleted_game", fmt::format("{:016X}", (unsigned long long)g.app_id));
}

// Seven bars, oldest first, today last (teal); `values[k]`: k days ago.
template <typename T>
void show_bars(const std::vector<brls::View*>& bars, const T* values, float most)
{
    brls::Theme theme = brls::Application::getTheme();
    uint64_t top = 0;
    for (int k = 0; k < 7; k++) top = std::max<uint64_t>(top, values[k]);
    for (size_t i = 0; i < bars.size() && i < 7; i++) {
        const uint64_t v = values[6 - i];
        bars[i]->setHeight(v && top ? std::max(4.0f, most * (float)v / (float)top) : 3.0f);
        bars[i]->setBackgroundColor(i == 6 ? theme["cadran/teal_fill"] : theme["cadran/bar"]);
    }
}
}   // namespace

CadranActivityTab::CadranActivityTab()
    : list(this)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    brls::Box* column = this->list.content();
    column->addView(brls::View::createFromXMLResource("view/cadran_activity.xml"), 0);
    this->list.park({ "ac_account", "ac_today", "ac_week", "ac_total", "ac_period", "ac_export" });
    this->list.apply({ { "ac_account_default", "user" }, { "ac_games", "list" } });

    // The game cards, built once, two rows of two; filled by show_hero().
    brls::Box* row = nullptr;
    for (size_t i = 0; i < CARDS; i++) {
        Card c;
        c.box = kit::card(true, true);
        c.box->setAxis(brls::Axis::ROW);
        c.box->setAlignItems(brls::AlignItems::CENTER);
        c.box->setPadding(14, 22, 14, 14);
        c.box->setMarginBottom(12);
        if (i % 2 == 0) c.box->setMarginRight(12);
        c.icon_fallback = new brls::Box();
        c.icon_fallback->setWidth(60);
        c.icon_fallback->setHeight(60);
        c.icon_fallback->setCornerRadius(14);
        c.icon_fallback->setBackgroundColor(theme["cadran/surf2"]);
        c.icon_fallback->setMarginRight(16);
        c.box->addView(c.icon_fallback);
        c.icon = new brls::Image();
        c.icon->setWidth(60);
        c.icon->setHeight(60);
        c.icon->setCornerRadius(14);
        c.icon->setMarginRight(16);
        c.icon->setScalingType(brls::ImageScalingType::FIT);
        c.icon->setVisibility(brls::Visibility::GONE);
        c.box->addView(c.icon);
        auto* text = new brls::Box(brls::Axis::COLUMN);
        text->setGrow(1.0f);
        text->setWidth(0);
        c.name = kit::label("", 19);
        c.name->setSingleLine(true);
        text->addView(c.name);
        // On two lines when a language needs them.
        auto* line = new brls::Box(brls::Axis::ROW);
        line->setAlignItems(brls::AlignItems::FLEX_START);
        line->setMarginTop(4);
        c.today = new CadranDigits();
        c.today->setFontSize(15);
        c.today->setMarginTop(1);
        c.today->setMarginRight(6);
        c.today->setShrink(0);
        line->addView(c.today);
        c.rest = kit::label("", 15, "cadran/text2");
        c.rest->setGrow(1.0f);
        c.rest->setWidth(0);
        line->addView(c.rest);
        text->addView(line);
        c.grew = kit::label("", 15, "cadran/amber");
        c.grew->setSingleLine(true);
        c.grew->setMarginTop(2);
        c.grew->setVisibility(brls::Visibility::GONE);
        text->addView(c.grew);
        c.box->addView(text);
        auto* spark = new brls::Box(brls::Axis::ROW);
        spark->setAlignItems(brls::AlignItems::FLEX_END);
        spark->setHeight(SPARK_MAX);
        spark->setMarginLeft(12);
        for (int k = 0; k < 7; k++) {
            c.bars[k] = new brls::Box();
            c.bars[k]->setWidth(8);
            c.bars[k]->setHeight(3);
            c.bars[k]->setCornerRadius(3);
            if (k < 6) c.bars[k]->setMarginRight(3);
            spark->addView(c.bars[k]);
        }
        c.box->addView(spark);
        if (i % 2 == 0) {
            row = new brls::Box(brls::Axis::ROW);
            row->setAlignItems(brls::AlignItems::STRETCH);
            games_grid->addView(row);
        }
        row->addView(c.box);
        // The game's own screen, as All games opens it.
        c.box->registerClickAction([this, i](brls::View*) {
            const auto data = shown_data();
            if (!data) return true;
            for (uint32_t g = 0; g < data->count; g++) {
                if (data->games[g].app_id != this->cards[i].app_id) continue;
                std::vector<unsigned char> bytes;
                if (const auto* b = game_icons::bytes(data->games[g].app_id)) bytes = *b;
                brls::Application::pushActivity(new GameActivity(data->games[g], data, std::move(bytes), shown_account() < 0));
                break;
            }
            return true;
        });
        this->cards.push_back(c);
    }

    // In the games' place when none was played over the period.
    this->empty = kit::empty("gamepad");
    this->empty.text->setText("playguard/cadran/activity/empty_note"_i18n);
    this->see_all = kit::chip("playguard/cadran/activity/see_all"_i18n, 44);
    this->empty.box->setMarginBottom(12);
    this->see_all->setMarginTop(18);
    this->see_all->registerClickAction([this](brls::View*) {
        this->period = AllTime;
        config::get().activity_period = AllTime;   // as the period's own picker
        ui::save_config();
        this->rebuild();
        if (this->empty.box->getVisibility() != brls::Visibility::VISIBLE)
            brls::Application::giveFocus(period_chip.getView());
        return true;
    });
    this->empty.box->addView(this->see_all);
    this->empty.box->setVisibility(brls::Visibility::GONE);
    if (auto* parent = dynamic_cast<brls::Box*>(games_grid->getParent())) {
        const auto& views = parent->getChildren();
        const auto at = std::find(views.begin(), views.end(), games_grid.getView());
        parent->addView(this->empty.box, at - views.begin());
    }

    // The period and the export: the tab's own lines.
    period_chip->registerClickAction([this](brls::View*) {
        kit::press(this->getView("ac_period"));
        return true;
    });
    export_chip->registerClickAction([this](brls::View*) {
        kit::press(this->getView("ac_export"));
        return true;
    });
    this->build_chips();
    this->show_hero();
}

CadranActivityTab::~CadranActivityTab()
{
    *this->alive = false;
}

void CadranActivityTab::build_chips()
{
    brls::Theme theme = brls::Application::getTheme();
    const auto& accounts = play_data::accounts();
    // No account to choose between on a console with one (as the tab's line).
    if (accounts.size() < 2) return;
    for (int i = -1; i < (int)accounts.size(); i++) {
        auto* chip = new brls::Box(brls::Axis::ROW);
        chip->setAlignItems(brls::AlignItems::CENTER);
        chip->setHeight(44);
        chip->setPadding(0, 18, 0, i < 0 ? 18 : 6);
        chip->setCornerRadius(22);
        chip->setFocusable(true);
        chip->setHideHighlightBackground(true);   // the ring alone: a chosen chip keeps its fill
        chip->setHighlightCornerRadius(22);
        chip->setMarginRight(10);
        chip->setShrink(0);
        if (i < 0) {
            chip->addView(cadran::kit::label("playguard/activity/account_all"_i18n, 16));
        } else {
            const std::string nick = accounts[i].nickname[0] ? std::string(accounts[i].nickname) : "?";
            auto* avatar = new brls::Box(brls::Axis::ROW);
            avatar->setWidth(32);
            avatar->setHeight(32);
            avatar->setCornerRadius(16);
            avatar->setBackgroundColor(AVATARS[i % 4]);
            avatar->setAlignItems(brls::AlignItems::CENTER);
            avatar->setJustifyContent(brls::JustifyContent::CENTER);
            avatar->setMarginRight(10);
            auto* initial = new CadranDigits();
            initial->setFontSize(15);
            // The first character, whole (a nickname is UTF-8).
            size_t len = 1;
            while (len < nick.size() && (nick[len] & 0xC0) == 0x80) len++;
            initial->setText(nick.substr(0, len));
            initial->setTextColor(theme["cadran/navy"]);   // 4.6:1 or more on every avatar colour
            avatar->addView(initial);
            chip->addView(avatar);
            brls::Label* name = cadran::kit::label(nick, 16);
            name->setSingleLine(true);
            chip->addView(name);
        }
        chip->registerClickAction([this, i](brls::View*) {
            this->show_account(i);
            return true;
        });
        accounts_row->addView(chip);
        this->chips.push_back(chip);
    }
}

void CadranActivityTab::refresh()
{
    ActivityTab::refresh();
    this->list.sync();
    cadran::set_pill("");
}

void CadranActivityTab::rebuild()
{
    ActivityTab::rebuild();
    this->show_hero();
}

void CadranActivityTab::show_hero()
{
    brls::Theme theme = brls::Application::getTheme();
    const int who = shown_account();
    for (size_t i = 0; i < this->chips.size(); i++) {
        const bool on = (int)i - 1 == who;
        this->chips[i]->setBackgroundColor(on ? theme["cadran/teal_fill"] : theme["cadran/surf"]);
        this->chips[i]->setBorderColor(on ? theme["cadran/none"] : theme["cadran/line"]);
        this->chips[i]->setBorderThickness(on ? 0 : 1);
        for (brls::View* v : this->chips[i]->getChildren())
            if (auto* l = dynamic_cast<brls::Label*>(v)) l->setTextColor(on ? theme["cadran/navy"] : theme["cadran/text"]);
    }
    const int p = this->period;
    const std::string period_name = brls::getStr(fmt::format("playguard/activity/periods/{}", p));
    period_label->setText(period_name);
    games_title->setText(cadran::upper("playguard/activity/section_games"_i18n + " · " + period_name));

    const auto data = shown_data();
    if (!data) {
        // Nothing read for this account yet: the tab's spinner says so.
        for (CadranDigits* v : { today_value.getView(), week_value.getView(), total_value.getView() }) v->setText("—");
        for (brls::Label* l : { today_sub.getView(), week_sub.getView(), total_sub.getView() }) l->setText("");
        games_note->setText("playguard/activity/loading"_i18n);
        ui::set_visible(games_note, true);
        ui::set_visible(this->empty.box, false);
        for (Card& c : this->cards) ui::set_visible(c.box, false);
        return;
    }
    const PlayStats& s = *data;

    // Today: on today's limit (every account: the limit is the console's).
    uint64_t today_s = 0;
    int games_today = 0;
    for (uint32_t g = 0; g < s.count; g++) {
        today_s += s.games[g].today_s;
        games_today += s.games[g].today_s > 0;
    }
    std::string sub = brls::getStr("playguard/cadran/activity/games", games_today);
    if (who < 0) {
        const PtState& pt = this->play_timer();
        const uint16_t limit = pt.fw_supported && pt.valid ? pt.day_min[ui::today_weekday()] : PT_DAY_NOLIMIT;
        sub = (limit == PT_DAY_NOLIMIT ? "playguard/cadran/week/no_limit"_i18n
                                       : brls::getStr("playguard/cadran/activity/on_limit", ui::fmt_minutes(limit))) + " · " + sub;
    }
    today_value->setText(s.windows_ok ? ui::fmt_play_time(today_s) : "—");
    today_sub->setText(s.windows_ok ? sub : "");

    // The last seven days, and their bars.
    const auto week = activity_summary::summarize(s, activity_summary::Week);
    week_value->setText(s.windows_ok ? ui::fmt_play_time(week.total_s) : "—");
    week_sub->setText(s.windows_ok && week.days_played >= 0
                          ? brls::getStr("playguard/activity/summary/days_played_value", week.days_played) + " · " +
                                brls::getStr("playguard/cadran/activity/per_day", ui::fmt_play_time(week.per_day_s))
                          : "");
    // Against the 7 days before, per day: more (amber), less (teal) or about
    // the same (under 5 minutes either way).
    int64_t diff = 0;
    const bool trend = activity_summary::week_trend_min(s, diff);
    if (trend) {
        const int64_t same = activity_summary::TREND_SAME_MIN;
        brls::Theme theme = brls::Application::getTheme();
        const uint16_t amount = (uint16_t)std::min<int64_t>(24 * 60, diff < 0 ? -diff : diff);
        ui::set_text(week_trend, diff >= same    ? brls::getStr("playguard/cadran/activity/trend_up", ui::fmt_minutes(amount))
                                 : diff <= -same ? brls::getStr("playguard/cadran/activity/trend_down", ui::fmt_minutes(amount))
                                                 : "playguard/cadran/activity/trend_same"_i18n);
        week_trend->setTextColor(theme[diff >= same ? "cadran/amber" : diff <= -same ? "cadran/teal" : "cadran/text3"]);
    }
    ui::set_visible(week_trend.getView(), trend);
    uint64_t day_total[7] = {};
    for (uint32_t g = 0; g < s.count; g++)
        for (int k = 0; k < 7; k++) day_total[k] += s.games[g].day_s[k];
    show_bars(week_bars->getChildren(), day_total, WEEK_BAR_MAX);

    // All time, and the game most played.
    const auto all = activity_summary::summarize(s, activity_summary::AllTime);
    total_value->setText(all.known ? ui::fmt_play_time(all.total_s) : "—");
    total_sub->setText(all.known && all.top >= 0 ? brls::getStr("playguard/cadran/activity/top", game_name(s.games[all.top])) : "");

    // The games of the period, most played first.
    std::vector<const GameStat*> games;
    for (uint32_t g = 0; g < s.count; g++)
        if (activity_summary::value(s.games[g], p) > 0) games.push_back(&s.games[g]);
    std::sort(games.begin(), games.end(), [p](const GameStat* a, const GameStat* b) {
        return activity_summary::value(*a, p) > activity_summary::value(*b, p);
    });
    // Unread (the line under All games says why), or nothing played: the
    // empty card says so instead of that line.
    const bool unread = R_FAILED(s.rc) || !activity_summary::period_read(s, p);
    const bool none   = games.empty() && !unread;
    ui::set_visible(games_note, false);
    ui::set_visible(this->empty.box, none);
    ui::set_visible(this->see_all, p != AllTime && all.known && all.total_s > 0);
    if (none) this->empty.title->setText(brls::getStr(fmt::format("playguard/activity/none/{}", p)));
    if (brls::View* status = this->getView("ac_status"); status && none) ui::set_visible(status, false);
    // Over the last 7 days, the card of the game played most more than the
    // week before says by how much (half an hour at least).
    const GameStat* grew = nullptr;
    long grew_s = GREW_MIN * 60 - 1;
    if (p == Week && s.windows_ok && s.prev_week_ok)
        for (size_t i = 0; i < games.size() && i < this->cards.size(); i++) {
            const long d = (long)games[i]->week_s - (long)games[i]->prev_week_s;
            if (d > grew_s) {
                grew   = games[i];
                grew_s = d;
            }
        }
    std::vector<u64> ids;
    for (size_t i = 0; i < this->cards.size(); i++) {
        Card& c = this->cards[i];
        const bool shown = i < games.size();
        ui::set_visible(c.box, shown);
        if (!shown) continue;
        ui::set_visible(c.grew, games[i] == grew);
        if (games[i] == grew)
            ui::set_text(c.grew, brls::getStr("playguard/cadran/activity/card_grew", ui::fmt_play_time((u64)grew_s)));
        const GameStat& g = *games[i];
        c.app_id = g.app_id;
        ids.push_back(g.app_id);
        c.name->setText(game_name(g));
        c.today->setText(s.windows_ok && g.today_s ? ui::fmt_play_time(g.today_s) : "—");
        c.today->setTextColor(g.today_s ? theme["cadran/text"] : theme["cadran/text3"]);
        // The period is the section's title: "in all" over it.
        c.rest->setText(p == Today ? "playguard/cadran/activity/card_today"_i18n
                                   : brls::getStr("playguard/cadran/activity/card_total", ui::fmt_play_time(activity_summary::value(g, p))));
        std::vector<brls::View*> bars(std::begin(c.bars), std::end(c.bars));
        show_bars(bars, g.day_s, SPARK_MAX);
    }
    // A row with both its cards hidden takes no room.
    for (brls::View* row : games_grid->getChildren()) {
        bool any = false;
        for (brls::View* card : static_cast<brls::Box*>(row)->getChildren()) any = any || card->getVisibility() == brls::Visibility::VISIBLE;
        row->setVisibility(any ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    }
    this->show_icons();
    if (!game_icons::wanted()) return;
    std::weak_ptr<bool> weak = this->alive;
    game_icons::load(ids, [this, weak]() {
        if (!weak.expired()) this->show_icons();
    });
}

void CadranActivityTab::show_icons()
{
    for (Card& c : this->cards) {
        bool owned = false;
        const int texture = c.app_id ? game_icons::texture(c.app_id, owned) : 0;
        if (texture) {
            // As GameCell: the flag set once the old one is replaced.
            c.icon->innerSetImage(texture);
            c.icon->setFreeTexture(owned);
        }
        c.icon->setVisibility(texture ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        c.icon_fallback->setVisibility(texture ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    }
}

brls::View* CadranActivityTab::create() { return new CadranActivityTab(); }
