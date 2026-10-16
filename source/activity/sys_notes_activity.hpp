// SysNotesActivity — Tools › Console › Atmosphère, hekate or Firmware: the
// release notes of the installed version and of each newer one, a tab each
// (util/sys_notes.hpp's tabs(): at most five newer ones). Ⓛ / Ⓡ move from
// one tab to the next (their icons on either side of the tabs); the screen
// opens on the closest newer version, or on the installed one when it is
// the newest. For the firmware, what updating to the newest needs first
// (Atmosphère's and hekate's notes: update them, or wait). Each version:
// tags (installed or new, the newest firmware it supports, security fixes,
// parental controls changed), its title and date, its notes as Markdown
// (view/markdown_view.hpp; in English, as published), See online (its page
// as a QR code), and when they were fetched. The notes kept on the SD card
// show at once; a bar shows while they are fetched again (action/
// sys_notes_flow.hpp), Ⓧ fetches them now.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <vector>

#include "util/sys_notes.hpp"
#include "view/loading_bar.hpp"

class SysNotesActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/sys_notes.xml");

    static void show(sys_notes::Which which);
    // A fetch of `which` started: the bar, while its screen is open.
    static void changed(sys_notes::Which which);
    // It ended (`ok`: the notes kept were replaced): the screen again.
    static void fetched(sys_notes::Which which, bool ok);

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
    brls::View* focus_target();

    sys_notes::Which    which;
    std::vector<size_t> shown;        // tabs(): indexes into the list kept
    size_t              tab = 0;      // the chosen one, in `shown`
    bool                tab_chosen = false;
    std::string         page;         // the chosen one's page

    BRLS_BIND(brls::Box,    loading_box,  "sn_loading");
    BRLS_BIND(LoadingBar,   loading_bar,  "sn_loading_bar");
    BRLS_BIND(brls::Label,  loading_text, "sn_loading_text");
    BRLS_BIND(brls::Box,    tabs,         "sn_tabs");
    BRLS_BIND(brls::Label,  tab_l,        "sn_tab_l");
    BRLS_BIND(brls::Label,  tab_r,        "sn_tab_r");
    BRLS_BIND(brls::Box,    tab_list,     "sn_tab_list");
    BRLS_BIND(brls::Label,  advice,       "sn_advice");
    BRLS_BIND(brls::Box,    tags,         "sn_tags");
    BRLS_BIND(brls::Header, header,       "sn_header");
    BRLS_BIND(brls::Label,  language,     "sn_language");
    BRLS_BIND(brls::Box,    notes,        "sn_notes");
    BRLS_BIND(brls::Label,  empty,        "sn_empty");
    BRLS_BIND(brls::Label,  more,         "sn_more");
    BRLS_BIND(brls::Button, online,       "sn_online");
    BRLS_BIND(brls::Label,  checked,      "sn_checked");
};
