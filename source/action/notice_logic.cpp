// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/notice_logic.hpp"

#include <algorithm>
#include <cstdio>

#include "util/date.hpp"
#include "util/text.hpp"

namespace notice_logic
{

namespace
{
// The fixed order of the entries, by group. The parental controls' first:
// what leaves them open or not counting, then what changed them, then
// reminders and news. Then the console's: what stops games or PlayGuard,
// then releases and notes. An id given with a trailing "_" is a prefix.
const char* const RANKS[] = {
    // Parental controls.
    "pin_missing", "unlocked", "alarm_off", "not_counting", "outside", "clock", "extra_restore",
    "pin_weak", "pin_lock_off", "new_account", "pairing", "over_limit", "week", "clock_off",
    "console_lock_long", "backup", "update", "installed", "crash", "upload",
    // The console (CONSOLE below).
    "hbmenu_route", "games", "patches", "sd_space", "sd_", "data_", "rescue_old", "rescue_restart", "went_back",
    "hbmenu_lifted", "hbmenu_removed", "forwarder_gone", "gate_refusals", "gate_paused", "gate_remake", "gate_climb", "compat", "sys_firmware", "sys_atmosphere", "sys_hekate", "serial", "other_system",
};
// The first of RANKS about the console: it and every one after it.
const int CONSOLE = 20;

bool matches(const std::string& id, const std::string& key)
{
    return key.back() == '_' ? text::starts_with(id, key) : id == key;
}

int index_of(const std::string& id)
{
    for (int i = 0; i < (int)(sizeof(RANKS) / sizeof(RANKS[0])); i++)
        if (matches(id, RANKS[i])) return i;
    return -1;
}
}   // namespace

Group group_of(const std::string& id)
{
    return index_of(id) >= CONSOLE ? Group::Console : Group::Parental;
}

int rank_of(const std::string& id)
{
    const int i = index_of(id);
    return i < 0 ? (int)(sizeof(RANKS) / sizeof(RANKS[0])) : i;
}

std::string tab_of(const std::string& id)
{
    // The network clock is a screen of the Console tab, and Today
    // says the changes seen outside and the limits to put back.
    if (group_of(id) == Group::Console || id == "clock" || id == "clock_off") return "console";
    if (id == "pin_missing" || id == "pin_weak" || id == "pin_lock_off" || id == "pairing" ||
        id == "console_lock_long")
        return "security";
    if (id == "alarm_off" || id == "not_counting" || id == "over_limit") return "play_timer";
    if (id == "outside" || id == "extra_restore") return "dashboard";
    if (id == "week" || id == "new_account") return "activity";
    if (id == "backup") return "preferences";
    if (id == "update" || id == "installed" || id == "crash" || id == "upload") return "about";
    return "";
}

Store::Slot* Store::find(const std::string& id)
{
    for (auto& s : this->slots)
        if (s.item.id == id) return &s;
    return nullptr;
}

const Store::Slot* Store::find(const std::string& id) const
{
    for (const auto& s : this->slots)
        if (s.item.id == id) return &s;
    return nullptr;
}

bool Store::set(const std::string& id, Level level, const std::string& title, const std::string& detail)
{
    if (Slot* s = this->find(id)) {
        Item& it = s->item;
        if (it.level == level && it.title == title && it.detail == detail) return false;
        // Ignored as something to do, now only for information (or the
        // other way round): a new question.
        if (it.level != level) s->acked = false;
        it.level  = level;
        it.title  = title;
        it.detail = detail;
        // Something new to say: shown again, even when put away before.
        s->dismissed = false;
        return true;
    }
    Slot s;
    s.item.id     = id;
    s.item.level  = level;
    s.item.title  = title;
    s.item.detail = detail;
    s.item.group  = group_of(id);
    s.item.rank   = rank_of(id);
    s.item.order  = this->next_order++;
    this->slots.push_back(s);
    return true;
}

bool Store::clear(const std::string& id)
{
    auto it = std::find_if(this->slots.begin(), this->slots.end(), [&id](const Slot& s) { return s.item.id == id; });
    if (it == this->slots.end()) return false;
    const bool shown = !it->dismissed && !it->acked;
    this->slots.erase(it);
    return shown;
}

bool Store::dismiss(const std::string& id)
{
    Slot* s = this->find(id);
    if (!s || s->dismissed || s->item.level != Level::Info) return false;
    s->dismissed = true;
    return true;
}

bool Store::ack(const std::string& id)
{
    Slot* s = this->find(id);
    if (!s || s->acked || s->item.level != Level::Action || !ackable(id)) return false;
    s->acked = true;
    return true;
}

bool Store::unack(const std::string& id)
{
    Slot* s = this->find(id);
    if (!s || !s->acked) return false;
    s->acked = false;
    return true;
}

bool Store::has(const std::string& id) const
{
    const Slot* s = this->find(id);
    return s && !s->dismissed && !s->acked;
}

bool Store::acked(const std::string& id) const
{
    const Slot* s = this->find(id);
    return s && s->acked;
}

std::vector<Item> Store::visible() const
{
    return this->sorted(false);
}

std::vector<Item> Store::ignored() const
{
    return this->sorted(true);
}

std::vector<Item> Store::sorted(bool acked) const
{
    std::vector<Item> out;
    for (const auto& s : this->slots)
        if (!s.dismissed && s.acked == acked) out.push_back(s.item);
    std::stable_sort(out.begin(), out.end(), [](const Item& a, const Item& b) {
        if (a.group != b.group) return a.group == Group::Parental;
        if (a.level != b.level) return a.level == Level::Action;
        if (a.rank != b.rank) return a.rank < b.rank;
        return a.order < b.order;
    });
    return out;
}

int Store::count(Group group, Level level) const
{
    int n = 0;
    for (const auto& s : this->slots)
        if (!s.dismissed && !s.acked && s.item.group == group && s.item.level == level) n++;
    return n;
}

int Store::action_count() const
{
    return this->count(Group::Parental, Level::Action);
}

int Store::info_count() const
{
    return this->count(Group::Parental, Level::Info);
}

int Store::console_count() const
{
    return this->count(Group::Console, Level::Action);
}

int Store::tab_count(const std::string& tab) const
{
    int n = 0;
    for (const auto& s : this->slots)
        if (!s.dismissed && !s.acked && s.item.level == Level::Action && tab_of(s.item.id) == tab) n++;
    return n;
}

int Store::count_of(const std::vector<std::string>& ids) const
{
    int n = 0;
    for (const auto& s : this->slots)
        if (!s.dismissed && !s.acked && s.item.level == Level::Action &&
            std::find(ids.begin(), ids.end(), s.item.id) != ids.end())
            n++;
    return n;
}

bool shown_kind(const std::string& id, const Shown& shown)
{
    if (text::starts_with(id, "sys_")) return shown.system;
    if (id == "week" || id == "over_limit") return shown.summary;
    if (id == "backup" || id == "other_system" || id == "console_lock_long") return shown.reminders;
    return true;
}

std::string read_key(const std::string& id, const std::string& title, const std::string& detail)
{
    uint32_t h = 2166136261u;   // FNV-1a
    for (const std::string* part : { &title, &detail })
        for (unsigned char c : *part) h = (h ^ c) * 16777619u;
    char hex[9];
    std::snprintf(hex, sizeof(hex), "%08x", (unsigned)h);
    return id + ":" + hex;
}

bool ackable(const std::string& id)
{
    return id == "pin_weak" || id == "pin_lock_off" || id == "alarm_off" || id == "sd_space";
}

std::string ack_key(const std::string& id, const std::string& print, const std::string& day)
{
    return id + ":" + print + ":" + day;
}

std::string ack_day(const std::vector<std::string>& keys, const std::string& id, const std::string& print)
{
    const std::string head = id + ":" + print + ":";
    for (const auto& k : keys)
        if (k.size() == head.size() + 10 && text::starts_with(k, head)) {
            const std::string day = k.substr(head.size());
            if (!date::add_days(day, 0).empty()) return day;
        }
    return "";
}

bool ack_holds(const std::vector<std::string>& keys, const std::string& id, const std::string& print,
               const std::string& today, bool remind)
{
    const std::string day = ack_day(keys, id, print);
    return !day.empty() && (!remind || !days_ago(day, 30, today));
}

std::string ack_until(const std::vector<std::string>& keys, const std::string& id, const std::string& print,
                      bool remind)
{
    const std::string day = ack_day(keys, id, print);
    return remind && !day.empty() ? date::add_days(day, 30) : "";
}

bool ack_forget(std::vector<std::string>& keys, const std::string& id)
{
    const std::string head = id + ":";
    const auto end = std::remove_if(keys.begin(), keys.end(),
                                    [&head](const std::string& k) { return text::starts_with(k, head); });
    const bool gone = end != keys.end();
    keys.erase(end, keys.end());
    return gone;
}

std::string backup_date(const std::string& name)
{
    if (name.size() < 8) return "";
    for (int i = 0; i < 8; i++)
        if (name[i] < '0' || name[i] > '9') return "";
    return name.substr(0, 4) + "-" + name.substr(4, 2) + "-" + name.substr(6, 2);
}

bool days_ago(const std::string& day, int days, const std::string& today)
{
    const std::string then = date::add_days(day, days);
    return !then.empty() && then <= today;
}

bool backup_due(const std::vector<std::string>& backups, const std::string& today)
{
    if (backups.empty()) return true;
    const std::string last = backup_date(backups.front());
    return !last.empty() && days_ago(last, 30, today);
}

bool console_lock_long(bool on, const std::string& since, const std::string& today)
{
    return on && !since.empty() && days_ago(since, 7, today);
}

bool sd_space_low(bool known, uint64_t free_bytes)
{
    return known && free_bytes < (500ULL << 20);
}

bool week_due(bool known, uint64_t total_s, const std::string& seen, const std::string& today)
{
    return known && total_s > 0 && (seen.empty() || days_ago(seen, 7, today));
}

int over_limit_days(const PtState& pt, const PlayStats& stats)
{
    int over = 0;
    if (!pt.valid || !pt.alarm_only) return 0;
    for (int k = 0; k < 7; k++) {
        const uint16_t limit = pt.day_min[stats.day_wday[k] % 7];
        if (limit == PT_DAY_NOLIMIT) continue;
        uint64_t played = 0;
        for (uint32_t g = 0; g < stats.count; g++) played += stats.games[g].day_s[k];
        if (played > (uint64_t)limit * 60) over++;
    }
    return over;
}

}   // namespace notice_logic
