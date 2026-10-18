// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/activity_tab.hpp"

#include <fmt/format.h>
#include <string>
#include <vector>

#include "action/export_flow.hpp"
#include "action/outside_watch.hpp"
#include "action/play_data.hpp"
#include "activity/games_activity.hpp"
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

std::string game_name(const GameStat& g)
{
    if (g.name[0]) return g.name;
    return brls::getStr("playguard/activity/deleted_game", fmt::format("{:016X}", (unsigned long long)g.app_id));
}

}   // namespace

ActivityTab::ActivityTab()
    : TabBase("xml/tab/activity.xml")
{
    status->setSingleLine(false);
    capped->setSingleLine(false);
    note->setSingleLine(false);
    // Ⓧ reads the data again, however recent the last read.
    this->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->pt_read = false;   // the limits too
        this->fetch(true);
        return true;
    });
    account->registerClickAction([this](brls::View*) {
        const auto& list = play_data::accounts();
        std::vector<std::string> labels = { account_label(nullptr) };
        for (const auto& a : list) labels.push_back(account_label(&a));
        ui::pick("playguard/activity/account"_i18n, labels, s_cache.account + 1, [this](int index) {
            s_cache.account = index - 1;
            // The next run starts there, when Activity starts on the last one shown.
            auto& cfg = config::get();
            const std::string key = play_data::key_of(chosen_account());
            if (cfg.activity_account_last != key) {
                cfg.activity_account_last = key;
                ui::save_config();
            }
            this->rebuild();
            if (!play_data::fresh(play_data::key_of(chosen_account()), MAX_AGE)) this->fetch(false);
        });
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
    sort->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (int p = 0; p < 3; p++) labels.push_back(brls::getStr(fmt::format("playguard/activity/periods/{}", p)));
        ui::pick("playguard/activity/period"_i18n, labels, this->period, [this](int index) {
            this->period = index;
            config::get().activity_period = index;   // the next visit starts there
            ui::save_config();
            this->rebuild();
        });
        return true;
    });
    games->registerClickAction([this](brls::View*) {
        // Every game of the period, as a list or as covers, on its own screen.
        brls::Application::pushActivity(new GamesActivity(this->period, s_cache.account));
        return true;
    });
    export_cell->registerClickAction([this](brls::View*) {
        this->export_to_sd();
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
    const bool several = play_data::accounts().size() > 1;
    ui::set_visible(account.getView(), several);
    ui::set_visible(account_default.getView(), several);
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

void ActivityTab::refresh()
{
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
    sort->setDetailText(brls::getStr(fmt::format("playguard/activity/periods/{}", this->period)));
    account->setDetailText(account_label(who));
    const bool reading = play_data::busy(play_data::key_of(who));
    if (!reading) this->spinner = false;
    const std::shared_ptr<const PlayStats> data = shown_stats();
    if (!data) {
        // Nothing read for this account yet: no figures, not the previous ones.
        games->setDetailText(ui::opens());
        ui::set_visible(capped, false);
        this->show_summary(nullptr);
        ui::set_visible(progress.getView(), reading);
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
    this->show_status(s, played == 0);
}

// The play time today, over the last 7 days and in all, and the chart.
void ActivityTab::show_totals(const PlayStats& s, const PlayAccount* who)
{
    const std::string na = "playguard/common/unavailable"_i18n;

    uint64_t today_total = 0, week_total = 0, all_total = 0;
    for (uint32_t i = 0; i < s.count; i++) {
        today_total += s.games[i].today_s;
        week_total  += s.games[i].week_s;
        if (s.games[i].totals_ok) all_total += s.games[i].total_s;
    }
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
    today->setDetailText(s.windows_ok ? ui::fmt_play_time(today_total) : na);
    week->setDetailText(s.windows_ok ? ui::fmt_play_time(week_total) : na);
    total->setDetailText(R_SUCCEEDED(s.stats_rc) ? ui::fmt_play_time(all_total) : na);
}

// Under All games: why it is empty, or what could not be read.
void ActivityTab::show_status(const PlayStats& s, bool empty)
{
    const int p = this->period;
    std::string text;
    bool error = true;
    if (R_FAILED(s.rc))
        text = "playguard/activity/err_list"_i18n + " — " + ui::rc_text(s.rc);
    else if (p != AllTime && !s.windows_ok)
        text = "playguard/activity/err_log"_i18n + " — " + ui::rc_text(s.events_rc);
    else if (p == AllTime && R_FAILED(s.stats_rc))
        text = "playguard/activity/err_stats"_i18n + " — " + ui::rc_text(s.stats_rc);
    else if (empty) {
        text  = brls::getStr(fmt::format("playguard/activity/none/{}", p));
        error = false;
    }
    status->setText(text);
    status->setTextColor(error ? ui::color_warn() : ui::color_note());
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
        top->setDetailText(game_name(data->games[sum.top]) + " · " + ui::fmt_play_time(sum.top_s));
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

// Every game with any figure, in the order of the list on screen; minutes as
// numbers, so a spreadsheet can add them up.
void ActivityTab::export_to_sd() const
{
    const std::shared_ptr<const PlayStats> data = shown_stats();
    if (!data) {
        ui::notify("playguard/activity/export_not_ready"_i18n);
        return;
    }
    const int p = this->period;
    const std::string who = account_label(chosen_account());
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
        text.total    = "playguard/activity/columns/total"_i18n;
        text.launches = "playguard/activity/launches"_i18n;
        text.first    = "playguard/activity/first"_i18n;
        text.last     = "playguard/activity/last"_i18n;
        text.day = [](int wday) {
            return brls::getStr("playguard/activity/columns/day", brls::getStr(fmt::format("playguard/days_short/{}", wday)));
        };
        text.name = [](const GameStat& g) { return game_name(g); };
        text.time = [](uint64_t posix) { return ui::time_text(posix); };
        return activity_summary::export_table(s, p, text);
    });
}

brls::View* ActivityTab::create()
{
    return new ActivityTab();
}
