// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/activity_tab.hpp"

#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <limits>
#include <string>
#include <vector>

#include <map>
#include <set>

#include <stb_image.h>

#include "action/icon_logic.hpp"
#include "action/outside_watch.hpp"
#include "action/play_data.hpp"
#include "activity/game_activity.hpp"
#include "ui/ui.hpp"
#include "util/activity_summary.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/table_export.hpp"

using namespace brls::literals;

namespace
{
// Coming back to the tab (after a dialog, the sort picker …) within this time
// reuses the last read instead of walking the activity log again.
constexpr auto MAX_AGE = std::chrono::seconds(60);

// Icons for the first rows of the list only: the list can hold hundreds of
// games.
constexpr size_t ICON_ROWS = 16;

// Rows built at first: each is a cell inflated from XML, and a large library
// holds hundreds of games. "Show all" builds the rest on demand.
constexpr size_t ROWS_SHOWN = 50;

// Rows "Show all" builds at most in applet mode (opened from the album, a
// small heap): about 50 KB each.
constexpr size_t APPLET_ROWS_MAX = 100;

// Decoded icons kept for the run, at most this many: past it, a cell decodes
// its own and frees it.
constexpr size_t TEXTURES_KEPT = 48;

// Icons are decoded at this size at most: drawn at 44 px, 88 keeps them sharp
// docked. The control data's are 256 x 256 (256 KB of texture each, 30 here).
constexpr int ICON_PX = 88;

// Icons' bytes kept, for this many games at most (up to 128 KiB each): one
// pushed out is read again from the console when its row needs it.
constexpr size_t ICON_BYTES_KEPT = 64;

// Shared by every ActivityTab (borealis rebuilds the tab each time the
// sidebar reaches it); the play data itself is play_data's. UI thread only.
struct
{
    ActivityTab* shown = nullptr;       // the tab on screen, if any
    int account = -1;                   // index in play_data::accounts(), -1 every account
    // "Show every game" chosen: kept when the sidebar leaves the tab and
    // comes back, until another period or account.
    bool show_all = false;
    // Icons read so far (the bytes as the control data holds them: the game's
    // screen shows them larger), and the games known to have none (icons do
    // not change).
    icon_logic::Lru icons{ ICON_BYTES_KEPT };
    std::map<u64, int> textures;        // icons decoded once, small (NVG images), for the run
    std::set<u64> no_icon;
    bool icons_busy = false;
    bool icons_wanted = true;           // false in applet mode (little memory)
    size_t rows_max = std::numeric_limits<size_t>::max();   // APPLET_ROWS_MAX in applet mode
} s_cache;

// An icon as a texture no larger than ICON_PX (0 when it cannot be decoded):
// what nvgCreateImageMem does, shrunk before it reaches the GPU.
int icon_texture(const std::vector<unsigned char>& bytes)
{
    int w = 0, h = 0, comp = 0;
    unsigned char* rgba = stbi_load_from_memory(bytes.data(), (int)bytes.size(), &w, &h, &comp, 4);
    if (!rgba) return 0;
    const auto size = icon_logic::fit(w, h, ICON_PX);
    NVGcontext* vg = brls::Application::getNVGContext();
    int tex = 0;
    if (size.first == w && size.second == h) {
        tex = nvgCreateImageRGBA(vg, w, h, 0, rgba);
    } else {
        const std::vector<unsigned char> small = icon_logic::shrink(rgba, w, h, size.first, size.second);
        if (!small.empty()) tex = nvgCreateImageRGBA(vg, size.first, size.second, 0, small.data());
    }
    stbi_image_free(rgba);
    return tex;
}

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

std::shared_ptr<const PlayStats> shown_stats()
{
    return play_data::latest(play_data::key_of(chosen_account()));
}

uint64_t value_of(const GameStat& g, int period)
{
    return activity_summary::value(g, period);
}

// Games shown under "To rediscover", at most.
constexpr size_t REDISCOVER_SHOWN = 3;

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
            s_cache.show_all = false;
            this->rebuild();
            if (!play_data::fresh(play_data::key_of(chosen_account()), MAX_AGE)) this->fetch(false);
        });
        return true;
    });
    sort->registerClickAction([this](brls::View*) {
        std::vector<std::string> labels;
        for (int p = 0; p < 3; p++) labels.push_back(brls::getStr(fmt::format("playguard/activity/periods/{}", p)));
        ui::pick("playguard/activity/period"_i18n, labels, this->period, [this](int index) {
            this->period = index;
            s_cache.show_all = false;
            config::get().activity_period = index;   // the next visit starts there
            ui::save_config();
            this->rebuild();
        });
        return true;
    });
    export_cell->registerClickAction([this](brls::View*) {
        this->export_to_sd();
        return true;
    });
    this->period = config::get().activity_period;
    s_cache.shown = this;
    this->listener = play_data::listen([this]() {
        const auto stack = brls::Application::getActivitiesStack();
        if (!stack.empty() && stack.back() != this->getParentActivity()) {
            this->stale = true;
            return;
        }
        this->rebuild();
    });
    // No account to choose between on a console with one (or none listed).
    ui::set_visible(account.getView(), play_data::accounts().size() > 1);
    // An account not seen before (a fresh play record is a known trick).
    new_account->setSingleLine(false);
    std::string fresh;
    for (const auto& l : outside_watch::new_account_lines()) fresh += (fresh.empty() ? "" : "\n") + l;
    new_account->setText(fresh);
    ui::set_visible(new_account, !fresh.empty());
    // Applet mode (opened from the album): a few MB for icons is too much,
    // and hundreds of rows too.
    SysInfo si;
    sysinfo_get(&si);
    s_cache.icons_wanted = !si.applet_mode;
    if (si.applet_mode) s_cache.rows_max = APPLET_ROWS_MAX;
    ui::set_visible(progress.getView(), false);
    this->rebuild();
}

ActivityTab::~ActivityTab()
{
    play_data::unlisten(this->listener);
    if (s_cache.shown == this) s_cache.shown = nullptr;
}

void ActivityTab::refresh()
{
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
        // Nothing read for this account yet: an empty list, not the previous one.
        list->clearViews();
        this->cells.clear();
        ui::set_visible(capped, false);
        this->show_summary(nullptr);
        ui::set_visible(progress.getView(), reading);
        return;
    }
    const PlayStats& s = *data;
    this->show_totals(s, who);

    // The games played in the chosen period, most played first.
    std::vector<const GameStat*> rows;
    for (uint32_t i = 0; i < s.count; i++)
        if (value_of(s.games[i], this->period) > 0) rows.push_back(&s.games[i]);
    activity_summary::sort_by_play(rows, this->period, false);

    this->show_list(rows, data, reading);
    this->apply_icons();
    this->load_icons();
    this->show_summary(data);
    this->show_status(s, rows.empty());
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

// The list on screen: the first ROWS_SHOWN rows unless "Show all" was
// chosen (then every row, or rows_max in applet mode). When the same games
// are already there in the same order (a read in the background, a
// refresh), only their figures change: no cell is rebuilt and the focus
// stays where it is.
void ActivityTab::show_list(const std::vector<const GameStat*>& rows, const std::shared_ptr<const PlayStats>& data,
                            bool reading)
{
    const PlayStats& s = *data;
    const int p = this->period;
    const size_t shown_n = std::min(rows.size(), s_cache.show_all ? s_cache.rows_max : ROWS_SHOWN);
    const bool more = !s_cache.show_all && shown_n < rows.size();
    const size_t left_out = s_cache.show_all ? rows.size() - shown_n : 0;
    // Under the list: rows not built (applet mode), and games the read itself
    // could not hold (PLAYSTATS_MAX), which the totals above miss too.
    std::string cap_text = left_out ? brls::getStr("playguard/activity/applet_cap", (int)left_out) : std::string();
    if (s.truncated) cap_text += (cap_text.empty() ? "" : "\n") + brls::getStr("playguard/activity/truncated", PLAYSTATS_MAX);
    capped->setText(cap_text);
    ui::set_visible(capped, !cap_text.empty());
    // The cells there already are the first games, in order: kept (and the
    // rest added below them, after "Show every game").
    bool prefix = this->cells.size() <= shown_n;
    for (size_t i = 0; prefix && i < this->cells.size(); i++) prefix = this->cells[i].first == rows[i]->app_id;
    const bool grow = prefix && !this->cells.empty() && this->cells.size() < shown_n && this->more_cell;
    const bool same = prefix && this->cells.size() == shown_n && more == (this->more_cell != nullptr);
    ui::set_visible(progress.getView(), reading && this->spinner);
    if (same || grow) {
        for (size_t i = 0; i < this->cells.size(); i++) {
            GameCell* cell = this->cells[i].second;
            cell->setText(game_name(*rows[i]));
            cell->setDetailText(ui::fmt_play_time(value_of(*rows[i], p)));
            this->on_click(cell, *rows[i], data);
        }
        if (this->more_cell) this->more_cell->setDetailText(std::to_string(rows.size()));
    }
    if (grow) this->grow_list(rows, shown_n, more, data);
    else if (!same) this->build_list(rows, shown_n, more, data);
}

// "Show every game": the rest below, then the focus on the first of
// them (next frame, once they are laid out, so the list scrolls to it).
void ActivityTab::grow_list(const std::vector<const GameStat*>& rows, size_t shown_n, bool more,
                            const std::shared_ptr<const PlayStats>& data)
{
    const int p = this->period;
    const size_t first_new = this->cells.size();
    for (size_t i = first_new; i < shown_n; i++) {
        auto* cell = new GameCell(false);
        cell->setText(game_name(*rows[i]));
        cell->setDetailText(ui::fmt_play_time(value_of(*rows[i], p)));
        this->on_click(cell, *rows[i], data);
        list->addView(cell, list->getChildren().size() - 1);   // above "Show every game"
        this->cells.emplace_back(rows[i]->app_id, cell);
    }
    brls::Application::giveFocus(this->cells[first_new].second);
    if (!more) {
        list->removeView(this->more_cell);   // deletes it
        this->more_cell = nullptr;
    }
    std::weak_ptr<bool> weak = this->alive;
    const u64 id = this->cells[first_new].first;
    brls::sync([this, weak, id]() {
        if (weak.expired()) return;
        for (const auto& c : this->cells)
            if (c.first == id) brls::Application::giveFocus(c.second);
    });
    brls::Logger::info("activity list: {} of {} games", shown_n, rows.size());
}

// Every cell built again.
void ActivityTab::build_list(const std::vector<const GameStat*>& rows, size_t shown_n, bool more,
                             const std::shared_ptr<const PlayStats>& data)
{
    const int p = this->period;
    // The cells are about to be deleted: never leave the focus on one, and
    // give it back to the same game afterwards when it is still listed.
    u64 focused = 0;
    bool focus_in_list = false;
    for (brls::View* v = brls::Application::getCurrentFocus(); v && !focus_in_list; v = v->getParent()) {
        for (const auto& c : this->cells)
            if (v == c.second) focused = c.first;
        focus_in_list = v == list.getView();
    }
    if (focus_in_list) brls::Application::giveFocus(sort);
    list->clearViews();
    this->cells.clear();
    this->more_cell = nullptr;
    brls::View* refocus = nullptr;
    for (size_t i = 0; i < shown_n; i++) {
        const GameStat* g = rows[i];
        auto* cell = new GameCell(s_cache.icons_wanted && this->cells.size() < ICON_ROWS);
        cell->setText(game_name(*g));
        cell->setDetailText(ui::fmt_play_time(value_of(*g, p)));
        this->on_click(cell, *g, data);
        list->addView(cell);
        this->cells.emplace_back(g->app_id, cell);
        if (focused && g->app_id == focused) refocus = cell;
    }
    if (more) {
        this->more_cell = new brls::DetailCell();
        this->more_cell->setText("playguard/activity/show_all"_i18n);
        this->more_cell->setDetailText(std::to_string(rows.size()));
        this->more_cell->registerClickAction([this](brls::View*) {
            // Next frame: rebuilding deletes this cell, whose action runs now.
            std::weak_ptr<bool> weak = this->alive;
            brls::sync([this, weak]() {
                if (weak.expired()) return;
                s_cache.show_all = true;
                this->rebuild();
            });
            return true;
        });
        list->addView(this->more_cell);
    }
    if (refocus) brls::Application::giveFocus(refocus);
    brls::Logger::info("activity list: {} of {} games", shown_n, rows.size());
}

// Under the list: why it is empty, or what could not be read.
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
        const std::string avg = ui::fmt_play_time(sum.per_day_s);
        per_day->setDetailText(p == AllTime ? brls::getStr("playguard/activity/summary/per_day_over", avg, sum.days) : avg);
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

    // To rediscover: the same games again (a read in the background) keep
    // their cells and the focus; others replace them.
    const std::vector<int> picks = data ? activity_summary::rediscover(*data, REDISCOVER_SHOWN) : std::vector<int>();
    std::vector<u64> ids;
    for (int i : picks) ids.push_back(data->games[i].app_id);
    if (ids != this->rediscover_ids) {
        bool focus_in = false;
        for (brls::View* v = brls::Application::getCurrentFocus(); v && !focus_in; v = v->getParent())
            focus_in = v == rediscover_list.getView();
        if (focus_in) brls::Application::giveFocus(sort);
        rediscover_list->clearViews();
        for (size_t n = 0; n < picks.size(); n++) rediscover_list->addView(new GameCell(false));
        this->rediscover_ids = ids;
    }
    const auto& cells_on_screen = rediscover_list->getChildren();
    for (size_t n = 0; n < picks.size() && n < cells_on_screen.size(); n++) {
        auto* cell = static_cast<GameCell*>(cells_on_screen[n]);
        const GameStat& g = data->games[picks[n]];
        cell->setText(game_name(g));
        cell->setDetailText(ui::fmt_play_time(g.total_s));
        this->on_click(cell, g, data);
    }
    ui::set_visible(rediscover.getView(), !picks.empty());
}

void ActivityTab::on_click(GameCell* cell, const GameStat& game, const std::shared_ptr<const PlayStats>& data)
{
    const GameStat copy = game;
    const bool every = chosen_account() == nullptr;
    cell->registerClickAction([copy, data, every](brls::View*) {
        // Its own screen: the seven days as bars, the figures, each account.
        std::vector<unsigned char> icon;
        if (const auto* bytes = s_cache.icons.get(copy.app_id)) icon = *bytes;
        brls::Application::pushActivity(new GameActivity(copy, data, std::move(icon), every));
        return true;
    });
}

void ActivityTab::apply_icons()
{
    for (size_t i = 0; i < this->cells.size() && i < ICON_ROWS; i++) {
        const u64 id = this->cells[i].first;
        // Decoded once for the run (a rebuild, another period, another
        // account reuse it), up to TEXTURES_KEPT icons.
        auto tex = s_cache.textures.find(id);
        const std::vector<unsigned char>* bytes = s_cache.icons.get(id);   // kept as recently used
        if (tex == s_cache.textures.end()) {
            if (!bytes || bytes->empty()) continue;
            if (s_cache.textures.size() < TEXTURES_KEPT) {
                const int t = icon_texture(*bytes);
                if (t) tex = s_cache.textures.emplace(id, t).first;
            }
        }
        if (tex != s_cache.textures.end()) this->cells[i].second->set_icon_texture(tex->second, false);
        else if (const int t = icon_texture(*bytes)) this->cells[i].second->set_icon_texture(t, true);
    }
}

void ActivityTab::load_icons()
{
    if (!s_cache.icons_wanted || s_cache.icons_busy) return;
    auto wanted = std::make_shared<std::vector<PlayIcon>>();
    for (size_t i = 0; i < this->cells.size() && i < ICON_ROWS; i++) {
        const u64 id = this->cells[i].first;
        if (s_cache.icons.contains(id) || s_cache.no_icon.count(id)) continue;
        wanted->push_back(PlayIcon{ id, nullptr, 0 });
    }
    if (wanted->empty()) return;
    s_cache.icons_busy = true;
    brls::async([wanted]() {
        playstats_icons(wanted->data(), wanted->size());
        if (playstats_cancelled()) {   // quitting: nobody to show them
            for (PlayIcon& icon : *wanted) std::free(icon.jpeg);
            return;
        }
        brls::sync([wanted]() {
            s_cache.icons_busy = false;
            for (PlayIcon& icon : *wanted) {
                if (icon.jpeg && icon.size) s_cache.icons.put(icon.app_id, std::vector<unsigned char>(icon.jpeg, icon.jpeg + icon.size));
                else s_cache.no_icon.insert(icon.app_id);
                std::free(icon.jpeg);
                icon.jpeg = nullptr;
            }
            // The list may have changed meanwhile (another period): put what
            // arrived on the cells now on screen, then ask for what they lack.
            if (s_cache.shown) {
                s_cache.shown->apply_icons();
                s_cache.shown->load_icons();
            }
        });
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
    std::vector<std::string> labels;
    for (int f = 0; f < 4; f++) labels.push_back(brls::getStr(fmt::format("playguard/activity/formats/{}", f)));
    const int p = this->period;
    const std::string who = account_label(chosen_account());
    ui::pick("playguard/activity/export_title"_i18n, labels, config::get().export_format, [data, p, who](int index) {
        config::get().export_format = index;
        ui::save_config();
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
        const table_export::Table t = activity_summary::export_table(s, p, text);
        // Encoded (a PDF, a zip for XLSX) and written off the UI thread.
        const auto format = (table_export::Format)index;
        const std::string failed = "playguard/activity/export_err"_i18n;
        ui::write_later("export", [t, format, failed]() {
            std::string err;
            const std::string path = table_export::save(t, format, paths::exports_dir(), "activity", &err);
            if (path.empty()) ui::error(failed + ": " + err);
            else ui::notify(brls::getStr("playguard/activity/exported", path));
        });
    });
}

brls::View* ActivityTab::create()
{
    return new ActivityTab();
}
