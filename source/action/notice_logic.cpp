// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/notice_logic.hpp"

#include <algorithm>

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
    "games", "patches", "sd_space", "sd_", "data_", "rescue_old", "rescue_restart", "went_back",
    "compat", "sys_firmware", "sys_atmosphere", "sys_hekate", "serial", "other_system",
};
// The first of RANKS about the console: it and every one after it.
const int CONSOLE = 20;

bool matches(const std::string& id, const std::string& key)
{
    return key.back() == '_' ? id.compare(0, key.size(), key) == 0 : id == key;
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
    const bool shown = !it->dismissed;
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

bool Store::has(const std::string& id) const
{
    const Slot* s = this->find(id);
    return s && !s->dismissed;
}

std::vector<Item> Store::visible() const
{
    std::vector<Item> out;
    for (const auto& s : this->slots)
        if (!s.dismissed) out.push_back(s.item);
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
        if (!s.dismissed && s.item.group == group && s.item.level == level) n++;
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

}   // namespace notice_logic
