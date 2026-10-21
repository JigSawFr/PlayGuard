// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/backup_flow.hpp"

#include <algorithm>
#include <borealis.hpp>
#include <fmt/format.h>
#include <memory>
#include <vector>

#include "action/backup_logic.hpp"
#include "action/forwarder_flow.hpp"
#include "action/hbmenu_lock.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "action/pt_flow.hpp"
#include "app.hpp"
#include "ui/cadran_text.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/pt_block.hpp"

using namespace brls::literals;

namespace backup_flow
{

namespace
{
// "2 h every day", or "Mon 2 h · Tue 2 h · …" (Monday first).
std::string days_text(const std::array<uint16_t, 7>& days)
{
    bool uniform = true;
    for (int d = 1; d < 7; d++) uniform &= days[d] == days[0];
    if (uniform)
        return days[0] == PT_DAY_NOLIMIT ? "playguard/common/no_limit"_i18n
                                         : brls::getStr("playguard/play_timer/state/every_day", ui::fmt_minutes(days[0]));
    std::string out;
    for (int i = 1; i <= 7; i++) {
        const int d = i % 7;
        if (!out.empty()) out += " · ";
        out += brls::getStr(fmt::format("playguard/days_short/{}", d)) + " " + ui::fmt_minutes(days[d]);
    }
    return out;
}

// What a restore will change: one "now → backup" line per value that differs
// from the console's (a value the console does not report counts as
// different), and how many are already the same, counted in *tally. The
// limits are not listed: the confirmation draws the week instead
// (pt_flow::confirm_write).
std::string summary(const backup::Snapshot& s, backup::RestoreTally* tally)
{
    PctlStatus st;
    pctl_status_fetch(&st);
    PtState pt;
    pctl_play_timer_query(&pt);
    const std::string unknown = "?";
    cadran_text::Rows rows;   // what differs: now → backed up
    auto compare = [&](const std::string& label, bool known, const std::string& now, const std::string& then,
                       bool restored = true) {
        tally->add(known, now == then, restored);
        if (known && now == then) return;
        rows.push_back({ label, (known ? now : unknown) + " → " + then });
    };

    if (s.level_ok)
        compare("playguard/restrictions/section_level"_i18n, st.safety_level_ok,
                ui::level_name(st.safety_level), ui::level_name(s.level));
    if (s.level_ok && s.level == PctlSafetyLevel_Custom && s.custom_ok) {
        compare("playguard/restrictions/age"_i18n, st.settings_ok, ui::age_text(st.settings.rating_age), ui::age_text(s.rating_age));
        compare("playguard/restrictions/sns"_i18n, st.settings_ok, ui::yes_no(st.settings.sns_post_restriction), ui::yes_no(s.sns_restricted));
        compare("playguard/restrictions/comm"_i18n, st.settings_ok, ui::yes_no(st.settings.free_communication_restriction),
                ui::yes_no(s.comm_restricted));
    }
    if (s.vr_ok) compare("playguard/restrictions/vr"_i18n, st.stereo_vision_ok, ui::yes_no(st.stereo_vision_restricted), ui::yes_no(s.vr_restricted));
    if (s.rating_org_ok)
        compare("playguard/restrictions/org"_i18n, st.rating_org_ok, pctl_rating_org_name(st.rating_org),
                pctl_rating_org_name(s.rating_org));
    if (s.days_ok) {
        std::array<uint16_t, 7> now{};
        for (int d = 0; d < 7; d++) now[d] = pt.day_min[d];
        compare("playguard/play_timer/section_limit"_i18n, pt.valid, days_text(now), days_text(s.days));
    }
    // A debug-class command: written only with the advanced actions shown.
    if (s.alarm_ok) {
        auto on_off = [](bool disabled) { return disabled ? "playguard/common/off"_i18n : "playguard/common/on"_i18n; };
        const bool advanced = config::get().advanced;
        if (pt.alarm_disabled_valid && pt.alarm_disabled == s.alarm_disabled) tally->add(true, true);
        else
            compare("playguard/play_timer/alarm"_i18n, pt.alarm_disabled_valid, on_off(pt.alarm_disabled),
                    on_off(s.alarm_disabled) + (advanced ? "" : "playguard/backup/not_restored_advanced"_i18n), advanced);
    }

    // Nothing to write: a statement, not the restore question.
    const std::string created = s.created.empty() ? "?" : s.created;
    std::string out = brls::getStr(tally->nothing_to_write() ? "playguard/backup/backed_up_on" : "playguard/backup/restore_body",
                                   created) + "\n";
    out += "\n" + (rows.empty() ? "playguard/backup/nothing_differs"_i18n : cadran_text::table(rows));
    if (tally->same > 0 && !rows.empty()) out += "\n\n" + (tally->same == 1 ? "playguard/backup/same_count_one"_i18n : brls::getStr("playguard/backup/same_count", tally->same));
    // The PIN is not in the backup (after "Delete all", there is none).
    if (st.pin_length_ok && st.pin_length == 0) out += "\n\n" + "playguard/backup/no_pin_note"_i18n;
    // The bedtime is kept in the backup's raw block, but not written back:
    // say so when the backup had one on.
    pt_block::Block raw;
    if (!s.raw_block.empty() && pt_block::from_hex(s.raw_block, &raw)) {
        PtBedtime bed[7];
        pt_bedtime_decode(raw.data(), bed);
        bool any = false;
        for (const PtBedtime& b : bed) any = any || b.on;
        if (any) out += "\n\n" + "playguard/backup/bedtime_not_restored"_i18n;
    }
    return out;
}

// Writes every value of the backup, in the order the service needs (the level
// before the custom settings), and goes on after a failure so that one value
// the console refuses does not leave the others unrestored.
void write_all(const backup::Snapshot& s, bool did_unlock, std::function<void()> refresh)
{
    PctlStatus was;   // for the history: what each value was before
    pctl_status_fetch(&was);
    backup_logic::Tally tally;
    auto check = [&](Result rc, const std::string& what) { tally.add(rc, what); };
    if (s.level_ok) {
        const Result rc = pctl_set_safety_level(s.level);
        check(rc, "playguard/restrictions/section_level"_i18n);
        if (R_SUCCEEDED(rc) && was.safety_level_ok)
            history_flow::record_values("level", { (int)was.safety_level }, { (int)s.level }, "backup");
        if (s.level == PctlSafetyLevel_Custom && s.custom_ok) {
            PctlCustomSettings cs = { s.rating_age, s.sns_restricted, s.comm_restricted };
            const Result crc = pctl_set_custom_settings(&cs);
            check(crc, "playguard/restrictions/section_custom"_i18n);
            if (R_SUCCEEDED(crc) && was.settings_ok)
                history_flow::record_values("custom", history_flow::custom_values(was.settings), history_flow::custom_values(cs), "backup");
        }
    }
    if (s.vr_ok) {
        const Result rc = pctl_set_stereo_vision_restricted(s.vr_restricted);
        check(rc, "playguard/restrictions/vr"_i18n);
        if (R_SUCCEEDED(rc) && was.stereo_vision_ok)
            history_flow::record_values("vr", { was.stereo_vision_restricted ? 1 : 0 }, { s.vr_restricted ? 1 : 0 }, "backup");
    }
    if (s.rating_org_ok) {
        const Result rc = pctl_set_rating_org(s.rating_org);
        check(rc, "playguard/restrictions/org"_i18n);
        if (R_SUCCEEDED(rc) && was.rating_org_ok)
            history_flow::record_values("org", { (int)was.rating_org }, { (int)s.rating_org }, "backup");
    }
    if (s.days_ok) check(pt_flow::write_days(s.days.data(), "backup"), "playguard/play_timer/section_limit"_i18n);
    if (s.alarm_ok && config::get().advanced)
        check(pt_flow::write_alarm_disabled(s.alarm_disabled, "backup"), "playguard/play_timer/alarm"_i18n);
    // Recorded only when something was restored: every write refused
    // changed nothing.
    if (tally.any_written()) history_flow::record_event("restore", "", s.created);

    pt_flow::finish_write(tally.first, did_unlock, "playguard/backup/restored"_i18n,
                          brls::getStr("playguard/backup/restore_err", tally.failed_list()), refresh);
}

// The backup's forwarders that are not on the HOME menu now: offered to be
// put back (remembered again, installed again), after the rest. The lock
// is the parent's to put back (its checks first): only said.
void offer_forwarders(const backup::Snapshot& s)
{
    if (!s.forwarders_ok || app::read_only()) return;
    const std::vector<forwarders::Made> missing = forwarder_flow::missing_from(s.forwarders);
    const bool relock = s.hbmenu_locked && !hbmenu_lock::locked();
    const auto tell_lock = [relock]() {
        if (relock)
            brls::sync([]() {
                ui::notice("lock", ui::Check::Info, "playguard/backup/forwarders_lock_title"_i18n,
                           "playguard/backup/forwarders_lock"_i18n);
            });
    };
    if (missing.empty()) return tell_lock();
    std::string names;
    for (const auto& m : missing) names += (names.empty() ? "" : ", ") + m.name;
    brls::sync([missing, names, tell_lock]() {
        ui::confirm(brls::getStr("playguard/backup/forwarders_body", (int)missing.size(), names),
                    "playguard/backup/forwarders_put_back"_i18n,
                    [missing, tell_lock]() { forwarder_flow::put_back_all(missing, [tell_lock](bool) { tell_lock(); }); },
                    [tell_lock]() { tell_lock(); });
    });
}

void restore(const backup::Snapshot& s, std::function<void()> refresh)
{
    backup::RestoreTally tally;
    const std::string body = summary(s, &tally);
    // The console already has these values: say so, and touch nothing (no
    // unlock, no write, no "restored" in the history); its forwarders may
    // still be missing.
    if (tally.nothing_to_write()) {
        if (s.forwarders_ok && !forwarder_flow::missing_from(s.forwarders).empty()) offer_forwarders(s);
        else ui::info(body);
        return;
    }
    const auto then = [s, refresh]() {
        if (refresh) refresh();
        offer_forwarders(s);
    };
    auto run = [s, then](bool did_unlock) { write_all(s, did_unlock, then); };
    // Limits to write: through the play-timer gate (unlock first when the
    // timer is counting down). Restrictions only: a plain confirmation, as in
    // the Restrictions tab.
    // It overwrites the current settings: the confirm button says so by its colour.
    // The alarm is a play-timer write as well.
    const bool timer = s.days_ok || (s.alarm_ok && config::get().advanced);
    if (timer) pt_flow::confirm_write(body, "playguard/backup/restore_confirm"_i18n, run, s.days_ok ? s.days.data() : nullptr, true);
    else if (ui::refuse_read_only()) return;
    else ui::confirm(pt_flow::with_link_note(body), "playguard/backup/restore_confirm"_i18n, [run]() { run(false); }, nullptr, true);
}

backup_logic::Listing s_listing;   // the backups on the SD card

std::string save_snapshot(std::string* error)
{
    s_listing.forget();   // list again next time: one more file, fewer once pruned, or a failed save
    notices::sd_written();
    const backup::Snapshot s = capture();
    if (s.empty()) {
        *error = "playguard/backup/unreadable_console"_i18n;
        return "";
    }
    const std::string path = backup::save(s, error);
    // History › Backups kept: the oldest beyond that number go (never the new one).
    if (!path.empty()) backup::prune((size_t)config::get().backup_keep, path);
    return path;
}
}   // namespace

const std::vector<std::string>& names()
{
    return s_listing.names([]() { return backup::list(); });
}

size_t count()
{
    return names().size();
}

backup::Snapshot capture()
{
    PctlStatus st;
    pctl_status_fetch(&st);
    PtState pt;
    pctl_play_timer_query(&pt);
    backup::Snapshot s = backup_logic::snapshot_of(st, pt);

    SysInfo si;
    sysinfo_get(&si);
    const std::string fw = ui::hos_text(si.hos_version);
    s.firmware = fw;

    s.created = ui::now_stamp();
    // What Console › Forwarders made, and the lock, when there is any.
    s.forwarders = forwarders::list();
    s.hbmenu_locked = hbmenu_lock::locked();
    s.forwarders_ok = !s.forwarders.empty() || s.hbmenu_locked;
    return s;
}

void save_now()
{
    std::string err;
    const std::string path = save_snapshot(&err);
    if (path.empty()) ui::error("playguard/backup/save_err"_i18n, err);
    else ui::notify(brls::getStr("playguard/backup/saved", path));
}

void choose_and_restore(std::function<void()> refresh)
{
    // Listed again (what the parent picks from must be on the SD card now),
    // and kept for Today and the count.
    const auto names = backup::list();
    s_listing.set(names);
    if (names.empty()) {
        ui::notify("playguard/backup/none"_i18n);
        return;
    }
    // Loaded once: the restore uses exactly what the list showed.
    auto loaded = std::make_shared<std::vector<std::pair<bool, backup::Snapshot>>>();
    std::vector<std::string> labels;
    for (const auto& name : names) {
        backup::Snapshot s;
        const bool ok = backup::load(name, s);
        loaded->emplace_back(ok, s);
        if (!ok) labels.push_back(brls::getStr("playguard/backup/unreadable", name));
        else labels.push_back(backup_logic::list_label(name, s));
    }
    ui::pick("playguard/backup/pick_title"_i18n, labels, 0, [loaded, refresh](int index) {
        const auto& entry = (*loaded)[(size_t)index];
        if (!entry.first) ui::notify("playguard/backup/unreadable_file"_i18n);
        else restore(entry.second, refresh);
    }, false);
}

void backup_then(std::function<void()> next)
{
    std::string err;
    const std::string path = save_snapshot(&err);
    if (!path.empty()) {
        ui::notify(brls::getStr("playguard/backup/saved", path));
        next();
        return;
    }
    brls::sync([err, next]() {
        ui::confirm_danger(brls::getStr("playguard/backup/before_delete_err", err),
                           "playguard/backup/delete_anyway"_i18n, next);
    });
}

}   // namespace backup_flow
