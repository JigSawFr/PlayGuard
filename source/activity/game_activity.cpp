// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/game_activity.hpp"

#include <string>
#include <vector>

#include "action/play_data.hpp"
#include "action/rating_flow.hpp"
#include "action/rating_logic.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"

using namespace aurore::literals;

GameActivity::GameActivity(GameStat shown, std::shared_ptr<const PlayStats> play_stats, std::vector<unsigned char> cover,
                           bool all_accounts)
    : game(shown), stats(std::move(play_stats)), icon_bytes(std::move(cover)), every_account(all_accounts)
{
}

void GameActivity::onContentAvailable()
{
    accounts_note->setSingleLine(false);
    const std::string na = "cadran/common/unavailable"_i18n;
    const GameStat& g = this->game;

    name->setText(play_data::game_name(g));
    if (!this->icon_bytes.empty()) icon->setImageFromMem(this->icon_bytes.data(), (int)this->icon_bytes.size());
    else ui::set_visible(icon.getView(), false);

    const bool windows = this->stats && this->stats->windows_ok;
    ui::set_visible(days.getView(), windows);
    if (windows) days->show(g.day_s, this->stats->day_wday);
    today->setDetailText(windows ? ui::fmt_duration(g.today_s) : na);
    week->setDetailText(windows ? ui::fmt_duration(g.week_s) : na);
    total->setDetailText(g.totals_ok ? ui::fmt_duration(g.total_s) : na);
    launches->setDetailText(g.totals_ok ? std::to_string(g.launches) : na);
    first->setDetailText(g.totals_ok && g.first_played ? ui::time_text(g.first_played) : "—");
    last->setDetailText(g.totals_ok && g.last_played ? ui::time_text(g.last_played) : "—");
    // Its age rating by the console's organisation (the last game check), and
    // whether the level in force blocks it.
    std::string rated;
    if (const GameFacts* facts = rating_flow::facts_of(g.app_id)) {
        PctlStatus st = {};
        pctl_status_fetch(&st);
        rated = rating_flow::game_rating(st, *facts);
        const int age = st.safety_level_ok ? rating_flow::level_age(st, st.safety_level) : -1;
        if (!rated.empty() && age > 0 && rating_logic::blocked(*facts, st.rating_org, (uint8_t)age))
            rated = aurore::getStr("playguard/activity/rating_blocked", rated);
    }
    rating->setDetailText(rated);
    ui::set_visible(rating.getView(), !rated.empty());

    // All time per user account: up to eight statistics and profile reads,
    // off the UI thread (the screen is up meanwhile, the note says so).
    accounts_note->setText("playguard/activity/loading"_i18n);
    if (!g.totals_ok) {
        this->show_accounts({}, 0);
        return;
    }
    std::weak_ptr<bool> weak = this->alive;
    const u64 app_id = g.app_id;
    ui::in_background("account play", [this, weak, app_id]() {
        auto found = std::make_shared<std::vector<AccountPlay>>(PLAYSTATS_MAX_ACCOUNTS);
        Result rc = 0;
        found->resize(playstats_by_account(app_id, found->data(), found->size(), &rc));
        if (ui::quitting()) return;
        aurore::sync(ui::guarded(weak, [this, found, rc]() { this->show_accounts(*found, rc); }));
    });
}

void GameActivity::show_accounts(const std::vector<AccountPlay>& by_account, Result rc)
{
    const GameStat& g = this->game;
    for (const AccountPlay& a : by_account) {
        auto* cell = new aurore::DetailCell();
        cell->setFocusable(false);
        cell->setText(a.nickname[0] ? std::string(a.nickname) : "?");
        cell->setDetailText(aurore::getStr("playguard/activity/account_value", ui::fmt_duration(a.total_s), (int)a.launches));
        accounts->addView(cell);
    }
    // The game's own total less the accounts': time with no account chosen
    // (some games never ask) or on an account deleted since. Only against
    // every account's total (one account's is already its own), and the
    // accounts read.
    const bool comparable = this->every_account && g.totals_ok && R_SUCCEEDED(rc);
    const uint64_t nobody = comparable ? activity_summary::unattributed_s(g.total_s, by_account.data(), by_account.size()) : 0;
    if (nobody) {
        auto* cell = new aurore::DetailCell();
        cell->setFocusable(false);
        cell->setText("playguard/activity/account_nobody"_i18n);
        cell->setDetailText(ui::fmt_duration(nobody));
        accounts->addView(cell);
    }
    std::string note;
    if (!g.totals_ok) note = "playguard/activity/accounts_none"_i18n;
    else if (by_account.empty() && R_FAILED(rc)) note = "playguard/activity/err_stats"_i18n + " — " + ui::rc_text(rc);
    else if (by_account.empty()) note = "playguard/activity/accounts_none"_i18n;
    accounts_note->setText(note);
    ui::set_visible(accounts_note.getView(), !note.empty());
}
