// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/activity_tab.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <string>
#include <vector>

#include "action/export_flow.hpp"
#include "action/game_icons.hpp"
#include "action/outside_watch.hpp"
#include "action/play_data.hpp"
#include "action/week_qr.hpp"
#include "activity/game_activity.hpp"
#include "activity/games_activity.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"
#include "util/text.hpp"

using namespace brls::literals;

namespace
{
// Coming back to the tab (after a dialog, the sort picker …) within this time
// reuses the last read instead of walking the activity log again.
constexpr auto MAX_AGE = std::chrono::seconds(60);
constexpr size_t CARDS = 4;
constexpr float  WEEK_BAR_MAX = 56.0f;
constexpr float  SPARK_MAX = 40.0f;
// The accounts' avatars, in turn (no picture of theirs is read).
const char* const AVATARS[] = { "cadran/avatar0", "cadran/avatar1", "cadran/avatar2", "cadran/avatar3" };


// Shared by every ActivityTab (borealis rebuilds the tab each time the
// sidebar reaches it); the play data itself is play_data's. UI thread only.
struct
{
    int account = -1;                   // index in play_data::accounts(), -1 every account
    bool account_set = false;           // `account` taken from the config, once per run
} s_cache;

// The account the tab shows, or nullptr for every account.
const PlayAccount* chosen_account()
{
    const auto& list = play_data::accounts();
    return s_cache.account >= 0 && s_cache.account < (int)list.size() ? &list[s_cache.account] : nullptr;
}

std::string account_label(const PlayAccount* a)
{
    if (!a) return "playguard/activity/account_all"_i18n;
    return a->nickname[0] ? std::string(a->nickname) : "?";
}

// Each account's play_data key, in play_data::accounts() order.
std::vector<std::string> account_keys()
{
    std::vector<std::string> keys;
    for (const auto& a : play_data::accounts()) keys.push_back(play_data::key_of(&a));
    return keys;
}

// What Activity starts on (config's activity_account): the last one shown,
// every account or one.
std::string default_label(const std::string& setting)
{
    if (setting == "last") return "playguard/activity/account_last"_i18n;
    const auto& list = play_data::accounts();
    const int i = activity_summary::start_account(setting, "", account_keys());
    return account_label(i >= 0 ? &list[i] : nullptr);
}

std::shared_ptr<const PlayStats> shown_stats()
{
    return play_data::latest(play_data::key_of(chosen_account()));
}

uint64_t value_of(const GameStat& g, int period)
{
    return activity_summary::value(g, period);
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

ActivityTab::ActivityTab()
    : TabBase("xml/tab/activity.xml"), list(this)
{
    this->list.apply({ { "ac_account_default", "user" }, { "ac_games", "list" } });
    this->build_cards();
    period_chip->registerClickAction([this](brls::View*) {
        this->choose_period();
        return true;
    });
    // The figures to the SD card, or the week's summary as a QR code for a
    // phone: one chip, both a press away.
    export_chip->registerClickAction([this](brls::View*) {
        ui::pick_actions("playguard/cadran/activity/share"_i18n,
                         { ui::Choice("playguard/activity/share_sd"_i18n, [this]() { this->export_to_sd(); }, "download"),
                           ui::Choice("playguard/activity/qr"_i18n, []() {
                               if (const auto data = shown_stats()) week_qr::show(*data, account_label(chosen_account()));
                               else ui::notify("playguard/activity/export_not_ready"_i18n);
                           }, "phone") });
        return true;
    });
    this->build_chips();

    status->setSingleLine(false);
    capped->setSingleLine(false);
    note->setSingleLine(false);
    // Ⓧ reads the data again, however recent the last read.
    this->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->pt_read = false;   // the limits too
        this->fetch(true);
        return true;
    });
    account_default->registerClickAction([this](brls::View*) {
        // The last one shown, every account, then each account.
        const auto& list = play_data::accounts();
        const std::vector<std::string> keys = account_keys();
        std::vector<std::string> labels = { "playguard/activity/account_last"_i18n, account_label(nullptr) };
        for (const auto& a : list) labels.push_back(account_label(&a));
        const std::string& setting = config::get().activity_account;
        const int selected = setting == "last" ? 0 : activity_summary::start_account(setting, "", keys) + 2;
        ui::pick("playguard/activity/account_default"_i18n, labels, selected, [this, keys](int index) {
            config::get().activity_account = index == 0 ? "last" : index == 1 ? "" : keys[index - 2];
            ui::save_config();
            this->account_default->setDetailText(default_label(config::get().activity_account));
        });
        return true;
    });
    games->registerClickAction([this](brls::View*) {
        // Every game of the period, as a list or as covers, on its own screen.
        brls::Application::pushActivity(new GamesActivity(this->period, s_cache.account));
        return true;
    });
    this->period = config::get().activity_period;
    this->listener = play_data::listen([this]() {
        if (!ui::on_top(this->getParentActivity())) {
            this->stale = true;
            return;
        }
        this->rebuild();
    });
    // The account it starts on, once per run (the sidebar keeps the one chosen).
    if (!s_cache.account_set) {
        s_cache.account_set = true;
        const auto& cfg = config::get();
        s_cache.account = activity_summary::start_account(cfg.activity_account, cfg.activity_account_last, account_keys());
    }
    // No account to choose between on a console with one (or none listed).
    ui::set_visible(account_default.getView(), play_data::accounts().size() > 1);
    account_default->setDetailText(default_label(config::get().activity_account));
    // An account not seen before (a fresh play record is a known trick).
    new_account->setSingleLine(false);
    const std::string fresh = text::join(outside_watch::new_account_lines(), "\n");
    new_account->setText(fresh);
    ui::set_visible(new_account, !fresh.empty());
    ui::set_visible(progress.getView(), false);
    this->rebuild();
}

ActivityTab::~ActivityTab()
{
    play_data::unlisten(this->listener);
}

void ActivityTab::show_account(int index)
{
    s_cache.account = index;
    // The next run starts there, when Activity starts on the last one shown.
    auto& cfg = config::get();
    const std::string key = play_data::key_of(chosen_account());
    if (cfg.activity_account_last != key) {
        cfg.activity_account_last = key;
        ui::save_config();
    }
    this->rebuild();
    if (!play_data::fresh(play_data::key_of(chosen_account()), MAX_AGE)) this->fetch(false);
}

int ActivityTab::shown_account()
{
    return chosen_account() ? s_cache.account : -1;
}

std::shared_ptr<const PlayStats> ActivityTab::shown_data()
{
    return shown_stats();
}

void ActivityTab::refresh()
{
    this->list.sync();
    cadran::set_pill("");
    // The period, as All games may have changed it.
    if (this->period != config::get().activity_period) {
        this->period = config::get().activity_period;
        this->stale = true;
    }
    if (this->stale) {
        // Back on top: next frame, once borealis has put the focus back.
        this->stale = false;
        std::weak_ptr<bool> weak = this->alive;
        brls::sync([this, weak]() {
            if (!weak.expired()) this->rebuild();
        });
    }
    if (play_data::fresh(play_data::key_of(chosen_account()), MAX_AGE)) return;
    this->fetch(false);
}

void ActivityTab::fetch(bool shown)
{
    // A long log takes a few seconds. What is known stays on screen meanwhile
    // (the last read, this run's or the last run's), so a read of its own is
    // silent; Ⓧ, or nothing to show yet, gets the spinner.
    const bool nothing = !shown_stats();
    if (shown) this->spinner = true;
    if (shown || nothing) {
        ui::set_visible(progress.getView(), true);
        ui::set_visible(status, false);
    }
    play_data::fetch(chosen_account());   // one read per account at a time
}

void ActivityTab::rebuild()
{
    const PlayAccount* who = chosen_account();
    const bool reading = play_data::busy(play_data::key_of(who));
    if (!reading) this->spinner = false;
    const std::shared_ptr<const PlayStats> data = shown_stats();
    if (!data) {
        // Nothing read for this account yet: no figures, not the previous ones.
        games->setDetailText(ui::opens());
        ui::set_visible(capped, false);
        this->show_summary(nullptr);
        this->show_hours(nullptr);
        ui::set_visible(progress.getView(), reading);
        this->show_top();
        return;
    }
    const PlayStats& s = *data;
    this->show_totals(s, who);

    // The games played in the chosen period: All games lists them.
    size_t played = 0;
    for (uint32_t i = 0; i < s.count; i++)
        if (value_of(s.games[i], this->period) > 0) played++;
    games->setDetailText(ui::opens(std::to_string(played)));
    ui::set_visible(progress.getView(), reading && this->spinner);
    // Under it: the games the read itself could not hold (PLAYSTATS_MAX),
    // which the totals above miss too.
    capped->setText(s.truncated ? brls::getStr("playguard/activity/truncated", PLAYSTATS_MAX) : std::string());
    ui::set_visible(capped, s.truncated);

    this->show_summary(data);
    this->show_hours(data);
    this->show_status(s, played == 0);
    this->show_top();
}

// The chart of the last 7 days.
void ActivityTab::show_totals(const PlayStats& s, const PlayAccount* who)
{
    if (s.windows_ok) {
        // Every account: each day's current limit as a line (the limit is the
        // console's, not an account's). A day above it is drawn in amber.
        uint16_t limits[7];
        const PtState* pt = who ? nullptr : &this->play_timer();
        const bool with_limits = pt && pt->fw_supported && pt->valid;
        for (int k = 0; k < 7; k++) limits[k] = with_limits ? pt->day_min[s.day_wday[k] % 7] : PT_DAY_NOLIMIT;
        days->show(s, with_limits ? limits : nullptr);
    }
    ui::set_visible(days.getView(), s.windows_ok);
}

// Under All games: why it is empty, or what could not be read.
void ActivityTab::show_status(const PlayStats& s, bool empty)
{
    const int p = this->period;
    std::string text;
    bool error = true;
    if (R_FAILED(s.rc))
        text = "playguard/activity/err_list"_i18n + " — " + ui::rc_text(s.rc);
    else if (p != AllTime && !activity_summary::period_read(s, p))
        text = "playguard/activity/err_log"_i18n + " — " + ui::rc_text(s.events_rc);
    else if (p == AllTime && R_FAILED(s.stats_rc))
        text = "playguard/activity/err_stats"_i18n + " — " + ui::rc_text(s.stats_rc);
    else if (empty) {
        text  = brls::getStr(fmt::format("playguard/activity/none/{}", p));
        error = false;
    }
    status->setText(text);
    status->setTextColor(error ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text3"));
    ui::set_visible(status, !text.empty());
}

const PtState& ActivityTab::play_timer()
{
    const auto now = std::chrono::steady_clock::now();
    if (!this->pt_read || now - this->pt_at > MAX_AGE) {
        pctl_play_timer_query(&this->pt);
        this->pt_read = true;
        this->pt_at   = now;
    }
    return this->pt;
}

void ActivityTab::show_summary(const std::shared_ptr<const PlayStats>& data)
{
    const activity_summary::Summary sum = data ? activity_summary::summarize(*data, this->period)
                                               : activity_summary::Summary();
    const int p = this->period;
    if (sum.known && sum.days > 0) {
        per_day->setDetailText(p == AllTime ? ui::fmt_per_day_over(sum.per_day_s, sum.days) : ui::fmt_play_time(sum.per_day_s));
    }
    if (sum.known && sum.top >= 0)
        top->setDetailText(play_data::game_name(data->games[sum.top]) + " · " + ui::fmt_play_time(sum.top_s));
    if (sum.known && sum.days_played >= 0)
        days_played->setDetailText(brls::getStr("playguard/activity/summary/days_played_value", sum.days_played));
    if (sum.known && sum.busiest >= 0)
        busiest->setDetailText(brls::getStr(fmt::format("playguard/days/{}", (int)data->day_wday[sum.busiest] % 7)) + " · " +
                               ui::fmt_play_time(sum.busiest_s));
    if (sum.known && sum.session_s > 0) session->setDetailText(ui::fmt_play_time(sum.session_s));
    ui::set_visible_all({
        { per_day.getView(), sum.known && sum.days > 0 },
        { top.getView(), sum.known && sum.top >= 0 },
        { days_played.getView(), sum.known && sum.days_played >= 0 },
        { busiest.getView(), sum.known && sum.busiest >= 0 },
        { session.getView(), sum.known && sum.session_s > 0 },
    });
}

void ActivityTab::show_hours(const std::shared_ptr<const PlayStats>& data)
{
    const activity_summary::Hours h = data ? activity_summary::hours(*data) : activity_summary::Hours();
    if (data) hours->show(*data);
    if (h.sessions)
        sessions->setDetailText(brls::getStr("playguard/activity/hours/sessions_value", (int)h.sessions,
                                             ui::fmt_play_time(h.average_s)));
    if (h.longest_day >= 0)
        longest->setDetailText(ui::fmt_play_time(h.longest_s) + " · " +
                               brls::getStr(fmt::format("playguard/days/{}", (int)data->day_wday[h.longest_day] % 7)));
    if (h.busy_from >= 0)
        busy_hours->setDetailText(brls::getStr("playguard/activity/hours/busy_value", fmt::format("{:02d}:00", h.busy_from),
                                               fmt::format("{:02d}:00", h.busy_from + 2)));
    late->setDetailText(ui::fmt_play_time(h.late_s));
    ui::set_visible_all({
        { hours_header.getView(), h.known },
        { hours.getView(), h.known },
        { hours_legend.getView(), h.known },
        { sessions.getView(), h.known && h.sessions > 0 },
        { longest.getView(), h.known && h.longest_day >= 0 },
        { busy_hours.getView(), h.known && h.busy_from >= 0 },
        { late.getView(), h.known && h.sessions > 0 },
    });
}

void ActivityTab::choose_period()
{
    std::vector<std::string> labels;
    int selected = 0;
    for (int i = 0; i < activity_summary::PERIOD_COUNT; i++) {
        const int p = activity_summary::PERIODS[i];
        labels.push_back(brls::getStr(fmt::format("playguard/activity/periods/{}", p)));
        if (p == this->period) selected = i;
    }
    ui::pick("playguard/activity/period"_i18n, labels, selected, [this](int index) {
        this->period = activity_summary::PERIODS[index];
        config::get().activity_period = this->period;   // the next visit starts there
        ui::save_config();
        this->rebuild();
    });
}

void ActivityTab::build_cards()
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    // The game cards, built once, two rows of two; filled by show_top().
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
        c.icon_fallback->setCornerRadius(cadran::kit::RADIUS_ICON);
        c.icon_fallback->setBackgroundColor(theme["cadran/surf2"]);
        c.icon_fallback->setMarginRight(16);
        c.box->addView(c.icon_fallback);
        c.icon = new brls::Image();
        c.icon->setWidth(60);
        c.icon->setHeight(60);
        c.icon->setCornerRadius(cadran::kit::RADIUS_ICON);
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
        // Against the week before, as the week's tile says it: a caret, the
        // amount and "vs last week".
        c.grew_row = new brls::Box(brls::Axis::ROW);
        c.grew_row->setAlignItems(brls::AlignItems::CENTER);
        c.grew_row->setMarginTop(2);
        CadranIcon* up = kit::icon("caret-up", 14, "cadran/amber");
        up->setMarginRight(4);
        c.grew_row->addView(up);
        c.grew = kit::label("", 15, "cadran/amber");
        c.grew->setSingleLine(true);
        c.grew_row->addView(c.grew);
        c.grew_row->setVisibility(brls::Visibility::GONE);
        text->addView(c.grew_row);
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
}

void ActivityTab::build_chips()
{
    brls::Theme theme = brls::Application::getTheme();
    const auto& accounts = play_data::accounts();
    // No account to choose between on a console with one (as the tab's line).
    if (accounts.size() < 2) return;
    // Many accounts go on to a second line, clear of the period and export;
    // one line keeps its place (the chips' margins taken back).
    cadran::kit::wrap(accounts_row);
    accounts_row->setMarginTop(-4);
    accounts_row->setMarginBottom(-4);
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
        chip->setMarginTop(4);   // apart once on two lines (up to 8 accounts)
        chip->setMarginBottom(4);
        chip->setShrink(0);
        if (i < 0) {
            chip->addView(cadran::kit::label("playguard/activity/account_all"_i18n, 16));
        } else {
            const std::string nick = accounts[i].nickname[0] ? std::string(accounts[i].nickname) : "?";
            auto* avatar = new brls::Box(brls::Axis::ROW);
            avatar->setWidth(32);
            avatar->setHeight(32);
            avatar->setCornerRadius(16);
            avatar->setBackgroundColor(cadran::kit::color(AVATARS[i % 4]));
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

void ActivityTab::show_top()
{
    brls::Theme theme = brls::Application::getTheme();
    const int who = shown_account();
    for (size_t i = 0; i < this->chips.size(); i++) {
        // The account shown: a soft fill and its ink, the hairline kept; the
        // teal ring is the focus's alone.
        const bool on = (int)i - 1 == who;
        this->chips[i]->setBackgroundColor(on ? theme["cadran/teal_soft"] : theme["cadran/surf"]);
        this->chips[i]->setBorderColor(theme["cadran/line"]);
        this->chips[i]->setBorderThickness(1);
        for (brls::View* v : this->chips[i]->getChildren())
            if (auto* l = dynamic_cast<brls::Label*>(v)) l->setTextColor(on ? theme["cadran/teal"] : theme["cadran/text"]);
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
    const auto today = activity_summary::today_play(s);
    std::string sub = today.games == 1 ? "playguard/cadran/activity/games_one"_i18n
                                       : brls::getStr("playguard/cadran/activity/games", today.games);
    if (who < 0) {
        const PtState& pt = this->play_timer();
        const uint16_t limit = pt.fw_supported && pt.valid ? pt.day_min[ui::today_weekday()] : PT_DAY_NOLIMIT;
        sub = (limit == PT_DAY_NOLIMIT ? "playguard/cadran/week/no_limit"_i18n
                                       : brls::getStr("playguard/cadran/activity/on_limit", ui::fmt_minutes(limit))) + " · " + sub;
    }
    today_value->setText(s.windows_ok ? ui::fmt_play_time(today.total_s) : "—");
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
    using activity_summary::Trend;
    const auto trend = activity_summary::week_trend(s);
    if (trend.known) {
        brls::Theme theme = brls::Application::getTheme();
        const bool more = trend.trend == Trend::More, less = trend.trend == Trend::Less;
        ui::set_text(week_trend, more   ? brls::getStr("playguard/cadran/activity/trend_up", ui::fmt_minutes(trend.amount_min))
                                 : less ? brls::getStr("playguard/cadran/activity/trend_down", ui::fmt_minutes(trend.amount_min))
                                        : "playguard/cadran/activity/trend_same"_i18n);
        week_trend->setTextColor(theme[more ? "cadran/amber" : less ? "cadran/teal" : "cadran/text3"]);
        // Up or down said by the shape too, not by the colour alone.
        week_trend_icon->setIcon(trend.per_day_min >= 0 ? "caret-up" : "caret-down");
        week_trend_icon->setColor(theme[more ? "cadran/amber" : "cadran/teal"]);
        ui::set_visible(week_trend_icon.getView(), more || less);
    }
    ui::set_visible(week_trend_row.getView(), trend.known);
    uint64_t day_total[7];
    activity_summary::day_totals(s, day_total);
    show_bars(week_bars->getChildren(), day_total, WEEK_BAR_MAX);

    // All time, and the game most played.
    const auto all = activity_summary::summarize(s, activity_summary::AllTime);
    total_value->setText(all.known ? ui::fmt_play_time(all.total_s) : "—");
    total_sub->setText(all.known && all.top >= 0 ? brls::getStr("playguard/cadran/activity/top", play_data::game_name(s.games[all.top])) : "");

    // The games of the period, most played first.
    const std::vector<const GameStat*> games = activity_summary::ranked_games(s, p);
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
    const auto grew = activity_summary::grew_most(s, games, this->cards.size(), p);
    std::vector<u64> ids;
    for (size_t i = 0; i < this->cards.size(); i++) {
        Card& c = this->cards[i];
        const bool shown = i < games.size();
        ui::set_visible(c.box, shown);
        if (!shown) continue;
        ui::set_visible(c.grew_row, games[i] == grew.game);
        if (games[i] == grew.game)
            ui::set_text(c.grew, brls::getStr("playguard/cadran/activity/grew", ui::fmt_play_time(grew.more_s)));
        const GameStat& g = *games[i];
        c.app_id = g.app_id;
        ids.push_back(g.app_id);
        c.name->setText(play_data::game_name(g));
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

void ActivityTab::show_icons()
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

// Every game with any figure, in the order of the list on screen; minutes as
// numbers, so a spreadsheet can add them up.
void ActivityTab::export_to_sd() const
{
    export_stats(shown_stats(), this->period, chosen_account());
}

void ActivityTab::export_stats(std::shared_ptr<const PlayStats> data, int p, const PlayAccount* account)
{
    if (!data) {
        ui::notify("playguard/activity/export_not_ready"_i18n);
        return;
    }
    const std::string who = account_label(account);
    export_flow::choose("export", "activity", [data, p, who]() {
        const PlayStats& s = *data;
        activity_summary::ExportText text;
        text.title    = "PlayGuard — " + "playguard/tabs/activity"_i18n;
        text.subtitle = brls::getStr("playguard/activity/export_subtitle", ui::time_text(s.now), who);
        if (s.truncated) text.subtitle += " · " + brls::getStr("playguard/activity/truncated", PLAYSTATS_MAX);
        text.sheet    = "playguard/tabs/activity"_i18n;
        text.game     = "playguard/activity/columns/game"_i18n;
        text.id       = "playguard/activity/columns/id"_i18n;
        text.today    = "playguard/activity/columns/today"_i18n;
        text.week     = "playguard/activity/columns/week"_i18n;
        text.month    = "playguard/activity/columns/month"_i18n;
        text.total    = "playguard/activity/columns/total"_i18n;
        text.launches = "playguard/activity/launches"_i18n;
        text.first    = "playguard/activity/first"_i18n;
        text.last     = "playguard/activity/last"_i18n;
        text.day = [](int wday) {
            return brls::getStr("playguard/activity/columns/day", brls::getStr(fmt::format("playguard/days_short/{}", wday)));
        };
        text.name = [](const GameStat& g) { return play_data::game_name(g); };
        text.time = [](uint64_t posix) { return ui::time_text(posix); };
        return activity_summary::export_table(s, p, text);
    });
}

brls::View* ActivityTab::create()
{
    return new ActivityTab();
}
