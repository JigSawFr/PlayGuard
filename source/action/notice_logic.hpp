// notice_logic — what the notification bell holds (action/notices has the
// UI side): one entry per thing that needs the parent, set and cleared by
// the part of PlayGuard that sees it. Two kinds: "to do", counted on the
// bell, which stays until what it says is fixed (only its source clears it),
// and "for information", which the parent can put away; one put away comes
// back only when its source clears it and sets it again, or when its text
// changes. Each entry is about the parental controls or about the console
// PlayGuard runs on (the firmware, the games, the SD card, PlayGuard's own
// files): the bell counts the first, the Console tab the second, and the
// list shows them apart. Within a group and a kind, entries come in a fixed
// order (rank_of), whatever the order their sources set them in.
// Plain C++ so the host tests can run it (tests/notice_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace notice_logic
{

enum class Level
{
    Action,   // to do: counted on the bell, cleared only by its source
    Info,     // for information: the parent can put it away
};

enum class Group
{
    Parental,   // the parental controls and the play timer: the bell counts them
    Console,    // the console and PlayGuard's files: the Console tab counts them
};

// Which group an entry is in, from its id: the console's ones are listed
// in notice_logic.cpp; any other id is about the parental controls.
Group group_of(const std::string& id);
// Its place in its group and kind: the order of that list, then the ids not
// in it (in the order they were first set).
int rank_of(const std::string& id);

struct Item
{
    std::string id;
    Level       level = Level::Info;
    Group       group = Group::Parental;
    int         rank  = 0;
    std::string title;
    std::string detail;
    uint64_t    order = 0;   // when it was first set: entries keep their place
};

class Store
{
  public:
    // Adds the entry, or replaces the one with the same id. True when what
    // the bell shows changed.
    bool set(const std::string& id, Level level, const std::string& title, const std::string& detail);
    // Removes it, and forgets that it was put away. True when it was shown.
    bool clear(const std::string& id);
    // Puts away an Info entry. False for an Action entry or an unknown id.
    bool dismiss(const std::string& id);

    bool has(const std::string& id) const;   // set and shown

    // What the bell lists: the parental controls' Action entries, then their
    // Info ones, then the console's (Action, then Info); each kind in rank
    // order, then in the order they were first set.
    std::vector<Item> visible() const;
    int action_count() const;   // the parental controls' Action entries: the bell's number
    int info_count() const;     // the parental controls' Info entries shown
    int console_count() const;  // the console's Action entries: the Console tab's number

  private:
    struct Slot
    {
        Item item;
        bool dismissed = false;
    };
    std::vector<Slot> slots;
    uint64_t next_order = 1;

    int         count(Group group, Level level) const;
    Slot*       find(const std::string& id);
    const Slot* find(const std::string& id) const;
};

}   // namespace notice_logic
