// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/notice_logic.hpp"

#include <algorithm>

namespace notice_logic
{

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
        if (a.level != b.level) return a.level == Level::Action;
        return a.order < b.order;
    });
    return out;
}

int Store::action_count() const
{
    int n = 0;
    for (const auto& s : this->slots)
        if (!s.dismissed && s.item.level == Level::Action) n++;
    return n;
}

int Store::info_count() const
{
    int n = 0;
    for (const auto& s : this->slots)
        if (!s.dismissed && s.item.level == Level::Info) n++;
    return n;
}

}   // namespace notice_logic
