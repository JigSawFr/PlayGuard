// TabBase — common shape of every sidebar tab: inflate the XML, re-read the
// state each time the tab (re)appears, X refreshes on demand (on the tabs that
// read the console; the footer shows only what a tab can do), optional periodic
// refresh, and the read-only note (a Label with id "tab_ro_note", if present).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>
#include <vector>

#include "ui/guarded.hpp"

class TabBadge;

class TabBase : public brls::Box
{
  public:
    // `refreshes`: X reads the tab again. Off for a tab that shows only
    // PlayGuard's own settings and files, which change only from it.
    explicit TabBase(const std::string& xml_res, bool refreshes = true);
    ~TabBase() override;

    // Called when the tab is shown and when the main screen comes back on top
    // (after the per-day editor, a dropdown, a dialog …).
    void willAppear(bool resetState = false) override;

    // Re-reads the tab on screen, if any (after the read-only or developer
    // mode changed).
    static void refresh_shown();
    // The shown tab's screen ("tab/dashboard"), "" for none.
    static std::string shown_screen();
    // The main screen's tab last shown ("tab/console"): not a tab's view on a
    // screen of its own (the network clock, Help). A sub-screen's way back.
    static std::string main_screen();

    // Entered from the sidebar for the first time: the first row marked with
    // something to do (mark), else the first row.
    brls::View* getDefaultFocus() override;
    void        onChildFocusGained(brls::View* directChild, brls::View* focusedView) override;

  protected:
    virtual void refresh() = 0;

    // Goes with the tab: what finishes later (a flow's callback after the
    // network or a dialog, a background task's brls::sync) checks it, by
    // ui::guarded, as a tab replaced by another is freed meanwhile.
    ui::Alive alive = ui::make_alive();
    // refresh(), once a flow is over: nothing if the tab is gone by then.
    std::function<void()> refresher() { return ui::guarded(this->alive, [this]() { this->refresh(); }); }

    // This tab is the one on screen. A tab replaced by another is freed a
    // little later (borealis' free queue): until then, its timer and its
    // listeners must not refresh it, or what it sets for the whole screen
    // (the header's pill) would cover the new tab's.
    bool shown() const;

    // Calls refresh() every `period_ms` while the app is in the foreground,
    // this tab is the one shown and its screen is the top one (not under a
    // dialog or the per-day editor). Tabs are deleted when another one is
    // opened, which stops it.
    void enable_auto_refresh(int period_ms);

    // `row` (a DetailCell) is where the notification bell's entries `ids`
    // are fixed: while some are to do, their number on a badge by its title,
    // as the sidebar has by the tab's name, and on a row with no value of its
    // own, what it is for in a few words ("Easy to guess"). Marked in the
    // order of the tab.
    void mark(brls::Box* row, std::vector<std::string> ids);

  private:
    brls::RepeatingTimer auto_timer;
    std::string screen;   // "tab/overview": how the frame statistics name this tab

    struct Mark
    {
        brls::Box*               row;
        TabBadge*                badge;
        brls::Label*             reason;   // what the badge is for, on a row with no value of its own
        std::vector<std::string> ids;
    };
    std::vector<Mark> marks;
    int               notice_listener = 0;
    bool              focused_once    = false;
    bool              revealed        = false;   // risen into place once (ui/motion)

    void update_marks();

    void update_ro_note();
};
