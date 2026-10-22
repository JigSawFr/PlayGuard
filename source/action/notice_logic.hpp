// notice_logic — what the notification bell holds (action/notices has the
// UI side): one entry per thing that needs the parent, set and cleared by
// the part of PlayGuard that sees it. Two kinds: "to do", counted on the
// bell, which stays until what it says is fixed (only its source clears it),
// and "for information", which the parent can put away; one put away comes
// back only when its source clears it and sets it again, or when its text
// changes. A few to do (ackable) the parent can also choose to ignore: no
// longer counted, listed apart, until put back. Each entry is about the parental controls or about the console
// PlayGuard runs on (the firmware, the games, the SD card, PlayGuard's own
// files): the bell counts the first, and the list shows them apart. Each
// tab of the sidebar counts, on a badge, the entries to do fixed there
// (tab_of). Within a group and a kind, entries come in a fixed
// order (rank_of), whatever the order their sources set them in. The rules
// action/notices applies (when an entry is due, which kind it is) are here
// too, over what they read passed in.
// Plain C++ so the host tests can run it (tests/notice_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "util/pctl_ops_c.hpp"

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
    Console,    // the console and PlayGuard's files: the Console tab's badge counts them
};

// Which group an entry is in, from its id: the console's ones are listed
// in notice_logic.cpp; any other id is about the parental controls.
Group group_of(const std::string& id);
// Its place in its group and kind: the order of that list, then the ids not
// in it (in the order they were first set).
int rank_of(const std::string& id);
// The main screen's tab where what it says is seen and fixed, as the tab's
// name in the catalogs ("security", "console"): the sidebar counts there
// the entries to do. "" for an id no tab is about (the bell only).
std::string tab_of(const std::string& id);

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
    // Ignores an Action entry the parent may ignore (ackable): not counted,
    // not in visible() but in ignored(), until unack() or its kind changes.
    // False for anything else, or one already ignored.
    bool ack(const std::string& id);
    // Counted again. False when it was not ignored.
    bool unack(const std::string& id);

    bool has(const std::string& id) const;   // set and shown
    bool acked(const std::string& id) const;   // set and ignored

    // What the bell lists: the parental controls' Action entries, then their
    // Info ones, then the console's (Action, then Info); each kind in rank
    // order, then in the order they were first set.
    std::vector<Item> visible() const;
    // The entries ignored, in the same order.
    std::vector<Item> ignored() const;
    int action_count() const;   // the parental controls' Action entries: the bell's number
    int info_count() const;     // the parental controls' Info entries shown
    int console_count() const;  // the console's Action entries
    int tab_count(const std::string& tab) const;   // the Action entries of tab_of() `tab`: its number in the sidebar
    int count_of(const std::vector<std::string>& ids) const;   // the Action entries among `ids`: a row's badge
    // The first Action entry among `ids` counted (in the bell's order): what
    // a row's badge is for, said by it. "" when none.
    std::string first_of(const std::vector<std::string>& ids) const;

  private:
    struct Slot
    {
        Item item;
        bool dismissed = false;
        bool acked     = false;
    };
    std::vector<Slot> slots;
    uint64_t next_order = 1;

    int               count(Group group, Level level) const;
    std::vector<Item> sorted(bool acked) const;
    Slot*       find(const std::string& id);
    const Slot* find(const std::string& id) const;
};

// Preferences › Notifications: whether an entry's kind is shown (the
// system's news, the play summaries, the reminders). Every other entry
// always is.
struct Shown
{
    bool system = true, summary = true, reminders = true;
};
bool shown_kind(const std::string& id, const Shown& shown);

// An entry for information marked as read: its id and a hash of its text
// ("week:1a2b3c4d"), remembered in config.json, so it stays read at the next
// start until its text changes.
std::string read_key(const std::string& id, const std::string& title, const std::string& detail);

// The entries to do the parent may ignore: a weak PIN, PlayGuard open
// without the PIN, the "time's up" alarm off, the SD card nearly full. What
// leaves the console open or broken (no PIN, unlocked, the clock, the play
// timer not counting, PlayGuard's own files) never is.
bool ackable(const std::string& id);
// An entry ignored, remembered in config.json: its id, what it was about
// (`print`: the PIN's length for pin_weak, "" otherwise; never the PIN) and
// the day ("pin_weak:4:2026-10-17").
std::string ack_key(const std::string& id, const std::string& print, const std::string& day);
// The day `keys` says `id` was ignored, while about the same `print`; ""
// when it was not.
std::string ack_day(const std::vector<std::string>& keys, const std::string& id, const std::string& print);
// Still ignored on `today`: for 30 days with `remind` (counted again after
// them), for good without.
bool ack_holds(const std::vector<std::string>& keys, const std::string& id, const std::string& print,
               const std::string& today, bool remind);
// The day it is counted again ("" without `remind`, or not ignored).
std::string ack_until(const std::vector<std::string>& keys, const std::string& id, const std::string& print,
                      bool remind);
// `keys` without `id`'s. True when one went.
bool ack_forget(std::vector<std::string>& keys, const std::string& id);

// "2026-10-08" from a backup's name ("20261008_142530.json"); "" otherwise.
std::string backup_date(const std::string& name);
// `day` ("YYYY-MM-DD") is `days` days before `today` or more (false when it
// is not a date).
bool days_ago(const std::string& day, int days, const std::string& today);

// The reminder to back up: no backup yet, or the newest (`backups` newest
// first, by name) 30 days old or more.
bool backup_due(const std::vector<std::string>& backups, const std::string& today);
// The console lock on for a week or more, since `since` ("" unknown).
bool console_lock_long(bool on, const std::string& since, const std::string& today);
// The SD card nearly full (under 500 MiB free), when its free space is known.
bool sd_space_low(bool known, uint64_t free_bytes);
// The week's summary, once every 7 days until read (`seen`: when it last
// was, "" never), and only for a week with some play time.
bool week_due(bool known, uint64_t total_s, const std::string& seen, const std::string& today);
// The days of the last 7 the time played passed that day's limit, while the
// console only warns at the limit (alarm only); 0 otherwise.
int over_limit_days(const PtState& pt, const PlayStats& stats);

// A reading kept for `period_ms` (the SD card's free space, which
// Today would otherwise ask for every 5 s): due again once that long has
// gone by since it was taken, or once forgotten (something was written).
// Times in milliseconds of a steady clock.
class Every
{
  public:
    explicit Every(int64_t period_ms) : period(period_ms) {}
    bool due(int64_t now_ms) const { return !taken_once || now_ms - at < 0 || now_ms - at >= period; }
    void taken(int64_t now_ms)
    {
        at = now_ms;
        taken_once = true;
    }
    void forget() { taken_once = false; }

  private:
    int64_t period;
    int64_t at = 0;
    bool    taken_once = false;
};

}   // namespace notice_logic
