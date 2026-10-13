// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_activity_tab.hpp"

#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>

#include "action/play_data.hpp"
#include "activity/game_activity.hpp"
#include "tab/activity_tab.hpp"
#include "tab/cadran_list.hpp"
#include "tab/cadran_tab.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace
{
// The icons read so far (the bytes as the control data holds them), kept for the run.
std::map<uint64_t, std::vector<unsigned char>> s_icons;
std::set<uint64_t> s_no_icon;
bool s_icons_busy = false;

constexpr size_t CARDS = 4;
constexpr float  WEEK_BAR_MAX = 56.0f;
constexpr float  SPARK_MAX = 40.0f;
const NVGcolor AVATARS[] = { nvgRGB(0x7C, 0x6C, 0xF2), nvgRGB(0xFF, 0xB5, 0x47), nvgRGB(0x4F, 0xA3, 0xFF), nvgRGB(0xE9, 0x6A, 0xA8) };

brls::Label* label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setText(text);
    l->setFontSize(size);
    l->setTextColor(color);
    return l;
}

brls::Box* chip(float height, float padding)
{
    auto* b = new brls::Box(brls::Axis::ROW);
    b->setAlignItems(brls::AlignItems::CENTER);
    b->setHeight(height);
    b->setPaddingLeft(padding);
    b->setPaddingRight(padding);
    b->setCornerRadius(height / 2.0f);
    b->setFocusable(true);
    b->setHideHighlightBackground(true);   // the focus ring alone: a selected chip keeps its fill
    b->setMarginRight(10);
    return b;
}

std::string ago(uint64_t last, uint64_t now)
{
    if (!last || now <= last) return "";
    const uint64_t days = (now - last) / 86400;
    if (days < 14) return fmt::format("il y a {} jour{}", std::max<uint64_t>(1, days), days > 1 ? "s" : "");
    if (days < 60) return fmt::format("il y a {} semaines", days / 7);
    return fmt::format("il y a {} mois", days / 30);
}
}   // namespace

CadranActivityTab::CadranActivityTab()
    : TabBase("xml/tab/cadran_activity.xml")
{
    brls::Theme theme = brls::Application::getTheme();
    // The game cards, built once; filled by refresh().
    for (size_t i = 0; i < CARDS; i++) {
        Card c;
        c.box = new brls::Box(brls::Axis::ROW);
        c.box->setAlignItems(brls::AlignItems::CENTER);
        c.box->setGrow(1.0f);
        c.box->setWidth(0);
        c.box->setPadding(12, 18, 12, 12);
        c.box->setCornerRadius(20);
        c.box->setBackgroundColor(theme["cadran/surf"]);
        c.box->setBorderColor(theme["cadran/line"]);
        c.box->setBorderThickness(1);
        c.box->setFocusable(true);
        c.box->setHideHighlightBackground(true);
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
        c.name = label("", 19, theme["cadran/text"]);
        c.name->setSingleLine(true);
        text->addView(c.name);
        auto* line = new brls::Box(brls::Axis::ROW);
        line->setAlignItems(brls::AlignItems::CENTER);
        line->setMarginTop(4);
        c.today = new CadranDigits();
        c.today->setFontSize(14);
        c.today->setMarginRight(6);
        line->addView(c.today);
        c.rest = label("", 14, theme["cadran/text2"]);
        c.rest->setSingleLine(true);
        line->addView(c.rest);
        text->addView(line);
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
        this->cards.push_back(c);
    }
    // Two rows of two.
    auto* row1 = new brls::Box(brls::Axis::ROW);
    auto* row2 = new brls::Box(brls::Axis::ROW);
    row1->setAlignItems(brls::AlignItems::STRETCH);
    row2->setAlignItems(brls::AlignItems::STRETCH);
    row1->addView(this->cards[0].box);
    row1->addView(this->cards[1].box);
    row2->addView(this->cards[2].box);
    row2->addView(this->cards[3].box);
    games_grid->addView(row1);
    games_grid->addView(row2);
    for (size_t i = 0; i < CARDS; i++) {
        this->cards[i].box->registerAction("Détail du jeu", brls::BUTTON_A, [this, i](brls::View*) {
            const auto data = this->stats();
            if (!data) return true;
            for (uint32_t g = 0; g < data->count; g++) {
                if (data->games[g].app_id != this->cards[i].app_id) continue;
                std::vector<unsigned char> icon;
                const auto it = s_icons.find(data->games[g].app_id);
                if (it != s_icons.end()) icon = it->second;
                // Every account's figures when no account is chosen (main's GameActivity says so).
                brls::Application::pushActivity(new GameActivity(data->games[g], data, std::move(icon), this->account < 0));
                break;
            }
            return true;
        });
    }

    // The period and the export: the Activity tab's own (same setting, same files).
    this->period = std::clamp(config::get().activity_period, 0, 2);
    period_chip->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (int p = 0; p < 3; p++) labels.push_back(brls::getStr(fmt::format("playguard/activity/periods/{}", p)));
        ui::pick("playguard/activity/period"_i18n, labels, this->period, [this](int index) {
            this->period = index;
            config::get().activity_period = index;
            ui::save_config();
            this->refresh();
        });
        return true;
    });
    export_chip->registerClickAction([this](brls::View*) {
        const auto& accounts = play_data::accounts();
        const PlayAccount* acc = this->account >= 0 && this->account < (int)accounts.size() ? &accounts[this->account] : nullptr;
        ActivityTab::export_stats(this->stats(), this->period, acc);
        return true;
    });

    this->build_chips();
    this->listener = play_data::listen([this]() {
        this->refresh();
        this->apply_icons();
    });
}

CadranActivityTab::~CadranActivityTab()
{
    *this->alive = false;
    play_data::unlisten(this->listener);
}

std::shared_ptr<const PlayStats> CadranActivityTab::stats() const
{
    const auto& list = play_data::accounts();
    const PlayAccount* acc = this->account >= 0 && this->account < (int)list.size() ? &list[this->account] : nullptr;
    return play_data::latest(play_data::key_of(acc));
}

void CadranActivityTab::build_chips()
{
    brls::Theme theme = brls::Application::getTheme();
    accounts_row->clearViews();
    this->chips.clear();
    const auto& list = play_data::accounts();
    for (int i = -1; i < (int)list.size(); i++) {
        brls::Box* b = chip(44, 16);
        if (i < 0) {
            b->addView(label("Tous les comptes", 16, theme["cadran/text"]));
        } else {
            b->setPaddingLeft(6);
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
            const std::string nick = list[i].nickname[0] ? std::string(list[i].nickname) : "?";
            initial->setText(nick.substr(0, 1));
            initial->setTextColor(i % 4 == 1 ? theme["cadran/navy"] : nvgRGB(0xFF, 0xFF, 0xFF));
            avatar->addView(initial);
            b->addView(avatar);
            b->addView(label(nick, 16, theme["cadran/text"]));
        }
        b->registerClickAction([this, i](brls::View*) {
            this->account = i;
            const auto& accounts = play_data::accounts();
            const PlayAccount* acc = i >= 0 && i < (int)accounts.size() ? &accounts[i] : nullptr;
            if (!play_data::fresh(play_data::key_of(acc), std::chrono::minutes(5)) && !play_data::busy(play_data::key_of(acc)))
                play_data::fetch(acc);
            this->refresh();
            return true;
        });
        accounts_row->addView(b);
        this->chips.push_back(b);
    }
}

void CadranActivityTab::refresh()
{
    brls::Theme theme = brls::Application::getTheme();
    cadran::set_pill("");
    for (size_t i = 0; i < this->chips.size(); i++) {
        const bool on = (int)i - 1 == this->account;
        this->chips[i]->setBackgroundColor(on ? theme["cadran/teal_fill"] : theme["cadran/surf"]);
        this->chips[i]->setBorderColor(on ? theme["cadran/none"] : theme["cadran/line"]);
        this->chips[i]->setBorderThickness(on ? 0 : 1);
        for (brls::View* v : this->chips[i]->getChildren())
            if (auto* l = dynamic_cast<brls::Label*>(v)) l->setTextColor(on ? theme["cadran/navy"] : theme["cadran/text"]);
    }

    const auto data = this->stats();
    if (!data || !data->windows_ok) {
        if (!play_data::busy("") && !data) play_data::fetch(nullptr);
        today_value->setText("—");
        week_value->setText("—");
        total_value->setText("—");
        today_sub->setText("lecture des données de jeu de la console…");
        week_sub->setText("");
        total_sub->setText("");
        games_note->setText("Lecture des données de jeu de la console…");
        for (Card& c : this->cards) ui::set_visible(c.box, false);
        return;
    }
    const PlayStats& s = *data;

    // The three figures.
    PtState pt;
    pctl_play_timer_query(&pt);
    const int today = ui::today_weekday();
    int games_today = 0;
    for (uint32_t g = 0; g < s.count; g++) games_today += s.games[g].today_s > 0;
    today_value->setText(ui::fmt_play_time(play_data::today_total_s(s)));
    today_sub->setText((pt.valid && pt.enabled && pt.day_min[today] != PT_DAY_NOLIMIT ? "sur une limite de " + ui::fmt_minutes(pt.day_min[today]) : "pas de limite")
                       + fmt::format(" · {} jeu{}", games_today, games_today > 1 ? "x" : ""));
    const auto week = activity_summary::summarize(s, activity_summary::Week);
    week_value->setText(ui::fmt_play_time(week.total_s));
    week_sub->setText(week.days_played >= 0 ? fmt::format("{} jour{} joué{} · {} par jour", week.days_played, week.days_played > 1 ? "s" : "",
                                                          week.days_played > 1 ? "s" : "", ui::fmt_play_time(week.per_day_s))
                                            : "");
    uint64_t day_total[7] = {}, day_max = 0;
    for (uint32_t g = 0; g < s.count; g++)
        for (int k = 0; k < 7; k++) day_total[k] += s.games[g].day_s[k];
    for (int k = 0; k < 7; k++) day_max = std::max(day_max, day_total[k]);
    const auto& bars = week_bars->getChildren();
    for (size_t k = 0; k < bars.size() && k < 7; k++) {
        const uint64_t v = day_total[6 - k];   // oldest first, today last
        bars[k]->setHeight(v && day_max ? std::max(4.0f, WEEK_BAR_MAX * (float)v / (float)day_max) : 3.0f);
        bars[k]->setBackgroundColor(k == 6 ? theme["cadran/teal_fill"] : theme["cadran/bar"]);
    }
    const auto all = activity_summary::summarize(s, activity_summary::AllTime);
    total_value->setText(all.known ? ui::fmt_play_time(all.total_s) : "—");
    total_sub->setText(all.top >= 0 && s.games[all.top].name[0] ? std::string("le plus joué · ") + s.games[all.top].name : "");

    // The games of the period, most played first.
    const int p = this->period;
    const std::string period_name = brls::getStr(fmt::format("playguard/activity/periods/{}", p));
    period_label->setText(period_name);
    games_title->setText("JEUX · " + cadran::upper(period_name));
    std::vector<const GameStat*> games;
    for (uint32_t g = 0; g < s.count; g++)
        if (activity_summary::value(s.games[g], p) > 0) games.push_back(&s.games[g]);
    std::sort(games.begin(), games.end(), [p](const GameStat* a, const GameStat* b) {
        return activity_summary::value(*a, p) > activity_summary::value(*b, p);
    });
    static const char* none[] = { "Aucun jeu joué aujourd'hui.", "Aucun jeu joué ces 7 derniers jours.", "Aucun jeu joué." };
    games_note->setText(games.empty() ? none[p] : "");
    for (size_t i = 0; i < this->cards.size(); i++) {
        Card& c = this->cards[i];
        const bool shown = i < games.size();
        ui::set_visible(c.box, shown);
        if (!shown) continue;
        const GameStat& g = *games[i];
        c.app_id = g.app_id;
        c.name->setText(g.name[0] ? g.name : brls::getStr("playguard/activity/deleted_game", fmt::format("{:016X}", g.app_id)));
        c.today->setText(g.today_s ? ui::fmt_play_time(g.today_s) : "—");
        c.today->setTextColor(g.today_s ? theme["cadran/text"] : theme["cadran/text3"]);
        c.rest->setText(p == activity_summary::Today  ? std::string("aujourd'hui")
                        : p == activity_summary::Week ? "aujourd'hui · " + ui::fmt_play_time(g.week_s) + " sur 7 jours"
                                                      : "aujourd'hui · " + ui::fmt_play_time(g.total_s) + " en tout");
        uint32_t gmax = 0;
        for (int k = 0; k < 7; k++) gmax = std::max(gmax, g.day_s[k]);
        for (int k = 0; k < 7; k++) {
            const uint32_t v = g.day_s[6 - k];
            c.bars[k]->setHeight(v && gmax ? std::max(4.0f, SPARK_MAX * (float)v / (float)gmax) : 3.0f);
            c.bars[k]->setBackgroundColor(k == 6 ? theme["cadran/teal_fill"] : theme["cadran/bar"]);
        }
    }
    this->apply_icons();
    this->load_icons();

    // To rediscover.
    rediscover->clearViews();
    const std::vector<int> picks = activity_summary::rediscover(s, 2);
    rediscover->addView(label("À REDÉCOUVRIR", 14, theme["cadran/text3"]));
    for (size_t n = 0; n < picks.size(); n++) {
        const GameStat& g = s.games[picks[n]];
        auto* b = new brls::Box(brls::Axis::ROW);
        b->setAlignItems(brls::AlignItems::CENTER);
        b->setHeight(34);
        b->setPaddingLeft(8);
        b->setPaddingRight(14);
        b->setCornerRadius(17);
        b->setBorderColor(theme["cadran/line"]);
        b->setBorderThickness(1);
        b->setMarginLeft(10);
        auto* square = new brls::Box();
        square->setWidth(20);
        square->setHeight(20);
        square->setCornerRadius(6);
        square->setBackgroundColor(AVATARS[(n + 2) % 4]);
        square->setMarginRight(8);
        b->addView(square);
        const std::string when = ago(g.last_played, s.now);
        b->addView(label(std::string(g.name) + (when.empty() ? "" : " · " + when), 15, theme["cadran/text2"]));
        rediscover->addView(b);
    }
}

void CadranActivityTab::apply_icons()
{
    for (Card& c : this->cards) {
        const auto it = s_icons.find(c.app_id);
        const bool has = it != s_icons.end() && !it->second.empty();
        if (has) c.icon->setImageFromMem(it->second.data(), (int)it->second.size());
        c.icon->setVisibility(has ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        c.icon_fallback->setVisibility(has ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    }
}

void CadranActivityTab::load_icons()
{
    if (s_icons_busy) return;
    auto wanted = std::make_shared<std::vector<PlayIcon>>();
    for (const Card& c : this->cards)
        if (c.app_id && !s_icons.count(c.app_id) && !s_no_icon.count(c.app_id)) wanted->push_back(PlayIcon{ c.app_id, nullptr, 0 });
    if (wanted->empty()) return;
    s_icons_busy = true;
    std::weak_ptr<bool> weak = this->alive;
    brls::async([wanted, weak]() {
        playstats_icons(wanted->data(), wanted->size());
        brls::sync([wanted, weak]() {
            s_icons_busy = false;
            for (PlayIcon& icon : *wanted) {
                if (icon.jpeg && icon.size) s_icons[icon.app_id].assign(icon.jpeg, icon.jpeg + icon.size);
                else s_no_icon.insert(icon.app_id);
                std::free(icon.jpeg);
                icon.jpeg = nullptr;
            }
            if (auto keep = weak.lock()) {
                // The tab still exists: show them.
                TabBase::refresh_shown();
            }
        });
    });
}

brls::View* CadranActivityTab::create() { return new CadranActivityTab(); }
