// SysNotesActivity — Console › Atmosphère, hekate or Firmware, or About ›
// Release notes (PlayGuard's): the
// release notes of the installed version, of each newer one and, in the
// tabs left, of the older ones, a tab each (util/sys_notes.hpp's tabs(): at
// most five newer ones, six tabs). Ⓛ / Ⓡ move from one tab to the next
// (their icons on either side of the tabs); the screen opens on the closest
// newer version, or on the installed one when it is the newest. For the firmware, what updating to the newest needs first
// (Atmosphère's and hekate's notes: update them, or wait). Each version:
// tags (the newest firmware it supports, security fixes, parental controls
// changed), its title (installed or new marked beside it) and date, its notes as Markdown
// (view/markdown_view.hpp; in English, as published; PlayGuard's as in
// "What's new", view/release_notes.hpp: the summary as cards, then the
// lists, the installed version's from the app when GitHub's are not there),
// See online (its page
// as a QR code), and when they were fetched. The notes kept on the SD card
// show at once; a bar at the header's right shows while they are fetched
// again (action/sys_notes_flow.hpp), Ⓧ fetches them now.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <vector>

#include <aurore/cadran/view/loading_bar.hpp>

#include "util/sys_notes.hpp"

class SysNotesActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/sys_notes.xml");

    static void show(sys_notes::Which which);
    // A fetch of `which` started: the bar, while its screen is open.
    static void changed(sys_notes::Which which);
    // It ended (`ok`: the notes kept were replaced): the screen again.
    static void fetched(sys_notes::Which which, bool ok);
    // The notes kept on the SD card were read (in the background, at
    // start-up): the screen again, the bar while a fetch still runs.
    static void loaded();

    explicit SysNotesActivity(sys_notes::Which which);
    ~SysNotesActivity() override;

    void onContentAvailable() override;

  private:
    // The tabs and the chosen version from the notes kept.
    void fill();
    void show_tab(size_t tab);
    void next_tab(int step);
    void show_page();
    void set_loading(bool on);
    aurore::View* focus_target();

    sys_notes::Which    which;
    std::vector<size_t> shown;        // tabs(): indexes into the list kept
    size_t              tab = 0;      // the chosen one, in `shown`
    bool                tab_chosen = false;
    std::string         page;         // the chosen one's page
    aurore::Box*          mark = nullptr;   // installed or new, in the header after the version

    AURORE_BIND(aurore::Box,    loading_box,  "sn_loading");
    AURORE_BIND(LoadingBar,   loading_bar,  "sn_loading_bar");
    AURORE_BIND(aurore::Label,  loading_text, "sn_loading_text");
    AURORE_BIND(aurore::Box,    tabs,         "sn_tabs");
    AURORE_BIND(aurore::Label,  tab_l,        "sn_tab_l");
    AURORE_BIND(aurore::Label,  tab_r,        "sn_tab_r");
    AURORE_BIND(aurore::Box,    tab_list,     "sn_tab_list");
    AURORE_BIND(aurore::Label,  advice,       "sn_advice");
    AURORE_BIND(aurore::Box,    tags,         "sn_tags");
    AURORE_BIND(aurore::Header, header,       "sn_header");
    AURORE_BIND(aurore::Label,  language,     "sn_language");
    AURORE_BIND(aurore::Box,    notes,        "sn_notes");
    AURORE_BIND(aurore::Label,  empty,        "sn_empty");
    AURORE_BIND(aurore::Label,  more,         "sn_more");
    AURORE_BIND(aurore::Button, online,       "sn_online");
    AURORE_BIND(aurore::Label,  checked,      "sn_checked");
};
