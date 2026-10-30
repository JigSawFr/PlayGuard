// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/history_flow.hpp"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <string_view>
#include <aurore.hpp>
#include <fmt/format.h>

#include <aurore/cadran/ui/cadran_modal.hpp>
#include <aurore/cadran/ui/cadran_text.hpp>
#include <aurore/cadran/util/text.hpp>
#include <aurore/cadran/view/footer_hints.hpp>

#include "action/data_notice.hpp"
#include "action/history_logic.hpp"
#include "action/notices.hpp"
#include "action/outside_watch.hpp"
#include "action/pt_flow.hpp"
#include "action/pt_log_flow.hpp"
#include "action/sync_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/date.hpp"

using namespace aurore::literals;

namespace history_flow
{

namespace
{
// One recorded value set, worded for its kind ("" when it does not fit).
std::string value_text(const std::string& kind, const std::vector<int>& v)
{
    if ((kind == "limits" || kind == "outside_limits") && v.size() == 7) {
        uint16_t days[7];
        for (int i = 0; i < 7; i++) days[i] = (uint16_t)v[i];
        return ui::days_summary(days);
    }
    if (v.size() == 1) {
        if (kind == "level") return ui::level_name((uint32_t)v[0]);
        if (kind == "org") return pctl_rating_org_name((uint32_t)v[0]);
        if (kind == "vr") return ui::yes_no(v[0] != 0);
        if (kind == "alarm") return v[0] ? "playguard/common/off"_i18n : "playguard/common/on"_i18n;
        if (kind == "time_up") return v[0] ? "playguard/play_timer/time_up_alarm"_i18n : "playguard/play_timer/time_up_suspend"_i18n;
        if (kind == "console_lock") return v[0] ? "playguard/common/on"_i18n : "playguard/common/off"_i18n;
        if (kind == "countdown") return v[0] ? "playguard/history/counting"_i18n : "playguard/history/paused"_i18n;
    }
    if (kind == "custom" && v.size() == 3)
        return aurore::getStr("playguard/history/custom_value", ui::age_text(v[0]), ui::yes_no(v[1] != 0), ui::yes_no(v[2] != 0));
    return "";
}

std::string kind_label(const std::string& kind)
{
    // A kind this version does not know (written by a newer one): said as
    // such, with its name for a bug report.
    if (!history_logic::known_kind(kind)) return aurore::getStr("playguard/history/unknown_kind", kind);
    return aurore::getStr("playguard/history/kinds/" + kind);
}

std::string source_label(const history::Entry& e)
{
    if (e.source == "profile") return aurore::getStr("playguard/history/sources/profile", e.detail);
    if (e.source == "remote_profile") return aurore::getStr("playguard/history/sources/remote_profile", e.detail);
    if (history_logic::known_source(e.source)) return aurore::getStr("playguard/history/sources/" + e.source);
    return "";
}

// The console's value of `kind` now, as recorded values ({} when unread).
std::vector<int> current(const std::string& kind)
{
    if (kind == "limits" || kind == "alarm" || kind == "time_up") {
        PtState pt;
        pctl_play_timer_query(&pt);
        if (kind == "limits") return pt.valid ? days_values(pt.day_min) : std::vector<int>{};
        if (kind == "time_up") return pt.valid ? std::vector<int>{ pt.alarm_only ? 1 : 0 } : std::vector<int>{};
        return pt.alarm_disabled_valid ? std::vector<int>{ pt.alarm_disabled ? 1 : 0 } : std::vector<int>{};
    }
    PctlStatus st;
    pctl_status_fetch(&st);
    if (kind == "level" && st.safety_level_ok) return { (int)st.safety_level };
    if (kind == "org" && st.rating_org_ok) return { (int)st.rating_org };
    if (kind == "vr" && st.stereo_vision_ok) return { st.stereo_vision_restricted ? 1 : 0 };
    if (kind == "custom" && st.settings_ok) return custom_values(st.settings);
    return {};
}

// Writes `v` for `kind` (not the limits, the alarm or the time-up choice:
// they go through the play-timer gate).
Result write_value(const std::string& kind, const std::vector<int>& v)
{
    if (kind == "level") return pctl_set_safety_level((uint32_t)v[0]);
    if (kind == "org") return pctl_set_rating_org((uint32_t)v[0]);
    if (kind == "vr") return pctl_set_stereo_vision_restricted(v[0] != 0);
    if (kind == "custom") {
        PctlCustomSettings s = { (uint8_t)v[0], v[1] != 0, v[2] != 0 };
        return pctl_set_custom_settings(&s);
    }
    return NXM_RC_INVALID_ARGUMENT;
}
}   // namespace

std::vector<int> days_values(const uint16_t days[7])
{
    return std::vector<int>(days, days + 7);
}

std::vector<int> custom_values(const PctlCustomSettings& s)
{
    return { s.rating_age, s.sns_post_restriction ? 1 : 0, s.free_communication_restriction ? 1 : 0 };
}

// count(): unknown (-1) until the file is first read. Moved on by the
// write itself, off the UI thread: load() waits for those still queued, so
// once it has read the file no write it read is counted again.
static std::atomic<long> s_count{ -1 };
// Moved on by every entry asked to be written (UI thread): a list read
// before it is out of date.
static uint64_t s_changes = 0;
// The newest entry asked to be written, and when (offer_undo's change).
static history::Entry                        s_last;
static std::chrono::steady_clock::time_point s_last_at;

static void store(history::Entry e)
{
    s_changes++;
    // A change PlayGuard made: what it reads next is not a change made
    // outside (the outside_* entries are those, and a new account; a refused
    // change changed nothing). An optional module installed or removed
    // changes nothing it watches.
    if (!text::starts_with(e.kind, "outside_") && !text::starts_with(e.kind, "gate_") && e.kind != "new_account" &&
        e.kind != "module" && e.result.empty())
        outside_watch::own_change();
    if (e.when.empty()) e.when = ui::now_stamp();
    e.version = app::version();
    e.dev     = !app::is_release();
    e.commit  = e.dev ? app::commit() : "";
    e.pr      = e.dev ? app::pr() : 0;
    s_last    = e;
    s_last_at = std::chrono::steady_clock::now();
    // Off the UI thread: append() reads and rewrites the whole file.
    ui::append_later("history", "history", [e]() {
        std::string err;
        bool put_aside = false;
        const bool saved = history::append(e, &err, &put_aside);
        if (!saved) aurore::Logger::warning("history: not saved ({})", err);
        // A damaged file put aside: the history starts again with this entry.
        if (put_aside) s_count = saved ? 1 : 0;
        else if (saved) {
            long n = s_count.load();
            while (n >= 0 && !s_count.compare_exchange_weak(n, std::min<long>(n + 1, (long)history::MAX_ENTRIES))) {
            }
        }
        aurore::sync([saved, err]() { notices::note_write("history", saved, err); });
        if (put_aside) aurore::sync([]() { data_notice::history_put_aside(); });
    });
    sync_flow::changed();   // the remote link publishes the new state
    // Developer › Record the play timer: a line marking the change.
    std::string event = e.result.empty() ? e.kind : e.kind + " refused " + e.result;
    for (const std::string* part : { &e.source, &e.detail })
        if (!part->empty()) event += " " + *part;
    if (!e.before.empty() || !e.after.empty()) {
        auto list = [](const std::vector<int>& v) {
            std::string out;
            for (int x : v) out += (out.empty() ? "" : " ") + std::to_string(x);
            return out;
        };
        event += " [" + list(e.before) + "] -> [" + list(e.after) + "]";
    }
    pt_log_flow::note(event);
}

void record_values(const char* kind, std::vector<int> before, std::vector<int> after,
                   const std::string& source, const std::string& detail, const std::string& when)
{
    if (before == after) return;
    history::Entry e;
    e.when   = when;
    e.kind   = kind;
    e.source = source;
    e.detail = detail;
    e.before = std::move(before);
    e.after  = std::move(after);
    store(e);
}

void record_event(const char* kind, const std::string& source, const std::string& detail, const std::string& when)
{
    history::Entry e;
    e.when   = when;
    e.kind   = kind;
    e.source = source;
    e.detail = detail;
    store(e);
}

void record_seen(const char* kind, const std::string& when, const std::string& detail)
{
    history::Entry e;
    e.kind   = kind;
    e.detail = detail;
    e.when   = when;
    store(e);
}

void record_failure(const char* kind, Result rc, std::vector<int> before, std::vector<int> after,
                    const std::string& source, const std::string& detail)
{
    history::Entry e;
    e.kind   = kind;
    e.source = source;
    e.detail = detail;
    e.before = std::move(before);
    e.after  = std::move(after);
    e.result = fmt::format("0x{:08X}", (unsigned)rc);
    aurore::Logger::warning("history: {} refused ({})", e.kind, e.result);
    store(e);
}

std::vector<history::Entry> load()
{
    ui::settle_writes("history");
    std::vector<history::Entry> all = history::load();
    s_count = (long)all.size();
    return all;
}

size_t count()
{
    if (s_count < 0) load();
    return (size_t)s_count;
}

void counted(size_t n)
{
    s_count = (long)n;
}

uint64_t changes()
{
    return s_changes;
}

// The result code of a refused change, as recorded ("0x0001088E").
static Result result_of(const history::Entry& e)
{
    std::string_view hex = e.result;
    if (hex.starts_with("0x") || hex.starts_with("0X")) hex.remove_prefix(2);
    unsigned long v = 0;
    return std::from_chars(hex.data(), hex.data() + hex.size(), v, 16).ec == std::errc{} ? (Result)v : 0;
}

static std::string plain_title(const history::Entry& e)
{
    // One day of the week changed: that day alone, not the week's range.
    const std::vector<int> days = history_logic::changed_days(e);
    if (days.size() == 1) {
        const int d = days[0];
        return aurore::getStr("playguard/history/title_day_change", kind_label(e.kind), ui::day_name_in_text(d),
                            ui::fmt_limit((uint16_t)e.before[d]), ui::fmt_limit((uint16_t)e.after[d]));
    }
    const std::string before = value_text(e.kind, e.before), after = value_text(e.kind, e.after);
    if (!before.empty() && !after.empty())
        return aurore::getStr("playguard/history/title_change", kind_label(e.kind), before, after);
    // A detail that is the change itself (a bedtime "21:00 → 22:00"), not a
    // profile's name.
    if (!e.detail.empty() && (e.source.empty() || e.source == "remote"))
        return aurore::getStr("playguard/common/line", kind_label(e.kind), e.detail);
    return kind_label(e.kind);
}

std::string title(const history::Entry& e)
{
    const std::string t = plain_title(e);
    return e.result.empty() ? t : aurore::getStr("playguard/history/failed_title", t);
}

std::string details(const history::Entry& e)
{
    // The change's facts as a table; a detail in words of its own under it.
    cadran_text::Rows rows = { { "playguard/history/when"_i18n, e.when.empty() ? "?" : e.when } };
    const std::string from = source_label(e);
    if (!from.empty()) rows.push_back({ "playguard/history/how"_i18n, from });
    const std::string before = value_text(e.kind, e.before), after = value_text(e.kind, e.after);
    if (!before.empty()) rows.push_back({ "playguard/history/before"_i18n, before });
    if (!after.empty()) rows.push_back({ "playguard/history/after"_i18n, after });
    if (!e.version.empty())
        rows.push_back({ "playguard/history/version"_i18n,
                         e.dev ? aurore::getStr("playguard/history/version_dev", e.version) : e.version });
    // A development build: which one, to find the change it was made with.
    if (!e.commit.empty()) rows.push_back({ "playguard/history/commit"_i18n, e.commit });
    if (e.pr > 0) rows.push_back({ "playguard/history/pr"_i18n, "#" + std::to_string(e.pr) });
    std::string out = kind_label(e.kind) + "\n\n" + cadran_text::table(rows);
    if (!e.detail.empty() && (from.empty() || e.source == "remote")) out += "\n\n" + e.detail;
    // The week's limits: each day that changed, as it was and became.
    const std::vector<int> days = history_logic::changed_days(e);
    if (!days.empty()) {
        out += "\n\n" + "playguard/history/days_changed"_i18n;
        for (int d : days)
            out += "\n" + aurore::getStr("playguard/history/day_change", ui::day_name(d), ui::fmt_limit((uint16_t)e.before[d]),
                                        ui::fmt_limit((uint16_t)e.after[d]));
    }
    if (!e.result.empty())
        out += "\n\n" + aurore::getStr("playguard/history/failed_note", ui::rc_text(result_of(e), false));
    if (!history_logic::known_kind(e.kind)) out += "\n\n" + "playguard/history/unknown_note"_i18n;
    return out;
}

std::string time_of(const history::Entry& e)
{
    return e.when.size() >= 16 ? e.when.substr(11, 5) : (e.when.empty() ? "?" : e.when);
}

std::string day_title(const std::string& day)
{
    if (day.empty()) return "?";
    const std::string today = ui::today_date();
    using history_logic::Day;
    switch (history_logic::day_of(day, today, date::add_days(today, -1))) {
    case Day::Today: return "playguard/history/today"_i18n;
    case Day::Yesterday: return "playguard/history/yesterday"_i18n;
    case Day::Other: break;
    }
    const int wd = history_logic::weekday_of(day);
    return wd < 0 ? day : ui::day_name(wd) + " · " + day;
}

std::string filter_label(history_logic::Filter f)
{
    return aurore::getStr(fmt::format("playguard/history/filters/{}", (int)f));
}

table_export::Table table(const std::vector<history::Entry>& entries)
{
    table_export::Table t;
    t.title    = "PlayGuard — " + "playguard/history/title"_i18n;
    t.subtitle = ui::now_stamp();
    t.sheet    = "playguard/history/title"_i18n;
    t.columns  = {
        { "when", "playguard/history/when"_i18n, false },
        { "change", "playguard/history/columns/change"_i18n, false },
        { "from", "playguard/history/how"_i18n, false },
        { "details", "playguard/history/columns/details"_i18n, false },
        { "before", "playguard/history/before"_i18n, false },
        { "after", "playguard/history/after"_i18n, false },
        { "result", "playguard/history/columns/result"_i18n, false },
        { "kind", "playguard/history/columns/kind"_i18n, false },
        { "version", "playguard/history/version"_i18n, false },
        { "commit", "playguard/history/commit"_i18n, false },
        { "pr", "playguard/history/pr"_i18n, false },
    };
    for (const history::Entry& e : entries) {
        std::string from = source_label(e);
        if (from.empty()) from = e.source;   // a source this version does not know
        // A profile's name is in its source already.
        const std::string detail = e.source == "profile" ? "" : e.detail;
        t.rows.push_back({ e.when, kind_label(e.kind), from, detail, value_text(e.kind, e.before), value_text(e.kind, e.after),
                           e.result.empty() ? "playguard/history/columns/done"_i18n
                                            : aurore::getStr("playguard/history/columns/refused", e.result),
                           e.kind, e.dev ? aurore::getStr("playguard/history/version_dev", e.version) : e.version, e.commit,
                           e.pr > 0 ? "#" + std::to_string(e.pr) : "" });
    }
    return t;
}

void open(const history::Entry& e, std::function<void()> refresh)
{
    std::string body = details(e);
    if (!history::undoable(e)) {
        ui::info(body);
        return;
    }
    const std::vector<int> now = current(e.kind);
    using history_logic::Undo;
    switch (history_logic::undo_action(e, now, app::read_only(), config::get().advanced)) {
    case Undo::AlreadyBack:
        ui::info(body + "\n\n" + "playguard/history/already_back"_i18n);
        return;
    // Nothing to offer: the details alone, with the reason.
    case Undo::ReadOnly:
        ui::info(body + "\n\n" + "playguard/common/read_only_note"_i18n);
        return;
    case Undo::NeedsAdvanced:
        ui::info(body + "\n\n" + "playguard/history/needs_advanced"_i18n);
        return;
    case Undo::Offer:
        break;
    }
    if (history_logic::changed_since(e, now))
        body += "\n\n" + aurore::getStr("playguard/history/changed_since", value_text(e.kind, now));
    body += "\n\n" + aurore::getStr("playguard/history/undo_body", value_text(e.kind, e.before));

    if (e.kind == "limits") {
        // Through the play-timer gate, with the week as it will be drawn.
        uint16_t days[7];
        for (int i = 0; i < 7; i++) days[i] = (uint16_t)e.before[i];
        const std::vector<int> before = e.before;
        pt_flow::confirm_write(body, "playguard/history/undo_confirm"_i18n, [before, refresh](bool did_unlock) {
            uint16_t back[7];
            for (int i = 0; i < 7; i++) back[i] = (uint16_t)before[i];
            const Result rc = pt_flow::write_days(back, "undo");
            pt_flow::finish_write(rc, did_unlock, "playguard/history/undone"_i18n, "playguard/history/undo_err"_i18n, refresh);
        }, days);
        return;
    }
    if (e.kind == "alarm") {
        // A play-timer write too: through the gate.
        const bool disabled = e.before[0] != 0;
        pt_flow::confirm_write(body, "playguard/history/undo_confirm"_i18n, [disabled, refresh](bool did_unlock) {
            const Result rc = pt_flow::write_alarm_disabled(disabled, "undo");
            pt_flow::finish_write(rc, did_unlock, "playguard/history/undone"_i18n, "playguard/history/undo_err"_i18n, refresh);
        });
        return;
    }
    if (e.kind == "time_up") {
        const bool alarm_only = e.before[0] != 0;
        pt_flow::confirm_write(body, "playguard/history/undo_confirm"_i18n, [alarm_only, refresh](bool did_unlock) {
            const Result rc = pt_flow::write_alarm_only(alarm_only, "undo");
            pt_flow::finish_write(rc, did_unlock, "playguard/history/undone"_i18n, "playguard/history/undo_err"_i18n, refresh);
        });
        return;
    }
    const history::Entry entry = e;
    ui::confirm(pt_flow::with_link_note(body), "playguard/history/undo_confirm"_i18n, [entry, now, refresh]() {
        const Result rc = write_value(entry.kind, entry.before);
        if (R_SUCCEEDED(rc)) record_values(entry.kind.c_str(), history_logic::undo_replaces(entry, now), entry.before, "undo");
        ui::notify_result(rc, "playguard/history/undone"_i18n, "playguard/history/undo_err"_i18n);
        if (refresh) refresh();
    });
}

namespace
{
// The ZR action offer_undo put on the main screen, until it times out.
aurore::View*            s_undo_view   = nullptr;
aurore::ActionIdentifier s_undo_action = 0;
size_t                 s_undo_timer  = 0;

void withdraw_undo()
{
    if (!s_undo_view) return;
    aurore::cancelDelay(s_undo_timer);
    s_undo_view->unregisterAction(s_undo_action);
    s_undo_view = nullptr;
    FooterHints::set_timed("", 0, 0);
    aurore::Application::getGlobalHintsUpdateEvent()->fire();
}
}   // namespace

void offer_undo(std::function<void()> refresh)
{
    constexpr long OFFER_MS = 8000;
    aurore::Activity* main = ui::main_screen();
    if (!main || aurore::Application::getActivitiesStack().back() != main) return;
    withdraw_undo();
    // The change just recorded (a write records it before it ends), when it
    // can be put back; not an undo itself, and nothing when the write
    // recorded nothing (the same values again).
    if (std::chrono::steady_clock::now() - s_last_at > std::chrono::seconds(5)) return;
    if (!history::undoable(s_last) || s_last.source == "undo") return;
    const history::Entry change = s_last;
    s_undo_view = main->getContentView();
    s_undo_action = s_undo_view->registerAction("playguard/history/undo_hint"_i18n, aurore::BUTTON_RT, [change, refresh](aurore::View*) {
        withdraw_undo();
        open(change, refresh);
        return true;
    });
    aurore::Application::getGlobalHintsUpdateEvent()->fire();
    // The footer shows how long the offer stands (a line draining under it).
    FooterHints::set_timed("playguard/history/undo_hint"_i18n, aurore::getCPUTimeUsec() + (aurore::Time)OFFER_MS * 1000,
                           (aurore::Time)OFFER_MS * 1000);
    s_undo_timer = aurore::delay(OFFER_MS, []() { withdraw_undo(); });
}

}   // namespace history_flow
