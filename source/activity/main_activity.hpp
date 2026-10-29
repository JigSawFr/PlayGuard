// MainActivity — root screen: a TabFrame (sidebar of tabs) described in
// resources/xml/activity/main.xml. The tabs live in source/tab/.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

class MainActivity : public aurore::Activity
{
  public:
    // activity/main.xml.
    aurore::View* createContentView() override;

    void onContentAvailable() override;
    void onResume() override;

    ~MainActivity() override;

    // "PlayGuard", and the header's pill again (temporarily unlocked and
    // read-only say so on every tab: view/title_state).
    void update_title();

    // Shows tab `position` (ui::tab) and focuses its content: the view
    // with id `row` when given and shown, else its first one.
    void open_tab(int position, const std::string& row = "");

    // Quits, but first asks to lock again when parental controls are still
    // temporarily unlocked (B twice on the sidebar, the child screen's B).
    static void quit_or_relock();

  private:
    // Midnight while the app is open: what start-up does for a new day
    // (extra time from the day before) is done then too.
    aurore::RepeatingTimer day_timer;
    std::string day;

    // The notification bell: Y opens its list, the footer says how many
    // entries are to do; the state behind it read again every minute.
    aurore::RepeatingTimer notice_timer;
    int notice_listener = 0;
    void update_notice_hint();
};
